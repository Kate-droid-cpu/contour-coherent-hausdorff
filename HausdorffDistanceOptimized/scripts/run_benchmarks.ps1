$ErrorActionPreference = "Stop"

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

$RepoRoot = Resolve-Path "$ScriptDir\..\.."

$Preset = "x64-release"
$BuildDir = Join-Path $RepoRoot "out\build\$Preset"
$Exe = Join-Path $BuildDir "HausdorffDistanceOptimized\HausdorffDistanceOptimized.exe"

Push-Location $RepoRoot
try {
    cmake --preset $Preset
    cmake --build $BuildDir

    if (!(Test-Path $Exe)) {
        throw "Executable not found: $Exe"
    }

    & $Exe `
      --folder "$RepoRoot\TestImages\WalkingSilhouette" `
      --folder "$RepoRoot\TestImages\SmallDeformations" `
      --folder "$RepoRoot\TestImages\SyntheticDefects" `
      --folder "$RepoRoot\TestImages\SmallContoursDefects" `
      --folder "$RepoRoot\TestImages\snowflake" `
      --batches 5 `
      --algorithms contour,shuffle,bruteforce
}
finally {
    Pop-Location
}