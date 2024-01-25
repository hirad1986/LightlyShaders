set(lshelper_LIB_SRCS
    lshelper.h
    lshelper.cpp
)

kconfig_add_kcfg_files(lshelper_LIB_SRCS ../lightlyshaders/lightlyshaders_config.kcfgc)

add_library(lshelper ${lshelper_LIB_SRCS})

include(GenerateExportHeader)
generate_export_header(lshelper
    BASE_NAME liblshelper
    EXPORT_FILE_NAME liblshelper_export.h)

target_link_libraries(lshelper
    Qt5::Core
    Qt5::Gui
    Qt5::DBus

    kwineffects
    epoxy
    GL

    KF5::ConfigCore
    KF5::ConfigGui
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::GuiAddons
    KF5::WindowSystem
)

set_target_properties(lshelper PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

install(TARGETS lshelper ${INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY NAMELINK_SKIP)
