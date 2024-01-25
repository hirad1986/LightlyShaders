set(kwin_lightlyshaders_blur_config_SRCS blur_config.cpp)
ki18n_wrap_ui(kwin_lightlyshaders_blur_config_SRCS blur_config.ui)
kconfig_add_kcfg_files(kwin_lightlyshaders_blur_config_SRCS ../blurconfig.kcfgc)

qt5_add_dbus_interface(kwin_lightlyshaders_blur_config_SRCS "../../org.kde.kwin.Effects.xml" kwineffects_interface)

kwin_add_effect_config(kwin_lightlyshaders_blur_config ${kwin_lightlyshaders_blur_config_SRCS})

target_link_libraries(kwin_lightlyshaders_blur_config
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
)