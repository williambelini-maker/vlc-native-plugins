# Uninstalling ZoomPan

ZoomPan can be removed with the **same installer used to install it**.

## Recommended uninstall

1. Close VLC completely.
2. Run `VLCZoomPanSetup.exe` as administrator.
3. Select the VLC folder containing `vlc.exe`.
4. When asked what to do, choose **No / Desinstalar**.
5. The installer removes `libzoompan_plugin.dll`, removes `zoompan` from `extraintf`, restores the VLC settings changed by ZoomPan from the pre-install backup when available, and refreshes the VLC plugin cache.

The uninstall is intentionally handled by the same single-file setup executable; ZoomPan does not install a background uninstaller or resident service.

## Manual removal

If the setup executable is no longer available:

1. Close VLC completely.
2. Delete:

```text
<VLC folder>\plugins\control\libzoompan_plugin.dll
```

3. Open:

```text
%APPDATA%\vlc\vlcrc
```

4. Remove `zoompan` from the `extraintf=` setting.
5. The installer's pre-ZoomPan configuration backup is stored under:

```text
%LOCALAPPDATA%\VLCZoomPan\backup\vlcrc.before-zoompan
```

6. Refresh VLC's plugin cache with `vlc-cache-gen.exe` when available, or remove VLC's generated `plugins.dat` so VLC rebuilds it.

## Important note about configuration restore

The integrated uninstaller restores only the ZoomPan-related VLC settings it changed, using the original backup when available. This avoids replacing the user's entire current VLC configuration just to remove the plugin.
