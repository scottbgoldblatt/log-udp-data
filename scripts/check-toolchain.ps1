# scripts/check-toolchain.ps1
$ErrorActionPreference = "Stop"

Write-Host "Checking Windows C++ toolchain..."

$ok = $true

# Check cl.exe (MSVC)
$cl = & where.exe cl.exe 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Warning "cl.exe (MSVC) NOT found."
    $ok = $false
} else { Write-Host "cl.exe found: $cl" }

# Check CMake
$cm = & where.exe cmake 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Warning "cmake NOT found."
    $ok = $false
} else { Write-Host "cmake found: $cm" }

# Check Ninja (nice to have)
$ninja = & where.exe ninja 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Warning "ninja NOT found. CMake can still use MSBuild but Ninja recommended."
} else { Write-Host "ninja found: $ninja" }

# Check Git
$git = & where.exe git 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Warning "git NOT found."
    $ok = $false
} else { Write-Host "git found: $git" }

# Final advice
if ($ok) {
    Write-Host "Toolchain looks OK. You can build locally."
    exit 0
} else {
    Write-Host ""
    Write-Host "If you need to install the Build Tools, run the Visual Studio Installer and choose:"
    Write-Host "  -> 'Desktop development with C++' (MSVC v143, Windows SDK)"
    Write-Host ""
    Write-Host "For a click-through flow, ask your admin to run the installer or use the IT managed installer."
    exit 1
}