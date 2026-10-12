# Qt YIELD / CONTROL demo

Pure Qt 6 (Widgets + Test), CMake, using a shared Qt install.

## 1. Point CMake at your Qt

Use any shared Qt 6.5+ (e.g. from the official installer or your distro).

Linux / macOS (bash/zsh):

```bash
export CMAKE_PREFIX_PATH="/path/to/Qt/6.x.y/gcc_64${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
```

Windows (PowerShell):

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.x.y\msvc2019_64;$env:CMAKE_PREFIX_PATH"
```

Alternatively pass `-DCMAKE_PREFIX_PATH=...` or `-DQt6_DIR=.../lib/cmake/Qt6`
on the CMake command line.

## 2. Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## 3. Run

`./build/qt_yield_control` (macOS: `open build/qt_yield_control.app`).
The Qt shared libraries must be findable at runtime (`LD_LIBRARY_PATH` on
Linux, `PATH` on Windows, or run `windeployqt` / `macdeployqt` to bundle them).

Click **Open modal...**. The dialog starts in **YIELD** and QTest cycles
through radios (select, 1 s, unselect), checkboxes (same), then each text
field (types 15 chars at 0.5 s, then backspaces every 0.5 s until empty),
looping forever.

- **Ctrl+Shift+M** (Cmd+Shift+M on macOS) in YIELD switches to **CONTROL**: automation pauses.
- In CONTROL, any real mouse movement or the dialog regaining focus switches back to YIELD,
  which resumes from the step where it paused.
