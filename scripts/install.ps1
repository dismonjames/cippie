# Cippie Windows Installer (PowerShell)
param(
    [string]$version = "0.1.4",
    [string]$prefix = "$env:LOCALAPPDATA\Programs",
    [string]$installDir = "",
    [switch]$force = $false
)

$ErrorActionPreference = "Stop"

function Show-Help {
    @"
Cippie Installer (Windows)

Usage:
  install.ps1 [options]

Options:
  -version <ver>       Version to install (default: 0.1.4)
  -prefix <path>       Installation prefix (default: `$env:LOCALAPPDATA\Programs)
  -installDir <path>   Direct binary installation directory
  -force               Overwrite existing installation
  -help                Show this help message

Environment Variables:
  CIPPIE_INSTALL_VERSION   Version to install
  CIPPIE_INSTALL_PREFIX    Installation prefix
  CIPPIE_RELEASE_BASE_URL  Base download URL (for testing/mirrors; default: GitHub releases)
"@
}

if ($help -or ($args -contains "-help") -or ($args -contains "--help")) {
    Show-Help
    exit 0
}

# Check for version from env
if ($env:CIPPIE_INSTALL_VERSION) {
    $version = $env:CIPPIE_INSTALL_VERSION
}
if ($env:CIPPIE_INSTALL_PREFIX) {
    $prefix = $env:CIPPIE_INSTALL_PREFIX
}
$releaseBaseUrl = if ($env:CIPPIE_RELEASE_BASE_URL) { $env:CIPPIE_RELEASE_BASE_URL } else { "https://github.com/dismonjames/cippie/releases/download" }

# Detect architecture
$arch = if ([Environment]::Is64BitOperatingSystem) { "x86_64" } else { "i386" }
$platform = "windows-$arch"
$packageName = "cippie-$version-$platform"
$tarballName = "$packageName.tar.gz"
$checksumName = "$tarballName.sha256"

if ($installDir) {
    $destDir = $installDir
}
else {
    $destDir = "$prefix\Cippie"
}

$destBinary = "$destDir\cippie.exe"

# Build download URLs
if ($releaseBaseUrl -match "github.com") {
    $tarballUrl = "$releaseBaseUrl/v$version/$tarballName"
    $checksumUrl = "$releaseBaseUrl/v$version/$checksumName"
}
else {
    $tarballUrl = "$releaseBaseUrl/$tarballName"
    $checksumUrl = "$releaseBaseUrl/$checksumName"
}

# Handle existing binary
if (Test-Path $destBinary -PathType Leaf) {
    if (-not $force) {
        try {
            $existingVer = & $destBinary version 2>$null
            if ($existingVer -and $existingVer.Contains($version)) {
                Write-Host "Cippie $version is already installed at $destBinary."
                exit 0
            }
        }
        catch {}
        Write-Host "Binary already exists at $destBinary. Use -force to overwrite."
        exit 1
    }
}

Write-Host "Downloading Cippie $version ($platform)..."
Write-Host "  $tarballUrl"

# Create temp directory
$tmpDir = Join-Path $env:TEMP "cippie-install-$(Get-Random)"
New-Item -ItemType Directory -Path $tmpDir -Force | Out-Null

try {
    # Download checksum
    $checksumPath = Join-Path $tmpDir $checksumName
    Write-Host "Downloading checksum..."
    Invoke-WebRequest -Uri $checksumUrl -OutFile $checksumPath -UseBasicParsing

    # Download tarball
    $tarballPath = Join-Path $tmpDir $tarballName
    Write-Host "Downloading release package..."
    Invoke-WebRequest -Uri $tarballUrl -OutFile $tarballPath -UseBasicParsing

    # Verify checksum
    Write-Host "Verifying SHA-256 checksum..."
    $expectedHash = (Get-Content $checksumPath -Raw).Split(' ')[0]
    $actualHash = (Get-FileHash $tarballPath -Algorithm SHA256).Hash.ToLower()
    if ($expectedHash -ne $actualHash) {
        Write-Error "SHA-256 checksum verification failed"
        exit 1
    }

    # Extract tarball (requires tar.exe, available in Windows 10 1803+)
    Write-Host "Extracting archive..."
    $extractDir = Join-Path $tmpDir "ext"
    New-Item -ItemType Directory -Path $extractDir -Force | Out-Null
    tar -xzf $tarballPath -C $extractDir

    $sourceBinary = Join-Path $extractDir "$packageName\bin\cippie.exe"
    if (-not (Test-Path $sourceBinary -PathType Leaf)) {
        Write-Error "Extracted binary not found at $sourceBinary"
        exit 1
    }

    # Verify extracted binary
    Write-Host "Verifying extracted binary..."
    $extractedVer = & $sourceBinary version 2>$null
    if ($extractedVer -and $extractedVer.Contains($version)) {
        Write-Host "Extracted binary version matches."
    }
    else {
        Write-Error "Extracted binary version does not match expected '$version'"
        exit 1
    }

    # Atomic installation
    New-Item -ItemType Directory -Path $destDir -Force | Out-Null

    if (Test-Path $destBinary -PathType Leaf) {
        Move-Item -Path $destBinary -Destination "$destBinary.old" -Force
    }

    Copy-Item -Path $sourceBinary -Destination $destBinary -Force
    Write-Host ""
    Write-Host "Cippie $version installed successfully."
    Write-Host ""
    Write-Host "Binary:"
    Write-Host "  $destBinary"
    Write-Host ""

    # Clean up .old file
    if (Test-Path "$destBinary.old" -PathType Leaf) {
        Remove-Item "$destBinary.old" -Force
    }

    # PATH check
    $userPath = [Environment]::GetEnvironmentVariable("PATH", "User")
    if ($userPath -notlike "*$destDir*") {
        Write-Host "Add this directory to your PATH:"
        Write-Host ""
        Write-Host "  `$env:Path = `"$destDir;`$env:Path`""
        Write-Host "  [Environment]::SetEnvironmentVariable(`"Path`", `"$destDir;`$env:Path`", `"User`")"
        Write-Host ""
    }
}
finally {
    Remove-Item -Path $tmpDir -Recurse -Force -ErrorAction SilentlyContinue
}
