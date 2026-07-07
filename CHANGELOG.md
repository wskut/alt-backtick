# Changelog

All notable changes to this project will be documented in this file.

## [v0.2.0a1] - 2026-07-07

- Implemented **Alt+Tab process-level window switching** with visual overlay.
  - Overrides the system Alt+Tab via `WH_KEYBOARD_LL` hook.
  - Shows a centered, rounded-corner overlay with process icons and labels.
  - Tab/Shift+Tab cycles through processes; Esc cancels.
  - Alt release triggers the switch; deferred switch prevents menu bar activation.
- Added shared `AltTracker` for unified Alt key state tracking.
- Extracted shared `WindowFilter::IsSwitchableWindow()` / `BringWindowToForeground()`.
- Changed auto-start mechanism from registry `Run` key to **Task Scheduler**
  (`schtasks /create /sc onlogon /rl highest`) so the app runs as administrator
  at logon without a UAC prompt.
- Added `requireAdministrator` manifest for full UIPI bypass.
- Build system: Makefile with GCC/MinGW, debug/release targets.
- Fixed: stray Alt key reaching switched-to window, stuck Alt modifier, system
  process filtering, icon quality at native resolution.

## [v0.1.1] - 2026-03-22

- Added an auto-start feature to run the application automatically when Windows starts.
- Added a custom application icon for the executable and system tray.

## [v0.1.0] - 2026-01-02

- Implemented window switching within the same application using ``Alt + ` `` and ``Alt + Shift + ` `` shortcuts.
- Changed the application to run in the background by hiding the console window upon execution.
- Added a system tray icon to the taskbar.
- Added a context menu to the tray icon with an option to exit the application on right-click.
- Added code to allocate a console window for logging output when built in `DEBUG` mode.
- Fixed an issue where desktop windows (`Progman`, `WorkerW`) and unused system windows without titles were included in the window switching list.
