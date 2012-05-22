# Microsoft Developer Studio Project File - Name="ReMooD" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ReMooD - Win32 Debug SDL
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ReMooD.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ReMooD.mak" CFG="ReMooD - Win32 Debug SDL"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ReMooD - Win32 Debug Allegro" (based on "Win32 (x86) Application")
!MESSAGE "ReMooD - Win32 Release Allegro" (based on "Win32 (x86) Application")
!MESSAGE "ReMooD - Win32 Debug SDL" (based on "Win32 (x86) Application")
!MESSAGE "ReMooD - Win32 Release SDL" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ReMooD - Win32 Debug Allegro"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ReMooD___Win32_Debug_Allegro"
# PROP BASE Intermediate_Dir "ReMooD___Win32_Debug_Allegro"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ReMooD___Win32_Debug_Allegro"
# PROP Intermediate_Dir "ReMooD___Win32_Debug_Allegro"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\sdl" /I "..\..\win32" /I "..\..\src" /I "..\..\src_cl" /D "GAMECLIENT" /D "_WIN32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "ENABLEMULTITHREADING" /D "HIGHMEMORYUNICODE" /D "__REMOOD_SYSTEM_WINDOWS" /U "GAMESERVER" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\sdl" /I "..\..\win32" /I "..\..\src" /I "..\..\src_cl" /D "GAMECLIENT" /D "_WIN32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "ENABLEMULTITHREADING" /D "HIGHMEMORYUNICODE" /D "__REMOOD_SYSTEM_WINDOWS" /U "GAMESERVER" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib winmm.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\..\bin\remood-dbg.exe" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib alleg.lib winmm.lib ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"..\..\bin\remood-dbg.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
ProjDir=.
TargetDir=\cygwin\home\Steven\ReMooD\bin
InputPath=\cygwin\home\Steven\ReMooD\bin\remood-dbg.exe
SOURCE="$(InputPath)"

"$(TargetDir)\remood.wad" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(TargetDir)\rmdtex.exe  $(ProjDir)\..\..\wad\wadinfo.txt  $(TargetDir)\remood.wad  $(ProjDir)\..\..\wad\ 

# End Custom Build

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release Allegro"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReMooD___Win32_Release_Allegro"
# PROP BASE Intermediate_Dir "ReMooD___Win32_Release_Allegro"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReMooD___Win32_Release_Allegro"
# PROP Intermediate_Dir "ReMooD___Win32_Release_Allegro"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\sdl" /I "..\..\win32" /I "..\..\src_cl" /I "..\..\src" /D "GAMECLIENT" /D "_WIN32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "ENABLEMULTITHREADING" /D "__REMOOD_SYSTEM_WINDOWS" /U "GAMESERVER" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\sdl" /I "..\..\win32" /I "..\..\src_cl" /I "..\..\src" /D "GAMECLIENT" /D "_WIN32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "ENABLEMULTITHREADING" /D "__REMOOD_SYSTEM_WINDOWS" /U "GAMESERVER" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib winmm.lib /nologo /subsystem:console /machine:I386 /out:"..\..\bin\remood.exe"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib alleg.lib winmm.lib ws2_32.lib /nologo /subsystem:windows /machine:I386 /out:"..\..\bin\remood.exe"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
ProjDir=.
TargetDir=\cygwin\home\Steven\ReMooD\bin
InputPath=\cygwin\home\Steven\ReMooD\bin\remood.exe
SOURCE="$(InputPath)"

"$(TargetDir)\remood.wad" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(TargetDir)\rmdtex.exe  $(ProjDir)\..\..\wad\wadinfo.txt  $(TargetDir)\remood.wad  $(ProjDir)\..\..\wad\ 

