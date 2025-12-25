; அகம் (agam) - Inno Setup Installer Script
; Compile with Inno Setup Compiler (https://jrsoftware.org/isinfo.php)

#define MyAppName "agam"
#define MyAppNameTamil "அகம்"
#define MyAppVersion "0.1.2"
#define MyAppPublisher "agam Team"
#define MyAppURL "https://github.com/Aruvili/agam"
#define MyAppExeName "agam.exe"

[Setup]
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppNameTamil} ({#MyAppName}) {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}/releases
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile=..\LICENSE
InfoBeforeFile=..\README.md
OutputDir=..\dist\installer
OutputBaseFilename=agam-{#MyAppVersion}-setup
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
ChangesEnvironment=yes
SetupLogging=yes
UninstallDisplayIcon={app}\{#MyAppExeName}
VersionInfoVersion={#MyAppVersion}
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription=அகம் - Tamil Programming Language Installer
VersionInfoProductName={#MyAppName}
VersionInfoProductVersion={#MyAppVersion}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Types]
Name: "full"; Description: "Full installation (recommended)"
Name: "compact"; Description: "Compact installation (binary only)"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "main"; Description: "agam Interpreter"; Types: full compact custom; Flags: fixed
Name: "examples"; Description: "Example Programs"; Types: full
Name: "docs"; Description: "Documentation (14-chapter learning book)"; Types: full
Name: "vscode"; Description: "VS Code Extension"; Types: full

[Tasks]
Name: "addtopath"; Description: "Add agam to system PATH (recommended)"; GroupDescription: "Environment Configuration:"; Flags: checkedonce
Name: "desktopicon"; Description: "Create a desktop shortcut"; GroupDescription: "Additional Icons:"; Flags: unchecked
Name: "associatefile"; Description: "Associate .agam files with agam"; GroupDescription: "File Associations:"; Flags: checkedonce

[Files]
; Main binary
Source: "..\target\release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion; Components: main
; Documentation files
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion; Components: main
Source: "..\CHANGELOG.md"; DestDir: "{app}"; Flags: ignoreversion; Components: main
; Examples
Source: "..\examples\*"; DestDir: "{app}\examples"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: examples
; Documentation
Source: "..\docs\*"; DestDir: "{app}\docs"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: docs
; VS Code extension
Source: "..\vscode-extension\*"; DestDir: "{app}\vscode-extension"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: vscode

[Icons]
Name: "{group}\{#MyAppName} REPL"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{group}\Examples"; Filename: "{app}\examples"; Components: examples
Name: "{group}\Documentation"; Filename: "{app}\docs"; Components: docs
Name: "{group}\Install VS Code Extension"; Filename: "cmd"; Parameters: "/c code --install-extension ""{app}\vscode-extension\agam-0.1.0.vsix"" & pause"; Components: vscode
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Registry]
; Add to PATH
Root: HKCU; Subkey: "Environment"; ValueType: expandsz; ValueName: "Path"; ValueData: "{olddata};{app}"; Tasks: addtopath; Check: NeedsAddPath(ExpandConstant('{app}'))

; File association for .agam files
Root: HKCU; Subkey: "Software\Classes\.agam"; ValueType: string; ValueName: ""; ValueData: "agamFile"; Tasks: associatefile
Root: HKCU; Subkey: "Software\Classes\agamFile"; ValueType: string; ValueName: ""; ValueData: "agam Source File"; Tasks: associatefile
Root: HKCU; Subkey: "Software\Classes\agamFile\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"; Tasks: associatefile
Root: HKCU; Subkey: "Software\Classes\agamFile\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""; Tasks: associatefile
Root: HKCU; Subkey: "Software\Classes\agamFile\shell\edit"; ValueType: string; ValueName: ""; ValueData: "Edit with VS Code"; Tasks: associatefile
Root: HKCU; Subkey: "Software\Classes\agamFile\shell\edit\command"; ValueType: string; ValueName: ""; ValueData: """code"" ""%1"""; Tasks: associatefile

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch agam REPL"; Flags: nowait postinstall skipifsilent shellexec
Filename: "cmd"; Parameters: "/c code --install-extension ""{app}\vscode-extension\agam-0.1.0.vsix"""; Description: "Install VS Code Extension"; Flags: nowait postinstall skipifsilent runhidden; Components: vscode; Check: IsVSCodeInstalled

[UninstallRun]
Filename: "cmd"; Parameters: "/c code --uninstall-extension agam.agam"; Flags: runhidden; Components: vscode

[Code]
// Check if PATH addition is needed
function NeedsAddPath(Param: string): boolean;
var
  OrigPath: string;
begin
  if not RegQueryStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', OrigPath) then
  begin
    Result := True;
    exit;
  end;
  Result := Pos(';' + Param + ';', ';' + OrigPath + ';') = 0;
end;

// Check if VS Code is installed
function IsVSCodeInstalled(): boolean;
begin
  Result := FileExists(ExpandConstant('{localappdata}\Programs\Microsoft VS Code\Code.exe')) or
            FileExists(ExpandConstant('{pf}\Microsoft VS Code\Code.exe')) or
            FileExists(ExpandConstant('{pf64}\Microsoft VS Code\Code.exe'));
end;

// Remove from PATH on uninstall
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  Path: string;
  AppPath: string;
begin
  if CurUninstallStep = usPostUninstall then
  begin
    if RegQueryStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', Path) then
    begin
      AppPath := ExpandConstant('{app}');
      StringChangeEx(Path, ';' + AppPath, '', True);
      StringChangeEx(Path, AppPath + ';', '', True);
      StringChangeEx(Path, AppPath, '', True);
      RegWriteStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', Path);
    end;
  end;
end;

// Display post-install message
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    MsgBox('அகம் நிறுவல் வெற்றி!' + #13#10 + #13#10 +
           'agam has been installed successfully!' + #13#10 + #13#10 +
           'Try running: agam' + #13#10 +
           'Or run a file: agam examples\hello.agam' + #13#10 + #13#10 +
           'Note: Please restart your terminal for PATH changes to take effect.',
           mbInformation, MB_OK);
  end;
end;
