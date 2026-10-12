cmake_minimum_required(VERSION 3.21)
project(QtYieldControl VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Static C/C++ runtime ----------------------------------------------------
# MSVC: /MT (Qt must be configured with -static-runtime to match).
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# --- Qt (must be a STATIC build) --------------------------------------------
# Found via CMAKE_PREFIX_PATH / Qt6_DIR (see README.md).
find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Test)

get_target_property(_qt_core_type Qt6::Core TYPE)
if(NOT _qt_core_type STREQUAL "STATIC_LIBRARY")
    message(FATAL_ERROR
        "The Qt found at '${Qt6_DIR}' is a SHARED build. "
        "This project requires a static Qt. Point CMAKE_PREFIX_PATH at your static Qt install.")
endif()

qt_standard_project_setup()   # enables AUTOMOC etc.

qt_add_executable(qt_yield_control WIN32 MACOSX_BUNDLE
    main.cpp
    AutomationDialog.h
    AutomationDialog.cpp
)

# Qt6::Test provides QTest (mouseClick / keyClick) used at runtime.
target_link_libraries(qt_yield_control PRIVATE Qt6::Widgets Qt6::Test)

# GCC/Clang on Linux: statically link the C++ runtime as well.
# (Fully static glibc is not supported by Qt's platform plugins.)
if(UNIX AND NOT APPLE)
    target_link_options(qt_yield_control PRIVATE -static-libstdc++ -static-libgcc)
endif()

# With static Qt 6, qt_add_executable automatically imports the default
# platform plugin (xcb/wayland, cocoa, windows) at finalization time.
