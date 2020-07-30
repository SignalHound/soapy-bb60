<p align="center">
<img src="https://signalhound.com/sigdownloads/Other/SH-SOAPY.jpg" width="75%" />
</p>

## A [SoapySDR](https://github.com/pothosware/SoapySDR/wiki) driver for the [Signal Hound BB60C 6 GHz Real-Time Spectrum Analyzer](https://signalhound.com/products/bb60c/)

### Requirements

- 64-bit Linux operating system
    - Tested on Ubuntu 18.04
- Native USB 3.0 support

### Prerequisites

1. [Install SoapySDR](https://github.com/pothosware/PothosCore/wiki/Ubuntu).
    - Note: Python bindings are not needed for this driver.
    - Check installation with `SoapySDRUtil --info`.
2. [Install the Signal Hound SDK](https://signalhound.com/software/signal-hound-software-development-kit-sdk/).
    - Follow directions in _device_apis/bb_series/linux/README.txt_.

### Installation

1. Clone this repository.
2. Run the following commands from the __SoapyBB60C/__ directory of the repository:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
$ sudo make install
$ sudo ldconfig
```
Now if a BB60C device is plugged in, `SoapySDRUtil --find` should display its serial number.

### Usage

- `#include <SoapySDR/Device.hpp>` and use the functions in [Device.hpp](https://github.com/pothosware/SoapySDR/blob/master/include/SoapySDR/Device.hpp) to interface with the BB60C.
- Compile and run the example:
```
$ g++ example.cpp -o example -lSoapySDR
$ ./example
```
- Use with [other platforms](https://github.com/pothosware/SoapySDR/wiki#platforms) that are compatible with SoapySDR such as [GNURadio](https://www.gnuradio.org/), [CubicSDR](https://cubicsdr.com/), and many others.
