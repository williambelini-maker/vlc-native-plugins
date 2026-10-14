# Compatibility

## ZoomPan

| VLC version | Architecture | Status |
| --- | --- | --- |
| 3.0.21 | Windows x64 | **Tested and validated** |
| Other VLC 3.x | Windows x64 | Compatibility candidate; not yet validated |
| VLC 3.x | Windows x86 | Not built |
| VLC 4.x | Windows x64/x86 | Not supported by this build |

The Windows installer intentionally installs only into **VLC 3.0.21 x64**. Other VLC 3.x x64 versions remain candidates for future validation rather than supported installer targets.

## What "validated" means

The following behaviors were tested together on VLC 3.0.21 x64:

- Ctrl + mouse wheel changes VLC's native scale factor.
- Left-button dragging pans a zoomed video.
- Middle-button double-click resets the scale and fits the image.
- The plugin loads inside the VLC process through `extraintf=zoompan`.
- No background helper or Windows startup process is required.

## ABI note

ZoomPan is compiled as a VLC **interface module** using the VLC 3.x plugin ABI exposed by the VLC 3.0.21 headers. ABI compatibility alone does not guarantee identical window behavior across every VLC 3.x build, so additional versions should be tested before being listed as supported.
