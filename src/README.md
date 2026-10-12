# Qt YIELD / CONTROL demo

Pure Qt 6 (Widgets + Test), CMake, statically linked.

## 1. You need a STATIC Qt

The official Qt installers ship only shared libraries, so build `qtbase`
statically once (it contains QtCore, QtGui, QtWidgets and QtTest):

```bash
# Linux / macOS
tar xf qtbase-everywhere-src-6.x.y.tar.xz && cd qtbase-everywhere-src-6.x.y
./configure -static -release -prefix /opt/qt6-static -nomake examples -nomake tests
cmake --build . --parallel
cmake --install .
```

```bat
:: Windows (x64 Native Tools Command Prompt for VS)
configure.bat -static -static-runtime -release -prefix C:\Qt\6-static -nomake examples -nomake tests
cmake --build . --parallel
cmake --install .
```

Linux needs the X11/xcb and/or Wayland dev packages installed before
`configure` (e.g. `libxcb*-dev`, `libxkbcommon-x11-dev`, `libwayland-dev`),
otherwise no platform plugin gets built.

## 2. Export the environment variables

Linux / macOS (bash/zsh):

```bash
export QT_STATIC_ROOT=/opt/qt6-static
export Qt6_DIR="$QT_STATIC_ROOT/lib/cmake/Qt6"
export CMAKE_PREFIX_PATH="$QT_STATIC_ROOT${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
export PATH="$QT_STATIC_ROOT/bin:$PATH"
# Headers and libraries (CMake gets them from Qt6_DIR; these are for other tools/IDEs)
export QT_INCLUDE_DIR="$QT_STATIC_ROOT/include"
export QT_LIB_DIR="$QT_STATIC_ROOT/lib"
export CPLUS_INCLUDE_PATH="$QT_INCLUDE_DIR${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
export LIBRARY_PATH="$QT_LIB_DIR${LIBRARY_PATH:+:$LIBRARY_PATH}"
```

Windows (PowerShell):

```powershell
$env:QT_STATIC_ROOT    = "C:\Qt\6-static"
$env:Qt6_DIR           = "$env:QT_STATIC_ROOT\lib\cmake\Qt6"
$env:CMAKE_PREFIX_PATH = "$env:QT_STATIC_ROOT;$env:CMAKE_PREFIX_PATH"
$env:PATH              = "$env:QT_STATIC_ROOT\bin;$env:PATH"
$env:QT_INCLUDE_DIR    = "$env:QT_STATIC_ROOT\include"
$env:QT_LIB_DIR        = "$env:QT_STATIC_ROOT\lib"
$env:INCLUDE           = "$env:QT_INCLUDE_DIR;$env:INCLUDE"
$env:LIB               = "$env:QT_LIB_DIR;$env:LIB"
```

## 3. Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

CMake stops with an error if the Qt it finds is a shared build.
Linking: Qt fully static; MSVC uses the static CRT (`/MT`); Linux uses
`-static-libstdc++ -static-libgcc` (glibc itself stays dynamic, Qt's platform
plugins don't support fully static glibc).

## 4. Run

`./build/qt_yield_control` (macOS: `open build/qt_yield_control.app`).

Click **Open modal...**. The dialog starts in **YIELD** and QTest cycles
through radios (select, 1 s, unselect), checkboxes (same), then each text
field (types 15 chars at 0.5 s, then backspaces every 0.5 s until empty),
looping forever.

- **Ctrl+Shift+M** (Cmd+Shift+M on macOS) in YIELD switches to **CONTROL**: automation pauses.
- In CONTROL, any real mouse movement or the dialog regaining focus switches back to YIELD,
  which resumes from the step where it paused.
