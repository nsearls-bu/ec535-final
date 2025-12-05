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

engine_x86.o: bird_identification_engine.cpp bird_identification_engine.h
	$(CC_X86) -c $(CFLAGS_X86) bird_identification_engine.cpp -o engine_x86.o

SpectrogramWidget.moc.cpp: SpectrogramWidget.h
	moc SpectrogramWidget.h -o SpectrogramWidget.moc.cpp

SpectrogramWidget_x86.o SpectrogramWidget.moc.o: SpectrogramWidget.cpp SpectrogramWidget.h SpectrogramWidget.moc.cpp
	$(CC_X86) $(CFLAGS_X86) -std=c++17 -c -fPIC SpectrogramWidget.cpp -o SpectrogramWidget_x86.o $(QT5_CFLAGS)
	$(CC_X86) $(CFLAGS_X86) -std=c++17 -c -fPIC SpectrogramWidget.moc.cpp -o SpectrogramWidget.moc.o $(QT5_CFLAGS)

main_x86.o: main.cpp SpectrogramWidget.h
	$(CC_X86) $(CFLAGS_X86) -c -fPIC main.cpp $(QT5_CFLAGS) -o main_x86.o

bird_identifier_x86: main_x86.o SpectrogramWidget_x86.o SpectrogramWidget.moc.o engine_x86.o
	$(CC_X86) -pg main_x86.o SpectrogramWidget_x86.o SpectrogramWidget.moc.o engine_x86.o \
	    -o bird_identifier $(QT5_LIBS) $(LDFLAGS_X86)

# arm: bird_identification_engine
# 	$(CC_ARM) $(CFLAGS_ARM) bird_identification_engine.cpp -o bird_identification_engine.o $(LDFLAGS_ARM)


clean:
	rm -f bird_identifier_x86 *.o *.moc.cpp
 