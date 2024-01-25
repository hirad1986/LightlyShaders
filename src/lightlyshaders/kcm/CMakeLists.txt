set(kwin4_lightlyshaders_config_SOURCES lightlyshaders_kcm.cpp)

kconfig_add_kcfg_files(kwin4_lightlyshaders_config_SOURCES ../lightlyshaders_config.kcfgc)

ki18n_wrap_ui(kwin4_lightlyshaders_config_SOURCES lightlyshaders_config.ui)

qt5_add_dbus_interface(kwin4_lightlyshaders_config_SOURCES "../../org.kde.kwin.Effects.xml" kwineffects_interface)

kwin_add_effect_config(kwin4_lightlyshaders_config ${kwin4_lightlyshaders_config_SOURCES})

target_link_libraries(kwin4_lightlyshaders_config
    kwineffects
    Qt5::Widgets
    Qt5::Core
    Qt5::DBus
    Qt5::Gui
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::ConfigWidgets
    KF5::ConfigGui
    KF5::GlobalAccel
    KF5::WindowSystem
    KF5::I18n         

    lshelper
)

