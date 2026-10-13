# Architecture

ZoomPan is a native VLC interface module for Windows.

## Runtime model

```text
vlc.exe
  └─ loads libzoompan_plugin.dll
       ├─ observes mouse button state while VLC is foreground
       ├─ locates the VLC video output window
       ├─ pans the enlarged video window
       └─ invokes VLC's configured reset hotkey
```

There is no persistent helper process, Windows service, startup task, or alternate VLC launcher.

## VLC integration

The module declares `set_capability("interface", 0)` and `add_shortcut("zoompan")`, then is enabled by `extraintf=zoompan`.

## Zoom

Zoom remains VLC-native. The installer maps Ctrl + mouse wheel to VLC's scale-factor hotkeys.

## Pan

When the rendered video window is larger than its parent client area, holding the left mouse button and dragging repositions the video output window. Movement is clamped so the image cannot be dragged completely outside the viewport.

## Reset

Middle-button double-click triggers VLC's configured `key-zoom-original` shortcut and temporarily enforces a fitted video window while preserving the current aspect ratio.

## Platform

The current implementation relies on Win32 window discovery and input state APIs and therefore targets Windows.
