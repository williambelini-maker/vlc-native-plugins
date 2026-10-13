# How to Zoom in VLC with the Mouse Wheel

ZoomPan adds native-style mouse zoom and panning controls to **VLC Media Player on Windows**.

If you are looking for **VLC mouse wheel zoom**, **VLC zoom with Ctrl + mouse wheel**, or a way to **pan a zoomed video in VLC with the mouse**, ZoomPan provides those controls through a native VLC interface plugin.

## Controls

| Action | Mouse control |
| --- | --- |
| Zoom in | Ctrl + mouse wheel up |
| Zoom out | Ctrl + mouse wheel down |
| Pan zoomed video | Hold left mouse button and drag |
| Reset zoom / fit video | Double-click middle mouse button / wheel |

## Installation

1. Go to the [latest ZoomPan release](https://github.com/williambelini-maker/vlc-native-plugins/releases/latest).
2. Download `VLCZoomPanSetup.exe`.
3. Close VLC if it is running.
4. Run the installer as administrator.
5. Select the directory containing `vlc.exe`.
6. Open VLC normally.

The installer embeds the plugin DLL, installs it into VLC's plugin directory, updates the required VLC hotkeys, enables the `zoompan` interface module, and refreshes VLC's plugin cache.

## Does it run a helper in the background?

No. ZoomPan runs inside `vlc.exe` as a VLC interface module. It does not require a persistent helper application, Windows service, startup program, or special VLC launcher.

## Supported VLC version

The current release is validated on **VLC 3.0.21 x64 for Windows**.

Other VLC 3.x x64 versions may be ABI-compatible, but they are not listed as validated until tested. VLC 4.x is not supported by the current release.

## Português: zoom no VLC com a roda do mouse

Depois de instalar o ZoomPan:

- **Ctrl + roda do mouse** faz zoom no vídeo;
- **segure o botão esquerdo e arraste** para mover a imagem ampliada;
- **clique duas vezes na roda/botão do meio** para restaurar o vídeo.

O ZoomPan é um plugin nativo do VLC para Windows e roda dentro do próprio processo do VLC.
