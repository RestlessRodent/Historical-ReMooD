; Script generated by the HM NIS Edit Script Wizard.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "ReMooD"
!define PRODUCT_VERSION "0.8b"
!define PRODUCT_PUBLISHER "The ReMooD Team"
!define PRODUCT_WEB_SITE "http://remood.sourceforge.net/"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\remood.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange-uninstall.bmp"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "doc\LICENSE"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\remood-launcher.exe"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\readme.htm"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "ReMooDSetup.exe"
InstallDir "$PROGRAMFILES\ReMooD"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "bin\remood.exe"
  File "bin\remood-launcher.exe"
  CreateDirectory "$SMPROGRAMS\ReMooD"
  CreateShortCut "$SMPROGRAMS\ReMooD\ReMooD.lnk" "$INSTDIR\remood-launcher.exe"
  CreateShortCut "$DESKTOP\ReMooD.lnk" "$INSTDIR\remood-launcher.exe"
  File "bin\remood.wad"
  File "bin\SDL.dll"
  File "doc\scripts.htm"
  File "doc\readme.htm"
  File "doc\editing.htm"
;;; TODO: Enable Doom Builder Configuation stuff in Doom Builder Dir
  File "doc\DB2_ReMooD.cfg"
  File "doc\DB2_ReMooD_RS.cfg"
  File "doc\DB2_ReMooD_RA.cfg"
  File "doc\DB2_ReMooD_LS.cfg"
  File "doc\DB1_ReMooD.cfg"
SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\ReMooD\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\ReMooD\Uninstall.lnk" "$INSTDIR\uninst.exe"
  CreateShortCut "$SMPROGRAMS\ReMooD\Readme.lnk" "$INSTDIR\readme.htm"
  CreateShortCut "$SMPROGRAMS\ReMooD\Scripting.lnk" "$INSTDIR\scripts.htm"
  CreateShortCut "$SMPROGRAMS\ReMooD\Editing.lnk" "$INSTDIR\editing.htm"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\remood.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\remood.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\DB1_ReMooD.cfg"
  Delete "$INSTDIR\DB2_ReMooD_LS.cfg"
  Delete "$INSTDIR\DB2_ReMooD_RA.cfg"
  Delete "$INSTDIR\DB2_ReMooD_RS.cfg"
  Delete "$INSTDIR\DB2_ReMooD.cfg"
  Delete "$INSTDIR\editing.htm"
  Delete "$INSTDIR\readme.htm"
  Delete "$INSTDIR\scripts.htm"
  Delete "$INSTDIR\remood.wad"
  Delete "$INSTDIR\remood.exe"

  Delete "$INSTDIR\remood-launcher.exe"
  Delete "$INSTDIR\SDL.dll"

  Delete "$SMPROGRAMS\ReMooD\Uninstall.lnk"
  Delete "$SMPROGRAMS\ReMooD\Website.lnk"
  Delete "$SMPROGRAMS\ReMooD\Readme.lnk"
  Delete "$SMPROGRAMS\ReMooD\Scripting.lnk"
  Delete "$SMPROGRAMS\ReMooD\Editing.lnk"
  Delete "$DESKTOP\ReMooD.lnk"
  Delete "$SMPROGRAMS\ReMooD\ReMooD.lnk"

  RMDir "$SMPROGRAMS\ReMooD"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose false
SectionEnd
