
#include "esp32s3_fft.h"
#include "esp_heap_caps.h"

#define FFT_SIZE            1024
#define FFT_SAMPLES         1024
#define AUDIO_SAMPLE_RATE   16000
#define SINEWAVE_FREQ       1000

// Instanciate FFT library
ESP32S3_FFT fft;

void setup(void)
{
  // Enable serial port
  Serial.begin(115200);                  // init serial monitor 
  vTaskDelay(50);
  Serial.println("");

  // get info from fft to set up buffers
  fft_table_t *fft_table = fft.init(FFT_SIZE, FFT_SAMPLES, SPECTRAL_AVERAGE);

  // Allocate input & output buffers 
  float input_bufr[fft_table->size_input_bufr];
  float output_bufr[FFT_SAMPLES];

  // *** Buffers can alternatively be allocated in PSRAM to save SRAM resources.
  // psramInit();                // init PSRAM
  // float *input_bufr = (float *)heap_caps_malloc((fft_table->size_input_bufr * sizeof(float)), MALLOC_CAP_SPIRAM ); 
  // float *output_bufr = (float *)heap_caps_malloc(FFT_SAMPLES * sizeof(float), MALLOC_CAP_SPIRAM ); 

  // Create 1KHz sinewave in the input buffer
  float phase = 0.0f;
  const float phase_step = TWO_PI * SINEWAVE_FREQ / AUDIO_SAMPLE_RATE;   // sine freq defined here
  for (int j = 0; j < FFT_SAMPLES; j++) {
     input_bufr[j] = sinf(phase); 
     phase += phase_step;
  }
  // Perform FFT on the sinewave data
  fft.compute(input_bufr, output_bufr);

  /*
   * The FFT output_bufr should now consist of frequency bins with a spacing of sample_rate / FFT_size.
   * Example: 16000 / 1024 == 15.625 Hz per freq bin.
   * For real-valued signals, only the first half of the FFT output buffer (up to the Nyquist frequency)
   * contains unique frequency information (0 - 8 KHz). The 2nd half of the buffer is morrored data from the
   * first half.
   * 
   */

  // Simple plot function to dump values to the console
  for(int i=0; i<FFT_SIZE/2; i++) {    // Print freq bins from DC to Nyquist freq (8 KHz)
    if(i > 0 && i % 10 == 0)
      Serial.println("");
    int16_t sample = int(output_bufr[i]);
    Serial.printf("%d ", sample);
  }
  Serial.println("");
  // When finished with the FFT lib, call end() to deallocate memory.
  fft.end();
}

void loop(void) 
{
  // nothing to do here
}
