param([string]$VlcVersion = "3.0.21")

$ErrorActionPreference = "Stop"
$Repo = (Resolve-Path (Join-Path $PSScriptRoot "..\\..")).Path
$Deps = Join-Path $Repo ".deps"
$Build = Join-Path $Repo "build"
$Dist = Join-Path $Repo "dist"

$LlvmVersion = "20260602"
$LlvmUrl = "https://github.com/mstorsjo/llvm-mingw/releases/download/20260602/llvm-mingw-20260602-ucrt-x86_64.zip"
$LlvmSha = "3de3eda9377bbaf35f8c9001f190380f63b8ee981fa55d3ae9d7cce7c6ad7c70"
$VlcUrl = "https://download.videolan.org/pub/videolan/vlc/$VlcVersion/vlc-$VlcVersion.tar.xz"
$VlcSha = "24dbbe1d7dfaeea0994d5def0bbde200177347136dbfe573f5b6a4cee25afbb0"

function Get-VerifiedFile([string]$Url, [string]$Path, [string]$Sha256) {
    if (-not (Test-Path -LiteralPath $Path)) {
        Write-Host "Downloading $Url"
        Invoke-WebRequest -Uri $Url -OutFile $Path -UseBasicParsing
    }
    $actual = (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($actual -ne $Sha256.ToLowerInvariant()) {
        Remove-Item -LiteralPath $Path -Force -ErrorAction SilentlyContinue
        throw ("SHA-256 mismatch for " + $Path + [Environment]::NewLine + "Expected: " + $Sha256 + [Environment]::NewLine + "Actual:   " + $actual)
    }
}

New-Item -ItemType Directory -Force -Path $Deps, $Build, $Dist | Out-Null

$LlvmZip = Join-Path $Deps "llvm-mingw-$LlvmVersion.zip"
$LlvmDir = Join-Path $Deps "llvm-mingw"
Get-VerifiedFile $LlvmUrl $LlvmZip $LlvmSha
if (-not (Test-Path (Join-Path $LlvmDir "bin\\clang.exe"))) {
    $tmp = Join-Path $Deps "llvm-extract"
    Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force -Path $tmp | Out-Null
    Expand-Archive -LiteralPath $LlvmZip -DestinationPath $tmp -Force
    $top = Get-ChildItem -LiteralPath $tmp -Directory | Select-Object -First 1
    if (-not $top) { throw "llvm-mingw archive layout not recognized." }
    Remove-Item $LlvmDir -Recurse -Force -ErrorAction SilentlyContinue
    Move-Item -LiteralPath $top.FullName -Destination $LlvmDir
    Remove-Item $tmp -Recurse -Force
}

$VlcTar = Join-Path $Deps "vlc-$VlcVersion.tar.xz"
$VlcSrc = Join-Path $Deps "vlc-$VlcVersion"
Get-VerifiedFile $VlcUrl $VlcTar $VlcSha
if (-not (Test-Path (Join-Path $VlcSrc "include\\vlc_plugin.h"))) {
    Remove-Item $VlcSrc -Recurse -Force -ErrorAction SilentlyContinue
    & tar.exe -xf $VlcTar -C $Deps
    if ($LASTEXITCODE -ne 0) { throw "tar.exe failed to extract VLC source." }
}

$Clang = Join-Path $LlvmDir "bin\\clang.exe"
$Windres = Join-Path $LlvmDir "bin\\windres.exe"
$Include = Join-Path $VlcSrc "include"
$PluginSrc = Join-Path $Repo "plugins\\zoompan\\src\\VLCZoomPanPlugin.c"
$PluginDll = Join-Path $Build "libzoompan_plugin.dll"

Write-Host "Compiling ZoomPan plugin..."
& $Clang --target=x86_64-w64-windows-gnu -shared -O2 -Wall -Wextra -Wno-unreachable-code-generic-assoc -I $Include $PluginSrc -o $PluginDll -luser32 -lkernel32
if ($LASTEXITCODE -ne 0) { throw "Plugin compilation failed." }

$PluginSha = (Get-FileHash -LiteralPath $PluginDll -Algorithm SHA256).Hash
$Template = Join-Path $Repo "installer\\windows\\VLCZoomPanInstaller.ps1.in"
$Installer = Join-Path $Build "VLCZoomPanInstaller.ps1"
$installerText = Get-Content -LiteralPath $Template -Raw
if ($installerText -notmatch "@PLUGIN_SHA256@") { throw "Installer SHA token not found." }
$installerText = $installerText.Replace("@PLUGIN_SHA256@", $PluginSha)
Set-Content -LiteralPath $Installer -Value $installerText -Encoding UTF8

$tokens = $null
$parseErrors = $null
[System.Management.Automation.Language.Parser]::ParseFile($Installer, [ref]$tokens, [ref]$parseErrors) | Out-Null
if ($parseErrors.Count -gt 0) {
    $details = ($parseErrors | ForEach-Object { $_.Message }) -join [Environment]::NewLine
    throw ("Generated installer PowerShell has syntax errors:" + [Environment]::NewLine + $details)
}

$SetupSrc = Join-Path $Repo "installer\\windows\\VLCZoomPanSetup.c"
$Manifest = Join-Path $Repo "installer\\windows\\VLCZoomPanSetup.manifest"
$Rc = Join-Path $Build "VLCZoomPanSetup.rc"
$ResObj = Join-Path $Build "VLCZoomPanSetup.res.o"
$SetupExe = Join-Path $Dist "VLCZoomPanSetup.exe"
$pluginRc = $PluginDll.Replace("\\", "\\\\")
$scriptRc = $Installer.Replace("\\", "\\\\")
$manifestRc = $Manifest.Replace("\\", "\\\\")
$rcText = @("#include <windows.h>", "101 RCDATA `"$pluginRc`"", "102 RCDATA `"$scriptRc`"", "1 RT_MANIFEST `"$manifestRc`"") -join [Environment]::NewLine
Set-Content -LiteralPath $Rc -Value $rcText -Encoding ASCII

& $Windres --target=pe-x86-64 -i $Rc -o $ResObj -O coff
if ($LASTEXITCODE -ne 0) { throw "Resource compilation failed." }
& $Clang --target=x86_64-w64-windows-gnu -O2 -Wall -Wextra -municode -mwindows $SetupSrc $ResObj -o $SetupExe -luser32 -lkernel32
if ($LASTEXITCODE -ne 0) { throw "Installer compilation failed." }

$PluginDist = Join-Path $Dist "libzoompan_plugin.dll"
Copy-Item -LiteralPath $PluginDll -Destination $PluginDist -Force
$SetupSha = (Get-FileHash -LiteralPath $SetupExe -Algorithm SHA256).Hash
$PluginSha = (Get-FileHash -LiteralPath $PluginDist -Algorithm SHA256).Hash
$sums = @("$SetupSha  VLCZoomPanSetup.exe", "$PluginSha  libzoompan_plugin.dll") -join [Environment]::NewLine
Set-Content -LiteralPath (Join-Path $Dist "SHA256SUMS.txt") -Value $sums -Encoding ASCII
Write-Host "[OK] Build complete: $SetupExe"
Write-Host "SHA256 $SetupSha"
