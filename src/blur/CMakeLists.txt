add_subdirectory(kcm)

set(lightlyshaders_blur_SOURCES
    blur.cpp
    blur.qrc
    main.cpp
)

kconfig_add_kcfg_files(lightlyshaders_blur_SOURCES
    blurconfig.kcfgc
)

#kwin_add_builtin_effect(lightlyshaders_blur ${lightlyshaders_blur_SOURCES})
add_library(lightlyshaders_blur MODULE ${lightlyshaders_blur_SOURCES})
target_link_libraries(lightlyshaders_blur PRIVATE
    KWin::kwin

    KF6::ConfigGui

    KDecoration2::KDecoration

    lshelper
)

install(TARGETS lightlyshaders_blur DESTINATION /usr/lib64/qt6/plugins/kwin/effects/plugins)