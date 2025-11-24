
add_library(Qt5::QTsLibPlugin MODULE IMPORTED)


_populate_Gui_plugin_properties(QTsLibPlugin RELEASE "generic/libqtslibplugin.so" FALSE)

list(APPEND Qt5Gui_PLUGINS Qt5::QTsLibPlugin)
set_property(TARGET Qt5::Gui APPEND PROPERTY QT_ALL_PLUGINS_generic Qt5::QTsLibPlugin)
set_property(TARGET Qt5::QTsLibPlugin PROPERTY QT_PLUGIN_TYPE "generic")
set_property(TARGET Qt5::QTsLibPlugin PROPERTY QT_PLUGIN_EXTENDS "-")
set_property(TARGET Qt5::QTsLibPlugin PROPERTY QT_PLUGIN_CLASS_NAME "QTsLibPlugin")