# End Custom Build

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Debug SDL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "ReMooD___Win32_Debug_SDL"
# PROP BASE Intermediate_Dir "ReMooD___Win32_Debug_SDL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "ReMooD___Win32_Debug_SDL"
# PROP Intermediate_Dir "ReMooD___Win32_Debug_SDL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\sdl" /I "..\..\win32" /I "..\..\src" /I "..\..\src_cl" /D "GAMECLIENT" /D "_WIN32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "ENABLEMULTITHREADING" /D "HIGHMEMORYUNICODE" /D "__REMOOD_SYSTEM_WINDOWS" /U "GAMESERVER" /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\sdl" /I "..\..\win32" /I "..\..\src" /I "..\..\src_cl" /D "GAMECLIENT" /D "_WIN32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "ENABLEMULTITHREADING" /D "HIGHMEMORYUNICODE" /D "__REMOOD_SYSTEM_WINDOWS" /U "GAMESERVER" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib winmm.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\..\bin\remood-dbg.exe" /pdbtype:sept
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib winmm.lib ws2_32.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\..\bin\remood-dbg.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
ProjDir=.
TargetDir=\cygwin\home\Steven\ReMooD\bin
InputPath=\cygwin\home\Steven\ReMooD\bin\remood-dbg.exe
SOURCE="$(InputPath)"

"$(TargetDir)\remood.wad" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(TargetDir)\rmdtex.exe  $(ProjDir)\..\..\wad\wadinfo.txt  $(TargetDir)\remood.wad  $(ProjDir)\..\..\wad\ 

# End Custom Build

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release SDL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReMooD___Win32_Release_SDL"
# PROP BASE Intermediate_Dir "ReMooD___Win32_Release_SDL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReMooD___Win32_Release_SDL"
# PROP Intermediate_Dir "ReMooD___Win32_Release_SDL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /I "..\..\sdl" /I "..\..\win32" /I "..\..\src_cl" /I "..\..\src" /D "GAMECLIENT" /D "_WIN32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "ENABLEMULTITHREADING" /D "__REMOOD_SYSTEM_WINDOWS" /U "GAMESERVER" /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\sdl" /I "..\..\win32" /I "..\..\src_cl" /I "..\..\src" /D "GAMECLIENT" /D "_WIN32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "ENABLEMULTITHREADING" /D "__REMOOD_SYSTEM_WINDOWS" /U "GAMESERVER" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib winmm.lib /nologo /subsystem:console /machine:I386 /out:"..\..\bin\remood.exe"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDLmain.lib winmm.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"..\..\bin\remood.exe"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
ProjDir=.
TargetDir=\cygwin\home\Steven\ReMooD\bin
InputPath=\cygwin\home\Steven\ReMooD\bin\remood.exe
SOURCE="$(InputPath)"

"$(TargetDir)\remood.wad" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(TargetDir)\rmdtex.exe  $(ProjDir)\..\..\wad\wadinfo.txt  $(TargetDir)\remood.wad  $(ProjDir)\..\..\wad\ 

# End Custom Build

!ENDIF 

# Begin Target

# Name "ReMooD - Win32 Debug Allegro"
# Name "ReMooD - Win32 Release Allegro"
# Name "ReMooD - Win32 Debug SDL"
# Name "ReMooD - Win32 Release SDL"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "am_"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\am_map.c
# End Source File
# End Group
# Begin Group "c_"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\c_lib.c
# End Source File
# End Group
# Begin Group "d_"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\d_block.c
# End Source File
# Begin Source File

SOURCE=..\..\src\d_clisrv.c
# End Source File
# Begin Source File

SOURCE=..\..\src\d_info.c
# End Source File
# Begin Source File

SOURCE=..\..\src\d_items.c
# End Source File
# Begin Source File

SOURCE=..\..\src\d_main.c
# End Source File
# Begin Source File

SOURCE=..\..\src\d_net.c
# End Source File
# Begin Source File

SOURCE=..\..\src\d_netcmd.c
# End Source File
# Begin Source File

SOURCE=..\..\src\d_prof.c
# End Source File
# Begin Source File

SOURCE=..\..\src\d_rdf.c
# End Source File
# Begin Source File

SOURCE=..\..\src\d_rmod.c
# End Source File
# End Group
# Begin Group "f_"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\f_finale.c
# End Source File
# Begin Source File

SOURCE=..\..\src\f_wipe.c
# End Source File
# End Group
# Begin Group "g_"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\g_game.c
# End Source File
# Begin Source File

SOURCE=..\..\src\g_input.c
# End Source File
# Begin Source File

SOURCE=..\..\src\g_state.c
# End Source File
# End Group
# Begin Group "hu_"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\hu_stuff.c
# End Source File
# End Group
# Begin Group "i_"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\i_util.c
# End Source File
# Begin Source File

SOURCE=..\..\src\i_utlnet.c
# End Source File
# Begin Source File

