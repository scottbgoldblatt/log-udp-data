# scripts/bootstrap-vcpkg.ps1
$ErrorActionPreference = "Stop"

$VCPKG_ROOT = Join-Path $env:USERPROFILE "vcpkg"
Write-Host "VCPKG_ROOT = $VCPKG_ROOT"

if (-not (Test-Path $VCPKG_ROOT)) {
    Write-Host "Cloning vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git $VCPKG_ROOT
} else {
    Write-Host "vcpkg already present at $VCPKG_ROOT"
}

Push-Location $VCPKG_ROOT
if (-not (Test-Path ".\vcpkg.exe")) {
    Write-Host "Bootstrapping vcpkg..."
    .\bootstrap-vcpkg.bat
} else { Write-Host "vcpkg.exe found, skipping bootstrap." }

Write-Host "Installing manifest dependencies (if vcpkg.json present in workspace)..."
try {
    if (Test-Path "$PSScriptRoot/../vcpkg.json") {
        .\vcpkg.exe install --manifest
    } else {
        Write-Host "No vcpkg.json in repo root; installing default packages: boost-system, boost-asio"
        .\vcpkg.exe install boost-system boost-asio --triplet x64-windows
    }
} catch {
    Write-Warning "Failed to install packages via vcpkg (non-fatal): $_"
}

# Optional: integrate (non-blocking)
try { .\vcpkg.exe integrate install } catch {}

Pop-Location
Write-Host "Done. Set env var VCPKG_ROOT to $VCPKG_ROOT or let workspace settings point at it."