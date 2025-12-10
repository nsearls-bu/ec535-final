cmake ..   -DCMAKE_BUILD_TYPE=Release   -DFLATBUFFERS_BUILD_TESTS=OFF   -DFLATBUFFERS_BUILD_FLATC=ON   -DCMAKE_INSTALL_PREFIX=/home/ned/Documents/ec535-final/flatbuffers-host

Build from tensorflow 2.13.0

cmake ../tensorflow/tensorflow/lite/c   -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake   -DCMAKE_BUILD_TYPE=Release   -DTFLITE_C_BUILD_SHARED_LIBS=ON   -DTFLITE_ENABLE_XNNPACK=OFF   -DTFLITE_ENABLE_RUY=ON   -DCMAKE_CXX_STANDARD=17