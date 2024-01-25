add_subdirectory(kcm)

set(LIGHTLYSHADERS kwin4_effect_lightlyshaders)

set(LIGHTLYSHADERS_SRCS
    lightlyshaders.h
    lightlyshaders.qrc
    lightlyshaders.cpp
)

kconfig_add_kcfg_files(LIGHTLYSHADERS_SRCS lightlyshaders_config.kcfgc)

add_library(${LIGHTLYSHADERS} MODULE ${LIGHTLYSHADERS_SRCS})

target_link_libraries(${LIGHTLYSHADERS}

    Qt5::Core
    Qt5::Gui
    Qt5::DBus
    ${KWIN_EFFECTS}
    ${KWIN_GLUTILS}

    epoxy
    GL

    KF5::ConfigCore
    KF5::ConfigGui
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::GuiAddons
    KF5::WindowSystem

    lshelper
)


#install(TARGETS ${LIGHTLYSHADERS} DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwin/effects/plugins)
#install(TARGETS ${LIGHTLYSHADERS} DESTINATION /usr/lib64/Qt5/plugins/kwin/effects/plugins)

install(TARGETS ${LIGHTLYSHADERS} DESTINATION ${MODULEPATH}/kwin/effects/plugins)