SOURCE=..\..\src\i_utlsfx.c
# End Source File
# End Group
# Begin Group "m_"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\m_argv.c
# End Source File
# Begin Source File

SOURCE=..\..\src\m_bbox.c
# End Source File
# Begin Source File

SOURCE=..\..\src\m_cheat.c
# End Source File
# Begin Source File

SOURCE=..\..\src\m_fixed.c
# End Source File
# Begin Source File

SOURCE=..\..\src\m_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\src\m_menudr.c
# End Source File
# Begin Source File

SOURCE=..\..\src\m_menufn.c
# End Source File
# Begin Source File

SOURCE=..\..\src\m_misc.c
# End Source File
# Begin Source File

SOURCE=..\..\src\m_random.c
# End Source File
# End Group
# Begin Group "p_"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\p_ceilng.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_demcmp.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_doors.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_enemy.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_fab.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_floor.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_genlin.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_info.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_inter.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_lights.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_map.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_maputl.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_mobj.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_plats.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_pspr.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_saveg.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_setup.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_sight.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_spec.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_switch.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_telept.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_tick.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_user.c
# End Source File
# End Group
# Begin Group "r_"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\r_bsp.c
# End Source File
# Begin Source File

SOURCE=..\..\src\r_data.c
# End Source File
# Begin Source File

SOURCE=..\..\src\r_draw.c
# End Source File
# Begin Source File

SOURCE=..\..\src\r_main.c
# End Source File
# Begin Source File

SOURCE=..\..\src\r_plane.c
# End Source File
# Begin Source File

SOURCE=..\..\src\r_segs.c
# End Source File
# Begin Source File

SOURCE=..\..\src\r_sky.c
# End Source File
# Begin Source File

SOURCE=..\..\src\r_splats.c
# End Source File
# Begin Source File

SOURCE=..\..\src\r_things.c
# End Source File
# End Group
# Begin Group "st_"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\st_lib.c
# End Source File
# Begin Source File

SOURCE=..\..\src\st_stuff.c
# End Source File
# End Group
# Begin Group "t_"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\t_comp.c
# End Source File
# Begin Source File

SOURCE=..\..\src\t_dscc.c
# End Source File
# Begin Source File

SOURCE=..\..\src\t_dsvm.c
# End Source File
# Begin Source File

SOURCE=..\..\src\t_func.c
# End Source File
# Begin Source File

SOURCE=..\..\src\t_oper.c
# End Source File
# Begin Source File

SOURCE=..\..\src\t_parse.c
# End Source File
# Begin Source File

SOURCE=..\..\src\t_prepro.c
# End Source File
# Begin Source File

SOURCE=..\..\src\t_script.c
# End Source File
# Begin Source File

SOURCE=..\..\src\t_spec.c
# End Source File
# Begin Source File

SOURCE=..\..\src\t_vari.c
# End Source File
# End Group
# Begin Group "SDL Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\sdl\i_main.c

!IF  "$(CFG)" == "ReMooD - Win32 Debug Allegro"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release Allegro"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Debug SDL"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release SDL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\sdl\i_music.c

!IF  "$(CFG)" == "ReMooD - Win32 Debug Allegro"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release Allegro"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Debug SDL"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release SDL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\sdl\i_net.c

!IF  "$(CFG)" == "ReMooD - Win32 Debug Allegro"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release Allegro"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Debug SDL"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release SDL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\sdl\i_sound.c

!IF  "$(CFG)" == "ReMooD - Win32 Debug Allegro"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release Allegro"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Debug SDL"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release SDL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\sdl\i_system.c

!IF  "$(CFG)" == "ReMooD - Win32 Debug Allegro"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release Allegro"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Debug SDL"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release SDL"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\sdl\i_video.c

!IF  "$(CFG)" == "ReMooD - Win32 Debug Allegro"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release Allegro"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Debug SDL"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release SDL"

!ENDIF 

# End Source File
# End Group
# Begin Group "Allegro Interface"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\allegro\i_main.c

!IF  "$(CFG)" == "ReMooD - Win32 Debug Allegro"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release Allegro"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Debug SDL"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release SDL"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\allegro\i_music.c

!IF  "$(CFG)" == "ReMooD - Win32 Debug Allegro"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release Allegro"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Debug SDL"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release SDL"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\allegro\i_net.c

