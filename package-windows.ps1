# Author: m3rlinek.
param(
    [ValidateSet('x64', 'x86')][string]$Architecture = 'x64',
    [string]$Version = '0.85-cpp.2',
    [string]$PackageDirectory = '',
    [string]$OutputDirectory = '',
    [string]$InstallerCompiler = ''
)
$ErrorActionPreference = 'Stop'
if ($Version -notmatch '^\d+\.\d+-cpp\.\d+$') { throw 'Invalid package version' }
if (-not $PackageDirectory) {
    $PackageDirectory = Join-Path $PSScriptRoot $(if ($Architecture -eq 'x64') { 'dist' } else { 'dist/x86' })
}
if (-not $OutputDirectory) { $OutputDirectory = Join-Path $PSScriptRoot 'out' }
$PackageDirectory = (Resolve-Path -LiteralPath $PackageDirectory).Path
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$OutputDirectory = (Resolve-Path -LiteralPath $OutputDirectory).Path
if (-not $InstallerCompiler) { $InstallerCompiler = (Get-Command ISCC.exe -ErrorAction Stop).Source }
$programs = 'putty','puttytel','plink','pscp','psftp','puttygen','pageant','pterm','psocks'
$files = @($programs | ForEach-Object { "${_}.exe" })
$files += 'LICENCE','THIRD-PARTY-LLVM-LICENSE.txt','putty-transfer.sh','FILE-TRANSFER.md','DUPLICATE-SESSION.md'
$expectedMachine = if ($Architecture -eq 'x64') { 0x8664 } else { 0x14c }
foreach ($program in $programs) {
    $bytes = [IO.File]::ReadAllBytes((Join-Path $PackageDirectory "$program.exe"))
    $offset = [BitConverter]::ToInt32($bytes, 0x3c)
    if ([BitConverter]::ToUInt32($bytes, $offset) -ne 0x4550 -or
        [BitConverter]::ToUInt16($bytes, $offset + 4) -ne $expectedMachine) {
        throw "Incorrect PE architecture for $program.exe"
    }
}
$paths = @($files | ForEach-Object { (Get-Item -LiteralPath (Join-Path $PackageDirectory $_)).FullName })
$archive = Join-Path $OutputDirectory "PuTTY-$Version-windows-$Architecture-portable.zip"
Compress-Archive -LiteralPath $paths -DestinationPath $archive -Force
& $InstallerCompiler "/DPackageDir=$PackageDirectory" "/DOutputDir=$OutputDirectory" "/DAppVersion=$Version" "/DTargetArch=$Architecture" (Join-Path $PSScriptRoot 'installer/putty.iss')
if ($LASTEXITCODE -ne 0) { throw "Installer compiler failed: $LASTEXITCODE" }
Get-Item -LiteralPath $archive, (Join-Path $OutputDirectory "PuTTY-$Version-windows-$Architecture-setup.exe")
