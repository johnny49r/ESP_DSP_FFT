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
   if(hann_window) free(hann_window);
   if(fft_buffer) free(fft_buffer);   
   if(fft_output) free(fft_output); 
}


/********************************************************************
 * @brief init() Initialize the FFT engine with specified params
 * 
 * @param (params) 
 *    fft_size = discrete fft points (typically 1024). @note : fft_size
 *       should be power of 2 - i.e. 128 / 256 / 512 / 1024 etc.
 *    total_samples = normaly the same as 'fft_size'. If larger, a sliding
 *       fft will be implemented with a 50% overlap. @note : total_samples should
 *       be an even multiple of 'fft_size'.
 * @return recalculated sample size for input buffer. 
 */
uint32_t ESP32S3_FFT::init(uint32_t fft_size, uint32_t total_samples)
{
   uint16_t i;
   _fft_size = fft_size;            // must be a power of 2: 64, 128, 256, 512, etc.
   _original_samples = total_samples;  // num of samples to be fft'd
   _hop_size = _fft_size / 2;

   // Round total samples to be evenly divisible by fft_size.
   // This guarantees all samples are processed.
   uint16_t tmod = _original_samples % _fft_size;   
   if(tmod == 0)           // already evenly divisible by _fft_size
      _total_samples = _original_samples;
   else 
      _total_samples = _original_samples + (_fft_size - tmod);
   _num_sliding_frames = (_total_samples / _hop_size) - 1;
      Serial.printf("orig samples=%d, resized buf=%d, hop size=%d, num sliding frames=%d\n", _original_samples, _total_samples, _hop_size, _num_sliding_frames);
   if(_num_sliding_frames < 0) _num_sliding_frames = 0;

   // allocate memory in PSRAM for internal buffers
   if(hann_window)
      free(hann_window);
   hann_window = (float *) heap_caps_malloc(sizeof(float) * _fft_size, MALLOC_CAP_SPIRAM); // discrete hann window
   if(fft_buffer)
      free(fft_buffer);
   fft_buffer = (float *) heap_caps_aligned_alloc(16, (sizeof(float) * (_fft_size * 2)) + 16, MALLOC_CAP_SPIRAM);
   if(fft_output)
      free(fft_output);
   fft_output = (float *) heap_caps_aligned_alloc(16, sizeof(float) * _fft_size, MALLOC_CAP_SPIRAM);

   if (!hann_window || !fft_buffer || !fft_output) {  // check if memory allocated OK
      Serial.println("FFT memory alloc failed!");
      return 0;
   } 
   
   // init the ESP32-S3 DSP engine
   dsps_fft2r_init_fc32(NULL, _fft_size);

   // Create hann window 
   for (int i = 0; i < _fft_size; i++) {
      hann_window[i] = 0.5 * (1.0 - cos(2.0 * PI * i / (_fft_size - 1)));
   }     
   return _total_samples;
}


/********************************************************************
 * @brief Compute FFT from source_data and return result in output_data
 */
void ESP32S3_FFT::compute(float *source_data, float *output_data)
{
   uint16_t frame, start, i, j;

   // zero the result buffer
   memset(fft_output, 0x0, sizeof(float) * _fft_size); 

   // --- Sliding FFT Loop ---         
   for (frame = 0; frame < _num_sliding_frames; frame++) {
      start = frame * _hop_size;

      // Multiply input by Hann window directly & save into fft_buffer's real parts
      dsps_mul_f32(&source_data[start], hann_window, fft_buffer, _fft_size, 1, 1, 2);

      // Clear imaginary parts (odd indices) for FFT calc
      for (int i = 0; i < _fft_size; i++) {
         fft_buffer[2 * i + 1] = 0.0f;
      }   

      dsps_fft2r_fc32(fft_buffer, _fft_size);
      dsps_bit_rev_fc32(fft_buffer, _fft_size);    

      // compute magnitudes
      for (j = 0; j < _fft_size / 2; j++) {
         float real = fft_buffer[2 * j];
         float imag = fft_buffer[2 * j + 1];
         float mag = sqrtf(real * real + imag * imag);
         fft_output[j] += mag;
      }
   }   

   // transfer averaged FFT to 'output_data'
   if(output_data) {
      for(i=0; i < _fft_size / 2; i++) {
         output_data[i] = fft_output[i] / float(_num_sliding_frames);
      }
   } else {
      fft_output[j] /= float(_num_sliding_frames);
   }
}