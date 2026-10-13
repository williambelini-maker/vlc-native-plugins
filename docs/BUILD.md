# Building on Windows

The supported build entry point is:

```powershell
pwsh .\scripts\windows\build.ps1
```

## Pinned dependencies

- **llvm-mingw 20260602 UCRT x86_64**
  - SHA-256: `3de3eda9377bbaf35f8c9001f190380f63b8ee981fa55d3ae9d7cce7c6ad7c70`
- **VLC 3.0.21 source**
  - SHA-256: `24dbbe1d7dfaeea0994d5def0bbde200177347136dbfe573f5b6a4cee25afbb0`

The VLC source package is used for public plugin headers only. It is not installed.

## Output

```text
dist/
  VLCZoomPanSetup.exe
  libzoompan_plugin.dll
  SHA256SUMS.txt
```

The installer embeds the plugin DLL and installation script, so end users only need `VLCZoomPanSetup.exe`.

## Reproducibility

All dependency URLs and hashes are pinned in the build script. If any downloaded file does not match the expected SHA-256, the build stops before extraction or compilation.
