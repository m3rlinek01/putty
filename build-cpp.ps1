param(
    [switch]$Test,
    [int]$Jobs = 8,
    [string]$PythonExecutable = ''
)
$ErrorActionPreference = 'Stop'
$compiler = (Get-Command clang++ -ErrorAction Stop).Source
$cCompiler = (Get-Command clang -ErrorAction Stop).Source
$make = (Get-Command mingw32-make -ErrorAction Stop).Source
$cmake = (Get-Command cmake -ErrorAction Stop).Source
$build = Join-Path $PSScriptRoot 'build'
$configure = @('-S', $PSScriptRoot, '-B', $build, '-G', 'MinGW Makefiles',
    "-DCMAKE_C_COMPILER=$cCompiler", "-DCMAKE_CXX_COMPILER=$compiler",
    "-DCMAKE_MAKE_PROGRAM=$make", '-DCMAKE_BUILD_TYPE=Release',
    '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON')
$localPython = $PythonExecutable
if (-not $localPython) {
    $localPython = Join-Path $PSScriptRoot '.venv\Scripts\python.exe'
    if (-not (Test-Path -LiteralPath $localPython)) {
        $localPython = Join-Path (Split-Path $PSScriptRoot -Parent) '.venv\Scripts\python.exe'
    }
}
if (Test-Path -LiteralPath $localPython) {
    $configure += "-DPython3_EXECUTABLE=$localPython"
}
& $cmake @configure
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $cmake --build $build --parallel $Jobs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
if ($Test) {
    & (Get-Command ctest -ErrorAction Stop).Source --test-dir $build --output-on-failure
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
$dist = Join-Path $PSScriptRoot 'dist'
New-Item -ItemType Directory -Path $dist -Force | Out-Null
foreach ($program in 'putty','puttytel','plink','pscp','psftp','puttygen','pageant','pterm','psocks') {
    Copy-Item -LiteralPath (Join-Path $build "$program.exe") -Destination $dist
}
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'LICENCE') -Destination $dist
Write-Host "Ready: $dist"
