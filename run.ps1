$ErrorActionPreference = "Stop"
$root = $PSScriptRoot
Set-Location $root

if (-not (Test-Path "$root\build\CMakeCache.txt")) {
  throw "Build not configured. See README: run cmake -B build (e.g. with the vcpkg CMAKE_TOOLCHAIN_FILE on Windows), then re-run this script."
}
$cache = Get-Content "$root\build\CMakeCache.txt" -Raw
if ($cache -match "CMAKE_GENERATOR:INTERNAL=Visual Studio" -or
    $cache -match "CMAKE_GENERATOR:INTERNAL=Ninja Multi-Config" -or
    $cache -match "CMAKE_GENERATOR:INTERNAL=Xcode") {
  $config = "Release"
  if ($args.Count -ge 1) { $config = $args[0] }
  cmake --build $root\build --config $config --target run
} else {
  cmake --build $root\build --target run
}
