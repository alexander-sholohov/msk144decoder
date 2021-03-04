### MSK144 Stream Decoder.

**Input:** Reads audio stream from stdin. Format: 12000 samples per second, 16 bits signed, mono.  
**Output:** Prints decoded lines to stdout, optionally writes log files and posts results to a specific HTTP service.

**How to compile:**

Prereqirements:

```shell
sudo apt-get install build-essential
sudo apt-get install cmake
sudo apt-get install gfortran
sudo apt-get install libfftw3-dev
sudo apt-get install libboost-dev
sudo apt-get install libcurl4-openssl-dev

```

The project utilizes original WSJT decoder. WSJTX repository linked as git submodule.  
After cloning this repository, execute the following commands:
```shell
cd msk144decoder
git submodule init
git submodule update --progress
```

Commands to build:
```shell
mkdir build
cd build
cmake ..
cmake --build . 
```

Executable file *msk144decoder* will appear at the current directory.

Wav-file reading example:
```shell
cat ../demo/000000_000001.wav | ./msk144decoder
# Actually, when using cat command the wav header is also read. Not a big problem for decoder.
```

Get brief help:
```shell
./msk144decoder --help
```

Getting stream from rtl_tcp example:
```shell
nc 192.168.1.200 2223 | ./csdr convert_u8_f | ./csdr shift_addition_cc 0.312451171875 | ./csdr fir_decimate_cc 64 0.005 HAMMING | ./csdr bandpass_fir_fft_cc 0.0 0.12 0.06 | ./csdr realpart_cf | ./csdr rational_resampler_ff 3 8 | ./csdr gain_ff 100.0 | ./csdr limit_ff | ./csdr convert_f_s16 | ./msk144decoder
```

Script *./scripts/generate_cmd_line.py* will help you build csdr commands chain. Just make necessary changes in it and run.
```shell
cd scripts
python generate_cmd_line.py
```

Links:  
- [WSJT-X Software by Joe K1JT](https://physics.princeton.edu/pulsar/k1jt/wsjtx.html)
- [WSJT Git Repository at sourceforge](https://sourceforge.net/p/wsjt/wsjtx/ci/master/tree/)

Acknowledgements to K1JT Joe Taylor and WSJT Development Group. The algorithms, source code, and protocol specifications for the mode MSK144 are Copyright © 2001-2021 by one or more of the following authors: Joseph Taylor, K1JT; Bill Somerville, G4WJS; Steven Franke, K9AN; Nico Palermo, IV3NWV; Greg Beam, KI7MT; Michael Black, W9MDB; Edson Pereira, PY2SDR; Philip Karn, KA9Q; and other members of the WSJT Development Group.

Alexander, RA9YER.  
ra9yer@yahoo.com
