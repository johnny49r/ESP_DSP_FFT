# Fast FFT function library for the ESP32-S3 MCU.
The library provides functions to implement a very fast FFT specifically on the ESP32-S3 MCU with PSRAM.
These functions are similar to the popular **arduinoFFT** but are several times faster due
to the ESP32-S3 DSP support. 
PSRAM is required to implement several external and internal buffers.

## Usage:
1) Copy the two files to your project source folder and add #include **esp32s3_fft.h** to your header file.

2) In your main code instanciate the library: **ESP32S3_FFT fft**;

3) To prepare for an FFT operation, call **fft.init(fft_size, fft_samples)**. * *fft_size* * is typically 512
and the size must be in powers of 2. * *fft_samples* * are the number of time domain samples to be converted to frequency domain.  
The **fft.init()** function returns a value representing the number of samples plus any extra overlap. The value is always
equal to or larger than the original fft_samples. A sample input buffer (source_bufr) should be created with this refactored length and
filled with the samples to be converted. Any space beyond the samples should be padded with 0.0f.

4) Call __fft.compute(float *source_bufr, float *output_bufr);__ to convert source data (audio or ???). The
results of the FFT are written to 'output_bufr'.

5) Frequency bins (FFT frequency resolution) are calculated by dividing the sample rate by the fft_size. Example: 16000 (sample rate) / 
512 (fft_size) yields 31.25 Hz for each successive point in the output_bufr. If there is a peak value at
offset 32, this equates to energy at 1000 Hz. 

