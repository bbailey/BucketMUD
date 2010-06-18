#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>

#define emberver "EmberMUD v0.9.44a"
#define defport 9000
#define logdir "..\\log"
#define defindex 1000
#define maxindex 2000

void main( int argc, char *argv[] )
{
    FILE *fp;
    char logfile[1024];
    char buf[1024];
    int port = defport;
    int index = defindex - 1;
    int loop;

    memset( logfile, '\0', sizeof( logfile ) );

    if ( argc > 1 )
    {
        port = atoi( argv[1] );
    }

    // Make sure ebmer.exe exists.
    fp = fopen( "ember.exe", "r" );
    if ( fp == NULL )
    {
        fprintf( stdout, "Could not open ember.exe!" );
        exit( 1 );
    }

    fclose( fp );
    fp = NULL;

    // Remove any stale shutdown.txt files that are found.
    fp = fopen( "shutdown.txt", "r" );
    if ( fp != NULL )
    {
        fclose( fp );
        fp = NULL;
        system( "del shutdown.txt" );
    }

    // Keep re-starting the MUD until a shutdown.txt is found.
    while ( fp == NULL )
    {
        loop = 1;
        while ( loop )
        {
            index++;
            if ( index > maxindex )
            {
                fprintf( stdout,
                         "Could not start %s.  Max logfile index reached (%d.log)",
                         ( char * ) emberver, index - 1 );
                exit( 1 );
            }

            sprintf( logfile, "%s\\%d.log", ( char * ) logdir, index );
            fp = fopen( logfile, "r" );
            if ( fp == NULL )
            {
                loop = 0;
                fp = fopen( logfile, "w" );
                if ( fp == NULL )
                {
                    fprintf( stdout, "Could not write to logfile: %s",
                             logfile );
                    exit( 1 );
                }
                else
                {
                    fclose( fp );
                    fp = NULL;
                }
            }
            else
            {
                fclose( fp );
                fp = NULL;
            }
        }

        fprintf( stdout, "\nOpening logfile: %s\n\n", logfile );
        fprintf( stdout, "%s is rockin' on port %d...\n", ( char * ) emberver,
                 port );

        sprintf( buf, "..\\src\\ember %d > %s", port, logfile );
        system( buf );

        fp = fopen( "shutdown.txt", "r" );
        if ( fp == NULL )
            fprintf( stdout, "\nREBOOT!\n" );
    }

    fprintf( stdout, "\nSHUTDOWN!\n" );
    fclose( fp );
}
