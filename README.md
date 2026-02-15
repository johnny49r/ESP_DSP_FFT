# ESP32-S3 DSP Function Library
The ESP32-S3 is an enhanced member of the ESP32 family that incorporates hardware features optimized for Digital Signal 
Processing (DSP). This library builds on those capabilities to provide efficient, real-time audio signal-processing 
primitives, including FFT-based spectral analysis and low-pass IIR filtering for embedded applications.

These library functions are used extensively in the **Watt-IZ** project found here: https://github.com/johnny49r/watt-iz.

## Fast Fourier Transform (FFT) Library for the ESP32-S3
This library provides an efficient and ESP32-S3–optimized interface for performing Fast Fourier Transforms (FFT) on audio data. 
It is built on top of the ESP-DSP library and is designed for real-time, streaming audio applications where predictable 
performance and memory usage are critical.
Compared to generic Arduino-based FFT implementations (such as arduinoFFT), this library leverages the ESP32-S3’s DSP 
acceleration and floating-point hardware to achieve significantly higher performance, particularly when processing larger
FFT sizes or continuous audio streams.


Hardware support: ESP32-S3 MCU with PSRAM.

### Usage:
1) Copy the two files to your project source folder and add **#include esp32s3_fft.h** to your header file.

2) In your main code instanciate the library: **ESP32S3_FFT fft**;

3) To prepare for an FFT operation, call **fft.init(fft_size, fft_samples, spectral_output)**. * ***fft_size*** * is typically 512 but
can be any value in powers of 2; Example: 128, 256, 512, 1024... 
***fft_samples*** are the number of time domain samples to be converted to frequency domain.
Input data samples can be of arbitrary length but FFT accuracy improves if the number of samples is a even multiple of **fft_size**.
For info on **spectral_output** options, see below.
The **fft.init()** function returns a pointer to a fft_table_t structure containing parameters for allocating input & output buffers.

4) Call __fft.compute(float *source_bufr, float *output_bufr);__ to convert source data (audio or ???). The
results of the FFT are written to **output_bufr**. 

### FFT Table Structure
- num_original_samples: This is the same as the **fft_samples** parameter of the init() function.
- size_input_bufr: Allocate this number of SAMPLES for the data input buffer.
- num_sliding_frames: Allocate the output buffer in PSRAM using the formula: num_sliding_frames * fft_size * sizeof(float).
- hop_size: The size of the sliding window (normally 50% of the **fft_size**.

### Spectral Output Options
FFT data can be processed and returned in three different ways:

1) SPECTRAL AVERAGE - FFT data is returned to the output buffer whose legnth is equal to 'fft_size'.
The resulting data is the average of all transforms across the input data using a 50% sliding window.

2) SPECTRAL NO SLIDING - FFT data is returned in multiple blocks without sliding window overlap. 
   
3) SPECTRAL SLIDING - FFT data is returned using a sliding window with a 50% overlap. 

Google "FFT sliding window" for an explaination of the technique.

## Low-Pass IIR Audio Filter Library for the ESP32-S3
This library provides a low-pass audio filtering implementation for the ESP32-S3 based on the ESP-DSP IIR (biquad) filter functions.
It is designed for real-time audio processing and encapsulates coefficient generation and filter state management for streaming use cases.
Special attention is given to the relationship between cutoff frequency and Q factor, making the filter behavior predictable and 
suitable for audio preprocessing tasks such as noise reduction and signal conditioning.

### Usage:
1) Copy the two files to your project source folder and add **#include esp32s3_fft.h** to your header file.

2) In your main code instanciate the library: **ESP32S3_LP_FILTER lp_filter**;

3) Initialize the filter behaviour by calling "lp_filter.init(cutoff_freq, sample_rate, Qfactor)" where **cutoff_freq** is the desired
-3db attenuation frequency, example: 3300. **sample_rate** is the base signal sampling frequency in Hz, example: 16000. And
Qfactor is the 


