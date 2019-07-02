CONFIG +=  compile_examples system-sqlite qpa largefile nostrip precompile_header sse2 sse3 ssse3 sse4_1 sse4_2 avx avx2 pcre
QT_BUILD_PARTS += libs tools examples
QT_NO_DEFINES =  IMAGEFORMAT_JPEG TABLET XINPUT ZLIB
QT_QCONFIG_PATH = 
host_build {
    QT_CPU_FEATURES.x86_64 =  mmx sse sse2
} else {
    QT_CPU_FEATURES.x86_64 =  mmx sse sse2
}
QT_COORD_TYPE = double
QT_CFLAGS_PSQL   = -I/usr/include/postgresql
QT_CFLAGS_MYSQL   = -I/usr/include/mysql
QT_LFLAGS_MYSQL   = -lmysqlclient_r -lpthread -lz -lm -ldl
QT_CFLAGS_SQLITE   =  
QT_LFLAGS_SQLITE   = -lsqlite3  
QT_LFLAGS_ODBC   = -lodbc
QMAKE_CFLAGS = -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2
QMAKE_CXXFLAGS = -g -O2 -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2
QMAKE_LFLAGS = -Wl,-Bsymbolic-functions -Wl,-z,relro -Wl,--as-needed
styles += mac fusion windows
DEFINES += QT_NO_MTDEV
CONFIG += use_libmysqlclient_r
QT_CFLAGS_DBUS = -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include  
QT_LIBS_DBUS = -ldbus-1  
QT_CFLAGS_GLIB = -pthread -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include  
QT_LIBS_GLIB = -pthread -lgthread-2.0 -lglib-2.0  
QT_CFLAGS_QGTKSTYLE = -pthread -I/usr/include/gtk-2.0 -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/pango-1.0 -I/usr/include/gio-unix-2.0/ -I/usr/include/freetype2 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/libpng12 -I/usr/include/harfbuzz  
QT_LIBS_QGTKSTYLE = -lgobject-2.0 -lglib-2.0  
QT_CFLAGS_QGTK2 = -pthread -I/usr/include/gtk-2.0 -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/pango-1.0 -I/usr/include/gio-unix-2.0/ -I/usr/include/freetype2 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/libpng12 -I/usr/include/harfbuzz  
QT_LIBS_QGTK2 = -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgio-2.0 -lpangoft2-1.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lcairo -lpango-1.0 -lfontconfig -lgobject-2.0 -lglib-2.0 -lfreetype  
QT_CFLAGS_PULSEAUDIO = -D_REENTRANT -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include  
QT_LIBS_PULSEAUDIO = -lpulse-mainloop-glib -lpulse -lglib-2.0  
QMAKE_CFLAGS_FONTCONFIG = -I/usr/include/freetype2  
QMAKE_LIBS_FONTCONFIG = -lfontconfig -lfreetype  
QMAKE_INCDIR_LIBUDEV =  
QMAKE_LIBS_LIBUDEV = -ludev  
QMAKE_CFLAGS_XKBCOMMON =  
QMAKE_LIBS_XKBCOMMON = -lxkbcommon  
QMAKE_VERSION_XKBCOMMON = 0.4.1
QMAKE_INCDIR_EGL = /usr/include/libdrm  
QMAKE_LIBS_EGL = -lEGL  
QMAKE_CFLAGS_EGL =  
QMAKE_CFLAGS_XCB =  
QMAKE_LIBS_XCB = -lxcb-sync -lxcb-xfixes -lxcb-render -lxcb-shape -lxcb-randr -lxcb-image -lxcb-shm -lxcb-keysyms -lxcb-icccm -lxcb  
sql-drivers = 
sql-plugins =  mysql odbc psql sqlite tds
