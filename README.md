# Multimedia Arithmetic Coding

This project implements arithmetic coding for PGM (P2) grayscale images.

## Build Instructions (Windows, MinGW)

```bash
mkdir build
cd build
cmake ..
make
````

## Usage

Run the batch script to encode and decode test images:

```bash
run.bat
```

Or run manually:

```bash
# Encode a PGM file
build\Release\arithmetic_coder.exe encode input\image.pgm output\image.codestream

# Decode a codestream
build\Release\arithmetic_coder.exe decode output\image.codestream output\image_rec.pgm
```

## Input/Output

* Input images: `input/*.pgm`
* Output codestreams and decoded images: `results/`
