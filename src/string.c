/***************************************************************************
 *  File: string.c                                                         *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

void show_line_numbers args( ( CHAR_DATA * ch, char *oldstring ) );
char *line_replace args( ( char *orig, int line, char *arg3 ) );
int count_lines args( ( const char *orig ) );
char *line_delete args( ( char *orig, int line ) );

/*
   The line_replace, count_lines, and line_delete functions
   were provided by Thanatos (Jonathan Rose).  I wrote the new 
   line_add and show_line_numbers functions and modified the 
   string_add function.  --Kyle Boyd
*/

/*****************************************************************************
 Name:		line_replace
 Purpose:	Substitutes one line of text for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char *line_replace( char *orig, int line, char *arg3 )
{

    unsigned int len;
    int count = 0;
    char *pOut, outbuf[4 * MAX_STRING_LENGTH], buf[4 * MAX_STRING_LENGTH];
    bool copy = TRUE;

    strcpy( buf, orig );
    outbuf[0] = '\0';
    pOut = outbuf;
    if ( line == 1 )
    {
        *pOut = '\0';
        strcat( outbuf, arg3 );
        pOut += strlen( arg3 );
        if ( *pOut - 2 != '\n' )
            *pOut++ = '\n';
        if ( *pOut - 1 != '\r' )
            *pOut++ = '\r';
        copy = FALSE;
    }

    for ( len = 0; len < strlen( buf ); len++ )
    {
        if ( buf[len] == '\r' )
        {
            if ( copy )
                *pOut++ = '\r';
            count++;
            if ( count == line - 1 )
            {
                *pOut = '\0';
                strcat( outbuf, arg3 );
                pOut += strlen( arg3 );
                if ( *pOut - 2 != '\n' )
                    *pOut++ = '\n';
                if ( *pOut - 1 != '\r' )
                    *pOut++ = '\r';
                copy = FALSE;
            }
            else if ( count == line )
                copy = TRUE;
        }
        else if ( copy )
            *pOut++ = buf[len];
    }
    *pOut = '\0';
    free_string( &orig );
    return str_dup( outbuf );
}

/*****************************************************************************
 Name:		count_lines
 Purpose:	counts the number of lines in the string
 Called by:     string_add for use in line_delete
 *****************************************************************************/
int count_lines( const char *orig )
{
    int line;

    for ( line = 0; *orig; )
    {
        if ( *orig++ == '\r' )
            line++;
    }

    return line;
}

