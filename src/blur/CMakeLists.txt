add_subdirectory(kcm)

set(lightlyshaders_blur_SOURCES
    blur.cpp
    blur.qrc
    blurshader.cpp
    main.cpp
)

kconfig_add_kcfg_files(lightlyshaders_blur_SOURCES
    blurconfig.kcfgc
)

add_library(lightlyshaders_blur MODULE ${lightlyshaders_blur_SOURCES})
target_link_libraries(lightlyshaders_blur PRIVATE
    kwin
    kwineffects
    kwinglutils

    KF5::ConfigGui
    XCB::XCB

    KDecoration2::KDecoration

    lshelper
)

install(TARGETS lightlyshaders_blur DESTINATION ${MODULEPATH}/kwin/effects/plugins)