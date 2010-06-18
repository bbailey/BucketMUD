# Microsoft Developer Studio Project File - Name="Ember" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=Ember - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Ember.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Ember.mak" CFG="Ember - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Ember - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Ember - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Ember - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ws2_32.lib /nologo /subsystem:console /machine:I386 /out:"../Ember.exe"
# SUBTRACT LINK32 /profile /map

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /profile /map /debug /machine:I386 /out:"..\Ember.exe"

!ENDIF 

# Begin Target

# Name "Ember - Win32 Release"
# Name "Ember - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\act_comm.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\act_info.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\act_move.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\act_obj.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\act_wiz.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\auction.c
# End Source File
# Begin Source File

SOURCE=..\ban.c
# End Source File
# Begin Source File

SOURCE=..\bank.c
# End Source File
# Begin Source File

SOURCE=..\bit.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\board.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\clan.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\class.c
# End Source File
# Begin Source File

SOURCE=..\comm.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\const.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Win32Common\crypt_util.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\db.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\drunk2.c
# End Source File
# Begin Source File

SOURCE=..\factions.c
# End Source File
# Begin Source File

SOURCE=..\fight.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\handler.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\helpolc.c
# End Source File
# Begin Source File

SOURCE=..\interp.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\magic.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\marry.c
# End Source File
# Begin Source File

SOURCE=..\mem.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\mprog_commands.c
# End Source File
# Begin Source File

SOURCE=..\mprog_procs.c
# End Source File
# Begin Source File

SOURCE=..\mud_progs.c
# End Source File
# Begin Source File

SOURCE=..\newbits.c
# End Source File
# Begin Source File

SOURCE=..\olc.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\olc_act.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\olc_save.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\random.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\recycle.c
# End Source File
# Begin Source File

SOURCE=..\save.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\skills.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\socialolc.c
# End Source File
# Begin Source File

SOURCE=..\ssm.c
# End Source File
# Begin Source File

SOURCE=..\string.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\todoolc.c
# End Source File
# Begin Source File

SOURCE=..\update.c

!IF  "$(CFG)" == "Ember - Win32 Release"

!ELSEIF  "$(CFG)" == "Ember - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Win32Common\win32util.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\board.h
# End Source File
# Begin Source File

SOURCE=..\config.h
# End Source File
# Begin Source File

SOURCE=..\db.h
# End Source File
# Begin Source File

SOURCE=..\factions.h
# End Source File
# Begin Source File

SOURCE=..\include.h
# End Source File
# Begin Source File

SOURCE=..\interp.h
# End Source File
# Begin Source File

SOURCE=..\magic.h
# End Source File
# Begin Source File

SOURCE=..\merc.h
# End Source File
# Begin Source File

SOURCE=..\mud_progs.h
# End Source File
# Begin Source File

SOURCE=..\newbits.h
# End Source File
# Begin Source File

SOURCE=..\olc.h
# End Source File
# Begin Source File

SOURCE=..\Win32Common\patchlevel.h
# End Source File
# Begin Source File

SOURCE=..\recycle.h
# End Source File
# Begin Source File

SOURCE=..\Win32Common\telnet.h
# End Source File
# Begin Source File

SOURCE="..\Win32Common\ufc-crypt.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
