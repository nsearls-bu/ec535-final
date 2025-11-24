set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv7)

set(CMAKE_SYSROOT "/home/ned/Documents/ec535-final/ad/eng/courses/ec/ec535/bbb/buildroot-2021.02.1/output/host/arm-buildroot-linux-gnueabihf/sysroot")

set(CMAKE_C_COMPILER "/home/ned/Documents/ec535-final/ad/eng/courses/ec/ec535/bbb/buildroot-2021.02.1/output/host/opt/ext-toolchain/bin/arm-linux-gcc")
set(CMAKE_CXX_COMPILER "/home/ned/Documents/ec535-final/ad/eng/courses/ec/ec535/bbb/buildroot-2021.02.1/output/host/opt/ext-toolchain/bin/arm-linux-g++")

set(CMAKE_C_FLAGS "--sysroot=${CMAKE_SYSROOT} -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=hard")
set(CMAKE_CXX_FLAGS "--sysroot=${CMAKE_SYSROOT} -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=hard")

set(CMAKE_FIND_ROOT_PATH "${CMAKE_SYSROOT}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


