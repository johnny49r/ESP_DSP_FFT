/********************************************************************
 * @brief esp32s3.cpp source file
 * 
 * @note FFT functions using the ESP32-S3 DSP library. 
 * 
 * j. Hoeppner @ 2025
 */
#include "esp32s3_fft.h"


/********************************************************************
 * @brief ESP32S3_FFT class constructor
 */
ESP32S3_FFT::ESP32S3_FFT(void)
{
   hann_window = nullptr;
   fft_buffer = nullptr;
   fft_output = nullptr;
}


/********************************************************************
 * @brief ESP32S3_FFT class destructor
 */
ESP32S3_FFT::~ESP32S3_FFT(void)
{
   end();                                 // free internal buffer memory
}


/********************************************************************
 * @brief init() Initialize the FFT engine with specified params. Call
 *    this once before any number of calls to compute(). If end() is
 *    called, init() must be called again before any calls to compute().
 * 
 * @param (params) 
 *    fft_size = discrete fft points (typically 1024). @note : fft_size
 *       should be power of 2 - i.e. 128 / 256 / 512 / 1024 etc.
 *    total_samples = normaly the same as 'fft_size'. If larger, a sliding
 *       fft will be implemented with a 50% overlap. @note : total_samples should
 *       be an even multiple of 'fft_size'.
 * @return Pointer to a fft_table_t structure. If no memory available, returns NULL.
 *
 * @note *** Internal buffers are allocated in PSRAM. If your ESP32 variant does 
 * not contain PSRAM, internal buffers may be allocated in SRAM using the following
 * example:
 * hann_window = (float *) heap_caps_malloc(_fft_size * sizeof(float), MALLOC_CAP_INTERNAL | MALLOC_CAP_32BIT);
 */
fft_table_t * ESP32S3_FFT::init(uint32_t fft_size, uint32_t fft_samples, uint8_t spectral_select)
{
   uint16_t i;
   static fft_table_t fft_table;
   _spectral_select = spectral_select;
   _fft_size = fft_size;                  // must be a power of 2: 64, 128, 256, 512, etc.
   _original_samples = fft_samples;     // num of samples to be fft'd

   // Calculate 'hop' size
   if(_spectral_select == SPECTRAL_NO_SLIDING)
      _hop_size = _fft_size;          
   else
      _hop_size = _fft_size / 2;

   // Round total samples to be evenly divisible by fft_size.
   // This guarantees all samples are processed.
   uint16_t tmod = _original_samples % _fft_size;   
   if(tmod == 0)           // already evenly divisible by _fft_size
      _total_samples = _original_samples;
   else 
      _total_samples = _original_samples + (_fft_size - tmod); // 50% sliding window

   // Calculate number of sliding frames
   if(_spectral_select == SPECTRAL_NO_SLIDING)
      _num_sliding_frames = _total_samples / _hop_size;
   else 
      _num_sliding_frames = 1 + ((_original_samples - _fft_size) / _hop_size);       

   if(_num_sliding_frames < 0) _num_sliding_frames = 0;  // avoid crash

   // allocate memory in PSRAM for internal buffers.
   // free memory if previously allocated
   if(hann_window)
      free(hann_window);
   hann_window = (float *) heap_caps_malloc(_fft_size * sizeof(float), MALLOC_CAP_SPIRAM | MALLOC_CAP_32BIT); // discrete hann window
   
   // fft buffer - internal working buffer. Aligned to 32 byte blocks.
   if(fft_buffer)
      free(fft_buffer);
   fft_buffer = (float *) heap_caps_aligned_alloc(32, ((_fft_size * 2) * sizeof(float)) + 64, MALLOC_CAP_SPIRAM);
   
   // fft output buffer - intermediate buffer used for averaging. Aligned to 32 byte blocks.
   if(fft_output)
      free(fft_output);
   fft_output = (float *) heap_caps_aligned_alloc(32, _fft_size * sizeof(float), MALLOC_CAP_SPIRAM);

   if (!hann_window || !fft_buffer || !fft_output) {  // check if memory allocated OK
      return NULL;
   } 
   
   // init the ESP32-S3 DSP engine
   dsps_fft2r_init_fc32(NULL, _fft_size);

   // Create hann window 
   for (int i = 0; i < _fft_size; i++) {
      hann_window[i] = 0.5 * (1.0 - cos(2.0 * PI * i / (_fft_size - 1)));
   }     

   // Create output table
   fft_table.num_original_samples = _original_samples;
   fft_table.hop_size = _hop_size;
   fft_table.num_sliding_frames = _num_sliding_frames;
   fft_table.size_input_bufr = _total_samples;
   return &fft_table;
}


