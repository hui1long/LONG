
if (CMAKE_VERSION VERSION_LESS 2.8.3)
    message(FATAL_ERROR "Qt 5 requires at least CMake version 2.8.3")
endif()

get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
# Use original install prefix when loaded through a
# cross-prefix symbolic link such as /lib -> /usr/lib.
get_filename_component(_realCurr "${_IMPORT_PREFIX}" REALPATH)
get_filename_component(_realOrig "/usr/lib/x86_64-linux-gnu/cmake/Qt5LinguistTools" REALPATH)
if(_realCurr STREQUAL _realOrig)
    get_filename_component(_qt5_linguisttools_install_prefix "/usr/lib/x86_64-linux-gnu/../../" ABSOLUTE)
else()
    get_filename_component(_qt5_linguisttools_install_prefix "${CMAKE_CURRENT_LIST_DIR}/../../../../" ABSOLUTE)
endif()
unset(_realOrig)
unset(_realCurr)
unset(_IMPORT_PREFIX)

if (NOT TARGET Qt5::lrelease)
    add_executable(Qt5::lrelease IMPORTED)

    set_target_properties(Qt5::lrelease PROPERTIES
        IMPORTED_LOCATION "${_qt5_linguisttools_install_prefix}/lib/x86_64-linux-gnu/qt5/bin/lrelease"
    )
endif()

if (NOT TARGET Qt5::lupdate)
    add_executable(Qt5::lupdate IMPORTED)

    set_target_properties(Qt5::lupdate PROPERTIES
        IMPORTED_LOCATION "${_qt5_linguisttools_install_prefix}/lib/x86_64-linux-gnu/qt5/bin/lupdate"
    )
endif()

set(Qt5_LRELEASE_EXECUTABLE Qt5::lrelease)
set(Qt5_LUPDATE_EXECUTABLE Qt5::lupdate)

include("${CMAKE_CURRENT_LIST_DIR}/Qt5LinguistToolsMacros.cmake")
