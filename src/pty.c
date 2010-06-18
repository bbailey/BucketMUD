#if defined(WIN32)
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <stdio.h>
#include <fcntl.h>
#include "merc.h"

int master_pty( char *ptyname )
{
    char *i = NULL;
    char *j = NULL;
    int fd;

    for ( i = "pqr"; *i; i++ )
    {
        for ( j = "0123456789abcdef"; *j; j++ )
        {
            sprintf( ptyname, "/dev/pty%c%c", *i, *j );

            fd = open( ptyname, O_RDWR );

            if ( fd >= 0 )
                return fd;
        }
    }

    return -1;
}

int slave_tty( char *ptyname, char *ttyname )
{
    sprintf( ttyname, "%s", ptyname );

    *( ttyname + 5 ) = 't';

    return open( ttyname, O_RDWR );
}
