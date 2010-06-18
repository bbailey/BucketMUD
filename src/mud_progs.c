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
 * MudProgs has been completely rewritten by Zane.  The only thing that    *
 * remains the same is how triggers are handled.  -Zane                    *
 *                                                                         *
 * Original MobProgs by N'Atas-ha.                                         *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "merc.h"
#include "interp.h"
#include "mud_progs.h"

/*
 * This is a string that contains a list of valid seperators in the script like
 * ()/-=*+, etc...
 */
#define SEPERATOR_STRING "(),=<>!&|*+-/%^"

#define Success_ 0
#define Error_ -1
#define R_ERROR -2

/*
 * Locals
 */

void release_supermob( void );
void set_supermob( void *source, int prog_type );
void init_supermob( void );
void mprog_wordlist_check( char *arg, int trigger_type, int prog_type );
bool mprog_percent_check( int trigger_type );
char *parse_script( char *script );
bool is_seperator( char c );
char *parse_if( char *instring );
char *parse_command( char *instring );
void parse_command_var( char var, char *outbuf );
int parse_expression( char *instring );
int parse_proc( char *proc );
int exec_proc( char *procname, int intarg, char *chararg );
int evaluate( char *line, double *val );
void strip_whitespace( char *instring );
void *mprog_get_actor( char *arg, char type );
void mprog_driver( char *prog );

CHAR_DATA *ProgSource = NULL;
CHAR_DATA *ProgTriggeredBy = NULL;
CHAR_DATA *ProgVictim = NULL;
OBJ_DATA *ProgObjectSource = NULL;
OBJ_DATA *ProgObjectVictim = NULL;
ROOM_INDEX_DATA *ProgRoomSource = NULL;
char *ProgExtraArgs = NULL;
CHAR_DATA *supermob = NULL;
int ExecMudProgID = 0;

/* Random/Semi-Random ch's */
CHAR_DATA *RandomNPC = NULL;
bool RandomNPC_picked;
void mprog_get_RandomNPC( void );
CHAR_DATA *RandomPC = NULL;
bool RandomPC_picked;
void mprog_get_RandomPC( void );
CHAR_DATA *RandomANY = NULL;
bool RandomANY_picked;
void mprog_get_RandomANY( void );
CHAR_DATA *MostEvilFighter = NULL;
bool MostEvilFighter_picked;
void mprog_get_MostEvilFighter( void );

/*
 * Main driver function, called by all triggers.
 */
void mprog_driver( char *prog )
{
    /* Clear out our temp picks */
    RandomNPC = NULL;
    RandomNPC_picked = FALSE;
    RandomPC = NULL;
    RandomPC_picked = FALSE;
    RandomANY = NULL;
    RandomANY_picked = FALSE;
    MostEvilFighter = NULL;
    MostEvilFighter_picked = FALSE;

    /* Increment our temp prog ID */
    ExecMudProgID++;

    parse_script( prog );

    /* Clear out locals */
    ProgSource = NULL;
    ProgTriggeredBy = NULL;
    ProgVictim = NULL;
    ProgObjectSource = NULL;
    ProgObjectVictim = NULL;
    ProgRoomSource = NULL;
    ProgExtraArgs = NULL;
}

/*
 * Main script parsing function.  Passes strings off to parse_if and parse_command
 * for processing.  Also correctly handles breaks.
 */
char *parse_script( char *script )
{
    char buf[MAX_STRING_LENGTH];
    char *curr;
    char *ptr = script;
    int pos;

    pos = 0;

    while ( 1 )
    {
        /* Skip leading spaces */
        while ( isspace( *ptr ) )
            if ( *++ptr == '\0' )
            {
                bug( "Parse_Script: Script ended without 'break'.\r\n" );
                return NULL;
            }

        curr = ptr;
        pos = 0;

        while ( 1 )
        {
            buf[pos++] = *ptr;

            if ( isspace( *ptr ) || *ptr == '(' )
            {
                buf[--pos] = '\0';

                if ( !strcmp( buf, "if" ) )
                {
                    while ( isspace( *ptr ) )
                        ptr++;

                    if ( *ptr != '(' )
                    {
                        bug( "Parse_Script: Syntax error - If statment missing opening '('.\r\n" );
                        return NULL;
                    }

                    if ( ( ptr = parse_if( ptr ) ) == NULL )
                        return NULL;
                    break;
                }
                else if ( !strcmp( buf, "break" ) )
                {
                    if ( *ptr == '(' )
                    {
                        bug( "Parse_Script: Syntax error - '(' encountered after 'break'.\r\n" );
                        return NULL;
                    }
                    return NULL;
                }
                else if ( !strcmp( buf, "else" ) )
                {
                    if ( *ptr == '(' )
                    {
                        bug( "Parse_Script: Syntax error - '(' encountered after 'else'.\r\n" );
                        return NULL;
                    }
                    return curr;
                }
                else if ( !strcmp( buf, "elseif" ) )
                    return curr;
                else if ( !strcmp( buf, "endif" ) )
                {
                    if ( *ptr == '(' )
                    {
                        bug( "Parse_Script: Syntax error - '(' encountered after 'endif'.\r\n" );
                        return NULL;
                    }
                    return curr;
                }
                else
                {
                    if ( *ptr == '(' )
                    {
                        bug( "Parse_Script: Syntax error - '(' encountered after a command.\r\n" );
                        return NULL;
                    }
                    if ( ( ptr = parse_command( curr ) ) == NULL )
                        return NULL;
                    break;
                }
            }

            if ( ++ptr == '\0' )
            {
                bug( "Parse_Script: EOF encountered in read.\r\n" );
                return NULL;
            }
        }
    }

    return ptr;
}

/*
 * The main loop found and if and passed the remaining string to us. Now we process the if statement and then execute
 * commands.
 */
char *parse_if( char *instring )
{
    char buf[MAX_STRING_LENGTH];
    char *ptr = instring;
    char *tmpptr;
    int parans = 0;
    int pos = 0;
    int ifvalue = 0;
    bool elsedone = FALSE;

    /* Skip leading spaces */
    while ( isspace( *ptr ) )
        if ( ++ptr == '\0' )
        {
            bug( "Parse_If: EOF encountered in read.\r\n" );
            return NULL;
        }

    /* Evaluate the conditional portion of the if statement */
    while ( 1 )
    {
        if ( isspace( *ptr ) )
        {
            ptr++;
            continue;
        }
        else if ( *ptr == '(' )
        {
            if ( parans++ == 0 )
            {
                ptr++;
                continue;
            }

        }
        else if ( *ptr == ')' )
        {
            parans--;

            if ( parans < 0 )
            {
                bug( "Parse_If: ')' without prior '('.\r\n" );
                return NULL;
            }
            else if ( parans == 0 )
            {
                /* Check to make sure we don't have an extra ')' */
                tmpptr = ++ptr;
                while ( *tmpptr && *tmpptr != '(' )
                    if ( *tmpptr++ == ')' )
                    {
                        bug( "Parse_If: Too many ')'s.\r\n" );
                        return NULL;
                    }

                buf[pos] = '\0';
                ifvalue = parse_expression( buf );

                break;
            }
        }

        buf[pos++] = *ptr++;

        if ( *ptr == '\0' )
        {
            bug( "Parse_If: Missing ')' in script.\r\n" );
            return NULL;
        }
    }

    /* Call parse_script again.  This will process commands and allow for nested if's */
    if ( ifvalue )
        if ( ( ptr = parse_script( ptr ) ) == NULL )
            return NULL;

    /* Check for elseif and endif */

    while ( 1 )
    {
        /* Skip leading spaces */
        while ( isspace( *ptr ) )
            if ( ++ptr == '\0' )
            {
                bug( "Parse_If: EOF encountered in read.\r\n" );
                return NULL;
            }

        pos = 0;

        while ( 1 )
        {
            buf[pos++] = *ptr;

            if ( isspace( *ptr ) || *ptr == '(' )
            {
                buf[--pos] = '\0';

                if ( !strcmp( buf, "else" ) )
                {
                    if ( elsedone )
                    {
                        bug( "Parse_If: Multiple else's in if statement.\r\n" );
                        return NULL;
                    }

                    elsedone = TRUE;

                    if ( ( ptr = parse_script( ptr ) ) == NULL )
                        return NULL;

                    while ( isspace( *ptr ) )
                        if ( ++ptr == '\0' )
                        {
                            bug( "Parse_If: EOF encountered in read.\r\n" );
                            return NULL;
                        }

                    break;
                }
                else if ( !strcmp( buf, "elseif" ) )
                {
                    while ( isspace( *ptr ) )
                        ptr++;

                    if ( *ptr != '(' )
                    {
                        bug( "Parse_If: Syntax error - Elseif statment missing opening '('.\r\n" );
                        return NULL;
                    }

                    if ( !ifvalue )
                        return ( ptr = parse_if( ptr ) );

                    /* read to the end of the line */
                    while ( *ptr != '\r' && *ptr != '\n' && *ptr != '\0' )
                        ptr++;

                    break;
                }
                else if ( !strcmp( buf, "endif" ) )
                {
                    if ( *ptr == '(' )
                    {
                        bug( "Parse_If: Syntax error - '(' encountered after 'endif'.\r\n" );
                        return NULL;
                    }

                    return ptr;
                }
                else
                {
                    /*
                     * If we reached this point then we had an elseif with a prior sucessfull if
                     * so ignore all these lines untill we get to endif
                     */

                    /* read to the end of the line */
                    while ( *ptr != '\r' && *ptr != '\n' && *ptr != '\0' )
                        ptr++;

                    break;
                }
            }

            if ( ++ptr == '\0' )
            {
                bug( "Parse_If: EOF encountered in read.\r\n" );
                return NULL;
            }
        }
    }

    return ptr;
}

