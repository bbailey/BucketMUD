/* Code graciously borrowed and converted from MUD++ */
/*
 * Pseudo-terminals - Whheee!
 * This code is not very obvious.
 * It is concerned with handled pseudo-terminals (pty for short)
 * To run terminal emulation across a standard network descriptor
 * we open a master pty, fork a slave which opens the slave side of the pty,
 * maps the pty onto stdin/sdtdout/stderr and then execs the shell process.
 * Code for BSD is completely different from SYSV so please report problems.
 * Recommended reading:
 *    -Stevens' UNIX Network Programming
 *    -Berkeley Software Distribution source code (telnetd and rlogind) 
*/

#include <fcntl.h>
#include <termio.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/file.h>
#include <arpa/telnet.h>

int route_io( int, int );

/*
 * Mini-telnetd
 * This is not meant to be an attempt to implement telnetd
 * Berkeley telnetd is about 7000 lines of code. A lot of it is
 * conditional switches for various non-standard OS. I just want
 * to implement enough to be functional. With time it will probably
 * grow as reports come in from OS that need different stuff.
 */

#define ST_DATA	   0
#define ST_IAC	   1
#define ST_SE	   2
#define ST_OPT	   3
#define ST_HOW	   4
#define ST_WILL	   5
#define ST_WONT	   6
#define ST_DO	   7
#define ST_DONT	   8
#define ST_CRLF	   9
#define ST_IP	   10

