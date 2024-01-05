add_subdirectory(kcm)

set(LIGHTLYSHADERS_SRCS
    lightlyshaders.h
    lightlyshaders.cpp
)

kconfig_add_kcfg_files(LIGHTLYSHADERS_SRCS lightlyshaders_config.kcfgc)

add_library(${LIGHTLYSHADERS} MODULE ${LIGHTLYSHADERS_SRCS})

target_link_libraries(${LIGHTLYSHADERS}

    Qt6::Core
    Qt6::CorePrivate
    Qt6::Gui
    Qt6::DBus

    KWin::kwin
    epoxy::epoxy
    GL

    KF6::ConfigCore
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::ConfigWidgets
    KF6::GuiAddons
    KF6::WindowSystem

    KDecoration2::KDecoration
)


#install(TARGETS ${LIGHTLYSHADERS} DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwin/effects/plugins)
install(TARGETS ${LIGHTLYSHADERS} DESTINATION /usr/lib64/qt6/plugins/kwin/effects/plugins)

#install(FILES shaders_110/lightlyshaders.frag DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwin/shaders/1.10)
install(FILES shaders_110/lightlyshaders.frag DESTINATION /usr/share/kwin/shaders/1.10)
#install(FILES shaders_140/lightlyshaders.frag DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwin/shaders/1.40)
install(FILES shaders_140/lightlyshaders.frag DESTINATION /usr/share/kwin/shaders/1.40)