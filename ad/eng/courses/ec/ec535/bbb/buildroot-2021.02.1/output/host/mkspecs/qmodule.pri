host_build {
    QT_CPU_FEATURES.x86_64 = mmx sse sse2
} else {
    QT_CPU_FEATURES.arm = 
}
QT.global_private.enabled_features = alloca_h alloca dlopen gui network posix_fallocate reduce_exports release_tools sql system-zlib testlib widgets xml
QT.global_private.disabled_features = sse2 alloca_malloc_h android-style-assets avx2 dbus dbus-linked private_tests gc_binaries intelcet libudev reduce_relocations relocatable stack-protector-strong zstd
PKG_CONFIG_EXECUTABLE = /ad/eng/courses/ec/ec535/bbb/buildroot-2021.02.1/output/host/bin/pkg-config
QMAKE_LIBS_LIBDL = 
QT_COORD_TYPE = double
QMAKE_LIBS_ZLIB = -lz
CONFIG -= precompile_header
CONFIG += cross_compile compile_examples enable_new_dtags largefile
QT_BUILD_PARTS += examples libs
QT_HOST_CFLAGS_DBUS += 
