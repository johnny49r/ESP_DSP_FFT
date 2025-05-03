Functions to implement very fast FFT specifically on the ESP32-S3 MCU with PSRAM.
This code leverages support for the DSP hardware to perform fast math ops and FFT.
1) Add the two files to your project source folder and add #include "esp32s3_fft.h" to your header file.
2) In your main code instanciate the library: ESP32S3_FFT fft;
3) Call 'fft.init(fft_size, fft_samples). fft_size is typically 512 and must be in powers of 2. The 
function returns a refactored input buffer size which is always equal to or larger than the
fft_samples. A new buffer should be created with the refactored length and padded with 0.0f after the
end of the samples: | N samples | padded with 0.0 |.
4) Call 'fft.compute(float *source_bufr, float *output_bufr);' to convert source data (audio or ???). The
results of the FFT are written to 'output_bufr'.