!IF  "$(CFG)" == "ReMooD - Win32 Debug Allegro"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release Allegro"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Debug SDL"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release SDL"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\allegro\i_sound.c

!IF  "$(CFG)" == "ReMooD - Win32 Debug Allegro"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release Allegro"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Debug SDL"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release SDL"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\allegro\i_system.c

!IF  "$(CFG)" == "ReMooD - Win32 Debug Allegro"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release Allegro"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Debug SDL"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release SDL"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\allegro\i_video.c

!IF  "$(CFG)" == "ReMooD - Win32 Debug Allegro"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release Allegro"

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Debug SDL"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Release SDL"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "b_"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\b_bot.c
# End Source File
# Begin Source File

SOURCE=..\..\src\b_ghost.c
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\src\command.c
# End Source File
# Begin Source File

SOURCE=..\..\src\console.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dbopl.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dehacked.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dstrings.c
# End Source File
# Begin Source File

SOURCE=..\..\src\info.c
# End Source File
# Begin Source File

SOURCE=..\..\src\md5.c
# End Source File
# Begin Source File

SOURCE=..\..\src\s_sound.c
# End Source File
# Begin Source File

SOURCE=..\..\src\screen.c
# End Source File
# Begin Source File

SOURCE=..\..\src\sounds.c
# End Source File
# Begin Source File

SOURCE=..\..\src\tables.c
# End Source File
# Begin Source File

SOURCE=..\..\src\v_video.c
# End Source File
# Begin Source File

SOURCE=..\..\src\v_widget.c
# End Source File
# Begin Source File

SOURCE=..\..\src\w_wad.c
# End Source File
# Begin Source File

SOURCE=..\..\src\wi_stuff.c
# End Source File
# Begin Source File

SOURCE=..\..\src\z_zone.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\am_map.h
# End Source File
# Begin Source File

SOURCE=..\..\src\b_bot.h
# End Source File
# Begin Source File

SOURCE=..\..\src\bootdata.h
# End Source File
# Begin Source File

SOURCE=..\..\src\c_lib.h
# End Source File
# Begin Source File

SOURCE=..\..\src\command.h
# End Source File
# Begin Source File

SOURCE=..\..\src\console.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_block.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_clisrv.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_englsh.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_event.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_french.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_info.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_items.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_main.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_net.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_netcmd.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_player.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_prof.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_rdf.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_rmod.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_think.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_ticcmd.h
# End Source File
# Begin Source File

SOURCE=..\..\src\dbopl.h
# End Source File
# Begin Source File

SOURCE=..\..\src\dehacked.h
# End Source File
# Begin Source File

SOURCE=..\..\src\doomdata.h
# End Source File
# Begin Source File

SOURCE=..\..\src\doomdef.h
# End Source File
# Begin Source File

SOURCE=..\..\src\doomstat.h
# End Source File
# Begin Source File

SOURCE=..\..\src\doomtype.h
# End Source File
# Begin Source File

SOURCE=..\..\src\dstrings.h
# End Source File
# Begin Source File

SOURCE=..\..\src\f_finale.h
# End Source File
# Begin Source File

SOURCE=..\..\src\f_wipe.h
# End Source File
# Begin Source File

SOURCE=..\..\src\g_game.h
# End Source File
# Begin Source File

SOURCE=..\..\src\g_input.h
# End Source File
# Begin Source File

SOURCE=..\..\src\g_state.h
# End Source File
# Begin Source File

SOURCE=..\..\src\hu_stuff.h
# End Source File
# Begin Source File

SOURCE=..\..\src\i_net.h
# End Source File
# Begin Source File

SOURCE=..\..\src\i_sound.h
# End Source File
# Begin Source File

SOURCE=..\..\src\i_system.h
# End Source File
# Begin Source File

SOURCE=..\..\src\i_util.h
# End Source File
# Begin Source File

SOURCE=..\..\src\i_video.h
# End Source File
# Begin Source File

SOURCE=..\..\src\info.h
# End Source File
# Begin Source File

SOURCE=..\..\src\keys.h
# End Source File
# Begin Source File

SOURCE=..\..\src\m_argv.h
# End Source File
# Begin Source File

SOURCE=..\..\src\m_bbox.h
# End Source File
# Begin Source File

SOURCE=..\..\src\m_cheat.h
# End Source File
# Begin Source File

