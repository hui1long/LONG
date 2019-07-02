#configuration
CONFIG +=  shared qpa no_mocdepend release qt_no_framework
host_build {
    QT_ARCH = x86_64
    QT_TARGET_ARCH = x86_64
} else {
    QT_ARCH = x86_64
    QMAKE_DEFAULT_LIBDIRS = /usr/lib /usr/lib/gcc/x86_64-linux-gnu/4.8 /usr/lib/gcc/x86_64-linux-gnu/x86_64-linux-gnu /usr/lib/gcc/lib /lib/x86_64-linux-gnu /lib /usr/lib/x86_64-linux-gnu /usr/lib/gcc/x86_64-linux-gnu
    QMAKE_DEFAULT_INCDIRS = /usr/include/c++/4.8 /usr/include/x86_64-linux-gnu/c++/4.8 /usr/include/c++/4.8/backward /usr/lib/gcc/x86_64-linux-gnu/4.8/include /usr/local/include /usr/lib/gcc/x86_64-linux-gnu/4.8/include-fixed /usr/include/x86_64-linux-gnu /usr/include
}
QT_EDITION = OpenSource
QT_CONFIG +=  minimal-config small-config medium-config large-config full-config gtk2 gtkstyle fontconfig libudev evdev xlib xcb-render xcb-glx xcb-xlib xcb-sm xrender accessibility-atspi-bridge linuxfb kms c++11 accessibility egl eglfs openvg opengl shared qpa reduce_exports reduce_relocations clock-gettime clock-monotonic posix_fallocate mremap getaddrinfo ipv6ifname getifaddrs inotify eventfd system-jpeg system-png png system-freetype system-harfbuzz system-zlib nis cups iconv glib dbus dbus-linked openssl xcb xinput2 alsa pulseaudio icu concurrent audio-backend release

#versioning
QT_VERSION = 5.2.1
QT_MAJOR_VERSION = 5
QT_MINOR_VERSION = 2
QT_PATCH_VERSION = 1

#namespaces
QT_LIBINFIX = 
QT_NAMESPACE = 

QT_GCC_MAJOR_VERSION = 4
QT_GCC_MINOR_VERSION = 8
QT_GCC_PATCH_VERSION = 0
