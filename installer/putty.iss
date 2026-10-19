; Author: m3rlinek.
#ifndef PackageDir
  #error PackageDir is required
#endif
#ifndef OutputDir
  #error OutputDir is required
#endif
#ifndef AppVersion
  #define AppVersion "0.85-cpp.2"
#endif
#ifndef TargetArch
  #define TargetArch "x64"
#endif

[Setup]
AppId=PuTTY-m3rlinek-{#TargetArch}
AppName=PuTTY by m3rlinek ({#TargetArch})
AppVersion={#AppVersion}
AppPublisher=m3rlinek
AppPublisherURL=https://github.com/m3rlinek01/putty
DefaultDirName={localappdata}\Programs\PuTTY-m3rlinek-{#TargetArch}
DefaultGroupName=PuTTY by m3rlinek ({#TargetArch})
PrivilegesRequired=lowest
MinVersion=10.0
#if TargetArch == "x64"
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
#else
ArchitecturesAllowed=x86compatible
#endif
OutputDir={#OutputDir}
OutputBaseFilename=PuTTY-{#AppVersion}-windows-{#TargetArch}-setup
SetupIconFile=..\windows\putty.ico
UninstallDisplayIcon={app}\putty.exe
LicenseFile={#PackageDir}\LICENCE
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
CloseApplications=no
RestartApplications=no
UninstallDisplayName=PuTTY by m3rlinek ({#TargetArch})

[Languages]
Name: "polish"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; Flags: unchecked

[Files]
Source: "{#PackageDir}\putty.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageDir}\puttytel.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageDir}\plink.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageDir}\pscp.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageDir}\psftp.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageDir}\puttygen.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageDir}\pageant.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageDir}\pterm.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageDir}\psocks.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageDir}\LICENCE"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageDir}\THIRD-PARTY-LLVM-LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageDir}\FILE-TRANSFER.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageDir}\DUPLICATE-SESSION.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#PackageDir}\putty-transfer.sh"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\PuTTY"; Filename: "{app}\putty.exe"
Name: "{group}\PuTTYgen"; Filename: "{app}\puttygen.exe"
Name: "{group}\Pageant"; Filename: "{app}\pageant.exe"
Name: "{group}\PSFTP"; Filename: "{app}\psftp.exe"
Name: "{autodesktop}\PuTTY ({#TargetArch})"; Filename: "{app}\putty.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\putty.exe"; Description: "{cm:LaunchProgram,PuTTY}"; Flags: nowait postinstall skipifsilent
