# AltBacktick

A lightweight utility providing macOS-style window switching within the same application for Windows.

## Core Features

- **Same-App Window Cycling**: Quickly cycle through multiple windows of the currently active application using shortcuts.
- **Process Switching (Alt+Tab Override)**: Press **Alt+Tab** to switch between running applications with a visual overlay. Tab cycles forward, Shift+Tab cycles backward, Esc cancels. Release Alt to confirm.
- **Background Execution**: Runs silently in the background without a console window, managed via a system tray icon.
- **Minimalist Design**: Compiled in C++ using Win32 API for high performance and low resource consumption.

## Requirements

- **Administrator privileges** required. The app uses a low-level keyboard hook (`WH_KEYBOARD_LL`) which cannot intercept input to elevated processes at a lower integrity level.
- Windows 10 or later.

## Usage (Shortcuts)

- **``Alt + ` ``**: Switch to the next window of the current application.
- **``Alt + Shift + ` ``**: Switch to the previous window of the current application.
- **Alt + Tab**: Switch between running applications with a visual overlay.
- **Esc**: Cancel the Alt+Tab switcher.

## Execution and Termination

- **Run**: Execute the `bin/alt-backtick.exe` file as administrator.
- **Exit**: Right-click the system tray icon in the taskbar and select **Exit AltBacktick**.
- **Auto-start**: Right-click the tray icon → **Run on Startup** to register via Task Scheduler. The app will launch with admin privileges at every logon without a UAC prompt.

## Build (For Developers)

Requires **MinGW GCC 15+** and **GNU Make** (or Visual Studio 2022 Build Tools).

```bash
make release   # GUI build, no console
make debug     # Debug build with console logging
make clean     # Remove build artifacts
```
