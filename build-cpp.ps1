# Author: m3rlinek.
param(
    [ValidateSet('x64', 'x86')][string]$Architecture = 'x64',
    [switch]$Test,
    [int]$Jobs = 8,
    [string]$PythonExecutable = '',
    [string]$DistDirectory = ''
)
$ErrorActionPreference = 'Stop'
$target = if ($Architecture -eq 'x86') { 'i686' } else { 'x86_64' }
$compiler = (Get-Command "$target-w64-mingw32-clang++" -ErrorAction Stop).Source
$cCompiler = (Get-Command "$target-w64-mingw32-clang" -ErrorAction Stop).Source
$make = (Get-Command mingw32-make -ErrorAction Stop).Source
$cmake = (Get-Command cmake -ErrorAction Stop).Source
$build = Join-Path $PSScriptRoot $(if ($Architecture -eq 'x64') { 'build' } else { 'build-x86' })
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
$dist = if ($DistDirectory) { $DistDirectory } else { Join-Path $PSScriptRoot $(if ($Architecture -eq 'x64') { 'dist' } else { 'dist/x86' }) }
New-Item -ItemType Directory -Path $dist -Force | Out-Null
$readobj = Join-Path (Split-Path $compiler -Parent) 'llvm-readobj.exe'
foreach ($program in 'putty','puttytel','plink','pscp','psftp','puttygen','pageant','pterm','psocks') {
    $executable = Join-Path $build "$program.exe"
    $imports = & $readobj --coff-imports $executable
    if ($LASTEXITCODE -ne 0) { throw "Cannot inspect imports of $program.exe" }
    if ($imports -match '(?i)Name: (libc\+\+|libunwind|libwinpthread|libgcc[^\s]*|libstdc\+\+)[^\s]*\.dll') {
        throw "$program.exe requires a toolchain runtime DLL; portable packaging refused."
    }
    Copy-Item -LiteralPath $executable -Destination $dist
}
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'LICENCE') -Destination $dist
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'THIRD-PARTY-LLVM-LICENSE.txt') -Destination $dist
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'contrib\putty-transfer.sh') -Destination $dist
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'docs\FILE-TRANSFER.md') -Destination $dist
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'docs\DUPLICATE-SESSION.md') -Destination $dist
Write-Host "Ready: $dist"