int parse_expression( char *instring )
{
    char buf[MAX_STRING_LENGTH];
    char calcbuf[MAX_STRING_LENGTH];
    char *ptr = instring;
    char *tmpptr = NULL;
    int bufpos = 0;
    int calcpos = 0;
    int parans = 0;
    double retval = 0;

    *buf = '\0';
    *calcbuf = '\0';

    strip_whitespace( instring );

    while ( *ptr != '\0' )
    {
        if ( isalpha( *ptr ) )
        {
            if ( !tmpptr )
            {
                bufpos = 0;
                tmpptr = ptr;
            }

            buf[bufpos++] = *ptr++;
        }
        else if ( *ptr == '(' && tmpptr )
        {
            buf[bufpos++] = *ptr++;
            parans++;
        }
        else if ( *ptr == ')' && tmpptr )
        {
            if ( parans <= 0 )
            {
                bug( "Parse_Expression: ')' without prior '('.\r\n" );
                return 0;
            }

            buf[bufpos++] = *ptr++;

            if ( --parans <= 0 )
            {
                buf[bufpos] = '\0';
                calcbuf[calcpos] = '\0';
                sprintf( &calcbuf[calcpos], "%d", parse_proc( buf ) );
                calcpos = strlen( calcbuf );
                tmpptr = NULL;
            }
        }
        else
        {
            if ( parans > 0 )
                buf[bufpos++] = *ptr++;
            else
            {
                tmpptr = NULL;
                calcbuf[calcpos++] = *ptr++;
            }
        }
    }

    if ( tmpptr )
    {
        bug( "Parse_Expression: NULL encountered before ')'.\r\n" );
        return 0;
    }

    calcbuf[calcpos] = '\0';

    if ( calcpos > 0 )
    {
        if ( evaluate( calcbuf, &retval ) == Success_ )
            return ( int ) retval;
        else
        {
            bug( "Parse_Expression: Error evaluating expression '%s'.\r\n",
                 calcbuf );
            return 0;
        }
    }
    else
        return 0;
}

