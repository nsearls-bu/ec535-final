Created by Ned Searls and Abdülkerim Korkmaz

# EC535 Embedded BirdNET

Optimized to run BirdNET-Analyzer on a BeagleBone Black. Depedencies are a BirdNet pretrained tflite model and the EC535 buildkit. All credit goes to the BirdNET-Analyzer team for creating the model.

# Build Instructions

## 1. Clone Sources
```bash
git clone https://github.com/tensorflow/tensorflow.git
cd tensorflow
git checkout v2.13.0
cd ..

git clone https://github.com/google/flatbuffers.git
git clone https://github.com/libsndfile/libsndfile.git
git clone https://github.com/libsndfile/ libsamplerate.git
```

## 2. Build Flatbuffers (this needs to be x86)
```bash
mkdir flatbuffers-host
cd flatbuffers-host
cmake ../flatbuffers
make -j4
cd ..
```

## 3. Build TensorFlow Lite C Library
```bash
mkdir tensorflow_build
cd tensorflow_build
cmake ../tensorflow/tensorflow/lite/c \
  -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -DTFLITE_C_BUILD_SHARED_LIBS=ON \
  -DTFLITE_ENABLE_XNNPACK=OFF \
  -DTFLITE_ENABLE_RUY=ON \
  -DCMAKE_CXX_STANDARD=17 \
make -j4
cd ..
```

## 4. Build libsndfile
```bash
cd libsndfile
cmake .
make -j4
cd ..
```

## 5. Build libsamplerate
```bash
cd libsamplerate
cmake .
make -j4
cd ..
```

## 6. Prepare lib_arm
```bash
mkdir lib_arm

cp tensorflow_build/libtensorflowlite_c.so lib_arm/
cp libsamplerate/src/.libs/libsamplerate.a lib_arm/
cp libsndfile/src/.libs/libsndfile.a lib_arm/
```

## 7. Build Embedded BirdNET
```bash
cd embedded_birdnet
make bird_identifier_x86
make bird_identifier_arm
cd ..
```

## 8. Install QT
### Linux:
```
sudo apt install qtbase5-dev qt5-qmake qtbase5-dev-tools
```

### BeagleBone Black:
Use the EC535 stock image.

# Running Inference

## 1. Convert audio to WAV and upsample to 48 kHz
```bash
ffmpeg -i input.file -ar 48000 audio.wav
```

## 2. Run Application
### x86:
```bash
./bird_identifier_x86 audio.wav
```

### ARM:
```bash
./bird_identifier_arm audio.wav
```
