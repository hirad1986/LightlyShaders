cmake_minimum_required(VERSION 3.16.0)

set(LIGHTLYSHADERS kwin4_effect_lightlyshaders)

project(${LIGHTLYSHADERS})
set(PROJECT_VERSION "2.2.0")
set(PROJECT_VERSION_MAJOR 0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DQT_NO_DEBUG_OUTPUT")

set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.102.0")

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Gui
    Core
    UiTools
    Widgets
    X11Extras
    OpenGL
    Network
    Xml
)

include_directories(${Qt5Widgets_INCLUDE_DIRS} ${Qt5X11Extras_INCLUDE_DIRS} ${Qt5Network_INCLUDE_DIRS} ${Qt5OpenGL_INCLUDE_DIRS} ${Qt5Xml_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

# required frameworks by Core
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    ConfigWidgets
    CoreAddons
    Crash
    GlobalAccel
    I18n
    KIO
    Service
    Init
    Notifications
    Service
    WidgetsAddons
    WindowSystem
    GuiAddons
)

find_package(X11 REQUIRED)
find_package(XCB REQUIRED)
find_package(KDecoration2 REQUIRED)

execute_process(COMMAND kf5-config --install module OUTPUT_VARIABLE MODULEPATH OUTPUT_STRIP_TRAILING_WHITESPACE)

find_path(EFFECTS_H kwineffects.h PATH_SUFFIXES kf5)

if (EFFECTS_H)
    include_directories(${EFFECTS_H})
else (EFFECTS_H)
    message(STATUS "didnt find kwineffects.h, not building effects")
endif (EFFECTS_H)

find_library(KWIN_EFFECTS NAMES kwineffects PATH_SUFFIXES kf5)
find_library(KWIN_GLUTILS NAMES kwinglutils PATH_SUFFIXES kf5)
find_library(OPENGL NAMES GL)

if (NOT KWIN_EFFECTS)
    message(STATUS "didnt find kwineffects lib, not building effects")
endif (NOT KWIN_EFFECTS)

if (NOT KWIN_GLUTILS)
    message(STATUS "didnt find kwin glutils lib, not building effects")
endif (NOT KWIN_GLUTILS)

if (NOT OPENGL)
    message(STATUS "didnt find opengl, not building effects")
endif (NOT OPENGL)

if (NOT EFFECTS_H OR NOT KWIN_GLUTILS OR NOT KWIN_EFFECTS OR NOT OPENGL)
    message(FATAL_ERROR "cant continue")
endif (NOT EFFECTS_H OR NOT KWIN_GLUTILS OR NOT KWIN_EFFECTS OR NOT OPENGL)

function(kwin_add_effect_config name)
    list(REMOVE_ITEM ARGV ${name})
    kcoreaddons_add_plugin(${name} INSTALL_NAMESPACE "kwin/effects/configs" SOURCES ${ARGV})
endfunction()

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

