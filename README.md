# ESP32-S3 DSP Function Library
The ESP32-S3 is an enhanced member of the ESP32 family that incorporates hardware features optimized for Digital Signal 
Processing (DSP). This library builds on those capabilities to provide efficient, real-time audio signal-processing 
primitives, including FFT-based spectral analysis and IIR (biquad) signal filtering for embedded applications.

These library functions are used extensively in the **Watt-IZ** speech enabled project found here: **https://github.com/johnny49r/watt-iz**
<br>
<br>

## Fast Fourier Transform (FFT) Library for the ESP32-S3
A Fast Fourier Transform (FFT) converts a time-domain signal into its frequency-domain representation, 
allowing analysis of how signal energy is distributed across frequencies.

On the ESP32-S3, this can be done efficiently using the built-in DSP instructions exposed by Espressif’s 
ESP-DSP library, designed for real-time, streaming audio applications where predictable 
performance and memory usage are critical.

Compared to generic Arduino-based FFT implementations (such as arduinoFFT), this library leverages the ESP32-S3’s DSP 
acceleration and floating-point hardware to achieve significantly higher performance, particularly when processing larger
FFT sizes or continuous audio streams.

Hardware supported: ESP32-S3 MCU with PSRAM.

### FFT Library Usage:
1) Copy the two files in the /src directory to your project source folder and add **#include esp32s3_dsp.h** to your header file.

2) In your main code instanciate the FFT library class: **ESP32S3_FFT fft**;

3) To prepare for an FFT operation, call **fft.init(fft_size, fft_samples, spectral_output)**. * ***fft_size*** * is typically 512 but
can be any value in powers of 2; Example: 128, 256, 512, 1024... 
***fft_samples*** are the number of time domain samples to be converted to frequency domain.
Input data samples can be of arbitrary length but FFT accuracy improves if the number of samples is a even multiple of **fft_size**.
For info on **spectral_output** options, see below.
The **fft.init()** function returns a pointer to a fft_table_t structure containing parameters for allocating input & output buffers.

4) Call **fft.compute(float \*source_bufr, float \*output_bufr);** to convert source data (audio or ???). The
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

### A Simple Example of a Sinewave Represented After FFT
The example below shows a plot of a 1000 Hz sinewave signal. The sinewave signal is digitized and is input to the FFT processor.
The output plot demonstrates that all the energy of the input signal is at 1 KHz. Each data point of the FFT output is a
frequency bin whose value is determined by the formula AUDIO_SAMPLE_RATE / FFT_SIZE. In the image example, each FFT bin is
15.625 Hz. The first bin in the output represents DC, the second bin is 15.625 Hz, the third bin is 31.25 Hz, and so on.
Bin 64 should contain the peak of the energy at 1 KHz (64 * 15.625 = 1000).

<img width="600" height="421" alt="image" src="https://github.com/user-attachments/assets/300d5dd9-5163-4efa-be26-9d893a8ee201" />

<br>
<br>

## Audio Filter Library for the ESP32-S3
This library provides several audio filter implementations for the ESP32-S3 based on the ESP-DSP IIR (biquad) filter functions.
It is designed for real-time audio processing and encapsulates coefficient generation and filter state management for streaming use cases.
Special attention is given to the relationship between cutoff frequency and Q factor, making the filter behavior predictable and 
suitable for audio preprocessing tasks such as noise reduction and signal conditioning.
Using the library functions it becomes easy to add various signal filtering to your audio pipeline.

## Filter Type Descriptions
### Low-Pass Filter
A low-pass filter allows frequencies below the cutoff frequency to pass while attenuating higher frequencies. It is commonly used to remove high-frequency noise, smooth signals, or limit bandwidth before further processing.

### High-Pass Filter
A high-pass filter allows frequencies above the cutoff frequency to pass while attenuating lower frequencies. It is often used to remove DC offset, suppress low-frequency rumble, or isolate higher-frequency signal content.

### Band-Pass Filter
A band-pass filter allows frequencies within a defined frequency band to pass while attenuating frequencies below and above that range. The band is defined by a center frequency and bandwidth (or Q factor), making it useful for isolating specific signal regions such as voice or sensor resonances.

### Notch (Band-Stop) Filter
A notch filter strongly attenuates a narrow frequency band around a specified center (notch) frequency, while leaving most other frequencies unaffected. It is commonly used to suppress interference such as power-line hum or tonal noise.

### Peaking (Bell) Filter
A peaking (bell) filter selectively boosts or attenuates a narrow frequency band around a center frequency, with the bandwidth controlled by the Q factor. Unlike band-pass or notch filters, frequencies outside the affected band return to unity gain, making it ideal for equalization and spectral shaping.


## Filter Library Usage:
1) Copy the two files in the /src directory to your project source folder and add **#include esp32s3_dsp.h** to your header file.

2) In your main code instanciate the desired filter library class such as: **ESP32S3_LP_FILTER lp_filter**;

3) Initialize the filter behaviour by calling **lp_filter.init(cutoff_freq, sample_rate, Qfactor)** where **cutoff_freq** is the desired
-3db attenuation frequency in Hz, example: 3300. **sample_rate** is the base signal sampling frequency in Hz, example: 16000.
**Qfactor** controls the damping of the filter around its cutoff frequency with a range of 0.5 to 1.0.
The init() command should be called once before using the filter or anytime filter parameters are changed.
Note that Notch and Bell filters have an additional parameter **gain_db** which controls the gain of the attenuation (notch) or peak above 0db (bell).

4) Filtering is applied by calling **lp_filter.apply(input, output, length)** where **input** is a pointer to the users buffer containing
the raw float data. **output** is a pointer to a buffer that will contain the filtered data. **length** is the number of float values
to filter.


