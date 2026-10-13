# VLC Native Plugins

Native, auditable VLC plugins for Windows, with reproducible builds and simple installers.

> Independent community project. Not affiliated with, endorsed by, or maintained by VideoLAN.

## ZoomPan

The first plugin in this repository adds mouse-driven zoom navigation to VLC:

- **Ctrl + mouse wheel** — zoom in/out through VLC's native scale-factor hotkeys
- **Left mouse drag** — pan the video while zoomed
- **Middle-button double-click** — reset and fit the video

The plugin runs **inside `vlc.exe`** as a native VLC interface module. It does not install a background service, startup monitor, or external helper.

## Compatibility

The current build is validated on **VLC 3.0.21 x64 for Windows**.

It targets the VLC 3.x plugin ABI. Other VLC 3.x x64 builds are compatibility candidates, but should be tested before being marked supported. VLC 4.x is not supported by this build.

See [docs/COMPATIBILITY.md](docs/COMPATIBILITY.md).

## Install

Download `VLCZoomPanSetup.exe` from the Releases page, run it as administrator, and select the folder containing `vlc.exe`.

The installer:

1. validates the selected VLC executable and architecture;
2. extracts the embedded native plugin into `plugins\control`;
3. backs up the current user configuration;
4. configures the required VLC hotkeys and `extraintf=zoompan`;
5. rebuilds the VLC plugin cache.

After that, open VLC normally.

## Build

On Windows PowerShell:

```powershell
pwsh .\scripts\windows\build.ps1
```

The build script downloads pinned dependencies, verifies SHA-256 checksums, compiles the native plugin, embeds it into the single-file installer, and writes release artifacts to `dist\`.

See [docs/BUILD.md](docs/BUILD.md).

## Credits

**William** — concept, requirements, hands-on testing, behavior validation, and project direction.

**ChatGPT (OpenAI)** — engineering assistance, implementation, debugging, build tooling, and documentation.

See [CREDITS.md](CREDITS.md).

## License

LGPL-2.1-or-later. See [LICENSE](LICENSE).
