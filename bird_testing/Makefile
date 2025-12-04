EXT_TOOLCHAIN := /home/ned/Documents/ec535-final/ad/eng/courses/ec/ec535/bbb/buildroot-2021.02.1/output/host/opt/ext-toolchain
SYSROOT      := /home/ned/Documents/ec535-final/ad/eng/courses/ec/ec535/bbb/buildroot-2021.02.1/output/host/arm-buildroot-linux-gnueabihf/sysroot

CC_ARM       := $(EXT_TOOLCHAIN)/bin/arm-linux-g++
CC_X86       := g++

QT5_CFLAGS  := $(shell pkg-config --cflags Qt5Widgets)
QT5_LIBS    := $(shell pkg-config --libs Qt5Widgets)

CFLAGS_X86 = -I../tensorflow -pg
LDFLAGS_X86 = -Llib_x86 -ltensorflowlite_c -lsndfile -lsamplerate

CFLAGS_ARM = \
    --sysroot=$(SYSROOT) \
    -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=hard \
    -I../tensorflow \
    -I../libsamplerate/include \
    -I../libsndfile/build-libsndfile/install/include

LDFLAGS_ARM = \
    --sysroot=$(SYSROOT) \
    -Llib_arm \
    -Llib_arm/tf_deps \
    -Wl,-rpath,'$$ORIGIN/lib_arm' \
    -Wl,-rpath,'$$ORIGIN/lib_arm/tf_deps' \
    -Wl,-rpath-link,lib_arm \
    -Wl,-rpath-link,lib_arm/tf_deps \
    -ltensorflowlite_c \
    -ltensorflow-lite \
    -lsndfile \
    -lsamplerate \
    -lpthread \
    -ldl \
    -lm \
    -lstdc++

x86:
	$(CC_X86) $(CFLAGS_X86) bird_inference.cpp -o bird_inference_x86 $(LDFLAGS_X86)

spectrogram_x86: main.moc
	$(CC_X86) -std=c++17 -fPIC spectrogram.cpp -o spectrogram_x86 $(QT5_CFLAGS) $(QT5_LIBS)
main.moc: spectrogram.cpp
	moc spectrogram.cpp -o main.moc
arm:
	$(CC_ARM) $(CFLAGS_ARM) bird_inference.cpp -o bird_inference_arm $(LDFLAGS_ARM)

clean:
	rm -f bird_inference_x86 bird_inference_arm

print-cc:
	@echo $(CC_ARM)
