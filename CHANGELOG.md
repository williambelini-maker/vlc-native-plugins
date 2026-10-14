# Changelog

## 0.1.1

- Added integrated uninstall to the same `VLCZoomPanSetup.exe`.
- Uninstall removes the plugin, cleans `extraintf=zoompan`, restores ZoomPan-modified VLC settings from the original backup when available, and refreshes the plugin cache.
- Added a direct end-user installer download link to the README.
- Installer now enforces the validated VLC 3.0.21 x64 target instead of accepting any VLC 3.x build.
- CI now validates the generated installer PowerShell syntax before compiling the setup executable.


## 0.1.0

Initial ZoomPan release.

- Native VLC interface module for Windows x64.
- Ctrl + mouse wheel zoom via VLC scale-factor hotkeys.
- Left-button drag panning while zoomed.
- Middle-button double-click reset/fit.
- Single-file Windows installer with embedded plugin.
- Validated on VLC 3.0.21 x64.
