# VLC Mouse Zoom & Pan Plugin for Windows

**ZoomPan** is a native **VLC Media Player zoom plugin** for Windows. It adds **VLC mouse wheel zoom**, click-and-drag video panning, and instant zoom reset while running directly inside `vlc.exe`.

[![Windows build](https://github.com/williambelini-maker/vlc-native-plugins/actions/workflows/windows-build.yml/badge.svg)](https://github.com/williambelini-maker/vlc-native-plugins/actions/workflows/windows-build.yml)
[![Release](https://img.shields.io/github/v/release/williambelini-maker/vlc-native-plugins)](https://github.com/williambelini-maker/vlc-native-plugins/releases/latest)

> Independent community project. Not affiliated with, endorsed by, or maintained by VideoLAN.

## How to zoom in VLC with the mouse wheel

After installing ZoomPan, open VLC normally:

- **Ctrl + mouse wheel** — zoom in or out
- **Hold left mouse button + drag** — pan the zoomed video
- **Double-click the middle mouse button / wheel** — reset zoom and fit the video

No alternate VLC launcher is required. No background helper, Windows service, startup monitor, or resident process is installed.

If you searched for **VLC zoom with mouse wheel**, **VLC mouse zoom**, **VLC pan video with mouse**, or a **VLC zoom plugin for Windows**, this is the problem ZoomPan is designed to solve.

## Why ZoomPan exists

I wasn't satisfied with the existing ways to zoom and navigate video in VLC. The available solutions felt too static, awkward, or dependent on workarounds for something that should feel immediate.

I wanted a more **dynamic, fluid, mouse-driven experience**: zoom with the wheel, grab the image directly, move it naturally, and reset it instantly — without leaving VLC or relying on a separate helper running in the background.

ZoomPan was built to turn that idea into a native VLC workflow.

## ZoomPan

ZoomPan is implemented as a native VLC **interface module**. The plugin loads inside the VLC process and uses VLC's own scale-factor controls for zoom.

The project is designed to keep the behavior inside VLC rather than requiring an external mouse utility.

## Download for Windows

**You do not need to clone this repository or build anything.**

### [Download VLCZoomPanSetup.exe](https://github.com/williambelini-maker/vlc-native-plugins/releases/latest/download/VLCZoomPanSetup.exe)

That is the end-user installer. The source-code tree intentionally does not contain generated `.exe` or `.dll` binaries; downloadable binaries are published under **GitHub Releases**.

Run `VLCZoomPanSetup.exe` as administrator and select the folder that contains `vlc.exe`.

The installer:

1. validates the selected VLC executable and architecture;
2. installs `libzoompan_plugin.dll` under `plugins\control`;
3. backs up the current user VLC configuration;
4. configures VLC mouse-wheel zoom hotkeys and `extraintf=zoompan`;
5. rebuilds the VLC plugin cache.

After installation, launch **VLC Media Player normally**.

## Como dar zoom no VLC com o mouse

Para quem procura **zoom no VLC com mouse**, **zoom no VLC com a roda do mouse** ou **arrastar vídeo ampliado no VLC**:

- **Ctrl + roda do mouse** — aumenta ou diminui o zoom
- **Botão esquerdo pressionado + arrastar** — move o vídeo ampliado
- **Clique duplo no botão do meio / roda** — restaura o vídeo para caber na janela

O plugin funciona dentro do próprio VLC, sem helper externo rodando em segundo plano.

## Compatibility

The current release is validated on **VLC 3.0.21 x64 for Windows**.

It targets the VLC 3.x plugin ABI. Other VLC 3.x x64 builds are compatibility candidates, but should be tested before being marked supported. VLC 4.x is not supported by the current build.

See [docs/COMPATIBILITY.md](docs/COMPATIBILITY.md).

## Limitations and uninstall

- **Validated:** VLC 3.0.21 x64 on Windows.
- Other VLC 3.x x64 versions may work, but are not marked validated until tested.
- **VLC 4.x is not supported** by the current release.
- The current installer is **not digitally signed**, so Windows SmartScreen may show an "unknown publisher" or reputation warning even though the file is downloadable and executable.
- The installer includes an **integrated uninstall flow**: run the same `VLCZoomPanSetup.exe` again and choose **No / Desinstalar**.

See [docs/UNINSTALL.md](docs/UNINSTALL.md) for removal details.

## Guides

- [How to zoom in VLC with the mouse wheel](docs/how-to-zoom-in-vlc-with-mouse-wheel.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Build from source](docs/BUILD.md)
- [Compatibility](docs/COMPATIBILITY.md)

## Build from source

On Windows PowerShell:

```powershell
pwsh .\scripts\windows\build.ps1
```

The build script downloads pinned dependencies, verifies SHA-256 checksums, compiles the native plugin, embeds it into the single-file installer, and writes release artifacts to `dist\`.

## Repository layout

```text
plugins/
  zoompan/
    src/VLCZoomPanPlugin.c
installer/
  windows/
scripts/
  windows/
docs/
.github/
  workflows/
```

## Credits

**William** — concept, requirements, hands-on testing, behavior validation, and project direction.

**ChatGPT (OpenAI)** — engineering assistance, implementation, debugging, build tooling, and documentation.

See [CREDITS.md](CREDITS.md).

## License

LGPL-2.1-or-later. See [LICENSE](LICENSE).
