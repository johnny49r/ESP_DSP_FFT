# Fast FFT function library for the ESP32-S3 MCU.
The library provides functions to implement a very fast FFT specifically on the ESP32-S3 MCU with PSRAM.
These functions are similar to the popular **arduinoFFT** but are several times faster due
to the ESP32-S3 DSP support. 

Hardware support: ESP32-S3 MCU with PSRAM.

## Usage:
1) Copy the two files to your project source folder and add **#include esp32s3_fft.h** to your header file.

2) In your main code instanciate the library: **ESP32S3_FFT fft**;

3) To prepare for an FFT operation, call **fft.init(fft_size, fft_samples, spectral_output)**. * ***fft_size*** * is typically 512
and the size must be in powers of 2. ***fft_samples*** are the number of time domain samples to be converted to frequency domain.
Input data samples can be of arbitrary length but FFT results improve if the number of samples is a multiple of the fft_size.
For info on **spectral_output** options, see below.
The **fft.init()** function returns a pointer to a fft_table_t structure containing parameters for allocating input & output buffers.

4) Call __fft.compute(float *source_bufr, float *output_bufr);__ to convert source data (audio or ???). The
results of the FFT are written to **output_bufr**.

## FFT Table Structure
- size_input_bufr: Allocate this number of SAMPLES for the data input buffer.
- num_sliding_frames: Allocate the output buffer in PSRAM using the formula: num_sliding_frames * fft_size * sizeof(float). 

## Spectral Output Options
FFT data can be processed and returned in three different ways:

1) SPECTRAL AVERAGE - FFT data is returned to the output buffer (always fft_size samples).
The resulting data is the rolling average of all transforms across the input data using 50% sliding window.

2) SPECTRAL NO SLIDING - FFT data is returned in multiple blocks without sliding window overlap. 
   
3) SPECTRAL SLIDING - FFT data is returned using a sliding window with a 50% overlap. 

Google "FFT sliding window" for an explaination of the technique.
