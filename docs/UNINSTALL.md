# Uninstalling ZoomPan

The current ZoomPan installer does not include an integrated uninstall option.

## Manual removal

1. Close VLC completely.
2. Open the VLC installation directory.
3. Delete:

```text
plugins\control\libzoompan_plugin.dll
```

4. Open the current user's VLC configuration file:

```text
%APPDATA%\vlc\vlcrc
```

5. Remove `zoompan` from the `extraintf=` setting. If `zoompan` is the only extra interface, leave the setting empty or restore your previous value.
6. If you want to restore the configuration exactly as it was before the first ZoomPan installation, use the backup created by the installer:

```text
%LOCALAPPDATA%\VLCZoomPan\backup
```

The installer also creates a one-time pre-ZoomPan VLC configuration backup named:

```text
vlcrc.before-zoompan
```

7. Start VLC normally.

## Plugin cache

If VLC still lists the removed plugin, refresh the plugin cache using VLC's `vlc-cache-gen.exe` when available, or remove VLC's generated `plugins.dat` cache so VLC rebuilds it on the next scan.

## Notes

Removing the DLL disables ZoomPan itself. Restoring the previous VLC configuration also removes the ZoomPan-specific hotkey/config changes made during installation.
