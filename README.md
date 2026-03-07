# AltBacktick

A lightweight utility providing macOS-style window switching within the same application for Windows.

## Core Features

- **Same-App Window Cycling**: Quickly cycle through multiple windows of the currently active application using shortcuts.
- **Background Execution**: Runs silently in the background without a console window, managed via a system tray icon.
- **Minimalist Design**: Compiled in C++ using Win32 API for high performance and low resource consumption.

## Usage (Shortcuts)

- **``Alt + ` ``**: Switch to the next window of the current application.
- **``Alt + Shift + ` ``**: Switch to the previous window of the current application.

## Execution and Termination

- **Run**: Execute the `bin/alt-backtick.exe` file.
- **Exit**: Right-click the system tray icon in the taskbar and select **Exit AltBacktick**.

## Build (For Developers)

Requires **Visual Studio 2022 Build Tools** installed on your system.
