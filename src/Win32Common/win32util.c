// Extra functions for C++ Builder - Zane
#ifndef _CBADDINS
#define _CBADDINS
#if defined(WIN32)

#include <stdio.h>
#include <windows.h>
#include <time.h>
#include "..\merc.h"

#if defined(cbuilder)
extern int RestartTime;
#endif

//Replacement for Unix GetTimeOfDay.  Not as good as it should be though
//Second and Milisecond are derived from different clocks.  Someday
//someone who gives a damn should find a decent Win32 GetTimeOfDay equivelant.
//Second parameter is for compatibility with Unix GetTimeOfDay
int gettimeofday( struct timeval *timey, void *crap )
{
    timey->tv_sec = time( NULL );
    timey->tv_usec = GetTickCount(  ) * 1000;

    return 0;
}

// Replaces missing bzero in C++ Builder
void bzero( char *data, unsigned long length )
{
    unsigned long x = 0;

    if ( !length )
        return;

    do
    {
        data[x] = 0;
        x++;
    }
    while ( x < length );
}

#if defined(cbuilder)
// Update_GUI monitors the GUI for commands and Updates GUI stats.
void update_gui( void )
{
    RestartTime += 250;
}
#endif

#endif
#endif
