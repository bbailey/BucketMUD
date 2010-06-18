/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1998 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@hypercube.org)				   *
*	    Gabrielle Taylor (gtaylor@hypercube.org)			   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#if defined(WIN32)
#include <windows.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"

/* stuff for recycling ban structures */
BAN_DATA *ban_free;

BAN_DATA *new_ban( void )
{
    static BAN_DATA ban_zero;
    BAN_DATA *ban;

    if ( ban_free == NULL )
        ban = alloc_perm( sizeof( *ban ) );
    else
    {
        ban = ban_free;
        ban_free = ban_free->next;
    }

    *ban = ban_zero;
    VALIDATE( ban );
    ban->name = &str_empty[0];
    return ban;
}

void free_ban( BAN_DATA * ban )
{
    if ( !IS_VALID( ban ) )
        return;

    free_string( &ban->name );
    INVALIDATE( ban );

    ban->next = ban_free;
    ban_free = ban;
}

BUFFER *buf_free;

/* buffer sizes */
const int buf_size[MAX_BUF_LIST] = {
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int get_size( int val )
{
    int i;

    for ( i = 0; i < MAX_BUF_LIST; i++ )
        if ( buf_size[i] >= val )
        {
            return buf_size[i];
        }

    return -1;
}

BUFFER *new_buf(  )
{
    BUFFER *buffer;

    if ( buf_free == NULL )
        buffer = alloc_perm( sizeof( *buffer ) );
    else
    {
        buffer = buf_free;
        buf_free = buf_free->next;
    }

    buffer->next = NULL;
    buffer->state = BUFFER_SAFE;
    buffer->size = get_size( BASE_BUF );

    buffer->string = alloc_mem( buffer->size );
    buffer->string[0] = '\0';
    VALIDATE( buffer );

    return buffer;
}

BUFFER *new_buf_size( int size )
{
    BUFFER *buffer;

    if ( buf_free == NULL )
        buffer = alloc_perm( sizeof( *buffer ) );
    else
    {
        buffer = buf_free;
        buf_free = buf_free->next;
    }

    buffer->next = NULL;
    buffer->state = BUFFER_SAFE;
    buffer->size = get_size( size );
    if ( buffer->size == -1 )
    {
        bug( "new_buf: buffer size %d too large.", size );
        exit( 1 );
    }
    buffer->string = alloc_mem( buffer->size );
    buffer->string[0] = '\0';
    VALIDATE( buffer );

    return buffer;
}

void free_buf( BUFFER * buffer )
{
    if ( !IS_VALID( buffer ) )
        return;

    free_mem( &buffer->string );
    buffer->size = 0;
    buffer->state = BUFFER_FREED;
    INVALIDATE( buffer );

    buffer->next = buf_free;
    buf_free = buffer;
}

bool add_buf( BUFFER * buffer, char *string )
{
    int len;
    char *oldstr;
    int oldsize;

    oldstr = buffer->string;
    oldsize = buffer->size;

    if ( buffer->state == BUFFER_OVERFLOW ) /* don't waste time on bad strings! */
        return FALSE;

    len = strlen( buffer->string ) + strlen( string ) + 1;

    while ( len >= buffer->size )   /* increase the buffer size */
    {
        buffer->size = get_size( buffer->size + 1 );
        {
            if ( buffer->size == -1 )   /* overflow */
            {
                buffer->size = oldsize;
                buffer->state = BUFFER_OVERFLOW;
                bug( "buffer overflow past size %d", buffer->size );
                return FALSE;
            }
        }
    }

    if ( buffer->size != oldsize )
    {
        buffer->string = alloc_mem( buffer->size );

        strcpy( buffer->string, oldstr );
        free_mem( &oldstr );
    }

    strcat( buffer->string, string );
    return TRUE;
}

void clear_buf( BUFFER * buffer )
{
    buffer->string[0] = '\0';
    buffer->state = BUFFER_SAFE;
}

char *buf_string( BUFFER * buffer )
{
    return buffer->string;
}
