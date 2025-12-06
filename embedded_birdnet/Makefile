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
    -pg \
    -march=armv7-a -mfpu=vfpv3-d16 -mfloat-abi=hard \
    -I../tensorflow \
    -I../libsamplerate/include \
    -I../libsndfile/build-libsndfile/install/include \
    -I$(SYSROOT)/usr/include/qt5 \
    -I$(SYSROOT)/usr/include/qt5/QtCore \
    -I$(SYSROOT)/usr/include/qt5/QtGui \
    -I$(SYSROOT)/usr/include/qt5/QtWidgets


LDFLAGS_ARM = \
    --sysroot=$(SYSROOT) \
    -Llib_arm \
    -Llib_arm/tf_deps \
    -Wl,-rpath,$$ORIGIN/lib_arm \
    -Wl,-rpath,$$ORIGIN/lib_arm/tf_deps \
    -Wl,-rpath-link,lib_arm \
    -Wl,-rpath-link,lib_arm/tf_deps \
    -lQt5Core \
    -lQt5Gui \
    -lQt5Widgets \
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

engine_arm.o: bird_identification_engine.cpp bird_identification_engine.h
	$(CC_ARM) $(CFLAGS_ARM) -c bird_identification_engine.cpp -o engine_arm.o

SpectrogramWidget.moc.cpp: SpectrogramWidget.h
	moc SpectrogramWidget.h -o SpectrogramWidget.moc.cpp

SpectrogramWidget_x86.o SpectrogramWidget.moc_x86.o: SpectrogramWidget.cpp SpectrogramWidget.h SpectrogramWidget.moc.cpp
	$(CC_X86) $(CFLAGS_X86) -std=c++17 -c -fPIC SpectrogramWidget.cpp -o SpectrogramWidget_x86.o $(QT5_CFLAGS)
	$(CC_X86) $(CFLAGS_X86) -std=c++17 -c -fPIC SpectrogramWidget.moc.cpp -o SpectrogramWidget.moc_x86.o $(QT5_CFLAGS)

SpectrogramWidget_arm.o SpectrogramWidget.moc_arm.o: SpectrogramWidget.cpp SpectrogramWidget.h SpectrogramWidget.moc.cpp
	$(CC_ARM) $(CFLAGS_ARM) -std=c++17 -c SpectrogramWidget.cpp -o SpectrogramWidget_arm.o
	$(CC_ARM) $(CFLAGS_ARM) -std=c++17 -c SpectrogramWidget.moc.cpp -o SpectrogramWidget.moc_arm.o

main_x86.o: main.cpp SpectrogramWidget.h
	$(CC_X86) $(CFLAGS_X86) -c -fPIC main.cpp $(QT5_CFLAGS) -o main_x86.o

main_arm.o: main.cpp SpectrogramWidget.h
	$(CC_ARM) $(CFLAGS_ARM) -c main.cpp -o main_arm.o

bird_identifier_x86: main_x86.o SpectrogramWidget_x86.o SpectrogramWidget.moc_x86.o engine_x86.o
	$(CC_X86) -pg main_x86.o SpectrogramWidget_x86.o SpectrogramWidget.moc_x86.o engine_x86.o \
	    -o bird_identifier_x86 $(QT5_LIBS) $(LDFLAGS_X86)

bird_identifier_arm: main_arm.o SpectrogramWidget_arm.o SpectrogramWidget.moc_arm.o engine_arm.o
	$(CC_ARM) -g -pg main_arm.o SpectrogramWidget_arm.o SpectrogramWidget.moc_arm.o engine_arm.o \
	    -o bird_identifier_arm $(LDFLAGS_ARM)

clean:
	rm -f bird_identifier_x86 bird_identifier_arm *.o *.moc.cpp