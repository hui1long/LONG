
add_library(Qt5::AccessibleQuickFactory MODULE IMPORTED)

_populate_Widgets_plugin_properties(AccessibleQuickFactory RELEASE "accessible/libqtaccessiblequick.so")

list(APPEND Qt5Widgets_PLUGINS Qt5::AccessibleQuickFactory)