int route_io( int fd1, int fd2 )
{
    struct timeval sleep_time;
    fd_set r_set, w_set, o_set;
    int state = ST_DATA;
    char netbuf[1024];
    char netbuf2[1024];
    char ptybuf[1024];
    char ptybuf2[1024 * 2];
    int fd1eofd = 0;
    int fd2eofd = 0;
    int netbytes = 0;
    int ptybytes = 0;
    int maxdesc = fd1;

    if ( maxdesc < fd2 )
        maxdesc = fd2;

    FD_ZERO( &r_set );
    FD_ZERO( &w_set );
    FD_ZERO( &o_set );

    for ( ;; )
    {
        sleep_time.tv_sec = 1;
        sleep_time.tv_usec = 0;

        FD_SET( fd1, &r_set );
        FD_SET( fd1, &w_set );
        FD_SET( fd1, &o_set );
        FD_SET( fd2, &r_set );
        FD_SET( fd2, &w_set );

        /*
         * First sleep until data to be read. Then poll the writeable
         * descriptors too. This is best for conserving CPU and gives
         * the best interactive performance from my tests. Of course
         * data only gets written on 'read' pulses but testing hasn't
         * shown this to be a problem.
         * Don't bother checking error codes here.
         */
        select( maxdesc + 1, &r_set, 0, 0, &sleep_time );

        /* Now do the write poll which almost always returns ready. */
        sleep_time.tv_sec = 1;
        sleep_time.tv_usec = 0;

        while ( select( maxdesc + 1, 0, &w_set, &o_set, &sleep_time ) < 0 )
        {
            if ( errno == EINTR )
                continue;
            perror( "route_io:select" );
            return -1;
        }

        if ( FD_ISSET( fd1, &o_set ) )
        {
            perror( "Got OOB data.\n" );
            // add code to deal with it soon
        }

        if ( FD_ISSET( fd1, &r_set ) && netbytes == 0 )
        {
            int bytes = 0;
            int error;

            while ( 1 )
            {
                if ( ( error =
                       read( fd1, netbuf + bytes, sizeof( netbuf ) ) ) > 0 )
                    bytes += error;
                else if ( !error )
                {
                    fd1eofd = 1;
                    netbytes = bytes;
                    break;
                }
                else
                {
                    if ( errno == EAGAIN || errno == EWOULDBLOCK )
                    {
                        netbytes = bytes;
                        break;
                    }
                    else if ( errno == ECONNRESET )
                    {
                        fd1eofd = 1;
                        netbytes = 0;
                        break;
                    }
                    else if ( errno == EINTR )
                        continue;
                    else
                    {
                        netbytes = error;
                        break;
                    }
                }
            }

            if ( netbytes < 0 )
            {
                perror( "route_io:read-from-fd1" );
                return -1;
            }
            else if ( netbytes == 0 )
                if ( fd1eofd )
                    break;

        }

        if ( FD_ISSET( fd2, &w_set ) && netbytes > 0 )
        {
            int i = 0;
            int j = 0;
            int ch;

            /*
             * After about 80% I looked at Berkeley telnetd and was surprised
             * that it uses a similar state machine. Also the CRLF
             * stuff here is based on my understanding of Berkeley sources.
             */
            while ( i < netbytes )
            {
                ch = netbuf[i++] & 0377;
                switch ( state )
                {
                default:
                case ST_CRLF:  /* Skip '\n' if '\r\n' */
                    state = ST_DATA;
                    if ( ch == '\n' || ch == '\0' )
                        continue;
                    /* Fall through to data */
                case ST_DATA:
                    if ( ch == IAC )
                    {
                        state = ST_IAC;
                        continue;
                    }

                    else if ( ch == '\r' )
                        state = ST_CRLF;

                    netbuf2[j++] = ch;
                    continue;
                case ST_IAC:
                    /* Messy. Eat up IACs for now. */
                    if ( ch == SB )
                        state = ST_SE;

                    else if ( ch == IP )
                    {
                        /* Kludgy! Will handle data-mark and urgent
                         * data later, right now just kill the shell. */
                        exit( 0 );
                    }

                    else if ( ch == IAC )   /* double IAC can happen */
                    {
                        netbuf2[j++] = ch;
                        state = ST_DATA;
                    }

                    else
                        state = ST_OPT;
                    break;

                case ST_OPT:
                case ST_DO:
                case ST_DONT:
                case ST_WILL:
                case ST_WONT:
                    /* will/wont/do/dont - discarding for now */
                    state = ST_DATA;
                    break;

                case ST_SE:
                    if ( ch == SE )
                        state = ST_DATA;

                    break;

                }
            }

            /* If error assume shell exited */
            {
                int error;
                int bytes = 0;
                int max_write = 1024;

                if ( j < max_write )
                    max_write = j;

                while ( 1 )
                {
                    error = write( fd2, netbuf2 + bytes, max_write );

                    if ( error >= 0 )
                    {
                        bytes += error;

                        if ( bytes >= j )
                        {
                            break;
                        }
                        else
                            max_write = j - bytes;

                        continue;
                    }
                    else
                    {
                        if ( errno == EAGAIN || errno == EINTR )
                            continue;
                        else
                        {
                            perror( "fd2 write" );
                            return -1;
                        }
                    }
                }

                netbytes = 0;
            }
        }

        // Pty has data
        if ( FD_ISSET( fd2, &r_set ) && ptybytes == 0 )
        {
            int bytes = 0;
            int error;

            while ( 1 )
            {
                error = read( fd2, ptybuf + bytes, sizeof( ptybuf ) - bytes );

                if ( error > 0 )
                {
                    bytes += error;
                }
                else if ( !error )
                {
                    fd2eofd = 1;
                    ptybytes = bytes;
                    break;
                }
                else
                {
                    if ( errno == EAGAIN || errno == EWOULDBLOCK )
                    {
                        ptybytes = bytes;
                        break;
                    }
                    else if ( errno == ECONNRESET )
                    {
                        fd2eofd = 1;
                        ptybytes = 0;
                        break;
                    }
                    else if ( errno == EINTR )
                        continue;
                    else
                    {
                        ptybytes = error;
                        break;
                    }
                }
            }
        }

        if ( ptybytes < 0 )
        {
            perror( "fd2 read" );
            return -1;
        }

        else if ( ptybytes == 0 )
        {
            if ( fd2eofd )
                break;
        }

        if ( FD_ISSET( fd1, &w_set ) && ptybytes > 0 )
        {
            int i = 0;
            int j = 0;
            int ch;

            /*
             * This little snippet is from reading Berkeley sources
             * trying to understand the telnet protocol.
             * If we were supporting binary mode (RFC 856) we would need
             * more than what I have. IAC is a legal byte in binary
             * mode so if we get double IAC we just pass one thru.
             * ---
             * Concerning CRLF, since we are the server (UNIX side)
             * we only expect '\n' so translate for client. If we
             * see '\r' from server side, ignore since it was probably
             * part of some other app trying to do translation like us.
             * This is only my understanding, which is very limited.
             */
            while ( i < ptybytes )
            {
                ch = ptybuf[i++] & 0377;
                if ( ch == IAC )
                    ptybuf2[j++] = ch;
                else if ( ch == '\n' )
                {
                    ptybuf2[j++] = '\r';
                    ptybuf2[j++] = ch;
                }
                else if ( ch != '\r' )
                    ptybuf2[j++] = ch;
            }

            {
                int error;
                int bytes = 0;
                int max_write = 1024;

                if ( j < max_write )
                    max_write = j;

                while ( 1 )
                {
                    error = write( fd1, ( ptybuf2 + bytes ), max_write );

                    if ( error >= 0 )
                    {
                        bytes += error;

                        if ( bytes >= j )
                        {
                            break;
                        }
                        else
                        {
                            max_write = j - bytes;
                        }
                        continue;
                    }
                    else
                    {
                        if ( errno == EAGAIN || errno == EINTR )
                            continue;
                        else
                        {
                            perror( "write-to-fd1" );
                            return -1;
                        }
                    }
                }
            }

            ptybytes = 0;
        }
    }

    return 0;
}