SOURCE=..\..\src\m_fixed.h
# End Source File
# Begin Source File

SOURCE=..\..\src\m_menu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\m_misc.h
# End Source File
# Begin Source File

SOURCE=..\..\src\m_random.h
# End Source File
# Begin Source File

SOURCE=..\..\src\md5.h
# End Source File
# Begin Source File

SOURCE=..\..\src\p_chex.h
# End Source File
# Begin Source File

SOURCE=..\..\src\p_demcmp.h
# End Source File
# Begin Source File

SOURCE=..\..\src\p_fab.h
# End Source File
# Begin Source File

SOURCE=..\..\src\p_info.h
# End Source File
# Begin Source File

SOURCE=..\..\src\p_inter.h
# End Source File
# Begin Source File

SOURCE=..\..\src\p_local.h
# End Source File
# Begin Source File

SOURCE=..\..\src\p_maputl.h
# End Source File
# Begin Source File

SOURCE=..\..\src\p_mobj.h
# End Source File
# Begin Source File

SOURCE=..\..\src\p_pspr.h
# End Source File
# Begin Source File

SOURCE=..\..\src\p_saveg.h
# End Source File
# Begin Source File

SOURCE=..\..\src\p_setup.h
# End Source File
# Begin Source File

SOURCE=..\..\src\p_spec.h
# End Source File
# Begin Source File

SOURCE=..\..\src\p_tick.h
# End Source File
# Begin Source File

SOURCE=..\..\src\r_bsp.h
# End Source File
# Begin Source File

SOURCE=..\..\src\r_data.h
# End Source File
# Begin Source File

SOURCE=..\..\src\r_defs.h
# End Source File
# Begin Source File

SOURCE=..\..\src\r_draw.h
# End Source File
# Begin Source File

SOURCE=..\..\src\r_local.h
# End Source File
# Begin Source File

SOURCE=..\..\src\r_main.h
# End Source File
# Begin Source File

SOURCE=..\..\src\r_plane.h
# End Source File
# Begin Source File

SOURCE=..\..\src\r_segs.h
# End Source File
# Begin Source File

SOURCE=..\..\src\r_sky.h
# End Source File
# Begin Source File

SOURCE=..\..\src\r_splats.h
# End Source File
# Begin Source File

SOURCE=..\..\src\r_state.h
# End Source File
# Begin Source File

SOURCE=..\..\src\r_things.h
# End Source File
# Begin Source File

SOURCE=..\..\src\s_sound.h
# End Source File
# Begin Source File

SOURCE=..\..\src\screen.h
# End Source File
# Begin Source File

SOURCE=..\..\src\sounds.h
# End Source File
# Begin Source File

SOURCE=..\..\src\st_lib.h
# End Source File
# Begin Source File

SOURCE=..\..\src\st_stuff.h
# End Source File
# Begin Source File

SOURCE=..\..\src\t_comp.h
# End Source File
# Begin Source File

SOURCE=..\..\src\t_dsvm.h
# End Source File
# Begin Source File

SOURCE=..\..\src\t_func.h
# End Source File
# Begin Source File

SOURCE=..\..\src\t_oper.h
# End Source File
# Begin Source File

SOURCE=..\..\src\t_parse.h
# End Source File
# Begin Source File

SOURCE=..\..\src\t_prepro.h
# End Source File
# Begin Source File

SOURCE=..\..\src\t_script.h
# End Source File
# Begin Source File

SOURCE=..\..\src\t_spec.h
# End Source File
# Begin Source File

SOURCE=..\..\src\t_vari.h
# End Source File
# Begin Source File

SOURCE=..\..\src\tables.h
# End Source File
# Begin Source File

SOURCE=..\..\src\v_video.h
# End Source File
# Begin Source File

SOURCE=..\..\src\v_widc.h
# End Source File
# Begin Source File

SOURCE=..\..\src\v_widget.h
# End Source File
# Begin Source File

SOURCE=..\..\src\w_wad.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wi_stuff.h
# End Source File
# Begin Source File

SOURCE=..\..\src\z_zone.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\rc\remood.rc
# End Source File
# Begin Source File

SOURCE=..\..\rc\remood6.ico
# End Source File
# Begin Source File

SOURCE=..\..\rc\remoods.ico
# End Source File
# End Group
# End Target
# End Project