/*****************************************************************************
 Name:		line_delete
 Purpose:	deletes one line of text 
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char *line_delete( char *orig, int line )
{

    int len, buflen;
    int count = 0;
    char *pOut, outbuf[4 * MAX_STRING_LENGTH], buf[4 * MAX_STRING_LENGTH];

    strcpy( buf, orig );
    buflen = strlen( buf );
    outbuf[0] = '\0';
    pOut = outbuf;
    len = 0;
    if ( line == 1 )
    {
        *pOut = '\0';
        for ( ; buf[len] != '\r'; len++ )
            continue;
        len++;
    }

    for ( ; len < buflen; len++ )
    {
        if ( buf[len] == '\r' )
        {
            count++;
            if ( count == line - 1 )
            {
                for ( len++; len < buflen; len++ )
                {
                    if ( buf[len] == '\r' )
                    {
                        break;
                    }
                }
                *pOut++ = '\r';
            }
            else
                *pOut++ = buf[len];
        }
        else
            *pOut++ = buf[len];
    }
    *pOut = '\0';
    free_string( &orig );
    return str_dup( outbuf );
}

/*****************************************************************************
 Name:		line_add
 Purpose:	adds one line of text at line # specified
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
/* The old one was buggy, at lesat for my mud */
/* So I rewrote it.  -- Kyle Boyd */
char *line_add( char *orig, int line, char *add )
{
    char *string, outbuf[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
    int count = 1, pos;
    strcpy( buf, orig );
    string = buf;
    outbuf[0] = '\0';

    if ( line == 1 )
    {
        outbuf[0] = '\0';
        strcat( outbuf, add );
        strcat( outbuf, "\n\r" );
        strcat( outbuf, orig );
        free_string( &orig );
        return str_dup( outbuf );
    }
    for ( pos = 0; string[pos] != '\0'; pos++ )
    {
        if ( string[pos] == '\n' && string[pos + 1] != '\0' )
        {
            if ( string[pos + 1] == '\r' && string[pos + 2] != '\0' )
            {
                count++;
                if ( count == line )
                {
                    strncat( outbuf, orig, pos + 2 );
                    strcat( outbuf, add );
                    strcat( outbuf, "\n\r" );
                    break;
                }
            }
        }
    }
    for ( ; string[pos] != '\0'; pos++ )
    {
        if ( string[pos] == '\n' && string[pos + 1] != '\0' )
        {
            if ( string[pos + 1] == '\r' )
            {
                strcat( outbuf, &orig[pos + 2] );
                free_string( &orig );
                return str_dup( outbuf );
            }
        }
    }
    return orig;
}

/*****************************************************************************
 Name:		string_edit
 Purpose:	Clears string and puts player into editing mode.
 Called by:	none
 ****************************************************************************/
void string_edit( CHAR_DATA * ch, char **pString )
{
    send_to_char( "-========- Entering EDIT Mode -=========-\n\r", ch );
    send_to_char( "    Type .h on a new line for help\n\r", ch );
    send_to_char( " Terminate with a ~ or @ on a blank line.\n\r", ch );
    send_to_char( "-=======================================-\n\r", ch );

    if ( *pString == NULL )
    {
        *pString = str_dup( "" );
    }
    else
    {
        **pString = '\0';
    }

    ch->desc->pString = pString;

    return;
}

/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void string_append( CHAR_DATA * ch, char **pString )
{
    send_to_char( "-=======- Entering APPEND Mode -========-\n\r", ch );
    send_to_char( "    Type .h on a new line for help\n\r", ch );
    send_to_char( " Terminate with a ~ or @ on a blank line.\n\r", ch );
    send_to_char( "-=======================================-\n\r", ch );

    if ( *pString == NULL )
    {
        *pString = str_dup( "" );
    }
    show_line_numbers( ch, *pString );

    if ( *( *pString + strlen( *pString ) - 1 ) != '\r' )
        send_to_char( "\n\r", ch );

    ch->desc->pString = pString;

    return;
}

/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char *string_replace( char *orig, char *old, char *new )
{
    char xbuf[MAX_STRING_LENGTH];
    int i;

    xbuf[0] = '\0';
    strcpy( xbuf, orig );
    if ( strstr( orig, old ) != NULL )
    {
        i = strlen( orig ) - strlen( strstr( orig, old ) );
        xbuf[i] = '\0';
        strcat( xbuf, new );
        strcat( xbuf, &orig[i + strlen( old )] );
        free_string( &orig );
    }

    if ( orig )
        free_string( &orig );
    return str_dup( xbuf );
}

/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void string_add( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    /*
     * Thanks to James Seng
     */
    if ( *argument == '~' )
    {
        ch->desc->pString = NULL;
        return;
    }

    smash_tilde( argument );

    if ( *argument == '.' )
    {
        char arg2[MAX_INPUT_LENGTH];
        char arg3[MAX_INPUT_LENGTH];

        argument = one_argument( argument, arg );
        argument = first_arg( argument, arg2, FALSE );
        argument = first_arg( argument, arg3, FALSE );

        if ( !str_cmp( arg, ".c" ) )
        {
            send_to_char( "String cleared.\n\r", ch );
            **ch->desc->pString = '\0';
            return;
        }

        if ( !str_cmp( arg, ".s" ) )
        {
            send_to_char( "String so far:\n\r", ch );
            show_line_numbers( ch, *ch->desc->pString );
            return;
        }

        if ( !str_cmp( arg, ".r" ) )
        {
            if ( arg2[0] == '\0' )
            {
                send_to_char( "usage:  .r \"old string\" \"new string\"\n\r",
                              ch );
                return;
            }

            smash_tilde( arg3 );    /* Just to be sure -- Hugin */
            *ch->desc->pString =
                string_replace( *ch->desc->pString, arg2, arg3 );
            sprintf( buf, "'%s' replaced with '%s'.\n\r", arg2, arg3 );
            send_to_char( buf, ch );
            return;
        }

        if ( !str_cmp( arg, ".l" ) )
        {
            if ( !( *ch->desc->pString ) || !( *ch->desc->pString[0] ) )
            {
                send_to_char( "No lines to replace.\n\r", ch );
                return;
            }

            if ( !is_number( arg2 ) || !arg3[0] )
            {
                send_to_char( "usage:  .l line# 'new text'\n\r", ch );
                return;
            }

            if ( atoi( arg2 ) < 1 )
            {
                send_to_char( "Line numbers start at 1.\n\r", ch );
                return;
            }
            smash_tilde( arg3 );
            *ch->desc->pString =
                line_replace( *ch->desc->pString, atoi( arg2 ), arg3 );
            return;
        }

        if ( !str_cmp( arg, ".a" ) )
        {
            if ( !( *ch->desc->pString ) || !( *ch->desc->pString[0] ) )
            {
                send_to_char( "No lines to replace.\n\r", ch );
                return;
            }

            if ( !is_number( arg2 ) || !arg3[0] )
            {
                send_to_char( "usage:  .a line# 'new text'\n\r", ch );
                return;
            }

            if ( atoi( arg2 ) < 1 )
            {
                send_to_char( "Line numbers start at 1.\n\r", ch );
                return;
            }
            smash_tilde( arg3 );
            *ch->desc->pString =
                line_add( *ch->desc->pString, atoi( arg2 ), arg3 );
            return;
        }

        if ( !str_cmp( arg, ".d" ) )
        {
            int line;

            if ( !( *ch->desc->pString ) || !( *ch->desc->pString[0] ) )
            {
                send_to_char( "No lines to delete.\n\r", ch );
                return;
            }

            if ( arg2[0] == '\0' )
            {
                line = count_lines( *ch->desc->pString );
            }
            else
                line = atoi( arg2 );

            if ( line < 1 )
            {
                send_to_char( "Line numbers start at 1.\n\r", ch );
                return;
            }
            smash_tilde( arg3 );
            *ch->desc->pString = line_delete( *ch->desc->pString, line );
            return;
        }

        if ( !str_cmp( arg, ".f" ) )
        {
            *ch->desc->pString = format_string( *ch->desc->pString );
            send_to_char( "String formatted.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg, ".h" ) )
        {
            send_to_char( "Sedit help (commands on blank line):   \n\r", ch );
            send_to_char( ".r 'old' 'new'   - replace a substring \n\r", ch );
            send_to_char( "                   (requires '', \"\") \n\r", ch );
            send_to_char( ".l line# 'new'   - replace a line      \n\r", ch );
            send_to_char( ".d               - delete last line    \n\r", ch );
            send_to_char( ".d line#         - delete a line       \n\r", ch );
            send_to_char( ".a line# 'new'   - add a new line      \n\r", ch );
            send_to_char( ".h               - get help (this info)\n\r", ch );
            send_to_char( ".s               - show string so far  \n\r", ch );
            send_to_char( ".f               - (word wrap) string  \n\r", ch );
            send_to_char( ".c               - clear string so far \n\r", ch );
            send_to_char( "@                - end string          \n\r", ch );
            return;
        }

        send_to_char( "SEdit:  Invalid dot command.\n\r", ch );
        return;
    }

    if ( *argument == '~' || *argument == '@' )
    {
        ch->desc->pString = NULL;
        return;
    }

    strcpy( buf, *ch->desc->pString );

    /*
     * Truncate strings to MAX_STRING_LENGTH.
     * --------------------------------------
     */
    if ( strlen( buf ) + strlen( argument ) >= ( MAX_STRING_LENGTH - 4 ) )
    {
        send_to_char( "String too long, last line skipped.\n\r", ch );

        /* Force character out of editing mode. */
        ch->desc->pString = NULL;
        return;
    }

    /*
     * Ensure no tilde's inside string.
     * --------------------------------
     */
    smash_tilde( argument );

    strcat( buf, argument );
    strcat( buf, "\n\r" );
    free_string( ch->desc->pString );
    *ch->desc->pString = str_dup( buf );
    return;
}

/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 */
/*****************************************************************************
 Name:		format_string
 Purpose:	Special string formating and word-wrapping.
 Called by:	string_add(string.c) (many)olc_act.c
 ****************************************************************************/
char *format_string( char *oldstring /*, bool fSpace */  )
{
    char xbuf[MAX_STRING_LENGTH];
    char xbuf2[MAX_STRING_LENGTH];
    char *rdesc;
    int i = 0;
    bool cap = TRUE;

    xbuf[0] = xbuf2[0] = 0;

    i = 0;

    for ( rdesc = oldstring; *rdesc; rdesc++ )
    {
        if ( *rdesc == '\n' )
        {
            if ( xbuf[i - 1] != ' ' )
            {
                xbuf[i] = ' ';
                i++;
            }
        }
        else if ( *rdesc == '\r' );
        else if ( *rdesc == ' ' )
        {
            if ( xbuf[i - 1] != ' ' )
            {
                xbuf[i] = ' ';
                i++;
            }
        }
        else if ( *rdesc == ')' )
        {
            if ( xbuf[i - 1] == ' ' && xbuf[i - 2] == ' ' &&
                 ( xbuf[i - 3] == '.' || xbuf[i - 3] == '?'
                   || xbuf[i - 3] == '!' ) )
            {
                xbuf[i - 2] = *rdesc;
                xbuf[i - 1] = ' ';
                xbuf[i] = ' ';
                i++;
            }
            else
            {
                xbuf[i] = *rdesc;
                i++;
            }
        }
        else if ( *rdesc == '.' || *rdesc == '?' || *rdesc == '!' )
        {
            if ( xbuf[i - 1] == ' ' && xbuf[i - 2] == ' ' &&
                 ( xbuf[i - 3] == '.' || xbuf[i - 3] == '?'
                   || xbuf[i - 3] == '!' ) )
            {
                xbuf[i - 2] = *rdesc;
                if ( *( rdesc + 1 ) != '\"' )
                {
                    xbuf[i - 1] = ' ';
                    xbuf[i] = ' ';
                    i++;
                }
                else
                {
                    xbuf[i - 1] = '\"';
                    xbuf[i] = ' ';
                    xbuf[i + 1] = ' ';
                    i += 2;
                    rdesc++;
                }
            }
            else
            {
                xbuf[i] = *rdesc;
                if ( *( rdesc + 1 ) != '\"' )
                {
                    xbuf[i + 1] = ' ';
                    xbuf[i + 2] = ' ';
                    i += 3;
                }
                else
                {
                    xbuf[i + 1] = '\"';
                    xbuf[i + 2] = ' ';
                    xbuf[i + 3] = ' ';
                    i += 4;
                    rdesc++;
                }
            }
            cap = TRUE;
        }
        else
        {
            xbuf[i] = *rdesc;
            if ( cap )
            {
                cap = FALSE;
                xbuf[i] = UPPER( xbuf[i] );
            }
            i++;
        }
    }
    xbuf[i] = 0;
    strcpy( xbuf2, xbuf );

    rdesc = xbuf2;

    xbuf[0] = 0;

    for ( ;; )
    {
        for ( i = 0; i < 77; i++ )
        {
            if ( !*( rdesc + i ) )
                break;
        }
        if ( i < 77 )
        {
            break;
        }
        for ( i = ( xbuf[0] ? 76 : 73 ); i; i-- )
        {
            if ( *( rdesc + i ) == ' ' )
                break;
        }
        if ( i )
        {
            *( rdesc + i ) = 0;
            strcat( xbuf, rdesc );
            strcat( xbuf, "\n\r" );
            rdesc += i + 1;
            while ( *rdesc == ' ' )
                rdesc++;
        }
        else
        {
            bug( "No spaces", 0 );
            *( rdesc + 75 ) = 0;
            strcat( xbuf, rdesc );
            strcat( xbuf, "-\n\r" );
            rdesc += 76;
        }
    }
    while ( *( rdesc + i ) && ( *( rdesc + i ) == ' ' ||
                                *( rdesc + i ) == '\n' ||
                                *( rdesc + i ) == '\r' ) )
        i--;
    *( rdesc + i + 1 ) = 0;
    strcat( xbuf, rdesc );
    if ( xbuf[strlen( xbuf ) - 2] != '\n' )
        strcat( xbuf, "\n\r" );

    free_string( &oldstring );
    return ( str_dup( xbuf ) );
}

/*
 * Used above in string_add.  Because this function does not
 * modify case if fCase is FALSE and because it understands
 * parenthesis, it would probably make a nice replacement
 * for one_argument.
 */
/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quates, parenthesis (barring ) ('s) and
 		percentages.
 Called by:	string_add(string.c)
 ****************************************************************************/
char *first_arg( char *argument, char *arg_first, bool fCase )
{
    char cEnd;

    while ( *argument == ' ' )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"'
         || *argument == '%' || *argument == '(' )
    {
        if ( *argument == '(' )
        {
            cEnd = ')';
            argument++;
        }
        else
            cEnd = *argument++;
    }

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        if ( fCase )
            *arg_first = LOWER( *argument );
        else
            *arg_first = *argument;
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    while ( *argument == ' ' )
        argument++;

    return argument;
}

/*
 * Used in olc_act.c for aedit_builders.
 */
char *string_unpad( char *argument )
{
    char *s;
    char *result;
    char *tmp;

    s = argument;

    while ( *s == ' ' )
        s++;

    result = s;
    tmp = s;

    if ( *s != '\0' )
    {
        while ( *s != '\0' )
            if ( *s++ != ' ' )
                tmp = s - 1;

        *tmp = '\0';
    }

    free_string( &argument );
    return str_dup( result );
}

/*
 * Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder.
 */
char *string_proper( char *argument )
{
    char *s;

    s = argument;

    while ( *s != '\0' )
    {
        if ( *s != ' ' )
        {
            *s = UPPER( *s );
            while ( *s != ' ' && *s != '\0' )
                s++;
        }
        else
        {
            s++;
        }
    }

    return argument;
}

/* This functions by Kyle Boyd. */
/* If you see any bugs, check to see if all of your strings end like that. */

void show_line_numbers( CHAR_DATA * ch, char *string )
{
    char *ptr;
    char newstring[MAX_STRING_LENGTH];
    int line;

    if ( !*string )
    {
        send_to_char( "\n\r", ch );
        return;
    }

    ptr = newstring;
    *ptr = '\0';
    sprintf( ptr, " 1 " );
    ptr += 3;

    for ( line = 2; *string; string++ )
    {
        if ( *string == '\n' || *string == '\r' )
        {
            /*
             * Allow for both \n\r and \r\n and \n.  Don't expect two chars
             * because you could condense two rows down to one in the case of
             * \n\n
             */
            if ( ( *string == '\n' && *( string + 1 ) == '\r' )
                 || ( *string == '\r' && *( string + 1 ) == '\n' ) )
                string += 2;
            else
                string++;

            if ( *string == '\0' )
                break;

            sprintf( ptr, "\n\r%2d ", line++ );
            ptr += 5;
        }

        *ptr++ = *string;
    }

    sprintf( ptr, "\n\r" );
    send_to_char( newstring, ch );
    return;
}
