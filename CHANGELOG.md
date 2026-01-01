# Changelog

All notable changes to this project will be documented in this file.

## [v0.1.0] - 2026-01-02

- Implemented window switching within the same application using ``Alt + ` `` and ``Alt + Shift + ` `` shortcuts.
- Changed the application to run in the background by hiding the console window upon execution.
- Added a system tray icon to the taskbar.
- Added a context menu to the tray icon with an option to exit the application on right-click.
- Added code to allocate a console window for logging output when built in `DEBUG` mode.
- Fixed an issue where desktop windows (`Progman`, `WorkerW`) and unused system windows without titles were included in the window switching list.
