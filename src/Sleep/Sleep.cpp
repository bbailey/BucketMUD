// Sleep.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>

int main( int argc, char *argv[] )
{
    if ( argc < 2 )
    {
        printf( "Usage:  Sleep <Time in seconds>\n\r" );
        exit( 1 );
    }

    Sleep( atoi( argv[1] ) * 1000 );
    return 1;
}