/********************************************************************
 * @brief Compute FFT from source_data and return result in output_data
 */
void ESP32S3_FFT::compute(float *source_data, float *output_data, bool use_hann_window)
{
   uint16_t frame, start, i, j;

   // zero the result buffer
   for(i=0; i<_fft_size; i++)
      fft_output[i] = 0.0;

   // --- Sliding FFT Loop ---         
   for (frame = 0; frame < _num_sliding_frames; frame++) {
      start = frame * _hop_size;

      // Multiply input * Hann window directly & save into fft_buffer's real parts (even nums)
      if(use_hann_window) {
         dsps_mul_f32(&source_data[start], hann_window, fft_buffer, _fft_size, 1, 1, 2); //_fft_size, 1, 1, 2);
      }

      // Clear imaginary parts (odd indices) for FFT calc
      for (i = 0; i < _fft_size; i++) {
         if(!use_hann_window)
            fft_buffer[2 * i] = source_data[start + i];  // keep real data
         fft_buffer[2 * i + 1] = 0.0f;    // zero out imaginary data
      }     

      // compute FFT
      dsps_fft2r_fc32(fft_buffer, _fft_size);
      dsps_bit_rev_fc32(fft_buffer, _fft_size);    

      // compute magnitudes
      for (j = 0; j < _fft_size; j++) {
         float real = fft_buffer[2 * j];
         float imag = fft_buffer[2 * j + 1];
         float mag = sqrtf(real * real + imag * imag);  // sqrt ((real sqr) + (imag sqr))
         if(_spectral_select == SPECTRAL_AVERAGE) {     // output one averaged _hop_size
            fft_output[j] += mag;
         }
         else if(_spectral_select == SPECTRAL_NO_SLIDING || _spectral_select == SPECTRAL_SLIDING) {   // no frame averaging, output all sequential frames
            output_data[(frame * _fft_size) + j] = mag;          
         }
      }
   }   

   // transfer averaged FFT to 'output_data'
   if(output_data && _spectral_select == SPECTRAL_AVERAGE) {
      for(i=0; i < _fft_size; i++) {
         output_data[i] = fft_output[i] / float(_num_sliding_frames);
      }
   }
}


/********************************************************************
 * @brief Free internal buffer memory. init() must be called before 
 *    any more calls to compute().
 *    If this library was instanciated using 'new', calling 'delete'
 *    will automatically call end().
 */
void ESP32S3_FFT::end(void) 
{
   // free memory in PSRAM used for internal buffers
   if(hann_window) {
      free(hann_window);
      hann_window = nullptr;
   }

   if(fft_buffer) {
      free(fft_buffer);
      fft_buffer = nullptr;
   }

   if(fft_output) {
      free(fft_output);
      fft_output = nullptr;
   }
}


/********************************************************************
 * @brief Calculate frequency resolution for compile data points. This
 *    is analogous to the chart frequency on the X axis.
 * @return float frequency bin value. Example:
 *    16000 / 1024 == 15.625 hz. Bin 10 would be 156.25 hz.
 */
float ESP32S3_FFT::calcFreqBin(float sample_rate_hz, float fft_size)  // return freq / output data point
{
   return (sample_rate_hz / fft_size);
}


/**
 * @brief Low Pass Filter Class - Wrapper for the ESP32-S3 DSP IIR filter features
 */
// empty constructor
ESP32S3_LP_FILTER::ESP32S3_LP_FILTER(void) { }

// destructor
ESP32S3_LP_FILTER::~ESP32S3_LP_FILTER(void) { }


/********************************************************************
 * @brief Initialize the low pass filter. Call this once on first time 
 * use or when the filter parameters have changed.
 * @param cutoff_freq - Freq where filter has attenuated the data down
 * to -3db. Note that using a higher Q factor will cause the -3db freq
 * to move to a higher frequency.
 * @param sample_rate - Audio sample rate - default 16KHz
 * @param Qfactor - 0.5 (default) <smoother cutoff rate>, 1.0 <sharper cutoff rate>
 */
void ESP32S3_LP_FILTER::init(float cutoff_freq, float sample_rate, float Qfactor)
{
   dsps_biquad_gen_lpf_f32((float *)&coeffs, cutoff_freq / sample_rate, Qfactor); // nominal cutoff rate
}


/********************************************************************
 * @brief Apply LP filter to the data.
 * @param input - ptr to the input float data.
 * @param output - ptr to the output float data.
 * @param len - number of float samples to process.
 */
void ESP32S3_LP_FILTER::apply(float *input, float *output, uint32_t len) 
{
   dsps_biquad_f32_aes3(input, output, len, coeffs, delay_line);
}
