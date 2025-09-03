; Granted by hand
Name "VioletClient"

OutFile "VioletClient-Win64-Setup.exe"
RequestExecutionLevel user
Unicode True
InstallDir $PROGRAMFILES64\VioletClient
RequestExecutionLevel admin

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section ""
  SetOutPath $INSTDIR
  File /r "installer\*"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\VioletClient"
  CreateShortCut "$SMPROGRAMS\VioletClient\VioletClient.lnk" "$INSTDIR\VioletClient.exe"
  CreateShortCut "$SMPROGRAMS\VioletClient\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd
  
Section "Uninstall"
  Delete "$SMPROGRAMS\VioletClient\VioletClient.lnk"
  Delete "$SMPROGRAMS\VioletClient\Uninstall.lnk"
  RMDir "$SMPROGRAMS\VioletClient"
  RMDir /r "$INSTDIR"
  SetAutoClose true
SectionEnd
