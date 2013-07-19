# Microsoft Developer Studio Project File - Name="SDLxVC6" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Xbox Static Library" 0x0b04

CFG=SDLxVC6 - Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SDLxVC6.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SDLxVC6.mak" CFG="SDLxVC6 - Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SDLxVC6 - Xbox Release" (based on "Xbox Static Library")
!MESSAGE "SDLxVC6 - Xbox Debug" (based on "Xbox Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "SDLxVC6 - Xbox Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /GX /O2 /I "SDL\include" /I "smpeg" /I "timidity" /I "mikmod" /I "vorbis\include" /I "SDL" /I "SDL\src\thread\xbox" /I "SDL\src\thread" /I "SDL\src\audio" /I "SDL\src\timer" /I "SDL\src\events" /I "SDL\src\video" /I "SDL\src\cdrom" /I "SDL\src\joystick" /D "NDEBUG" /D "_XBOX" /D "_LIB" /D "_WINDOWS" /D "WIN32" /D "WAV_MUSIC" /D "MOD_MUSIC" /D "MID_MUSIC" /D "USE_TIMIDITY_MIDI" /D "ENABLE_DIRECTX" /D "_XBOX_DONT_USE_DEVICES" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"SDLx.lib"

!ELSEIF  "$(CFG)" == "SDLxVC6 - Xbox Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "SDL\include" /I "smpeg" /I "timidity" /I "mikmod" /I "vorbis\include" /I "SDL" /I "SDL\src\thread\xbox" /I "SDL\src\thread" /I "SDL\src\audio" /I "SDL\src\timer" /I "SDL\src\events" /I "SDL\src\video" /I "SDL\src\cdrom" /I "SDL\src\joystick" /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"SDLxd.lib"

!ENDIF 

# Begin Target

# Name "SDLxVC6 - Xbox Release"
# Name "SDLxVC6 - Xbox Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "audio"

# PROP Default_Filter ""
# Begin Group "xbox"

# PROP Default_Filter ""
# Begin Source File

SOURCE=SDL\src\audio\xbox\SDL_xboxaudio.c
# End Source File
# End Group
# Begin Source File

SOURCE=SDL\src\audio\SDL_audio.c
# End Source File
# Begin Source File

SOURCE=SDL\src\audio\SDL_audiocvt.c
# End Source File
# Begin Source File

SOURCE=SDL\src\audio\SDL_audiomem.c
# End Source File
# Begin Source File

SOURCE=SDL\src\audio\SDL_mixer.c
# End Source File
# Begin Source File

SOURCE=SDL\src\audio\SDL_wave.c
# End Source File
# End Group
# Begin Group "thread"

# PROP Default_Filter ""
# Begin Group "xbox No. 1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=SDL\src\thread\xbox\SDL_sysmutex.c
# End Source File
# Begin Source File

SOURCE=SDL\src\thread\xbox\SDL_syssem.c
# End Source File
# Begin Source File

SOURCE=SDL\src\thread\xbox\SDL_systhread.c
# End Source File
# End Group
# Begin Source File

SOURCE=SDL\src\thread\SDL_thread.c
# End Source File
# End Group
# Begin Group "timer"

# PROP Default_Filter ""
# Begin Group "xbox No. 2"

# PROP Default_Filter ""
# Begin Source File

SOURCE=SDL\src\timer\xbox\SDL_systimer.c
# End Source File
# End Group
# Begin Source File

SOURCE=SDL\src\timer\SDL_timer.c
# End Source File
# End Group
# Begin Group "file"

# PROP Default_Filter ""
# Begin Source File

SOURCE=SDL\src\file\SDL_rwops.c
# End Source File
# End Group
# Begin Group "endian"

# PROP Default_Filter ""
# Begin Source File

SOURCE=SDL\src\endian\SDL_endian.c
# End Source File
# End Group
# Begin Group "video"

# PROP Default_Filter ""
# Begin Group "xbox No. 3"

# PROP Default_Filter ""
# Begin Source File

SOURCE=SDL\src\video\xbox\SDL_xboxevents.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\xbox\SDL_xboxmouse.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\xbox\SDL_xboxvideo.c
# End Source File
# End Group
# Begin Source File

SOURCE=SDL\src\video\SDL_blit.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_blit_0.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_blit_1.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_blit_A.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_blit_N.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_bmp.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_cursor.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_gamma.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_pixels.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_RLEaccel.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_stretch.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_surface.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_video.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_yuv.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_yuv_mmx.c
# End Source File
# Begin Source File

SOURCE=SDL\src\video\SDL_yuv_sw.c
# End Source File
# End Group
# Begin Group "events"

# PROP Default_Filter ""
# Begin Source File

SOURCE=SDL\src\events\SDL_active.c
# End Source File
# Begin Source File

SOURCE=SDL\src\events\SDL_events.c
# End Source File
# Begin Source File

SOURCE=SDL\src\events\SDL_expose.c
# End Source File
# Begin Source File

SOURCE=SDL\src\events\SDL_keyboard.c
# End Source File
# Begin Source File

SOURCE=SDL\src\events\SDL_mouse.c
# End Source File
# Begin Source File

SOURCE=SDL\src\events\SDL_quit.c
# End Source File
# Begin Source File

SOURCE=SDL\src\events\SDL_resize.c
# End Source File
# End Group
# Begin Group "joystick"

# PROP Default_Filter ""
# Begin Group "xbox No. 4"

# PROP Default_Filter ""
# Begin Source File

SOURCE=SDL\src\joystick\xbox\SDL_sysjoystick.c
# End Source File
# End Group
# Begin Source File

SOURCE=SDL\src\joystick\SDL_joystick.c
# End Source File
# End Group
# Begin Group "cdrom"

# PROP Default_Filter ""
# Begin Group "xbox No. 5"

# PROP Default_Filter ""
# Begin Source File

SOURCE=SDL\src\cdrom\xbox\SDL_syscdrom.c
# End Source File
# End Group
# Begin Source File

SOURCE=SDL\src\cdrom\SDL_cdrom.c
# End Source File
# End Group
# Begin Source File

SOURCE=SDL\SDL.c
# End Source File
# Begin Source File

SOURCE=SDL\SDL_error.c
# End Source File
# Begin Source File

SOURCE=SDL\SDL_fatal.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