int parse_proc( char *proc )
{
    char procname[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    char *ptr = proc;
    char *tmpptr;

    tmpptr = procname;

    while ( *ptr != '(' )
    {
        if ( *ptr == '\0' )
        {
            bug( "Parse_Proc: NULL encountered before '('.\r\n" );
            return 0;
        }
        *tmpptr++ = *ptr++;
    }

    *tmpptr = '\0';

    if ( *++ptr == '\0' )
    {
        bug( "Parse_Proc: NULL encountered before ')'.\r\n" );
        return 0;
    }

    tmpptr = arg;

    while ( *ptr != ')' )
    {
        if ( *ptr == '\0' )
        {
            bug( "Parse_Proc: NULL encountered before '('.\r\n" );
            return 0;
        }
        *tmpptr++ = *ptr++;
    }

    *tmpptr = '\0';

    switch ( *arg )
    {
    case '$':
        return exec_proc( procname, 0, arg );
        break;
    case '\0':
        return exec_proc( procname, 0, NULL );
        break;
    default:
        return exec_proc( procname, parse_expression( arg ), NULL );
        break;
    }
}

int exec_proc( char *procname, int intarg, char *chararg )
{
    int cmd;
    bool found = FALSE;

    for ( cmd = 0; mprog_cmd_table[cmd].name[0] != '\0'; cmd++ )
        if ( procname[0] == mprog_cmd_table[cmd].name[0]
             && !str_cmp( procname, mprog_cmd_table[cmd].name ) )
        {
            found = TRUE;
            break;
        }

    if ( !found )
    {
        bug( "Exec_Proc:  Invalid proc name - %s.\r\n", procname );
        return 0;
    }

    switch ( UPPER( mprog_cmd_table[cmd].argtype ) )
    {
    case 'C':
        return ( *mprog_cmd_table[cmd].
                 mprog_fun ) ( mprog_get_actor( chararg, 'C' ) );
    case 'O':
        return ( *mprog_cmd_table[cmd].
                 mprog_fun ) ( mprog_get_actor( chararg, 'O' ) );
    case 'S':
        return ( *mprog_cmd_table[cmd].mprog_fun ) ( ( void * ) chararg );
    case 'I':
        return ( *mprog_cmd_table[cmd].mprog_fun ) ( ( void * ) &intarg );
    case 'N':
        return ( *mprog_cmd_table[cmd].mprog_fun ) ( NULL );
    default:
        bug( "Exec_Proc: Unknown argtype %c.\r\n",
             mprog_cmd_table[cmd].argtype );
        return 0;
    }
}

/*
 * Returns a pointer to a CHAR_DATA structure when passed a $* variable.
 */
void *mprog_get_actor( char *arg, char type )
{
    ROOM_INDEX_DATA *pRoom;

    if ( !ProgSource || ( pRoom = ProgSource->in_room ) == NULL )
        return NULL;

    if ( strlen( arg ) != 2 || *arg != '$' )
    {
        bug( "Mprog_Get_Actor:  Invalid variable '%s'.\r\n", arg );
        return NULL;
    }

    switch ( type )
    {
    case 'C':
        switch ( UPPER( arg[1] ) )
        {
        case 'I':
            return ProgSource;
        case 'N':
            return ProgTriggeredBy;
        case 'T':
            return ProgVictim;
        case 'R':
            if ( !RandomPC_picked )
                mprog_get_RandomPC(  );

            return RandomPC;
        case 'X':
            if ( !MostEvilFighter_picked )
                mprog_get_MostEvilFighter(  );

            return MostEvilFighter;
        case 'B':
            if ( !RandomNPC_picked )
                mprog_get_RandomNPC(  );

            return RandomNPC;
        case 'C':
            if ( !RandomANY_picked )
                mprog_get_RandomANY(  );

            return RandomANY;
        default:
            bug( "Mprog_Get_Actor:  Invalid variable '%s' for argtype 'C'.\r\n",
                 arg );
            return NULL;
        }
        break;
    case 'O':
        switch ( UPPER( arg[1] ) )
        {
        case 'O':
            return ProgObjectSource;
        case 'P':
            return ProgObjectVictim;
        default:
            bug( "Mprog_Get_Actor:  Invalid variable '%s' for argtype 'O'.\r\n",
                 arg );
            return NULL;
        }
        break;
    default:
        bug( "Mprog_Get_Actor:  Invalid argtype '%c'.\r\n", type );
        return NULL;
    }
}

char *parse_command( char *instring )
{
    char cmd[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char *ptr = instring;
    int pos = 0;
    int bufpos = 0;
    bool escape = FALSE;

    *buf = '\0';

    /* Get rid of spaces in the beginning */
    while ( *ptr == ' ' || *ptr == '\t' )
        ptr++;

    while ( *ptr != '\r' && *ptr != '\n' )
    {
        switch ( *ptr )
        {
        case '#':
            if ( !escape )
            {
                if ( *++ptr == '#' )
                    cmd[pos++] = *ptr++;
                else
                    escape = TRUE;
            }
            else if ( escape )
            {
                if ( *++ptr == '#' )
                    buf[bufpos++] = *ptr++;
                else
                {
                    buf[bufpos] = '\0';
                    sprintf( &cmd[pos], "%d", parse_expression( buf ) );
                    pos = strlen( cmd );
                    escape = FALSE;
                }
            }
            break;
        case '$':
            if ( !escape )
            {
                if ( *++ptr == '$' )
                    cmd[pos++] = *ptr++;
                else
                {
                    parse_command_var( *ptr, &cmd[pos] );
                    pos = strlen( cmd );
                    ptr++;
                }
            }
            else
                buf[bufpos++] = *ptr++;

            break;
        default:
            if ( escape )
                buf[bufpos++] = *ptr++;
            else
                cmd[pos++] = *ptr++;
            break;
        }

        if ( *ptr == '\0' )
        {
            bug( "Parse_Command: EOL encountered.\r\n" );
            return NULL;
        }
    }

    cmd[pos] = '\0';

    /* Pass cmd and arg to the command interpreter */
    interpret( ProgSource, cmd );

    return ptr;
}

/*
 * Takes the var name passed and converts it to a text string based on the var.
 */
void parse_command_var( char var, char *outbuf )
{
    char buf[MAX_STRING_LENGTH];
    static char *he_she[] = { "it", "he", "she" };
    static char *him_her[] = { "it", "him", "her" };
    static char *his_her[] = { "its", "his", "her" };

    *outbuf = '\0';
    *buf = '\0';

    switch ( var )
    {
    case 'a':
        if ( ProgObjectSource )
            switch ( *( ProgObjectSource->name ) )
            {
            case 'a':
            case 'e':
            case 'i':
            case 'o':
            case 'u':
                strcat( outbuf, "an" );
                break;
            default:
                strcat( outbuf, "a" );
                break;
            }
        return;
    case 'A':
        if ( ProgObjectVictim )
            switch ( *( ProgObjectVictim->name ) )
            {
            case 'a':
            case 'e':
            case 'i':
            case 'o':
            case 'u':
                strcat( outbuf, "an" );
                break;
            default:
                strcat( outbuf, "a" );
                break;
            }
        return;
    case 'b':
        if ( !RandomNPC_picked )
            mprog_get_RandomNPC(  );

        if ( RandomNPC )
        {
            one_argument( RandomNPC->name, buf );
            strcat( outbuf, buf );
        }

        return;
    case 'B':
        if ( !RandomNPC_picked )
            mprog_get_RandomNPC(  );

        if ( RandomNPC )
            strcat( outbuf, RandomNPC->short_descr );

        return;
    case 'c':
        if ( !RandomANY_picked )
            mprog_get_RandomANY(  );

        if ( RandomANY )
        {
            one_argument( RandomANY->name, buf );
            strcat( outbuf, buf );
        }

        return;
    case 'C':
        if ( !RandomANY_picked )
            mprog_get_RandomANY(  );

        if ( RandomANY )
            strcat( outbuf,
                    IS_NPC( RandomANY ) ? RandomANY->short_descr : RandomANY->
                    name );

        return;
    case 'e':
        if ( ProgTriggeredBy )
        {
            if ( can_see( ProgSource, ProgTriggeredBy ) )
                strcat( outbuf, he_she[ProgTriggeredBy->sex] );
            else
                strcat( outbuf, "someone" );
        }

        return;
    case 'E':
        if ( ProgVictim )
        {
            if ( can_see( ProgSource, ProgVictim ) )
                strcat( outbuf, he_she[ProgVictim->sex] );
            else
                strcat( outbuf, "someone" );
        }

        return;
    case 'i':
        if ( ProgSource )
        {
            one_argument( ProgSource->name, buf );
            strcat( outbuf, buf );
        }

        return;
    case 'I':
        if ( ProgSource )
            strcat( outbuf, ProgSource->short_descr );

        return;
    case 'j':
        if ( ProgSource )
            strcat( outbuf, he_she[ProgSource->sex] );

        return;
    case 'J':
        if ( !RandomPC_picked )
            mprog_get_RandomPC(  );

        if ( RandomPC )
            strcat( outbuf, he_she[RandomPC->sex] );

        return;
    case 'k':
        if ( ProgSource )
            strcat( outbuf, him_her[ProgSource->sex] );

        return;
    case 'K':
        if ( !RandomPC_picked )
            mprog_get_RandomPC(  );

        if ( RandomPC )
            strcat( outbuf, him_her[RandomPC->sex] );

        return;
    case 'l':
        if ( ProgSource )
            strcat( outbuf, his_her[ProgSource->sex] );

        return;
    case 'L':
        if ( !RandomPC_picked )
            mprog_get_RandomPC(  );

        if ( RandomPC )
            strcat( outbuf, his_her[RandomPC->sex] );

        return;
    case 'm':
        if ( ProgTriggeredBy )
        {
            if ( can_see( ProgSource, ProgTriggeredBy ) )
                strcat( outbuf, him_her[ProgTriggeredBy->sex] );
            else
                strcat( outbuf, "someone" );
        }

        return;
    case 'M':
        if ( ProgVictim )
        {
            if ( can_see( ProgSource, ProgVictim ) )
                strcat( outbuf, him_her[ProgVictim->sex] );
            else
                strcat( outbuf, "someone" );
        }

        return;
    case 'n':
        if ( ProgTriggeredBy )
        {
            one_argument( ProgTriggeredBy->name, buf );
            strcat( outbuf, buf );
        }

        return;
    case 'N':
        if ( ProgTriggeredBy )
            strcat( outbuf,
                    IS_NPC( ProgTriggeredBy ) ? ProgTriggeredBy->
                    short_descr : ProgTriggeredBy->name );

        return;
    case 'o':
        if ( ProgObjectSource )
        {
            one_argument( ProgObjectSource->name, buf );
            strcat( outbuf, buf );
        }

        return;
    case 'O':
        if ( ProgObjectSource )
            strcat( outbuf, ProgObjectSource->short_descr );

        return;
    case 'p':
        if ( ProgObjectVictim )
        {
            one_argument( ProgObjectVictim->name, buf );
            strcat( outbuf, buf );
        }

        return;
    case 'P':
        if ( ProgObjectVictim )
            strcat( outbuf, ProgObjectVictim->short_descr );

        return;
    case 'r':
        if ( !RandomPC_picked )
            mprog_get_RandomPC(  );

        if ( RandomPC )
        {
            one_argument( RandomPC->name, buf );
            strcat( outbuf, buf );
        }

        return;
    case 'R':
        if ( !RandomPC_picked )
            mprog_get_RandomPC(  );

        if ( RandomPC )
            strcat( outbuf, RandomPC->name );

        return;
    case 's':
        if ( ProgTriggeredBy )
        {
            if ( can_see( ProgSource, ProgTriggeredBy ) )
                strcat( outbuf, his_her[ProgTriggeredBy->sex] );
            else
                strcat( outbuf, "someone" );
        }

        return;
    case 'S':
        if ( ProgVictim )
        {
            if ( can_see( ProgSource, ProgVictim ) )
                strcat( outbuf, his_her[ProgVictim->sex] );
            else
                strcat( outbuf, "someone" );
        }

        return;
    case 't':
        if ( ProgVictim )
        {
            one_argument( ProgVictim->name, buf );
            strcat( outbuf, buf );
        }

        return;
    case 'T':
        if ( ProgVictim )
            strcat( outbuf,
                    IS_NPC( ProgVictim ) ? ProgVictim->
                    short_descr : ProgVictim->name );

        return;
    case 'x':
        if ( !MostEvilFighter_picked )
            mprog_get_MostEvilFighter(  );

        if ( MostEvilFighter )
        {
            one_argument( MostEvilFighter->name, buf );
            strcat( outbuf, buf );
        }

        return;
    case 'X':
        if ( !MostEvilFighter_picked )
            mprog_get_MostEvilFighter(  );

        if ( MostEvilFighter )
            strcat( outbuf,
                    IS_NPC( MostEvilFighter ) ? MostEvilFighter->
                    short_descr : MostEvilFighter->name );

        return;
    case 'd':
        if ( !ProgExtraArgs )
            return;
        else
        {
            char word[MAX_STRING_LENGTH];
            char *copy = strdup( ProgExtraArgs );
            one_argument( copy, word );
            strcat( outbuf, word );
            free( copy );
        }
        return;
    case 'D':
        if ( !ProgExtraArgs )
            return;
        else
            strcat( outbuf, ProgExtraArgs );
        return;
    default:
        bug( "Parse_Command_Var: Unknown command variable '$%c'.\r\n", var );
        return;
    }
}

void init_supermob( void )
{
    supermob = create_mobile( get_mob_index( MOB_VNUM_SUPERMOB ) );
    char_to_room( supermob, get_room_index( ROOM_VNUM_SUPERMOB ) );

    return;
}

void set_supermob( void *source, int prog_type )
{
    OBJ_DATA *pObj = NULL;
    ROOM_INDEX_DATA *pRoom = NULL;
    OBJ_DATA *in_obj = NULL;
    char buf[200];

    if ( !supermob )
        init_supermob(  );

    if ( !source )
        return;

    switch ( prog_type )
    {
    case OBJ_PROG:
        pObj = ( OBJ_DATA * ) source;

        for ( in_obj = pObj; in_obj->in_obj; in_obj = in_obj->in_obj )
            ;

        if ( in_obj->carried_by )
            pRoom = in_obj->carried_by->in_room;
        else
            pRoom = pObj->in_room;

        if ( !pRoom )
            return;

        if ( supermob->short_descr )
            free_string( &supermob->short_descr );

        supermob->short_descr = str_dup( pObj->short_descr );
        supermob->level = pObj->level;

        /* Added by Jenny to allow bug messages to show the vnum
           of the object, and not just supermob's vnum */
        sprintf( buf, "Object #%d", pObj->pIndexData->vnum );
        if ( supermob->description )
            free_string( &supermob->description );
        supermob->description = str_dup( buf );

        /*
         * Point the Supermob's prog list to the object's list for easier use.
         * Leave Supermob's progtypes at 0 so no progs are ever triggered.  -Zane
         */
        supermob->pIndexData->mudprogs = pObj->pIndexData->mudprogs;
        break;
    case ROOM_PROG:
        pRoom = ( ROOM_INDEX_DATA * ) source;

        if ( supermob->short_descr )
            free_string( &supermob->short_descr );
        supermob->short_descr = str_dup( pRoom->name );

        if ( supermob->name )
            free_string( &supermob->name );
        supermob->name = str_dup( pRoom->name );

        /* Added by Jenny to allow bug messages to show the vnum
           of the room, and not just supermob's vnum */
        sprintf( buf, "Room #%d", pRoom->vnum );
        if ( supermob->description )
            free_string( &supermob->description );
        supermob->description = str_dup( buf );

        /*
         * Point the Supermob's prog list to the object's list for easier use.
         * Leave Supermob's progtypes at 0 so no progs are ever triggered.  -Zane
         */
        supermob->pIndexData->mudprogs = pRoom->mudprogs;
        break;
    }

    char_from_room( supermob );
    char_to_room( supermob, pRoom );

    return;
}

void release_supermob( void )
{
    char_from_room( supermob );
    if ( supermob->name )
        free_string( &supermob->name );
    if ( supermob->short_descr )
        free_string( &supermob->short_descr );
    if ( supermob->description )
        free_string( &supermob->description );

    supermob->name = str_dup( "SuperMob" );
    supermob->level = HERO;
    supermob->pIndexData->mudprogs = NULL;
    char_to_room( supermob, get_room_index( ROOM_VNUM_SUPERMOB ) );
}

void mprog_get_RandomNPC( void )
{
    CHAR_DATA *ch;
    int count = 0;
    int random = 0;

    /* Get the number of NPC's in the room */
    for ( ch = ProgSource->in_room->people; ch; ch = ch->next_in_room )
        if ( IS_NPC( ch ) && can_see( ProgSource, ch ) )
            count++;

    random = number_range( 1, count );
    count = 0;

    /* get a random visible NPC who is in the room with the mob */
    for ( ch = ProgSource->in_room->people; ch; ch = ch->next_in_room )
        if ( IS_NPC( ch ) && can_see( ProgSource, ch ) && ++count == random )
        {
            RandomNPC = ch;
            RandomNPC_picked = TRUE;
        }
}

void mprog_get_RandomPC( void )
{
    CHAR_DATA *ch;
    int count = 0;
    int random = 0;

    /* Get the number of PC's in the room */
    for ( ch = ProgSource->in_room->people; ch; ch = ch->next_in_room )
        if ( !IS_NPC( ch ) && can_see( ProgSource, ch ) )
            count++;

    random = number_range( 1, count );
    count = 0;

    /* get a random visible mortal player who is in the room with the mob */
    for ( ch = ProgSource->in_room->people; ch; ch = ch->next_in_room )
        if ( !IS_NPC( ch ) && can_see( ProgSource, ch ) && ++count == random )
        {
            RandomPC = ch;
            RandomPC_picked = TRUE;
        }
}

void mprog_get_RandomANY( void )
{
    CHAR_DATA *ch;
    int count = 0;
    int random = 0;

    /* Get the number of visible PC's and NPC's in the room */
    for ( ch = ProgSource->in_room->people; ch; ch = ch->next_in_room )
        if ( can_see( ProgSource, ch ) )
            count++;

    random = number_range( 1, count );
    count = 0;

    /* get a random visible NPC or PC in the room with the mob */
    for ( ch = ProgSource->in_room->people; ch; ch = ch->next_in_room )
        if ( can_see( ProgSource, ch ) && ++count == random )
        {
            RandomANY = ch;
            RandomANY_picked = TRUE;
        }
}

void mprog_get_MostEvilFighter( void )
{
    CHAR_DATA *ch;
    int max_evil = 0;

    /* get the most evil visible mortal player who is in the room with the mob and is fighting */
    for ( ch = ProgSource->in_room->people; ch; ch = ch->next_in_room )
        if ( !IS_NPC( ch ) && can_see( ProgSource, ch ) )
            if ( ch->fighting && ch->alignment < max_evil )
            {
                max_evil = ch->alignment;
                MostEvilFighter = ch;
                MostEvilFighter_picked = TRUE;
            }
}

/*
 * Use the constant seperator string to decide wether or not c is a 
 * seperator
 */
bool is_seperator( char c )
{
    char *ptr = SEPERATOR_STRING;

    do
    {
        if ( c == *ptr )
            return TRUE;
    }
    while ( *++ptr != '\0' );

    return FALSE;
}

void strip_whitespace( char *instring )
{
    char *inptr = instring, *outptr = instring;

    while ( *inptr )
    {
        if ( isspace( *inptr ) )
            inptr++;
        else
            *outptr++ = *inptr++;
    }

    *outptr = '\0';
}

bool mprog_percent_check( int trigger_type )
{
    MPROG_LIST *pProgList;
    bool executed = FALSE;

    if ( !ProgSource || !ProgSource->pIndexData )
        return FALSE;

    for ( pProgList = ProgSource->pIndexData->mudprogs; pProgList;
          pProgList = pProgList->next )
        if ( ( pProgList->mudprog->trigger_type & trigger_type )
             && ( number_percent(  ) < atoi( pProgList->mudprog->arglist ) ) )
        {
            executed = TRUE;
            mprog_driver( pProgList->mudprog->comlist );
            if ( trigger_type != GREET_PROG && trigger_type != ALL_GREET_PROG
                 && trigger_type != ENTER_PROG )
                break;
        }

    return executed;
}

/* The next two routines are the basic trigger types. Either trigger
 *  on a certain percent, or trigger on a keyword or word phrase.
 *  To see how this works, look at the various trigger routines..
 */
void mprog_wordlist_check( char *arg, int trigger_type, int prog_type )
{
    char temp1[MAX_STRING_LENGTH];
    char temp2[MAX_INPUT_LENGTH];
    char word[MAX_INPUT_LENGTH];
    MPROG_DATA *mprg;
    MPROG_LIST *pProgList = NULL;
    char *list;
    char *start;
    char *dupl;
    char *end;
    unsigned int i;

    switch ( prog_type )
    {
    case ROOM_PROG:
        pProgList = ProgRoomSource->mudprogs;
        break;
    case OBJ_PROG:
        pProgList = ProgObjectSource->pIndexData->mudprogs;
        break;
    case MOB_PROG:
        pProgList = ProgSource->pIndexData->mudprogs;
        break;

    }

    for ( ; pProgList; pProgList = pProgList->next )
        if ( pProgList->mudprog->trigger_type & trigger_type )
        {
            mprg = pProgList->mudprog;

            strcpy( temp1, mprg->arglist );
            list = temp1;

            for ( i = 0; i < strlen( list ); i++ )
                list[i] = LOWER( list[i] );

            strcpy( temp2, arg );
            dupl = remove_color( temp2 );

            for ( i = 0; i < strlen( dupl ); i++ )
                dupl[i] = LOWER( dupl[i] );

            if ( ( list[0] == 'p' ) && ( list[1] == ' ' ) )
            {
                list += 2;
                while ( ( start = strstr( dupl, list ) ) )
                    if ( ( start == dupl || *( start - 1 ) == ' ' )
                         && ( *( end = start + strlen( list ) ) == ' '
                              || *end == '\n'
                              || *end == '\r' || *end == '\0' ) )
                    {
                        if ( prog_type == OBJ_PROG )
                        {
                            set_supermob( ProgObjectSource, prog_type );
                            ProgSource = supermob;
                        }
                        else if ( prog_type == ROOM_PROG )
                        {
                            set_supermob( ProgRoomSource, prog_type );
                            ProgSource = supermob;
                        }

                        mprog_driver( mprg->comlist );

                        if ( ProgSource == supermob )
                            release_supermob(  );

                        break;
                    }
                    else
                        dupl = start + 1;
            }
            else
            {
                list = one_argument( list, word );
                for ( ; word[0] != '\0'; list = one_argument( list, word ) )
                    while ( ( start = strstr( dupl, word ) ) )
                        if ( ( start == dupl || *( start - 1 ) == ' ' )
                             && ( *( end = start + strlen( word ) ) == ' '
                                  || *end == '\n'
                                  || *end == '\r' || *end == '\0' ) )
                        {
                            if ( prog_type == OBJ_PROG )
                            {
                                set_supermob( ProgObjectSource, prog_type );
                                ProgSource = supermob;
                            }
                            else if ( prog_type == ROOM_PROG )
                            {
                                set_supermob( ProgRoomSource, prog_type );
                                ProgSource = supermob;
                            }

                            mprog_driver( mprg->comlist );

                            if ( ProgSource == supermob )
                                release_supermob(  );

                            break;
                        }
                        else
                            dupl = start + 1;
            }
        }
    return;
}

/* The triggers.. These are really basic, and since most appear only
 * once in the code (hmm. i think they all do) it would be more efficient
 * to substitute the code in and make the mprog_xxx_check routines global.
 * However, they are all here in one nice place at the moment to make it
 * easier to see what they look like. If you do substitute them back in,
 * make sure you remember to modify the variable names to the ones in the
 * trigger calls.
 */
void mprog_act_trigger( char *txt, CHAR_DATA * actor )
{
    CHAR_DATA *vmob;

    for ( vmob = actor->in_room->people; vmob; vmob = vmob->next_in_room )
        if ( IS_NPC( vmob ) && ( vmob->pIndexData->progtypes & ACT_PROG ) )
        {
            if ( IS_NPC( actor ) && ( vmob->pIndexData == actor->pIndexData ) )
                continue;

            ProgSource = vmob;
            ProgTriggeredBy = actor;

            mprog_wordlist_check( txt, ACT_PROG, MOB_PROG );
        }

    return;
}

void mprog_bribe_trigger( CHAR_DATA * mob, CHAR_DATA * ch, int amount )
{
    char buf[MAX_STRING_LENGTH];
    MPROG_LIST *pList;
    OBJ_DATA *obj;

    if ( IS_NPC( mob ) && ( mob->pIndexData->progtypes & BRIBE_PROG ) )
    {
        if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
            return;
        obj = create_object( get_obj_index( OBJ_VNUM_MONEY_SOME ), 0 );
        sprintf( buf, obj->short_descr, amount );
        free_string( &obj->short_descr );
        obj->short_descr = str_dup( buf );
        obj->value[0] = amount;
        obj_to_char( obj, mob );
        mob->gold -= amount;

        for ( pList = mob->pIndexData->mudprogs; pList; pList = pList->next )
            if ( ( pList->mudprog->trigger_type & BRIBE_PROG )
                 && ( amount >= atoi( pList->mudprog->arglist ) ) )
            {
                ProgSource = mob;
                ProgTriggeredBy = ch;
                ProgObjectSource = obj;

                mprog_driver( pList->mudprog->comlist );
                break;
            }
    }

    return;
}

void mprog_death_trigger( CHAR_DATA * mob )
{

    if ( IS_NPC( mob ) && ( mob->pIndexData->progtypes & DEATH_PROG ) )
    {
        mob->position = POS_RESTING;

        ProgSource = mob;
        mprog_percent_check( DEATH_PROG );
        mob->position = POS_DEAD;
    }
    else
    {
        death_cry( mob );
    }
    return;
}

void mprog_entry_trigger( CHAR_DATA * mob )
{

    if ( IS_NPC( mob ) && ( mob->pIndexData->progtypes & ENTRY_PROG ) )
    {
        ProgSource = mob;
        mprog_percent_check( ENTRY_PROG );
    }

    return;

}

void mprog_fight_trigger( CHAR_DATA * mob, CHAR_DATA * ch )
{

    if ( IS_NPC( mob ) && ( mob->pIndexData->progtypes & FIGHT_PROG ) )
    {
        if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
            return;

        ProgSource = mob;
        ProgTriggeredBy = ch;
        mprog_percent_check( FIGHT_PROG );
    }

    return;

}

void mprog_fightgroup_trigger( CHAR_DATA * mob )
{
    CHAR_DATA *victim;

    if ( !mob || !mob->in_room )
        return;

    if ( IS_NPC( mob ) && ( mob->pIndexData->progtypes & FIGHTGROUP_PROG ) )
    {
        ProgSource = mob;

        for ( victim = mob->in_room->people; victim;
              victim = victim->next_in_room )
        {
            if ( victim != mob && victim->fighting == mob )
            {
                ProgTriggeredBy = victim;
                if ( mprog_percent_check( FIGHTGROUP_PROG ) )
                    break;
            }
        }
    }

    return;

}

void mprog_give_trigger( CHAR_DATA * mob, CHAR_DATA * ch, OBJ_DATA * obj )
{
    char buf[MAX_INPUT_LENGTH];
    MPROG_LIST *pList;

    if ( IS_NPC( mob ) && ( mob->pIndexData->progtypes & GIVE_PROG ) )
    {
        if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
            return;

        for ( pList = mob->pIndexData->mudprogs; pList; pList = pList->next )
        {
            one_argument( pList->mudprog->arglist, buf );

            if ( ( pList->mudprog->trigger_type & GIVE_PROG )
                 && ( ( !str_cmp( obj->name, pList->mudprog->arglist ) )
                      || ( !str_cmp( "all", buf ) ) ) )
            {
                ProgSource = mob;
                ProgTriggeredBy = ch;
                ProgObjectSource = obj;

                mprog_driver( pList->mudprog->comlist );
                break;
            }
        }
    }

    return;
}

void mprog_greet_trigger( CHAR_DATA * ch )
{
    CHAR_DATA *vmob;

    if ( IS_NPC( ch ) )
        return;

    for ( vmob = ch->in_room->people; vmob; vmob = vmob->next_in_room )
        if ( IS_NPC( vmob ) && ch != vmob && can_see( vmob, ch )
             && ( !vmob->fighting ) && IS_AWAKE( vmob )
             && ( vmob->pIndexData->progtypes & GREET_PROG ) )
        {
            ProgSource = vmob;
            ProgTriggeredBy = ch;

            mprog_percent_check( GREET_PROG );
        }
        else if ( IS_NPC( vmob )
                  && ( !vmob->fighting )
                  && IS_AWAKE( vmob )
                  && ( vmob->pIndexData->progtypes & ALL_GREET_PROG ) )
        {
            ProgSource = vmob;
            ProgTriggeredBy = ch;

            mprog_percent_check( ALL_GREET_PROG );
        }

    return;
}

void mprog_hitprcnt_trigger( CHAR_DATA * mob, CHAR_DATA * ch )
{
    MPROG_LIST *pList;

    if ( IS_NPC( mob ) && ( mob->pIndexData->progtypes & HITPRCNT_PROG ) )
    {
        if ( IS_NPC( ch ) && ch->pIndexData == mob->pIndexData )
            return;

        for ( pList = mob->pIndexData->mudprogs; pList; pList = pList->next )
            if ( ( pList->mudprog->trigger_type & HITPRCNT_PROG )
                 && ( ( 100 * mob->hit / mob->max_hit ) <
                      atoi( pList->mudprog->arglist ) ) )
            {
                ProgSource = mob;
                ProgTriggeredBy = ch;

                mprog_driver( pList->mudprog->comlist );
                break;
            }
    }

    return;
}

void mprog_random_trigger( CHAR_DATA * mob )
{
    if ( mob->pIndexData->progtypes & RAND_PROG )
    {
        ProgSource = mob;

        mprog_percent_check( RAND_PROG );
    }

    return;
}

void mprog_speech_trigger( char *txt, CHAR_DATA * actor )
{
    CHAR_DATA *vmob;

    for ( vmob = actor->in_room->people; vmob; vmob = vmob->next_in_room )
        if ( IS_NPC( vmob ) && ( vmob->pIndexData->progtypes & SPEECH_PROG ) )
        {
            if ( IS_NPC( actor ) && ( vmob->pIndexData == actor->pIndexData ) )
                continue;

            ProgSource = vmob;
            ProgTriggeredBy = actor;

            mprog_wordlist_check( txt, SPEECH_PROG, MOB_PROG );
        }

    return;
}

/* Written by Zak, Jan 13/1998
 * Goes in interpret() before regular command parsing.
 * This function checks all mobs in room with the character for a command
 * trigger matching what the character typed.
 * The function returns TRUE if the character's command should be run
 * through the regular interpreter too. */
bool mprog_command_trigger( char *txt, CHAR_DATA * ch, char *extra )
{
    char *argument;
    char arg[MAX_INPUT_LENGTH]; /* I'm assuming that the cmd_table won't
                                 * ever have a command longer than
                                 * MAX_INPUT_LENGTH                     */
    char *pMem;                 /* Pointer to the memory allocated by argument.
                                 * This is needed so we can free that memory up
                                 * at the end of the function */
    CHAR_DATA *vmob;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *room;
    MPROG_LIST *pList;
    int cmd;
    bool can_do = TRUE;

    pMem = strdup( txt );
    argument = pMem;

    /* First check for the command typed being a prefix of anything in 
     * the regular command table, and expand it to the full command. */

    argument = one_argument( argument, arg );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( arg[0] == cmd_table[cmd].name[0]
             && !str_prefix( arg, cmd_table[cmd].name ) )
        {
            strncpy( arg, cmd_table[cmd].name, sizeof( arg ) );
            break;
        }
    }

    /* Find mobs that are in_room that have a COMMAND_PROG */

    for ( vmob = ch->in_room->people; vmob; vmob = vmob->next_in_room )
        if ( IS_NPC( vmob ) && ( vmob->pIndexData->progtypes & COMMAND_PROG ) )
        {
            for ( pList = vmob->pIndexData->mudprogs; pList;
                  pList = pList->next )
            {
                if ( ( pList->mudprog->trigger_type & COMMAND_PROG )
                     && ( !str_cmp( pList->mudprog->arglist, arg ) ) )
                {
                    ProgSource = vmob;
                    ProgTriggeredBy = ch;
                    ProgExtraArgs = extra;

                    mprog_driver( pList->mudprog->comlist );
                    can_do = FALSE;
                }
            }
        }

    /* Obj and Room command_prog support by Kyle Boyd */

    /* objs in inventory and worn... */
    for ( obj = ch->carrying; obj; obj = obj->next_content )
        if ( obj->pIndexData->progtypes & COMMAND_PROG )
        {
            for ( pList = obj->pIndexData->mudprogs; pList;
                  pList = pList->next )
            {
                if ( ( pList->mudprog->trigger_type & COMMAND_PROG )
                     && ( !str_cmp( pList->mudprog->arglist, arg ) ) )
                {
                    set_supermob( obj, OBJ_PROG );
                    ProgSource = supermob;
                    ProgTriggeredBy = ch;
                    ProgObjectSource = obj;
                    ProgExtraArgs = extra;

                    mprog_driver( pList->mudprog->comlist );

                    release_supermob(  );
                    can_do = FALSE;
                }
            }
        }

    /* objs in room... */

    for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
        if ( obj->pIndexData->progtypes & COMMAND_PROG )
        {
            for ( pList = obj->pIndexData->mudprogs; pList;
                  pList = pList->next )
            {
                if ( ( pList->mudprog->trigger_type & COMMAND_PROG )
                     && ( !str_cmp( pList->mudprog->arglist, arg ) ) )
                {
                    set_supermob( obj, OBJ_PROG );
                    ProgSource = supermob;
                    ProgTriggeredBy = ch;
                    ProgObjectSource = obj;
                    ProgExtraArgs = extra;

                    mprog_driver( pList->mudprog->comlist );

                    release_supermob(  );
                    can_do = FALSE;
                }
            }
        }

    /* And finally, room progs */

    room = ch->in_room;
    if ( room->progtypes & COMMAND_PROG )
    {
        for ( pList = room->mudprogs; pList; pList = pList->next )
        {
            if ( ( pList->mudprog->trigger_type & COMMAND_PROG )
                 && ( !str_cmp( pList->mudprog->arglist, arg ) ) )
            {
                set_supermob( room, ROOM_PROG );
                ProgSource = supermob;
                ProgTriggeredBy = ch;
                ProgExtraArgs = extra;

                mprog_driver( pList->mudprog->comlist );

                release_supermob(  );
                can_do = FALSE;
            }
        }
    }

    free( pMem );

    return can_do;
}

/*****************************************************************
 *   ROOM PROG SUPPORT STARTS HERE:                              *
 *          most of this code was taken from the SMAUG code base *
 *          with modifications made to fit our mud...            *
 *****************************************************************/

/*
 * Triggers follow
 */
void rprog_act_trigger( char *txt, CHAR_DATA * ch )
{
    /* prevent circular triggers by not allowing mob to trigger itself */
    if ( IS_NPC( ch ) && ch->pIndexData == supermob->pIndexData )
        return;

    if ( ch->in_room && ch->in_room->progtypes & ACT_PROG )
    {
        ProgRoomSource = ch->in_room;
        ProgTriggeredBy = ch;

        /* supermob is set and released in mprog_wordlist_check */
        mprog_wordlist_check( txt, ACT_PROG, ROOM_PROG );
    }
    return;
}

void rprog_leave_trigger( CHAR_DATA * ch )
{
    if ( ch->in_room && ch->in_room->progtypes & LEAVE_PROG )
    {
        set_supermob( ch->in_room, ROOM_PROG );
        ProgSource = supermob;

        mprog_percent_check( LEAVE_PROG );

        release_supermob(  );
    }
    return;
}

void rprog_enter_trigger( CHAR_DATA * ch )
{
    if ( ch->in_room && ch->in_room->progtypes & ENTER_PROG )
    {
        set_supermob( ch->in_room, ROOM_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;

        mprog_percent_check( ENTRY_PROG );

        release_supermob(  );
    }
    return;
}

void rprog_sleep_trigger( CHAR_DATA * ch )
{
    if ( ch->in_room && ch->in_room->progtypes & SLEEP_PROG )
    {
        set_supermob( ch->in_room, ROOM_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;

        mprog_percent_check( SLEEP_PROG );

        release_supermob(  );
    }
    return;
}

void rprog_rest_trigger( CHAR_DATA * ch )
{
    if ( ch->in_room && ch->in_room->progtypes & REST_PROG )
    {
        set_supermob( ch->in_room, ROOM_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;

        mprog_percent_check( REST_PROG );

        release_supermob(  );
    }
    return;
}

void rprog_rfight_trigger( CHAR_DATA * ch )
{
    if ( ch->in_room && ch->in_room->progtypes & RFIGHT_PROG )
    {
        set_supermob( ch->in_room, ROOM_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;

        mprog_percent_check( RFIGHT_PROG );

        release_supermob(  );
    }
    return;
}

void rprog_death_trigger( CHAR_DATA * ch )
{
    if ( ch->in_room && ch->in_room->progtypes & RDEATH_PROG )
    {
        set_supermob( ch->in_room, ROOM_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;

        mprog_percent_check( RDEATH_PROG );

        release_supermob(  );
    }
    return;
}

void rprog_speech_trigger( char *txt, CHAR_DATA * ch )
{
    /* prevent circular triggers by not allowing mob to trigger itself */
    if ( IS_NPC( ch ) && ch->pIndexData == supermob->pIndexData )
        return;

    if ( ch->in_room && ch->in_room->progtypes & SPEECH_PROG )
    {
        ProgRoomSource = ch->in_room;
        ProgTriggeredBy = ch;

        /* supermob is set and released in mprog_wordlist_check */
        mprog_wordlist_check( txt, SPEECH_PROG, ROOM_PROG );
    }
    return;
}

void rprog_random_trigger( CHAR_DATA * ch )
{

    if ( ch->in_room && ch->in_room->progtypes & RAND_PROG )
    {
        set_supermob( ch->in_room, ROOM_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;

        mprog_percent_check( RAND_PROG );

        release_supermob(  );
    }
    return;
}

/*****************************************************************
 *   OBJECT PROG SUPPORT STARTS HERE:				             *
 *          most of this code was taken from the SMAUG code base *
 *          with modifications made to fit our mud...            *
 *****************************************************************/

/*
 * Triggers follow
 */
void oprog_greet_trigger( CHAR_DATA * ch )
{
    OBJ_DATA *vobj;

    for ( vobj = ch->in_room->contents; vobj; vobj = vobj->next_content )
        if ( vobj->pIndexData->progtypes & GREET_PROG )
        {
            set_supermob( vobj, OBJ_PROG ); /* not very efficient to do here */
            ProgSource = supermob;
            ProgTriggeredBy = ch;
            ProgObjectSource = vobj;

            mprog_percent_check( GREET_PROG );

            release_supermob(  );
        }

    return;
}

void oprog_speech_trigger( char *txt, CHAR_DATA * ch )
{
    OBJ_DATA *vobj;

    /* supermob is set and released in mprog_wordlist_check */
    for ( vobj = ch->in_room->contents; vobj; vobj = vobj->next_content )
        if ( vobj->pIndexData->progtypes & SPEECH_PROG )
        {
            ProgTriggeredBy = ch;
            ProgObjectSource = vobj;

            mprog_wordlist_check( txt, SPEECH_PROG, OBJ_PROG );
        }

    return;
}

/*
 * Called at top of obj_update
 * make sure to put an if(!obj) continue
 * after it
 */
void oprog_random_trigger( OBJ_DATA * obj )
{

    if ( obj->pIndexData->progtypes & RAND_PROG )
    {
        set_supermob( obj, OBJ_PROG );
        ProgSource = supermob;
        ProgObjectSource = obj;

        mprog_percent_check( RAND_PROG );

        release_supermob(  );
    }
    return;
}

/*
 * in wear_obj, between each successful equip_char 
 * the subsequent return
 */
void oprog_wear_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
    if ( obj->pIndexData->progtypes & WEAR_PROG )
    {
        set_supermob( obj, OBJ_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;
        ProgObjectSource = obj;

        mprog_percent_check( WEAR_PROG );

        release_supermob(  );
    }
    return;
}

bool oprog_use_trigger( CHAR_DATA * ch, OBJ_DATA * obj, CHAR_DATA * vict,
                        OBJ_DATA * targ )
{
    bool executed = FALSE;

    if ( obj->pIndexData->progtypes & USE_PROG )
    {
        set_supermob( obj, OBJ_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;
        ProgObjectSource = obj;

        if ( obj->item_type == ITEM_STAFF && vict )
            ProgVictim = vict;
        else
            ProgObjectVictim = targ;

        executed = mprog_percent_check( USE_PROG );

        release_supermob(  );
    }
    return executed;
}

/*
 * call in remove_obj, right after unequip_char   
 * do a if(!ch) return right after, and return TRUE (?)
 * if !ch
 */
void oprog_remove_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
    if ( obj->pIndexData->progtypes & REMOVE_PROG )
    {
        set_supermob( obj, OBJ_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;
        ProgObjectSource = obj;

        mprog_percent_check( REMOVE_PROG );

        release_supermob(  );
    }
    return;
}

/*
 * call in do_sac, right before extract_obj
 */
void oprog_sac_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
    if ( obj->pIndexData->progtypes & SAC_PROG )
    {
        set_supermob( obj, OBJ_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;
        ProgObjectSource = obj;

        mprog_percent_check( SAC_PROG );

        release_supermob(  );
    }
    return;
}

/*
 * call in do_get, right before check_for_trap
 * do a if(!ch) return right after
 */
void oprog_get_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
    if ( obj->pIndexData->progtypes & GET_PROG )
    {
        set_supermob( obj, OBJ_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;
        ProgObjectSource = obj;

        mprog_percent_check( GET_PROG );

        release_supermob(  );
    }
    return;
}

/*
 * called in damage_obj in act_obj.c
 */
void oprog_damage_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
    if ( obj->pIndexData->progtypes & DAMAGE_PROG )
    {
        set_supermob( obj, OBJ_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;
        ProgObjectSource = obj;

        mprog_percent_check( DAMAGE_PROG );

        release_supermob(  );
    }
    return;
}

/*
 * called in do_repair in shops.c
 */
void oprog_repair_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{

    if ( obj->pIndexData->progtypes & REPAIR_PROG )
    {
        set_supermob( obj, OBJ_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;
        ProgObjectSource = obj;

        mprog_percent_check( REPAIR_PROG );

        release_supermob(  );
    }
    return;
}

/*
 * call twice in do_drop, right after the act( AT_ACTION,...)
 * do a if(!ch) return right after
 */
void oprog_drop_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
    if ( obj->pIndexData->progtypes & DROP_PROG )
    {
        set_supermob( obj, OBJ_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;
        ProgObjectSource = obj;

        mprog_percent_check( DROP_PROG );

        release_supermob(  );
    }
    return;
}

/*
 * call towards end of do_examine, right before check_for_trap
 */
void oprog_examine_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
    if ( obj->pIndexData->progtypes & EXA_PROG )
    {
        set_supermob( obj, OBJ_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;
        ProgObjectSource = obj;

        mprog_percent_check( EXA_PROG );

        release_supermob(  );
    }
    return;
}

/*
 * call in fight.c, group_gain, after (?) the obj_to_room
 */
void oprog_zap_trigger( CHAR_DATA * ch, OBJ_DATA * obj )
{
    if ( obj->pIndexData->progtypes & ZAP_PROG )
    {
        set_supermob( obj, OBJ_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;
        ProgObjectSource = obj;

        mprog_percent_check( ZAP_PROG );

        release_supermob(  );
    }
    return;
}

void oprog_act_trigger( char *txt, CHAR_DATA * ch )
{
    OBJ_DATA *vobj;

    /* supermob is set and released in mprog_wordlist_check */
    for ( vobj = ch->in_room->contents; vobj; vobj = vobj->next_content )
        if ( vobj->pIndexData->progtypes & ACT_PROG )
        {
            ProgTriggeredBy = ch;
            ProgObjectSource = vobj;

            mprog_wordlist_check( txt, ACT_PROG, OBJ_PROG );
        }

    return;
}

/*
 * call in fight.c, one_hit at the end.
 */
void oprog_hit_trigger( CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj )
{
    if ( obj->pIndexData->progtypes & HIT_PROG )
    {
        set_supermob( obj, OBJ_PROG );
        ProgSource = supermob;
        ProgTriggeredBy = ch;
        ProgObjectSource = obj;
        ProgVictim = victim;

        mprog_percent_check( HIT_PROG );

        release_supermob(  );
    }
    return;
}

/************************************************************************
 *                                                                      *
 *  EVAL.C - A simple mathematical expression evaluator in C            *
 *                                                                      *
 *  operators supported: Operator               Precedence              *
 *                                                                      *
 *                         (                     Lowest                 *
 *                         )                     Highest                *
 *                         +   (addition)        Med-Low                *
 *                         -   (subtraction)     Med-Low                *
 *                         *   (multiplication)  Medium                 *
 *                         /   (division)        Medium                 *
 *                         %   (modulus)         High                   *
 *                         ^   (exponentiation)  High                   *
 *                         ==					 Low                    *
 *                         !=					 Low                    *
 *                         >=					 Low                    *
 *                         <=                    Low                    *
 *                         <                     Low                    *
 *                         >                     Low                    *
 *                         &&                    Lowest                 *
 *                         ||                    Lowest                 *
 *                         !                     Low                    *
 *                                                                      *
 *  Original Copyright 1991-93 by Robert B. Stout as part of            *
 *  the MicroFirm Function Library (MFL)                                *
 *                                                                      *
 *  The user is granted a free limited license to use this source file  *
 *  to create royalty-free programs, subject to the terms of the        *
 *  license restrictions specified below.                               *
 *                                                                      *
 ************************************************************************/

/***********************************************************************************
 *                                                                                 *
 * Portions of SNIPPETS code are Copyright 1987-1996 by Robert B. Stout dba        *
 * MicroFirm. The user is granted a free license to use these source files in      *
 * any way except for commercial publication other than as part of your own        *
 * program. This means you are explicitly granted the right to:                    *
 *                                                                                 *
 * 1.  Use these files to create applications for any use, private or commercial,  *
 *     without any license fee.                                                    *
 *                                                                                 *
 * 2.  Copy or otherwise publish copies of the source files so long as the         *
 *     original copyright notice is not removed and that such publication is       *
 *     free of any charges other than the costs of duplication and distribution.   *
 *                                                                                 *
 * 3.  Distribute modified versions of the source files so long as the original    *
 *     copyright notice is not removed, and that the modified nature is clearly    *
 *     noted in the source file, and that such distribution is free of any         *
 *     charges other than the costs of duplication and distribution.               *
 *                                                                                 *
 * 4.  Distribute object files and/or libraries compiled from the supplied         *
 *     source files, provided that such publication is free of any charges other   *
 *     than the costs of duplication and distribution.                             *
 *                                                                                 *
 * Rights reserved to the copyright holder include:                                *
 *                                                                                 *
 * 1.  The right to publish these works commercially including, but not limited    *
 *     to, books, magazines, and commercial software libraries.                    *
 *                                                                                 *
 * 2.  The commercial rights, as cited above, to all derivative source and/or      *
 *     object modules. Note that this explicitly grants to the user all rights,    *
 *     commercial or otherwise, to any and all executable programs created using   *
 *     MicroFirm copyrighted source or object files contained in SNIPPETS.         *
 *                                                                                 *
 * Users are specifically prohibited from:                                         *
 *                                                                                 *
 * 1.  Removing the copyright notice and claiming the source files or any          *
 *     derivative works as their own.                                              *
 *                                                                                 *
 * 2.  Publishing the source files, the compiled object files, or libraries of     *
 *     the compiled object files commercially.                                     *
 *                                                                                 *
 * In other words, you are free to use these files to make programs, not to make   *
 * money! The programs you make with them, you may sell or give away as you        *
 * like, but you many not sell either the source files or object files, alone      *
 * or in a library, even if you've modified the original source, without a         *
 * license from the copyright holder.                                              *
 *                                                                                 *
 *                                                                                 *
 * Bob Stout                                                                       *
 * FidoNet:    1:106/2000.6                                                        *
 *             C_Echo moderator (1990-1991, 1996-1997)                             *
 * Internet:   rbs@snippets.org                                                    *
 *             rbs@brokersys.com                                                   *
 * SnailMail:  MicroFirm                                                           *
 *             P.O. Box 428                                                        *
 *             Alief, TX 77411                                                     *
 *                                                                                 *
 ***********************************************************************************/

struct operator_type {
    char token;
    char *tag;
    size_t taglen;
    int precedence;
};

static struct operator_type verbs[] = {
    {'+', "+", 1, 2},
    {'-', "-", 1, 3},
    {'*', "*", 1, 4},
    {'/', "/", 1, 5},
    {'%', "%", 1, 5},
    {'^', "^", 1, 6},
    {'(', "(", 1, 0},
    {')', ")", 1, 99},
    {'=', "==", 2, 1},
    {'\\', "!=", 2, 1},
    {'G', "<=", 2, 1},
    {'L', ">=", 2, 1},
    {'<', "<", 1, 1},
    {'>', ">", 1, 1},
    {'&', "&&", 2, 0},
    {'|', "||", 2, 0},
    {'!', "!", 1, 1},
    {0, 0, 0, 0}
};

static char op_stack[256];      /* Operator stack       */
static double arg_stack[256];   /* Argument stack       */
static char token[256];         /* Token buffer         */
static int op_sptr,             /* op_stack pointer     */
 arg_sptr,                      /* arg_stack pointer    */
 parens,                        /* Nesting level        */
 state;                         /* 0 = Awaiting expression
                                   1 = Awaiting operator
                                 */
static int do_op( void );
static int do_paren( void );
static void push_op( char );
static void push_arg( double );
static int pop_arg( double * );
static int pop_op( int * );
static char *get_exp( char * );
static struct operator_type *get_op( char * );
static int getprec( char );
static int getTOSprec( void );

/************************************************************************/
/*                                                                      */
/*  evaluate()                                                          */
/*                                                                      */
/*  Evaluates an ASCII mathematical expression.                         */
/*                                                                      */
/*  Arguments: 1 - String to evaluate                                   */
/*             2 - Storage to receive double result                     */
/*                                                                      */
/*  Returns: Success_ if successful                                     */
/*           Error_ if syntax error                                     */
/*           R_ERROR if runtime error                                   */
/*                                                                      */
/*  Side effects: Removes all whitespace from the string and converts   */
/*                it to U.C.                                            */
/*                                                                      */
/************************************************************************/

int evaluate( char *line, double *val )
{
    double arg;
    char *ptr = line, *str, *endptr;
    int ercode;
    struct operator_type *op;

    str_upr( line );
    state = op_sptr = arg_sptr = parens = 0;

    while ( *ptr )
    {
        switch ( state )
        {
        case 0:
            if ( NULL != ( str = get_exp( ptr ) ) )
            {
                if ( NULL != ( op = get_op( str ) ) &&
                     strlen( str ) == op->taglen )
                {
                    push_op( op->token );
                    ptr += op->taglen;
                    break;
                }

                if ( Success_ == strcmp( str, "-" ) )
                {
                    push_op( *str );
                    ++ptr;
                    break;
                }

                else
                {
                    if ( 0.0 == ( arg = strtod( str, &endptr ) ) &&
                         NULL == strchr( str, '0' ) )
                    {
                        return Error_;
                    }
                    push_arg( arg );
                }
                ptr += strlen( str );
            }
            else
                return Error_;

            state = 1;
            break;

        case 1:
            if ( NULL != ( op = get_op( ptr ) ) )
            {
                if ( ')' == *ptr )
                {
                    if ( Success_ > ( ercode = do_paren(  ) ) )
                        return ercode;
                }
                else
                {
                    while ( op_sptr && op->precedence <= getTOSprec(  ) )
                    {
                        do_op(  );
                    }
                    push_op( op->token );
                    state = 0;
                }

                ptr += op->taglen;
            }
            else
                return Error_;

            break;
        }
    }

    while ( 1 < arg_sptr )
    {
        if ( Success_ > ( ercode = do_op(  ) ) )
            return ercode;
    }
    if ( !op_sptr )
        return pop_arg( val );
    else if ( op_sptr == 1 && op_stack[0] == '!' )
    {
        if ( Success_ > ( ercode = do_op(  ) ) )
            return ercode;
        else
        {
            pop_arg( val );
            return Success_;
        }
    }
    else
        return Error_;
}

/*
**  Evaluate stacked arguments and operands
*/

static int do_op( void )
{
    double arg1, arg2;
    int op;

    if ( Error_ == pop_op( &op ) )
        return Error_;

    pop_arg( &arg1 );
    pop_arg( &arg2 );

    switch ( op )
    {
    case '+':
        push_arg( arg2 + arg1 );
        break;

    case '-':
        push_arg( arg2 - arg1 );
        break;

    case '*':
        push_arg( arg2 * arg1 );
        break;

    case '/':
        if ( 0.0 == arg1 )
            return R_ERROR;
        push_arg( arg2 / arg1 );
        break;

    case '%':
        if ( 0.0 == arg1 )
            return R_ERROR;
        push_arg( fmod( arg2, arg1 ) );
        break;

    case '^':
        push_arg( pow( arg2, arg1 ) );
        break;

    case 'G':
        push_arg( arg2 <= arg1 );
        break;

    case 'L':
        push_arg( arg2 >= arg1 );
        break;

    case '=':
        push_arg( arg2 == arg1 );
        break;

    case '\\':
        push_arg( arg2 != arg1 );
        break;

    case '<':
        push_arg( arg2 < arg1 );
        break;

    case '>':
        push_arg( arg2 > arg1 );
        break;

    case '&':
        push_arg( arg2 && arg1 );
        break;

    case '|':
        push_arg( arg2 || arg1 );
        break;

    case '!':
        arg_sptr++;
        push_arg( !arg1 );
        break;

    case '(':
        arg_sptr += 2;
        break;

    default:
        return Error_;
    }
    if ( 1 > arg_sptr )
        return Error_;
    else
        return op;
}

/*
**  Evaluate one level
*/

static int do_paren( void )
{
    int op;

    if ( 1 > parens-- )
        return Error_;
    do
    {
        if ( Success_ > ( op = do_op(  ) ) )
            break;
    }
    while ( getprec( ( char ) op ) );
    return op;
}

/*
**  Stack operations
*/

static void push_op( char op )
{
    if ( !getprec( op ) )
        ++parens;
    op_stack[op_sptr++] = op;
}

static void push_arg( double arg )
{
    arg_stack[arg_sptr++] = arg;
}

static int pop_arg( double *arg )
{
    *arg = arg_stack[--arg_sptr];
    if ( 0 > arg_sptr )
        return Error_;
    else
        return Success_;
}

static int pop_op( int *op )
{
    if ( !op_sptr && op_stack[0] != '!' )
        return Error_;
    *op = op_stack[--op_sptr];
    return Success_;
}

/*
**  Get an expression
*/

static char *get_exp( char *str )
{
    char *ptr = str, *tptr = token;
    struct operator_type *op;

    while ( *ptr )
    {
        if ( NULL != ( op = get_op( ptr ) ) )
        {
            if ( '-' == *ptr )
            {
                if ( str == ptr && !isdigit( ptr[1] ) && '.' != ptr[1] )
                {
                    push_arg( 0.0 );
                    strcpy( token, op->tag );
                    return token;
                }
            }

            else if ( str == ptr )
            {
                strcpy( token, op->tag );
                return token;
            }

            else
                break;
        }

        *tptr++ = *ptr++;
    }
    *tptr = 0;

    return token;
}

/*
**  Get an operator
*/

static struct operator_type *get_op( char *str )
{
    struct operator_type *op;

    for ( op = verbs; op->token; ++op )
    {
        if ( !strncmp( str, op->tag, op->taglen ) )
            return op;
    }
    return NULL;
}

/*
**  Get precedence of a token
*/

static int getprec( char token )
{
    struct operator_type *op;

    for ( op = verbs; op->token; ++op )
    {
        if ( token == op->token )
            break;
    }
    if ( op->token )
        return op->precedence;
    else
        return 0;
}

/*
**  Get precedence of TOS token
*/

static int getTOSprec( void )
{
    if ( !op_sptr )
        return 0;
    return getprec( op_stack[op_sptr - 1] );
}
