/********************************************************************
 * @brief esp32s3_fft.h : fast FFT functions for the ESP32-S3 MCU. 
 * 
 * @note Dependencies: 
 * 1) The MCU should have PSRAM as deveral large buffers are required. The 
 * amount of memory required for these functions depends on the length 
 * of the transform and the total samples.
 * 2) This code is targeted specifically for the ESP32-S3 variant that has DSP 
 * support using the esp_dsp.h functions. 
 * 3) This code is designed for the arduino style development environment. 
 * Developed using VSCode / PlatformIO.
 * 
 * Performance: FFT computed in approx 2ms for 1024 samples.
 * 
 */
#pragma once

#include <Arduino.h>
#include "esp_dsp.h"
#include "esp_heap_caps.h"

// FFT constants
#define FFT_SAMPLING_FREQ  16000

// spectral output selection
enum {
    SPECTRAL_AVERAGE=0,                   // output one frame of averaged fft data for all input data (default)
    SPECTRAL_NO_SLIDING,                  // output fft frames for each block of input data
    SPECTRAL_SLIDING,                     // output 50% sliding fft - output is 50% larger than input
};

typedef struct {
   uint32_t num_original_samples;
   uint32_t size_input_bufr;              // number of samples for fft input buffer
   uint16_t num_sliding_frames;
   uint16_t hop_size;
} fft_table_t ;

class ESP32S3_FFT {
    public:
        ESP32S3_FFT(void);
        ~ESP32S3_FFT(void);  

        fft_table_t * init(uint32_t fft_size, uint32_t fft_samples, uint8_t spectral_select); // call on 1st use or when changing parameters
        void compute(float *source_data, float *output_data);  // call to perform FFT
        float calcFreqBin(float sample_rate_hz, float fft_size);  // return freq / output data point

    private:
        uint32_t _fft_size;               // fft block size - in powers of 2
        uint32_t _original_samples;       // caller data points
        uint32_t _total_samples;          // input buffer size rounded up to multiples of _fft_size
        uint16_t _hop_size;               // sliding fft scale - use 50%
        int32_t _num_sliding_frames;      // number of frames to calc over
        uint8_t _spectral_select;         // processing and output options - see above.
        float *hann_window;               // hann window to reduce spurious freq at start & end of input data
        float *fft_buffer;                // internal working buffer for real & imaginary FFT values
        float *fft_output;                // averaged FFT - same size as fft block
};