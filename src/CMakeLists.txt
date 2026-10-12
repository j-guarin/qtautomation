cmake_minimum_required(VERSION 3.21)
project(QtYieldControl VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Qt (shared build) -------------------------------------------------------
# Found via CMAKE_PREFIX_PATH / Qt6_DIR (see README.md).
find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Test)

qt_standard_project_setup()   # enables AUTOMOC etc.

qt_add_executable(qt_yield_control WIN32 MACOSX_BUNDLE
    main.cpp
    AutomationDialog.h
    AutomationDialog.cpp
)

# Qt6::Test provides QTest (mouseClick / keyClick) used at runtime.
target_link_libraries(qt_yield_control PRIVATE Qt6::Widgets Qt6::Test)
