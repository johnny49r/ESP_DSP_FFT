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
 * Performance: FFT in approx 2ms for 512 samples.
 * 
 */
#pragma once

#include <Arduino.h>
#include "esp_dsp.h"
#include "esp_heap_caps.h"

// FFT constants
#define FFT_SAMPLING_FREQ  16000

typedef struct {
    uint32_t _fft_size;                   // samples in discrete fft (typical = 1024)
    uint32_t _original_samples;           // actual number of data points to sample
    uint32_t _total_samples;              // total samples including those that are padded
    uint16_t _audio_sample_freq;          // typically 16KHz
    float *_audio_source;                 // ptr to audio source data (float)
    bool use_hann_window;
} fft_params_t ;


class ESP32S3_FFT {
    public:
        ESP32S3_FFT(void);
        ~ESP32S3_FFT(void);  

        uint32_t init(uint32_t fft_size, uint32_t total_samples); // call on 1st use or when changing parameters
        void compute(float *source_data, float *output_data);  // call to perform FFT

    private:
        uint32_t _fft_size;               // fft block size - in powers of 2
        uint32_t _original_samples;       // caller data points
        uint32_t _total_samples;          // input buffer size rounded up to multiples of _fft_size
        uint16_t _hop_size;               // sliding fft scale - use 50%
        int32_t _num_sliding_frames;      // number of frames to calc over
        float *hann_window;               // hann window to reduce spurious freq at start & end of input data
        float *fft_buffer;                // internal working buffer for real & imaginary FFT values
        float *fft_output;                // averaged FFT - same size as fft block
};