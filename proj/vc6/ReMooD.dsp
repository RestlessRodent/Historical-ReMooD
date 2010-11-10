# Microsoft Developer Studio Project File - Name="ReMooD" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ReMooD - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ReMooD.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ReMooD.mak" CFG="ReMooD - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ReMooD - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ReMooD - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ReMooD - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\..\sdl" /I "..\..\win32" /I "..\..\src_cl" /I "..\..\src" /D "GAMECLIENT" /U "GAMESERVER" /D "_WIN32" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "ENABLEMULTITHREADING" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib winmm.lib /nologo /subsystem:console /machine:I386 /out:"..\..\bin\remood.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "ReMooD - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "HIGHMEMORYUNICODE" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\..\sdl" /I "..\..\win32" /I "..\..\src" /I "..\..\src_cl" /D "GAMECLIENT" /U "GAMESERVER" /D "_WIN32" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "ENABLEMULTITHREADING" /D "HIGHMEMORYUNICODE" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib winmm.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\..\bin\remood-dbg.exe" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "ReMooD - Win32 Release"
# Name "ReMooD - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src_cl\am_map.c
# End Source File
# Begin Source File

SOURCE=..\..\src\byteptr.c
# End Source File
# Begin Source File

SOURCE=..\..\src\cmp_rle.c
# End Source File
# Begin Source File

SOURCE=..\..\src\command.c
# End Source File
# Begin Source File

SOURCE=..\..\src\console.c
# End Source File
# Begin Source File

SOURCE=..\..\src\d_clisrv.c
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

SOURCE=..\..\src\dehacked.c
# End Source File
# Begin Source File

SOURCE=..\..\isrc\sdl\dosstr.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dstrings.c
# End Source File
# Begin Source File

SOURCE=..\..\isrc\sdl\endtxt.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\f_finale.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\f_wipe.c
# End Source File
# Begin Source File

SOURCE=..\..\src\g_game.c
# End Source File
# Begin Source File

SOURCE=..\..\src\g_input.c
# End Source File
# Begin Source File

SOURCE=..\..\src\g_state.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\hu_stuff.c
# End Source File
# Begin Source File

SOURCE=..\..\isrc\sdl\i_cdmus.c
# End Source File
# Begin Source File

SOURCE=..\..\isrc\sdl\i_main.c
# End Source File
# Begin Source File

SOURCE=..\..\isrc\sdl\i_music.c
# End Source File
# Begin Source File

SOURCE=..\..\isrc\sdl\i_net.c
# End Source File
# Begin Source File

SOURCE=..\..\isrc\sdl\i_sound.c
# End Source File
# Begin Source File

SOURCE=..\..\isrc\sdl\i_system.c
# End Source File
# Begin Source File

SOURCE=..\..\isrc\sdl\i_tcp.c
# End Source File
# Begin Source File

SOURCE=..\..\isrc\sdl\i_thread.c
# End Source File
# Begin Source File

SOURCE=..\..\isrc\sdl\i_video.c
# End Source File
# Begin Source File

SOURCE=..\..\src\info.c
# End Source File
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

SOURCE=..\..\src_cl\m_menu.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\m_menudr.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\m_menufn.c
# End Source File
# Begin Source File

SOURCE=..\..\src\m_misc.c
# End Source File
# Begin Source File

SOURCE=..\..\src\m_random.c
# End Source File
# Begin Source File

SOURCE=..\..\src\m_swap.c
# End Source File
# Begin Source File

SOURCE=..\..\src\md5.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_ceilng.c
# End Source File
# Begin Source File

SOURCE=..\..\src\p_chex.c
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
# Begin Source File

SOURCE=..\..\src_cl\r_bsp.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_data.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_draw.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_main.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_plane.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_segs.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_sky.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_splats.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_things.c
# End Source File
# Begin Source File

SOURCE=..\..\src\rmd_frgl.c
# End Source File
# Begin Source File

SOURCE=..\..\src\rmd_func.c
# End Source File
# Begin Source File

SOURCE=..\..\src\rmd_main.c
# End Source File
# Begin Source File

SOURCE=..\..\src\s_amb.c
# End Source File
# Begin Source File

SOURCE=..\..\src\s_sound.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\sb_bar.c
# End Source File
# Begin Source File

SOURCE=..\..\src\screen.c
# End Source File
# Begin Source File

SOURCE=..\..\src\sounds.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\st_lib.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\st_stuff.c
# End Source File
# Begin Source File

SOURCE=..\..\src\t_comp.c
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
# Begin Source File

SOURCE=..\..\src\tables.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\v_video.c
# End Source File
# Begin Source File

SOURCE=..\..\src\w_wad.c
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\wi_stuff.c
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

SOURCE=..\..\src\byteptr.h
# End Source File
# Begin Source File

SOURCE=..\..\src\cmp_rle.h
# End Source File
# Begin Source File

SOURCE=..\..\src\command.h
# End Source File
# Begin Source File

SOURCE=..\..\src\console.h
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

SOURCE=..\..\src\d_items.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_main.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_net.h
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

SOURCE=..\..\src\d_think.h
# End Source File
# Begin Source File

SOURCE=..\..\src\d_ticcmd.h
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

SOURCE=..\..\isrc\sdl\endtxt.h
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\f_finale.h
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\f_wipe.h
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

SOURCE=..\..\src_cl\hu_stuff.h
# End Source File
# Begin Source File

SOURCE=..\..\src\i_joy.h
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

SOURCE=..\..\src\i_tcp.h
# End Source File
# Begin Source File

SOURCE=..\..\src\i_thread.h
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

SOURCE=..\..\src_cl\m_menu.h
# End Source File
# Begin Source File

SOURCE=..\..\src\m_misc.h
# End Source File
# Begin Source File

SOURCE=..\..\src\m_random.h
# End Source File
# Begin Source File

SOURCE=..\..\src\m_swap.h
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

SOURCE=..\..\src_cl\r_bsp.h
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_data.h
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_defs.h
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_draw.h
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_local.h
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_main.h
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_plane.h
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_segs.h
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_sky.h
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_splats.h
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_state.h
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\r_things.h
# End Source File
# Begin Source File

SOURCE=..\..\src\rmd_func.h
# End Source File
# Begin Source File

SOURCE=..\..\src\rmd_main.h
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

SOURCE=..\..\src_cl\st_lib.h
# End Source File
# Begin Source File

SOURCE=..\..\src_cl\st_stuff.h
# End Source File
# Begin Source File

SOURCE=..\..\src\t_comp.h
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

SOURCE=..\..\src_cl\v_video.h
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
# End Group
# End Target
# End Project
