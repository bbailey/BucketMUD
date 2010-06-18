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
 * File:  Clan.c                                                           *
 * Written by: Kyle Boyd                                                   *
 * Partially inspired by some code written by Zane(E.J. Wilburn), as well  *
 * as clan code from SMAUG(?) and ROM muds.  None of what you see here was *
 * directly copied from those versions of clan code, just some of the      *
 * ideas were used.                                                        *
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
#include <stdarg.h>
#include "merc.h"
#include "olc.h"
DECLARE_DO_FUN( do_look );
DECLARE_DO_FUN( do_save );

#define CEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )

const struct olc_cmd_type cedit_table[] = {
/*  {   command		    function	    }, */
    {"create", cedit_create},
    {"leader", cedit_leader},
    {"sponsor", cedit_sponsor},
    {"name", cedit_name},
    {"whoname", cedit_whoname},
    {"show", cedit_show},
    {"minlevel", cedit_minlevel},
    {"members", cedit_members},
    {"recall", cedit_recall},
    {"joinflags", cedit_joinflags},
    {"clanflags", cedit_clanflags},
/* To be implemented later... */
/* These will add clan-specific skills and commands. */
/* 
    {   "skill",		cedit_skill     },
*/
    {"rank", cedit_rank},
    {"cost", cedit_cost},
    {"?", show_help},
    {"version", show_version},
    {"commands", show_commands},
    {"", 0,}
};

void save_clans( void )
{
    FILE *fp;
    CLAN_DATA *clan;
    char buf[MAX_INPUT_LENGTH];
    int i;
    if ( clan_first == NULL )
        return;

    sprintf( buf, "%s/%s", sysconfig.area_dir, sysconfig.clans_file );
    fp = fopen( buf, "w" );
    if ( fp == NULL )
    {
        bug( "Cannot write to %s !!!", sysconfig.clans_file );
        return;
    }

    fprintf( fp, "#CLANS\n" );
    for ( clan = clan_first; clan != NULL; clan = clan->next )
    {
        fprintf( fp, "%d\n", clan->number );
        fprintf( fp, "%s~\n", clan->name );
        fprintf( fp, "%s~\n", clan->whoname );
        fprintf( fp, "%s~\n", clan->leader );
        fprintf( fp, "%s~\n", clan->god );
        fprintf( fp, "%d\n", clan->max_members );
        fprintf( fp, "%d\n", clan->min_level );
        fprintf( fp, "%d\n", clan->num_members );
        buf[0] = '\0';
        fprintf( fp, "%s\n", fwrite_flag( clan->clan_flags, buf ) );
        buf[0] = '\0';
        fprintf( fp, "%s\n", fwrite_flag( clan->join_flags, buf ) );
        fprintf( fp, "%d\n", clan->cost_gold );
        fprintf( fp, "%d\n", clan->recall_room );
        fprintf( fp, "%d\n", clan->clan_fund );
        /* The reason that all of these fprintf functions have a letter to start off 
           with is in case the MAX_* change, so the lines will be identifyable as what
           they are, since the number of them may change.
           Note: if you DECREASE the MAX_*'s, you'll need to edit the clans file and 
           remove the excess lines.  -Kyle */
        for ( i = 0; i < MAX_CLAN_MEMBERS; i++ )
        {
            if ( clan->members[i][0] != '\0' )
                fprintf( fp, "M %d %s~\n", i + 1, clan->members[i] );
        }
        for ( i = 0; i < MAX_CLAN; i++ )
        {
            if ( clan->kills[i] != 0 )
                fprintf( fp, "K %d %d\n", i + 1, clan->kills[i] );
        }
        for ( i = 0; i < MAX_CLAN; i++ )
        {
            if ( clan->deaths[i] != 0 )
                fprintf( fp, "D %d %d\n", i + 1, clan->deaths[i] );
        }
        for ( i = 0; i < MAX_RANK; i++ )
        {
            fprintf( fp, "R %d %s~\n", i + 1, clan->rank[i] );
        }
        fprintf( fp, "E\n" );   /* End of this clan. */
    }
    fprintf( fp, "999\n#$\n#$\n" ); /* I hope you don't make more than 998 clans... :) */
    fclose( fp );

    return;
}

/* Read in str as the next string of fp.  
Free it if there's a string there already. 
	-Kyle */
#define GETSTR( str )  if ( str != NULL ) \
				{  \
				 free_string( &str ); \
				 str = fread_string( fp ); \
				}	\
				else	\
				str = fread_string( fp );

void load_clans( FILE * fp )
{
    int i, x, num;
    CLAN_DATA *clan;
    bool done;

    for ( ;; )
    {
        num = fread_number( fp );
        if ( num == 999 )
            break;
        else
            clan = new_clan(  );
        if ( num != clan->number )
        {
            bug( "%s clan #%d misaligned.", sysconfig.clans_file, num );
            return;
        }
        GETSTR( clan->name );
        GETSTR( clan->whoname );
        GETSTR( clan->leader );
        GETSTR( clan->god );
        clan->max_members = fread_number( fp );
        clan->min_level = fread_number( fp );
        clan->num_members = fread_number( fp );
        clan->clan_flags = fread_flag( fp );
        clan->join_flags = fread_flag( fp );
        clan->cost_gold = fread_number( fp );
        clan->recall_room = fread_number( fp );
        clan->clan_fund = fread_number( fp );
        done = FALSE;
        for ( ; done == FALSE; )
        {
            switch ( fread_letter( fp ) )
            {
            default:
                bug( "Bad letter in %s", sysconfig.clans_file );
                break;
            case 'M':
                i = fread_number( fp );
                GETSTR( clan->members[i - 1] );
                {
                    for ( x = 0; x < MAX_CLAN_MEMBERS; x++ )
                    {
                        if ( x == ( i - 1 ) )
                            continue;
                        if ( !str_cmp
                             ( clan->members[i - 1], clan->members[x] ) )
                        {
                            free_string( &clan->members[i - 1] );
                            clan->members[i - 1] = str_dup( "" );
                        }
                    }
                }
                break;
            case 'K':
                i = fread_number( fp );
                clan->kills[i - 1] = fread_number( fp );
                break;
            case 'D':
                i = fread_number( fp );
                clan->deaths[i - 1] = fread_number( fp );
                break;
            case 'R':
                i = fread_number( fp );
                GETSTR( clan->rank[i - 1] );
                break;
            case 'E':
                done = TRUE;
                break;
            }
        }
    }
    return;
}

/*
 * This code written by Rahl (dwa1844@rit.edu) for Broken Shadows.
 *
 * Permission to use this code is given freely as long as my
 * name stays with it.
 * - Daniel Anderson
 */
/* This was originally a new "log_string" function by Rahl. 
   I changed the name of the file it records and added printf-like stuff. 
   -Kyle */
void clan_log( CLAN_DATA * clan, char *str, ... )
{
    char *strtime;
    char logfile[20];
    char buf[10];
    char temp[20];
    int i, j;
    FILE *log_file;
    FILE *tempfile;
    char param[4 * MAX_STRING_LENGTH];

/* Thanks to Tyrluk of MornMUD for showing me how to do this... -Kyle */
    {
        va_list args;

        va_start( args, str );
        vsprintf( param, str, args );
        va_end( args );
    }

    buf[0] = '\0';
    temp[0] = '\0';

    log_file = NULL;

    sprintf( buf, "%s", get_curdate(  ) );
    for ( i = 0; i < 10; i++ )
    {
        if ( buf[i] == '/' )
            buf[i] = '-';
    }

/* The log for clan #1 will look like: 
   ../clan/1.01-31-99
   This log will be accesable to clan leaders. -Kyle */

    sprintf( temp, "%s/%d.", sysconfig.clan_dir, clan->number );
    strcat( temp, buf );

    if ( strcmp( temp, logfile ) )
    {
        tempfile = fopen( temp, "a" );
        if ( log_file != NULL )
            fclose( log_file );
        for ( j = 0; j < 20; j++ )
            logfile[j] = temp[j];
        log_file = fopen( logfile, "a" );
        if ( tempfile != NULL )
            fclose( tempfile );

    }

    if ( log_file == NULL )
    {
        perror( "PERROR - " );
        logf_string( "Could not write to clan log file '%s'", logfile );
        return;
    }

    strtime = ctime( &current_time );
    strtime[strlen( strtime ) - 1] = '\0';
    fprintf( log_file, "%s:%s\n", &strtime[10], param );
    fclose( log_file );
    return;
}

CLAN_DATA *get_clan( int clannum )
{
    CLAN_DATA *clan;

    if ( top_clan == 0 )
        return NULL;

    for ( clan = clan_first; clan != NULL; clan = clan->next )
    {
        if ( clan->number == clannum )
            return clan;
    }

    return NULL;
}

CLAN_DATA *new_clan( void )
{
    CLAN_DATA *clan;
    int i;

    clan = alloc_perm( sizeof( CLAN_DATA ) );
    if ( clan == NULL )
    {
        bug( "Could not create clan.", 0 );
        return NULL;
    }
    if ( clan_first == NULL )
        top_clan = 0;
    top_clan++;

    clan->next = NULL;
    clan->number = top_clan;
    clan->name = str_dup( "Unnamed" );
    clan->leader = str_dup( "None" );
    clan->god = str_dup( "None" );
    clan->max_members = MAX_CLAN_MEMBERS;
    clan->min_level = MIN_CLAN_LEVEL;
    clan->recall_room = ROOM_VNUM_TEMPLE;
    clan->whoname = str_dup( "" );
    clan->command = str_dup( "" );
    clan->skill = str_dup( "" );
    for ( i = 0; i < MAX_RANK; i++ )
        clan->rank[i] = str_dup( "" );
    for ( i = 0; i < MAX_CLAN_MEMBERS; i++ )
        clan->members[i] = str_dup( "" );

    if ( clan_first == NULL )
    {
        clan_first = clan;
        clan_last = clan;
        return clan;
    }

    if ( clan_last == NULL )
    {
        bug( "clan_first but no clan_last.", 0 );
        clan_last = clan;
        return clan;
    }

    clan_last->next = clan;
    clan_last = clan;

    return clan;
}

void do_cedit( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CLAN_DATA *clan;
    int value;

    if ( IS_NPC( ch ) )
        return;

    argument = one_argument( argument, arg );

    if ( is_number( arg ) )
    {
        value = atoi( arg );

        if ( !( clan = get_clan( value ) ) )
        {
            send_to_char( "CEdit:  That clan does not exist.\n\r", ch );
            return;
        }

        ch->desc->pEdit = ( void * ) clan;
        ch->desc->editor = ED_CLAN;
        SET_BIT( ch->act, PLR_BUILDING );
        act( "$n has entered the Clan Editor.", ch, NULL, NULL, TO_ROOM );

        return;
    }
    else
    {
        if ( !str_cmp( arg, "create" ) )
        {
            if ( cedit_create( ch, argument ) )
            {
                ch->desc->editor = ED_CLAN;
                SET_BIT( ch->act, PLR_BUILDING );
                act( "$n has entered the Clan Editor.", ch, NULL, NULL,
                     TO_ROOM );

            }
            return;
        }
    }

    send_to_char( "CEdit:  There is no default clan to edit.\n\r", ch );
    return;
}

void cedit( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CLAN_DATA *clan;
    char command[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );
    EDIT_CLAN( ch, clan );

    if ( !str_cmp( command, "done" ) )
    {
        edit_done( ch );
        return;
    }

    /* Change this depending on who you want to be able to edit clans. */
    if ( get_trust( ch ) < ( MAX_LEVEL - 1 ) )
    {
        send_to_char( "CEdit: Insufficient security to modify clans.\n\r", ch );
        interpret( ch, arg );
        return;
    }

    if ( command[0] == '\0' )
    {
        cedit_show( ch, argument );
        return;
    }
    if ( clan == NULL )
    {
        send_to_char( "You are not currently editing a clan.", ch );
        interpret( ch, arg );
        return;
    }
    /* Search Table and Dispatch Command. */
    for ( cmd = 0; cedit_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( !str_prefix( command, cedit_table[cmd].name ) )
        {
            if ( ( *cedit_table[cmd].olc_fun ) ( ch, argument ) )
            {
                return;
            }
            else
                return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}

CEDIT( cedit_show )
{
    CLAN_DATA *clan;
    int i;

    EDIT_CLAN( ch, clan );

    if ( clan == NULL )
    {
        send_to_char( "You are not currently editing a clan.", ch );
        return FALSE;
    }

    printf_to_char( ch,
                    "Name:     [%s]\n\rNumber:   [%2d]\n\rLeader:   [%12s]\n\rSponsor:  [%12s]\n\r",
                    clan->name, clan->number, clan->leader, clan->god );

    printf_to_char( ch,
                    "Whoname: [%s]\n\rMaximum members: %d\n\rMinimum level: %d\n\r",
                    clan->whoname, clan->max_members, clan->min_level );

    printf_to_char( ch, "Clan recall: %d\n\rCost to join: %d\n\r",
                    clan->recall_room, clan->cost_gold );

    for ( i = 0; i < MAX_RANK; i++ )
    {
        printf_to_char( ch, "Rank %d:[%30s]\n\r", i + 1,
                        clan->rank[i][0] == '\0' ? "None" : clan->rank[i] );
    }
/*
	printf_to_char( ch, "Command: [%s]\n\rSkill:   [%s]\n\r", clan->command, clan->skill );
*/
    printf_to_char( ch, "Clan flags: [%s]\n\r",
                    flag_string( clan_flags, clan->clan_flags ) );
    printf_to_char( ch, "Join flags: [%s]\n\r",
                    flag_string( clan_join_flags, clan->join_flags ) );

    return FALSE;
}

CEDIT( cedit_create )
{
    CLAN_DATA *clan;

    if ( top_clan >= MAX_CLAN )
    {
        send_to_char( "Limit on clans has been reached.\n\r", ch );
        return FALSE;
    }

    clan = new_clan(  );
    if ( clan == NULL )
    {
        send_to_char( "ERROR! Could not create clan!\n\r", ch );
        return FALSE;
    }
    SET_BIT( clan->clan_flags, CLAN_CREATING );
    send_to_char( "Clan created.\n\r", ch );
    clan_log( clan, "Clan created today!" );
    ch->desc->pEdit = ( void * ) clan;
    return TRUE;
}

CEDIT( cedit_name )
{
    CLAN_DATA *clan;

    EDIT_CLAN( ch, clan );

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  name <clan name>\n\r", ch );
        return FALSE;
    }

    if ( strlen( argument ) != str_len( argument ) )    /* Check for color codes in the name */
    {
        send_to_char( "You may not use color codes in the clan name.\n\r", ch );
        return FALSE;
    }

    if ( strlen( argument ) > 20 )
    {
        send_to_char( "Clan names must be 20 characters or less.\n\r", ch );
        return FALSE;
    }

    free_string( &clan->name );
    clan->name = str_dup( argument );
    send_to_char( "Clan name set.\n\r", ch );
    return TRUE;
}

CEDIT( cedit_whoname )
{
    CLAN_DATA *clan;

    EDIT_CLAN( ch, clan );

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  whoname <string>\n\r", ch );
        return FALSE;
    }

    if ( str_len( argument ) > 20 )
    {
        send_to_char( "Clan whonames must be 20 characters or less.\n\r", ch );
        return FALSE;
    }

    free_string( &clan->whoname );
    clan->whoname = str_dup( argument );
    send_to_char( "Name visible on the \"who\" listing has been set.\n\r", ch );
    return TRUE;
}

CEDIT( cedit_rank )
{
    CLAN_DATA *clan;
    int i;
    char arg1[MAX_INPUT_LENGTH];
    EDIT_CLAN( ch, clan );

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  rank <#> <title>\n\r", ch );
        return FALSE;
    }

    argument = one_argument( argument, arg1 );

    if ( !is_number( arg1 ) )
    {
        send_to_char( "Syntax:  rank <#> <title>\n\r", ch );
        return FALSE;
    }

    i = atoi( arg1 );
    if ( i < 1 || i > MAX_RANK )
    {
        printf_to_char( ch, "Allowable rank numbers are 1 through %d.\n\r",
                        MAX_RANK );
        return FALSE;
    }

    if ( str_len( argument ) > 40 )
    {
        send_to_char( "Rank titles must be 40 characters or shorter.\n\r", ch );
        return FALSE;
    }

    free_string( &clan->rank[i - 1] );
    clan->rank[i - 1] = str_dup( argument );
    send_to_char( "Rank set.\n\r", ch );
    return TRUE;
}

CEDIT( cedit_cost )
{
    CLAN_DATA *clan;
    int i;

    EDIT_CLAN( ch, clan );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax: cost <cost in gold>\n\r", ch );
        return FALSE;
    }

    i = atoi( argument );
    if ( i < 0 || i > 100000000 )   /* I think 100 million should be about the limit for cost... */
    {
        send_to_char( "Illegal cost.\n\r", ch );
        return FALSE;
    }

    clan->cost_gold = i;
    send_to_char( "Cost to join set.\n\r", ch );
    return TRUE;
}

CEDIT( cedit_members )
{
    CLAN_DATA *clan;
    int i;

    EDIT_CLAN( ch, clan );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax: members <max members>\n\r", ch );
        return FALSE;
    }

    i = atoi( argument );
    if ( i < 1 || i > MAX_CLAN_MEMBERS )
    {
        send_to_char( "That number of members is not allowed.\n\r", ch );
        return FALSE;
    }

    clan->max_members = i;
    send_to_char( "Max members set.\n\r", ch );
    return TRUE;
}

CEDIT( cedit_minlevel )
{
    CLAN_DATA *clan;
    char buff[MAX_STRING_LENGTH];
    int i;

    EDIT_CLAN( ch, clan );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax: minlevel <level>\n\r", ch );
        return FALSE;
    }

    i = atoi( argument );
    if ( i < 1 || i > MAX_LEVEL )
    {
        sprintf( buff, "Range is 1 to %d.\n\r", MAX_LEVEL );
        send_to_char( buff, ch );
        return FALSE;
    }

    clan->min_level = i;
    send_to_char( "Minimum level to join set.\n\r", ch );
    return TRUE;
}

CEDIT( cedit_recall )
{
    CLAN_DATA *clan;
    int room;

    EDIT_CLAN( ch, clan );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax: recall <room vnum>\n\r", ch );
        return FALSE;
    }

    room = atoi( argument );
    if ( room < 0 || room > 32767 )
    {
        send_to_char( "Illegal room number.\n\r", ch );
        return FALSE;
    }

    if ( get_room_index( room ) == NULL )
    {
        send_to_char( "That room does not exist.\n\r", ch );
        return FALSE;
    }

    clan->recall_room = room;
    send_to_char( "Clan recall room set.\n\r", ch );
    return TRUE;
}

CEDIT( cedit_leader )
{
    CLAN_DATA *clan;
    FILE *fp;
    char buf[MAX_INPUT_LENGTH];

    EDIT_CLAN( ch, clan );

    if ( argument[0] == '\0' )
    {
        send_to_char( "You must supply an argument.\n\r", ch );
        return FALSE;
    }
#if defined(unix)
    /* decompress if .gz file exists */
    sprintf( buf, "%s/%s%s", sysconfig.player_dir, capitalize( argument ),
             ".gz" );
    if ( ( fp = fopen( buf, "r" ) ) != NULL )
    {
        char buf1[MAX_INPUT_LENGTH];
        fclose( fp );
        sprintf( buf1, "gzip -dfq %s", buf );
        system( buf1 );
    }
#endif
    sprintf( buf, "%s/%s", sysconfig.player_dir, capitalize( argument ) );
    if ( ( fp = fopen( buf, "r" ) ) != NULL )
    {
        printf_to_char( ch, "%s is now the the leader of %s.\n\r",
                        capitalize( argument ), clan->name );
        clan->leader = str_dup( capitalize( argument ) );
        fclose( fp );
        return TRUE;
    }
    send_to_char( "That character is not a player on this mud.\n\r", ch );
    if ( fp != NULL )
        fclose( fp );
    return FALSE;
}

CEDIT( cedit_sponsor )
{
    CLAN_DATA *clan;
    char buf[MAX_INPUT_LENGTH];
    FILE *fp;

    EDIT_CLAN( ch, clan );

    if ( argument[0] == '\0' )
    {
        send_to_char( "You must supply an argument.\n\r", ch );
        return FALSE;
    }
    sprintf( buf, "%s/%s", sysconfig.god_dir, capitalize( argument ) );
    if ( ( fp = fopen( buf, "r" ) ) != NULL )
    {
        printf_to_char( ch, "%s is now the god sponsor for %s.\n\r",
                        capitalize( argument ), clan->name );
        clan->god = str_dup( capitalize( argument ) );
        fclose( fp );
        return TRUE;
    }
    send_to_char( "That player is not a god on this mud.\n\r", ch );
    if ( fp != NULL )
        fclose( fp );
    return FALSE;
}

CEDIT( cedit_joinflags )
{
    CLAN_DATA *clan;
    int value;
    EDIT_CLAN( ch, clan );

    if ( argument[0] != '\0' )
    {
        if ( ( value = flag_value( clan_join_flags, argument ) ) != NO_FLAG )
        {
            if ( value == CLAN_NO_PK
                 && IS_SET( clan->join_flags, CLAN_REQ_PK ) )
            {
                clan->join_flags ^= CLAN_REQ_PK;
            }
            else if ( value == CLAN_REQ_PK
                      && IS_SET( clan->join_flags, CLAN_NO_PK ) )
            {
                clan->join_flags ^= CLAN_NO_PK;
            }
            clan->join_flags ^= value;
            send_to_char( "Join flag toggled.\n\r", ch );
            return TRUE;
        }
    }
    send_to_char( "Syntax:  joinflags [flag]\n\r"
                  "Type '? joinflags' for a list of flags.\n\r", ch );
    return FALSE;
}

CEDIT( cedit_clanflags )
{
    CLAN_DATA *clan;
    int value;
    EDIT_CLAN( ch, clan );

    if ( argument[0] != '\0' )
    {
        if ( ( value = flag_value( clan_flags, argument ) ) != NO_FLAG )
        {
            clan->clan_flags ^= value;
            send_to_char( "Clan flag toggled.\n\r", ch );
            return TRUE;
        }
    }
    send_to_char( "Syntax:  clanflags <auto-accept/private/secret>\n\r", ch );
    return FALSE;
}

bool can_ch_join( CHAR_DATA * ch, CLAN_DATA * clan, bool is_petitioning )
{
    if ( ch->level < clan->min_level )
    {
        printf_to_char( ch, "You must be level %d to join this clan.\n\r",
                        clan->min_level );
        return FALSE;
    }

    if ( ( IS_SET( clan->join_flags, CLAN_ANTI_MAGE )
           && ch->Class == CLASS_MAGE )
         || ( IS_SET( clan->join_flags, CLAN_ANTI_CLERIC )
              && ch->Class == CLASS_CLERIC )
         || ( IS_SET( clan->join_flags, CLAN_ANTI_WARRIOR )
              && ch->Class == CLASS_WARRIOR )
         || ( IS_SET( clan->join_flags, CLAN_ANTI_THIEF )
              && ch->Class == CLASS_THIEF ) )
    {
        send_to_char( "This clan does not allow your class to join.\n\r", ch );
        return FALSE;
    }

    if ( IS_SET( clan->join_flags, CLAN_REQ_PK )
         && !IS_SET( ch->act, PLR_KILLER ) )
    {
        send_to_char( "You must be a player killer to join this clan.\n\r",
                      ch );
        send_to_char( "Type PK twice to become a player killer.\n\r", ch );
        return FALSE;
    }

    if ( IS_SET( clan->join_flags, CLAN_NO_PK )
         && IS_SET( ch->act, PLR_KILLER ) )
    {
        send_to_char( "You may not be a player killer and join this clan.\n\r",
                      ch );
        send_to_char
            ( "Ask a high level god to remove your killer flag if you still wish to join.\n\r",
              ch );
        return FALSE;
    }

    if ( IS_SET( clan->join_flags, CLAN_ANTI_EVIL ) && IS_EVIL( ch ) )
    {
        send_to_char( "Evil-aligned players may not join this clan.\n\r", ch );
        return FALSE;
    }

    if ( IS_SET( clan->join_flags, CLAN_ANTI_GOOD ) && IS_GOOD( ch ) )
    {
        send_to_char( "Good-aligned players may not join this clan.\n\r", ch );
        return FALSE;
    }

    if ( !IS_SET( clan->clan_flags, CLAN_AUTO_ACCEPT ) && !is_petitioning )
    {
        send_to_char( "This clan does not auto-accept new members.\n\r", ch );
        send_to_char( "Use the PETITION command on one of its members.\n\r",
                      ch );
        return FALSE;
    }

    return TRUE;
}

void add_member( CHAR_DATA * ch, CLAN_DATA * clan )
{
    int i;

    for ( i = 0; i < clan->max_members; i++ )
    {
        if ( clan->members[i][0] == '\0' )
        {
            free_string( &clan->members[i] );
            clan->members[i] = str_dup( ch->name );
            clan->num_members++;
            clan_log( clan, "New member: %s", ch->name );
            save_clans(  );
            do_save( ch, "" );
            return;
        }
    }
    bug( "Add_member: No spots in clan %d!", clan->number );
    return;
}

void remove_member( CHAR_DATA * ch, CLAN_DATA * clan )
{
    int i;

    for ( i = 0; i < clan->max_members; i++ )
    {
        if ( !str_cmp( clan->members[i], ch->name ) )
        {
            free_string( &clan->members[i] );
            clan->members[i] = str_dup( "" );
            clan->num_members--;
            clan_log( clan, "Removed from clan: %s", ch->name );
            save_clans(  );
            do_save( ch, "" );
            return;
        }
    }
    bug( "Add_member: Name not found in clan %d!", clan->number );
    return;
}

void do_show( CHAR_DATA * ch, char *argument )
{
    CLAN_DATA *clan;
    char arg1[MAX_INPUT_LENGTH];
    int num = 0;
    bool found = FALSE;

    argument = one_argument( argument, arg1 );

    if ( clan_first == NULL )
    {
        send_to_char( "There are no clans at all yet.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "active" ) )
    {
        found = FALSE;
        send_to_char( "Clans actively recruiting:\n\r", ch );
        send_to_char( "===============================\n\r", ch );
        for ( clan = clan_first; clan != NULL; clan = clan->next )
        {
            if ( IS_SET( clan->clan_flags, CLAN_ACTIVE )
                 && ( !IS_SET( clan->clan_flags, CLAN_SECRET )
                      || IS_IMMORTAL( ch ) )
                 && clan->num_members < MAX_CLAN_MEMBERS )
            {
                num++;
                printf_to_char( ch, "[%3d] %s\n\r", num, clan->name );
                found = TRUE;
            }
        }
        if ( found == FALSE )
        {
            send_to_char( "No clans are publicly recruiting.\n\r", ch );
        }
        return;
    }

    if ( !str_cmp( arg1, "inactive" ) )
    {
        found = FALSE;
        send_to_char( "Clans not actively recruiting:\n\r", ch );
        send_to_char( "===============================\n\r", ch );
        for ( clan = clan_first; clan != NULL; clan = clan->next )
        {
            if ( ( IS_SET( clan->clan_flags, CLAN_INACTIVE )
                   && ( !IS_SET( clan->clan_flags, CLAN_SECRET )
                        || IS_IMMORTAL( ch ) ) )
                 ||
                 ( ( IS_SET( clan->clan_flags, CLAN_ACTIVE )
                     && !IS_SET( clan->clan_flags, CLAN_SECRET )
                     && clan->num_members >= MAX_CLAN_MEMBERS ) ) )
            {
                num++;
                printf_to_char( ch, "[%3d] %s\n\r", num, clan->name );
                found = TRUE;
            }
        }
        if ( found == FALSE )
        {
            send_to_char( "No clans are inactive at this time.\n\r", ch );
        }
        return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
        found = FALSE;
        send_to_char( "Current existing clans:\n\r", ch );
        send_to_char( "===============================\n\r", ch );
        for ( clan = clan_first; clan != NULL; clan = clan->next )
        {
            if ( ( !IS_SET( clan->clan_flags, CLAN_SECRET )
                   && ( IS_SET( clan->clan_flags, CLAN_ACTIVE )
                        || IS_SET( clan->clan_flags, CLAN_INACTIVE ) ) )
                 || IS_IMMORTAL( ch ) )
            {
                num++;
                printf_to_char( ch, "[%3d] %s\n\r", num, clan->name );
                found = TRUE;
            }
        }
        if ( found == FALSE )
        {
            send_to_char( "None.\n\r", ch );
        }
        return;
    }

    if ( !str_cmp( arg1, "members" ) )
    {
        if ( IS_NPC( ch ) )
            return;
        if ( ch->pcdata->clan > 0 )
        {
            clan = get_clan( ch->pcdata->clan );
            printf_to_char( ch, "Sponsor: %s\n\rLeader: %s\n\r", clan->god,
                            clan->leader );
            send_to_char( "Members:\n\r", ch );
            for ( num = 0; num < MAX_CLAN_MEMBERS; num++ )
            {
                printf_to_char( ch, "%s ", clan->members[num] );
                if ( num != 0 && num % 5 == 0 )
                    send_to_char( "\n\r", ch );
            }
            send_to_char( "\n\r", ch );
        }
        return;
    }
    if ( IS_NPC( ch ) )
        return;

    if ( ch->pcdata->clan == 0 )
    {
        send_to_char( "Syntax: show <active/inactive/all>\n\r", ch );
        return;
    }

    send_to_char( "Syntax: show <active/inactive/all/members>\n\r", ch );

    return;
}

void do_join( CHAR_DATA * ch, char *argument )
{
    CLAN_DATA *clan;
    int num, i = 0;
    char arg1[MAX_INPUT_LENGTH];

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Are you nuts? NPC's may not join clans!\n\r", ch );
        return;
    }

    if ( ch->pcdata == NULL )
        return;

    if ( ch->pcdata->clan != 0 )
    {
        send_to_char( "You are already a member of a clan!  Duh!\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' && ch->pcdata->join_status != JOIN_START )
    {
        send_to_char( "Syntax: join <clan number, 0 to cancel>\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );

    switch ( ch->pcdata->join_status )
    {
    case JOIN_START:
        send_to_char( "This is a list of actively recruiting clans.\n\r", ch );
        do_show( ch, "active" );
        send_to_char( "\n\rTo see this list again, type 'show active'.\n\r",
                      ch );
        send_to_char
            ( "To join a clan, type JOIN followed by the number of the clan.\n\r",
              ch );
        ch->pcdata->join_status = JOIN_SEE_LIST;
        return;
    case JOIN_SEE_LIST:
        if ( !is_number( arg1 ) )
        {
            send_to_char( "You must join a clan by using its number.\n\r", ch );
            return;
        }
        num = atoi( arg1 );

        if ( num == 0 )
        {
            send_to_char( "Cancelling your joining of a clan.\n\r", ch );
            ch->pcdata->join_status = JOIN_START;
            return;
        }

        for ( clan = clan_first; clan != NULL; clan = clan->next )
        {
            if ( IS_SET( clan->clan_flags, CLAN_ACTIVE )
                 && !IS_SET( clan->clan_flags, CLAN_SECRET )
                 && clan->num_members < MAX_CLAN_MEMBERS )
            {
                i++;
                if ( num == i )
                {
                    printf_to_char( ch,
                                    "You are attempting to join clan #%d, %s.\n\r",
                                    num, clan->name );
                    if ( can_ch_join( ch, clan, FALSE ) == FALSE )
                        return;
                    send_to_char( "You are eligable to join.\n\r", ch );
                    if ( clan->cost_gold > 0 )
                    {
                        printf_to_char( ch,
                                        "This clan costs %d gold to join.\n\r",
                                        clan->cost_gold );
                        if ( ch->gold < clan->cost_gold )
                        {
                            send_to_char( "You cannot afford to join!\n\r",
                                          ch );
                            return;
                        }
                    }
                    printf_to_char( ch, "To confirm, please type JOIN %d\n\r",
                                    num );
                    send_to_char( "Type JOIN 0 to cancel this choice.\n\r",
                                  ch );
                    ch->pcdata->join_status = JOIN_CONFIRM;
                    ch->pcdata->join_clan = num;
                    return;
                }
            }
        }
        send_to_char( "That clan does not exist.\n\r", ch );
        return;
    case JOIN_CONFIRM:
        if ( !is_number( arg1 ) )
        {
            send_to_char( "You must join a clan by using its number.\n\r", ch );
            return;
        }
        num = atoi( arg1 );

        if ( num == 0 )
        {
            send_to_char( "Your selection of a clan has been canceled.\n\r",
                          ch );
            ch->pcdata->join_status = JOIN_START;
            ch->pcdata->join_clan = 0;
            return;
        }

        if ( num != ch->pcdata->join_clan )
        {
            send_to_char( "Your clan selections do not match.\n\r", ch );
            printf_to_char( ch,
                            "Type JOIN 0 to cancel or JOIN %d to confirm.\n\r",
                            ch->pcdata->join_clan );
        }

        for ( clan = clan_first; clan != NULL; clan = clan->next )
        {
            if ( IS_SET( clan->clan_flags, CLAN_ACTIVE )
                 && !IS_SET( clan->clan_flags, CLAN_SECRET )
                 && clan->num_members < MAX_CLAN_MEMBERS )
            {
                i++;
                if ( num == i )
                {
                    if ( clan->cost_gold > 0 )
                    {
                        printf_to_char( ch,
                                        "This clan costs %d gold to join.\n\r",
                                        clan->cost_gold );
                        if ( clan->cost_gold > ch->gold )
                        {
                            send_to_char( "You cannot afford this clan!\n\r",
                                          ch );
                            do_join( ch, "0" );
                            return;
                        }
                        ch->gold -= clan->cost_gold;
                        clan->clan_fund += clan->cost_gold;
                        printf_to_char( ch, "You now have %d gold left.\n\r",
                                        ch->gold );
                    }
                    printf_to_char( ch,
                                    "You are now a member of clan #%d, %s!\n\r",
                                    i, clan->name );
                    ch->pcdata->clan = clan->number;
                    ch->pcdata->join_status = JOIN_START;
                    add_member( ch, clan );
                    ch->pcdata->clan_rank = MAX_RANK;
                    return;
                }
            }
        }
    case JOIN_PETITIONING:
        send_to_char( "You are currently petitioning to join a clan.\n\r", ch );
        send_to_char
            ( "If you are refused, then you may use the JOIN command again.\n\r",
              ch );
        return;
    case JOIN_DECIDING:
        send_to_char( "You have already been invited to join a clan.\n\r", ch );
        send_to_char( "Use the ACCEPT or DECLINE commands to join or not.\n\r",
                      ch );
        return;
    case JOIN_CONSIDERING:
    case JOIN_OFFERING:
        send_to_char( "Doh, a bug.  Ignore this.\n\r", ch );
        bug( "Ch without clan in JOIN_CONSIDERING or JOIN_OFFERING", 0 );
        ch->pcdata->join_status = JOIN_START;
        return;
    default:
        send_to_char( "Doh, a bug.  Ignore this.\n\r", ch );
        bug( "Unknown join_status %d!", ch->pcdata->join_status );
        ch->pcdata->join_status = JOIN_START;
        return;
    }
    return;
}

void do_petition( CHAR_DATA * ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    CLAN_DATA *clan;
    CHAR_DATA *victim;

    if ( IS_NPC( ch ) )
        return;

    if ( ch->pcdata == NULL )
        return;

    if ( ch->pcdata->clan != 0 )
    {
        send_to_char( "You are already a member of a clan!\n\r", ch );
        return;
    }

    if ( ch->pcdata->join_status == JOIN_PETITIONING )
    {
        send_to_char( "You are already petitioning to join a clan!\n\r", ch );
        return;
    }

    if ( ch->pcdata->join_status == JOIN_DECIDING )
    {
        send_to_char( "You have already been offered membership in a clan!\n\r",
                      ch );
        send_to_char( "Type ACCEPT or DECLINE.\n\r", ch );
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Petition who?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg1 );

    victim = get_char_world( ch, arg1 );
    if ( victim == NULL )
    {
        send_to_char( "Petition who?\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Only players can be petitioned!\n\r", ch );
        return;
    }

    if ( victim->pcdata == NULL )
        return;

    if ( victim->pcdata->clan == 0 )
    {
        send_to_char( "That person is not a member of a clan.\n\r", ch );
        return;
    }

    if ( victim->pcdata->clan_ch != ch && victim->pcdata->clan_ch != NULL )
    {
        send_to_char
            ( "That person is currently being petitioned by someone else.\n\r",
              ch );
        return;
    }

    if ( victim->pcdata->clan_ch == ch )
    {
        send_to_char( "You are already petitioning them!  Be patient!\n\r",
                      ch );
        return;
    }

    clan = get_clan( victim->pcdata->clan );

    if ( clan == NULL )
    {
        bug( "NULL clan %d", victim->pcdata->clan );
        return;
    }

    if ( IS_SET( clan->clan_flags, CLAN_SECRET ) )
    {
        send_to_char( "That person is not a member of a clan.\n\r", ch );
        return;
    }

    if ( victim->pcdata->clan_rank > RANK_CAN_ACCEPT
         || victim->pcdata->clan_rank < 1 )
    {
        send_to_char
            ( "That person is not of sufficient rank to accept you.\n\r", ch );
        return;
    }

    if ( can_ch_join( ch, clan, TRUE ) == FALSE )
        return;
    if ( clan->cost_gold > 0 )
    {
        printf_to_char( ch, "It costs %d gold to join.\n\r", clan->cost_gold );
        if ( clan->cost_gold > ch->gold )
        {
            send_to_char( "You cannot afford it!\n\r", ch );
            return;
        }
    }
    clan_log( clan, "%s petitions %s to join.", ch->name, victim->name );
    printf_to_char( victim, "%s has petitioned to join your clan!\n\r",
                    ch->name );
    send_to_char( "Type ACCEPT or DECLINE, or talk to them for a while.\n\r",
                  victim );
    send_to_char( "Petition sent!\n\r", ch );

    ch->pcdata->join_status = JOIN_PETITIONING;
    victim->pcdata->join_status = JOIN_CONSIDERING;
    ch->pcdata->clan_ch = victim;
    victim->pcdata->clan_ch = ch;
    return;
}

void do_accept( CHAR_DATA * ch, char *argument )
{
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) )
        return;
    if ( ch->pcdata == NULL )
        return;

    if ( ch->pcdata->join_status != JOIN_DECIDING
         && ch->pcdata->join_status != JOIN_CONSIDERING )
    {
        send_to_char( "What, exactly, are you trying to accept?\n\r", ch );
        return;
    }

    if ( ch->pcdata->join_status == JOIN_CONSIDERING )
    {
        clan = get_clan( ch->pcdata->clan );
        if ( clan->cost_gold > ch->pcdata->clan_ch->gold )
        {
            send_to_char( "They can no longer afford to join.\n\r", ch );
            send_to_char( "Use DECLINE to refuse, or wait come more.\n\r", ch );
            return;
        }
        clan_log( clan, "Petition accepted by %s", ch->name );
        printf_to_char( ch, "You accept %s into clan %s!\n\r",
                        ch->pcdata->clan_ch->name, clan->name );
        printf_to_char( ch->pcdata->clan_ch,
                        "You have been accepted into clan %s!\n\r",
                        clan->name );
        ch->pcdata->clan_ch->gold -= clan->cost_gold;
        clan->clan_fund += clan->cost_gold;
        ch->pcdata->clan_ch->pcdata->clan = clan->number;
        ch->pcdata->clan_ch->pcdata->clan_rank = MAX_RANK;
        ch->pcdata->clan_ch->pcdata->join_status = JOIN_START;
        ch->pcdata->clan_ch->pcdata->clan_ch = NULL;
        ch->pcdata->clan_ch->pcdata->join_clan = 0;
        add_member( ch->pcdata->clan_ch, clan );
        ch->pcdata->clan_ch = NULL;
        ch->pcdata->join_status = JOIN_START;
        return;
    }

    clan = get_clan( ch->pcdata->clan_ch->pcdata->clan );
    if ( ch->gold < clan->cost_gold )
    {
        send_to_char( "You cannot afford to join that clan!\n\r", ch );
        send_to_char
            ( "Use DECLINE to turn them down or go get more gold fast.\n\r",
              ch );
        return;
    }
    clan_log( clan, "%s accepts membership.", ch->name );
    printf_to_char( ch, "You accept membership in clan %s!\n\r", clan->name );
    printf_to_char( ch->pcdata->clan_ch,
                    "%s has accepted membership in your clan!\n\r", ch->name );
    ch->gold -= clan->cost_gold;
    clan->clan_fund += clan->cost_gold;
    ch->pcdata->clan = clan->number;
    ch->pcdata->clan_rank = MAX_RANK;
    ch->pcdata->join_status = JOIN_START;
    add_member( ch, clan );
    ch->pcdata->clan_ch->pcdata->clan_ch = NULL;
    ch->pcdata->clan_ch->pcdata->join_status = JOIN_START;
    ch->pcdata->clan_ch = NULL;
    return;
}

void do_decline( CHAR_DATA * ch, char *argument )
{
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) )
        return;
    if ( ch->pcdata == NULL )
        return;

    if ( ch->pcdata->join_status != JOIN_DECIDING
         && ch->pcdata->join_status != JOIN_CONSIDERING )
    {
        send_to_char( "What, exactly, are you trying to decline?\n\r", ch );
        return;
    }

    if ( ch->pcdata->join_status == JOIN_CONSIDERING )
    {
        clan = get_clan( ch->pcdata->clan );
        clan_log( clan, "%s declines the petition.", ch->name );
        printf_to_char( ch, "You decline to accept %s into clan %s.\n\r",
                        ch->pcdata->clan_ch->name, clan->name );
        printf_to_char( ch->pcdata->clan_ch,
                        "You have been denied membership into clan %s.\n\r",
                        clan->name );
        ch->pcdata->clan_ch->pcdata->join_status = JOIN_START;
        ch->pcdata->clan_ch->pcdata->clan_ch = NULL;
        ch->pcdata->clan_ch = NULL;
        ch->pcdata->join_status = JOIN_START;
        return;
    }

    clan = get_clan( ch->pcdata->clan_ch->pcdata->clan );
    clan_log( clan, "%s declines to join.", ch->name );
    printf_to_char( ch, "You decline membership in clan %s.\n\r", clan->name );
    printf_to_char( ch->pcdata->clan_ch,
                    "%s has declined membership in your clan.\n\r", ch->name );
    ch->pcdata->clan_ch->pcdata->join_status = JOIN_START;
    ch->pcdata->clan_ch->pcdata->clan_ch = NULL;
    ch->pcdata->clan_ch = NULL;
    ch->pcdata->join_status = JOIN_START;
    return;
}

void do_offer( CHAR_DATA * ch, char *argument )
{
    CLAN_DATA *clan;
    CHAR_DATA *victim;

    if ( IS_NPC( ch ) || ch->pcdata == NULL )
        return;

    if ( ch->pcdata->clan == 0 )
    {
        send_to_char
            ( "You can't offer membership in a clan if you are clanless!\n\r",
              ch );
        return;
    }

    victim = get_char_world( ch, argument );

    if ( victim == NULL )
    {
        send_to_char( "Offer membership to who?\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) || victim->pcdata == NULL )
    {
        send_to_char( "You cannot offer membership to NPC's.\n\r", ch );
        return;
    }

    if ( ch->pcdata->join_status != 0 && ch->pcdata->clan_ch != NULL )
    {
        printf_to_char( ch,
                        "You are currently being petitioned by or offering membership to %s.\n\r",
                        ch->pcdata->clan_ch->name );
        return;
    }

    clan = get_clan( ch->pcdata->clan );

    if ( ch->pcdata->clan_rank < 1 || ch->pcdata->clan_rank > RANK_CAN_ACCEPT )
    {
        send_to_char
            ( "You are not of high enough rank to offer membership to anyone.\n\r",
              ch );
        return;
    }

    if ( victim->pcdata->join_status > JOIN_SEE_LIST )
    {
        send_to_char
            ( "That player is considering membership in another clan.\n\r",
              ch );
        send_to_char( "Better make them change their mind, fast! :)\n\r", ch );
        return;
    }

    /* The reason the "That person is ineligable to join your clan." message is 
       everywhere is to preserve the secretness of the secret clans. */

    if ( victim->pcdata->clan != 0 )
    {
        CLAN_DATA *tmpclan;
        tmpclan = get_clan( victim->pcdata->clan );
        if ( IS_SET( tmpclan->clan_flags, CLAN_SECRET ) )
            send_to_char( "That person is ineligable to join your clan.\n\r",
                          ch );
        else
            send_to_char( "That person is already a member of a clan!\n\r",
                          ch );
        return;
    }

    if ( victim->level < clan->min_level )
    {
        send_to_char( "That person is ineligable to join your clan.\n\r", ch );
        send_to_char( "To find out why, they must petition you.\n\r", ch );
        return;
    }

    if ( ( IS_SET( clan->join_flags, CLAN_ANTI_MAGE )
           && victim->Class == CLASS_MAGE )
         || ( IS_SET( clan->join_flags, CLAN_ANTI_CLERIC )
              && victim->Class == CLASS_CLERIC )
         || ( IS_SET( clan->join_flags, CLAN_ANTI_WARRIOR )
              && victim->Class == CLASS_WARRIOR )
         || ( IS_SET( clan->join_flags, CLAN_ANTI_THIEF )
              && victim->Class == CLASS_THIEF ) )
    {
        send_to_char( "That person is ineligable to join your clan.\n\r", ch );
        return;
    }

    if ( IS_SET( clan->join_flags, CLAN_REQ_PK )
         && !IS_SET( victim->act, PLR_KILLER ) )
    {
        send_to_char( "That person is ineligable to join your clan.\n\r", ch );
        return;
    }

    if ( IS_SET( clan->join_flags, CLAN_NO_PK )
         && IS_SET( victim->act, PLR_KILLER ) )
    {
        send_to_char( "That person is ineligable to join your clan.\n\r", ch );
        return;
    }

    if ( IS_SET( clan->join_flags, CLAN_ANTI_EVIL ) && IS_EVIL( victim ) )
    {
        send_to_char( "That person is ineligable to join your clan.\n\r", ch );
        return;
    }

    if ( IS_SET( clan->join_flags, CLAN_ANTI_GOOD ) && IS_GOOD( victim ) )
    {
        send_to_char( "That person is ineligable to join your clan.\n\r", ch );
        return;
    }

    if ( victim->gold < clan->cost_gold )
    {
        send_to_char( "They cannot afford to join! they are too poor!\n\r",
                      ch );
        return;
    }
    printf_to_char( ch, "You offer %s membership in clan %s.\n\r", victim->name,
                    clan->name );
    printf_to_char( victim,
                    "You have been offered membership in clan %s by %s!\n\r",
                    clan->name, ch->name );
    if ( clan->cost_gold > 0 )
        printf_to_char( victim, "It will cost %d gold to join.\n\r",
                        clan->cost_gold );
    clan_log( clan, "Membership offered to %s by %s.", victim->name, ch->name );
    ch->pcdata->clan_ch = victim;
    ch->pcdata->join_status = JOIN_OFFERING;
    victim->pcdata->clan_ch = ch;
    victim->pcdata->join_status = JOIN_DECIDING;
    return;
}

void do_promote( CHAR_DATA * ch, char *argument )
{
    CLAN_DATA *clan;
    CHAR_DATA *victim;
    char buf[MAX_INPUT_LENGTH];

    if ( IS_NPC( ch ) )
        return;

    if ( ch->pcdata == NULL )
        return;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Promote who?\n\r", ch );
        return;
    }

    victim = get_char_world( ch, argument );
    if ( victim == NULL )
    {
        send_to_char( "Promote who?\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Um, you can't promote mobs.\n\r", ch );
        return;
    }

    if ( victim->pcdata->clan == 0 )
    {
        send_to_char( "Um, they must join a clan to be promoted.\n\r", ch );
        return;
    }

    if ( get_trust( ch ) < MAX_LEVEL )  /* IMPS can promote anyone :) */
    {
        if ( ch->pcdata->clan == 0 )
        {
            send_to_char( "You are not a member of a clan!\n\r", ch );
            return;
        }

        if ( ch->pcdata->clan != victim->pcdata->clan )
        {
            send_to_char( "They are not even IN your clan!\n\r", ch );
            return;
        }

        if ( victim->pcdata->clan_rank < 1 )
        {
            victim->pcdata->clan_rank = MAX_RANK;
        }

        if ( ch->pcdata->clan_rank > 2 )
        {
            send_to_char( "You are not high enough rank to promote anyone.\n\r",
                          ch );
            return;
        }

        clan = get_clan( ch->pcdata->clan );

        if ( str_cmp( ch->name, clan->leader ) )
        {
            if ( ( ch->pcdata->clan_rank == 2 && victim->pcdata->clan_rank < 4 )
                 || ( ch->pcdata->clan_rank == 1
                      && victim->pcdata->clan_rank < 3 ) )
            {
                send_to_char
                    ( "Only the clan leader, an IMP, or a higher rank can promote them.\n\r",
                      ch );
                return;
            }
        }

    }
    else
    {
        clan = get_clan( victim->pcdata->clan );
    }
    if ( victim->pcdata->clan_rank < 2 )
    {
        send_to_char( "That person's rank is as high as it can be!\n\r", ch );
        return;
    }
    printf_to_char( ch, "You promote %s to %s!\n\r", victim->name,
                    clan->rank[victim->pcdata->clan_rank - 2] );
    victim->pcdata->clan_rank--;
    printf_to_char( victim, "%s promotes you to %s in clan %s!\n\r",
                    ch->name, clan->rank[victim->pcdata->clan_rank - 1],
                    clan->name );
    sprintf( buf, "%s promoted %s in clan %d.", ch->name, victim->name,
             clan->number );
    log_string( buf );
    clan_log( clan, "%s promoted %s to %s.", ch->name, victim->name,
              clan->rank[victim->pcdata->clan_rank - 1] );
    return;
}

void do_clan( CHAR_DATA * ch, char *argument )
{
    CLAN_DATA *clan;
    const char syntax[] =
        "Syntax: clan list\n\r        clan snoop\n\r        clan <clan #> view\n\r        clan <clan #> member <add/remove> <member name>\n\r        clan <clan #> bank <deposit/withdraw> <amount>\n\r        clan <clan #> status <creating/active/inactive/disbanding/dead>\n\r";
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int i, active = 0, recruit = 0, x;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( syntax, ch );
        return;
    }

    if ( !str_cmp( arg1, "list" ) )
    {
        if ( clan_first == NULL )
        {
            send_to_char( "There are no clans.  Use 'edit clan create'.\n\r",
                          ch );
            return;
        }
        send_to_char
            ( "`wNumber Name                           Status               Fund\n\r",
              ch );
        send_to_char
            ( "`K====== ============================== ==================== ==========`w\n\r",
              ch );
        for ( clan = clan_first; clan != NULL; clan = clan->next )
        {
            printf_to_char( ch, "%6d %30s %20s %10d\n\r", clan->number,
                            clan->name, flag_string( clan_flags,
                                                     clan->clan_flags ),
                            clan->clan_fund );
            if ( clan->num_members < clan->max_members
                 && IS_SET( clan->clan_flags, CLAN_ACTIVE ) )
                recruit++;
            if ( IS_SET( clan->clan_flags, CLAN_ACTIVE ) )
                active++;
        }
        printf_to_char( ch, "Active Clans: %d\n\rRecruiting Clans: %d\n\r",
                        active, recruit );
        return;
    }

    if ( !str_cmp( arg1, "snoop" ) )
    {
        if ( IS_SET( ch->comm, COMM_SNOOP_CLAN ) )
        {
            REMOVE_BIT( ch->comm, COMM_SNOOP_CLAN );
            send_to_char( "You no longer are snooping the clan channels.\n\r",
                          ch );
            return;
        }
        else
        {
            SET_BIT( ch->comm, COMM_SNOOP_CLAN );
            send_to_char( "You are now snooping clans.\n\r", ch );
            return;
        }
        return;
    }

    if ( !is_number( arg1 ) )
    {
        send_to_char( syntax, ch );
        return;
    }

    i = atoi( arg1 );

    clan = get_clan( i );

    if ( clan == NULL )
    {
        send_to_char( "No such clan.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "view" ) )
    {
        printf_to_char( ch,
                        "Name:   [%s]\n\rNumber: [%2d]\n\rLeader: [%12s]\n\rSponsor:[%12s]\n\r",
                        clan->name, clan->number, clan->leader, clan->god );
        send_to_char( "Members:", ch );

        for ( x = 0; x < MAX_CLAN_MEMBERS; x++ )
        {
            printf_to_char( ch, " %s", clan->members[x] );
        }

        send_to_char( "\n\r", ch );
        printf_to_char( ch, "Recall: %d\n\rClan fund: %d\n\r",
                        clan->recall_room, clan->clan_fund );
        printf_to_char( ch, "Clan status: %s\n\r",
                        flag_string( clan_flags, clan->clan_flags ) );

        return;
    }

    if ( !str_cmp( arg2, "member" ) )
    {
        CHAR_DATA *victim;

        victim = get_char_world( ch, argument );

        if ( ( victim == NULL ) || ( argument[0] == '\0' ) )
        {
            send_to_char( "Player not found!\n\r", ch );
            return;
        }

        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPCs.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg3, "add" ) )
        {
            if ( victim->pcdata->clan != 0 )
            {
                send_to_char( "They are already in a clan.", ch );
                return;
            }

            if ( !can_ch_join( victim, clan, TRUE ) == FALSE )
            {
                send_to_char
                    ( "WARNING!  This player does not meet the requirements of that clan!\n\r",
                      ch );
            }

            victim->pcdata->clan = clan->number;
            victim->pcdata->clan_rank = MAX_RANK;
            victim->pcdata->join_status = JOIN_START;
            victim->pcdata->clan_ch = NULL;

            add_member( victim, clan );
            printf_to_char( victim, "You have been made a member of %s.\n\r",
                            clan->name );

            send_to_char( "Member added.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg3, "remove" ) )
        {
            if ( victim->pcdata->clan != clan->number )
            {
                send_to_char( "They are not a member of that clan!", ch );
                return;
            }

            victim->pcdata->clan = 0;
            victim->pcdata->clan_rank = 0;
            victim->pcdata->join_status = 0;
            victim->pcdata->clan_ch = NULL;

            remove_member( victim, clan );
            printf_to_char( victim, "You have been removed from %s.\n\r",
                            clan->name );

            send_to_char( "Member removed.\n\r", ch );
            return;
        }

        send_to_char( syntax, ch );
        return;
    }

    if ( !str_cmp( arg2, "bank" ) || !str_cmp( arg2, "fund" ) )
    {
        int mod;

        if ( argument[0] == '\0' || !is_number( argument ) )
        {
            send_to_char( syntax, ch );
            return;
        }

        mod = atoi( argument );

        if ( mod <= 0 )
        {
            send_to_char( "Positive numbers ONLY.\n\r", ch );
            return;
        }

        if ( !str_cmp( arg3, "deposit" ) )
        {
            clan->clan_fund += mod;
            printf_to_char( ch, "Balance: %d\n\r", clan->clan_fund );
            clan_log( clan, "%d credited by %s.", mod, ch->name );
            save_clans(  );
            return;
        }

        if ( !str_cmp( arg3, "withdraw" ) )
        {
            if ( mod > clan->clan_fund )
            {
                send_to_char( "The clan cannot afford that.\n\r", ch );
                return;
            }

            clan->clan_fund -= mod;
            printf_to_char( ch, "Balance: %d\n\r", clan->clan_fund );
            clan_log( clan, "%d funds deducted by %s.", mod, ch->name );
            save_clans(  );
            return;
        }

        send_to_char( syntax, ch );
        return;
    }

    if ( !str_cmp( arg2, "status" ) )
    {
        if ( !str_cmp( arg3, "creating" ) )
        {
            REMOVE_BIT( clan->clan_flags, CLAN_ACTIVE );
            REMOVE_BIT( clan->clan_flags, CLAN_INACTIVE );
            REMOVE_BIT( clan->clan_flags, CLAN_DISBANDING );
            REMOVE_BIT( clan->clan_flags, CLAN_DEAD );
            SET_BIT( clan->clan_flags, CLAN_CREATING );
            printf_to_char( ch, "Clan %s is now in the 'creating' stage.\n\r",
                            clan->name );
            save_clans(  );
            return;
        }

        if ( !str_cmp( arg3, "active" ) )
        {
            REMOVE_BIT( clan->clan_flags, CLAN_INACTIVE );
            REMOVE_BIT( clan->clan_flags, CLAN_CREATING );
            REMOVE_BIT( clan->clan_flags, CLAN_DISBANDING );
            REMOVE_BIT( clan->clan_flags, CLAN_DEAD );
            SET_BIT( clan->clan_flags, CLAN_ACTIVE );
            printf_to_char( ch, "Clan %s is now in the 'active' stage.\n\r",
                            clan->name );
            clan_log( clan, "Clan is now active." );
            save_clans(  );
            return;
        }

        if ( !str_cmp( arg3, "inactive" ) )
        {
            REMOVE_BIT( clan->clan_flags, CLAN_ACTIVE );
            REMOVE_BIT( clan->clan_flags, CLAN_CREATING );
            REMOVE_BIT( clan->clan_flags, CLAN_DISBANDING );
            REMOVE_BIT( clan->clan_flags, CLAN_DEAD );
            SET_BIT( clan->clan_flags, CLAN_INACTIVE );
            printf_to_char( ch, "Clan %s is now in the 'inactive' stage.\n\r",
                            clan->name );
            clan_log( clan, "The clan is no longer recruiting." );
            save_clans(  );
            return;
        }

        if ( !str_cmp( arg3, "disbanding" ) )
        {
            REMOVE_BIT( clan->clan_flags, CLAN_ACTIVE );
            REMOVE_BIT( clan->clan_flags, CLAN_INACTIVE );
            REMOVE_BIT( clan->clan_flags, CLAN_CREATING );
            REMOVE_BIT( clan->clan_flags, CLAN_DEAD );
            SET_BIT( clan->clan_flags, CLAN_DISBANDING );
            printf_to_char( ch, "Clan %s is now in the 'disbanding' stage.\n\r",
                            clan->name );
            clan_log( clan, "The clan is now disbanding." );
            save_clans(  );
            return;
        }

        if ( !str_cmp( arg3, "dead" ) )
        {
            REMOVE_BIT( clan->clan_flags, CLAN_ACTIVE );
            REMOVE_BIT( clan->clan_flags, CLAN_INACTIVE );
            REMOVE_BIT( clan->clan_flags, CLAN_CREATING );
            REMOVE_BIT( clan->clan_flags, CLAN_DISBANDING );
            SET_BIT( clan->clan_flags, CLAN_DEAD );
            printf_to_char( ch, "Clan %s is now in the 'dead' stage.\n\r",
                            clan->name );
            clan_log( clan, "The clan is now dead." );
            save_clans(  );
            return;
        }

        send_to_char( syntax, ch );
        return;
    }

    send_to_char( syntax, ch );
    return;
}

void do_clantalk( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Getting lonely?\n\r", ch );
        return;
    }

    if ( ( clan = get_clan( ch->pcdata->clan ) ) == NULL )
    {
        send_to_char( "You are not even in a clan!\n\r", ch );
        return;
    }

    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
        send_to_char( "You must turn off quiet mode first.\n\r", ch );
        return;
    }

    if ( IS_SET( ch->comm, COMM_NOCHANNELS ) )
    {
        send_to_char( "The gods have revoked your channel priviliges.\n\r",
                      ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        if ( IS_SET( ch->comm, COMM_NOCLAN ) )
        {
            send_to_char( "`BClan channel is now ON.\n\r`w", ch );
            REMOVE_BIT( ch->comm, COMM_NOCLAN );
        }
        else
        {
            send_to_char( "`BClan channel is now OFF.\n\r`w", ch );
            SET_BIT( ch->comm, COMM_NOCLAN );
        }
    }

    /* Re-enable the channel if it was disabled */
    REMOVE_BIT( ch->comm, COMM_NOCLAN );

#ifdef CLANTALK_DRUNK
    /* Make the words drunk if needed */
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
        argument = makedrunk( argument, ch );
#endif

    /* 
     * Loop through the players that are online and send the clan message to
     * fellow clan members.
     */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim;

        victim = d->original ? d->original : d->character;

        if ( d->connected == CON_PLAYING && ( ( !IS_NPC( victim ) &&
                                                d->character->position >=
                                                POS_SLEEPING
                                                && victim->pcdata->clan ==
                                                ch->pcdata->clan )
                                              || IS_SET( victim->comm,
                                                         COMM_SNOOP_CLAN ) )
             && !IS_SET( victim->comm, COMM_DEAF )
             && !IS_SET( victim->comm, COMM_QUIET )
             && !IS_SET( victim->comm, COMM_NOCLAN ) )
        {
            printf_to_char( victim, "`B%s`R<`w%s`R>`w %s`w\n\r",
                            clan->whoname[0] !=
                            '\0' ? clan->whoname : clan->name,
                            IS_NPC( ch ) ? ch->short_descr : ch->name,
                            argument );
        }
    }                           /* Descriptor Loop */
}

void do_resign( CHAR_DATA * ch, char *argument )
{
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "You cannot resign from a clan, you're a mob!\n\r", ch );
        return;
    }

    if ( ch->pcdata->clan == 0 )
    {
        send_to_char
            ( "You're not in a clan, so how can you resign from one?\n\r", ch );
        return;
    }

    clan = get_clan( ch->pcdata->clan );

    if ( !str_cmp( clan->leader, ch->name ) || !str_cmp( clan->god, ch->name ) )
    {
        send_to_char
            ( "You cannot resign from this clan, you're too high ranked!\n\r",
              ch );
        return;
    }

    if ( ch->pcdata->join_status != JOIN_RESIGNING )
    {
        send_to_char
            ( "You are attempting to remove yourself from your clan.\n\r", ch );
        send_to_char( "Please type RESIGN again to confirm.\n\r", ch );
        ch->pcdata->join_status = JOIN_RESIGNING;
        return;
    }

    clan_log( clan, "%s resigned from the clan!", ch->name );
    ch->pcdata->clan = 0;
    ch->pcdata->clan_rank = 0;
    ch->pcdata->join_status = 0;
    remove_member( ch, clan );
    send_to_char( "You are no longer a member of the clan.\n\r", ch );
    return;
}

void do_cdeposit( CHAR_DATA * ch, char *argument )
{
    CLAN_DATA *clan;
    char arg1[MAX_INPUT_LENGTH];
    int mod;

    argument = one_argument( argument, arg1 );

    if ( IS_NPC( ch ) && !IS_SET( ch->act, ACT_PET ) )
    {
        send_to_char( "Only players can deposit clan gold.\n\r", ch );
        return;
    }

    if ( !IS_NPC( ch ) && ch->pcdata->clan < 1 )
    {
        send_to_char( "The clan deposit command is for clan members only.\n\r",
                      ch );
        return;
    }

    clan = get_clan( ch->pcdata->clan );

    if ( arg1[0] == '\0' )
    {
        printf_to_char( ch, "Balance: %d\n\r", clan->clan_fund );
        return;
    }
    mod = atoi( arg1 );

    if ( mod <= 0 )
    {
        send_to_char( "Positive numbers ONLY.\n\r", ch );
        return;

    }
    if ( ch->gold < mod )
    {
        send_to_char( "CLAN FUND: You don't have that much gold.\n\r", ch );
        return;
    }

    {
        clan->clan_fund += mod;
        ch->gold -= mod;
        printf_to_char( ch, "Balance: %d\n\r", clan->clan_fund );
        clan_log( clan, "%d credited by %s.", mod, ch->name );
        save_clans(  );
        return;
    }
}

void do_crecall( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) && !IS_SET( ch->act, ACT_PET ) )
    {
        send_to_char( "Only players can recall.\n\r", ch );
        return;
    }

    if ( !IS_NPC( ch ) && ch->pcdata->clan < 1 )
    {
        send_to_char( "The clan recall command is for clan members only.\n\r",
                      ch );
        return;
    }

    act( "$n prays for transportation!", ch, 0, 0, TO_ROOM );

    if ( !IS_NPC( ch ) )
    {
        if ( ch->pcdata->clan == 0 )
        {
            send_to_char( "You can't use clan recall, you're not in a clan!",
                          ch );
            return;
        }
        clan = get_clan( ch->pcdata->clan );
        location = get_room_index( clan->recall_room );
    }
    else
    {
        clan = get_clan( ch->master->pcdata->clan );
        location = get_room_index( clan->recall_room );
    }

    if ( location == NULL )
    {
        send_to_char( "Whoops, no such room.\n\r", ch );
        bug( "NULL recall room %d", clan->recall_room );
        return;
    }
    if ( ch->in_room == location )
    {
        send_to_char( "Look around, look familiar?\n\r", ch );
        return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
         || IS_AFFECTED( ch, AFF_CURSE ) )
    {
        send_to_char( "The gods have forsaken you.\n\r", ch );
        return;
    }

    if ( ( victim = ch->fighting ) != NULL )
    {
        int lose, skill;

        if ( IS_NPC( ch ) )
            skill = 40 + ch->level;
        else
            skill = ch->pcdata->learned[gsn_recall];

        if ( number_percent(  ) < 80 * skill / 100 )
        {
            check_improve( ch, gsn_recall, FALSE, 6 );
            WAIT_STATE( ch, 4 );
            send_to_char( "You failed!\n\r", ch );
            return;
        }
        if ( !IS_NPC( ch ) )
        {
            lose = ( int ) ( 0.02 * exp_per_level( ch, ch->pcdata->points ) );
            gain_exp( ch, 0 - lose );
            check_improve( ch, gsn_recall, TRUE, 4 );
            sprintf( buf, "You recall from combat!  You lose %d exps.\n\r",
                     lose );
            send_to_char( buf, ch );
        }
        stop_fighting( ch, TRUE );

    }

    ch->move /= 2;
    send_to_char( "You pray to the gods for transportation!\n\r", ch );
    act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );
    act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );

    if ( ch->pet != NULL )
        do_crecall( ch->pet, "" );

    return;
}

void do_demote( CHAR_DATA * ch, char *argument )
{
    CLAN_DATA *clan;
    CHAR_DATA *victim;
    char buf[MAX_INPUT_LENGTH];

    if ( IS_NPC( ch ) )
        return;

    if ( ch->pcdata == NULL )
        return;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Demote who?\n\r", ch );
        return;
    }

    victim = get_char_world( ch, argument );
    if ( victim == NULL )
    {
        send_to_char( "Demote who?\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Um, you can't demote mobs.\n\r", ch );
        return;
    }

    if ( victim->pcdata->clan == 0 )
    {
        send_to_char( "Um, they must join a clan to be demoted.\n\r", ch );
        return;
    }

    if ( get_trust( ch ) < MAX_LEVEL )  /* IMPS can demote anyone :) */
    {
        if ( ch->pcdata->clan == 0 )
        {
            send_to_char( "You are not a member of a clan!\n\r", ch );
            return;
        }

        if ( ch->pcdata->clan != victim->pcdata->clan )
        {
            send_to_char( "They are not even IN your clan!\n\r", ch );
            return;
        }

        if ( victim->pcdata->clan_rank < 1 )
        {
            victim->pcdata->clan_rank = MAX_RANK;
        }

        if ( ch->pcdata->clan_rank > 1 )
        {
            send_to_char( "You are not high enough rank to demote anyone.\n\r",
                          ch );
            return;
        }

        clan = get_clan( ch->pcdata->clan );

        if ( str_cmp( ch->name, clan->leader ) )
        {
            if ( ( ch->pcdata->clan_rank == 2
                   && victim->pcdata->clan_rank <= 3 )
                 || ( ch->pcdata->clan_rank == 1
                      && victim->pcdata->clan_rank <= 2 ) )
            {
                send_to_char
                    ( "Only the clan leader, an IMP, or a higher rank can demote them.\n\r",
                      ch );
                return;
            }
        }

    }
    else
    {
        clan = get_clan( victim->pcdata->clan );
    }
    if ( victim->pcdata->clan_rank == MAX_RANK )
    {
        send_to_char( "That person's rank is as low as it can be!\n\r", ch );
        return;
    }
    printf_to_char( ch, "You demote %s to %s!\n\r", victim->name,
                    clan->rank[victim->pcdata->clan_rank] );
    victim->pcdata->clan_rank++;
    printf_to_char( victim, "%s demotes you to %s in clan %s! :(\n\r",
                    ch->name, clan->rank[victim->pcdata->clan_rank - 1],
                    clan->name );
    sprintf( buf, "%s demoted %s in clan %d.", ch->name, victim->name,
             clan->number );
    log_string( buf );
    clan_log( clan, "%s demoted %s to %s.", ch->name, victim->name,
              clan->rank[victim->pcdata->clan_rank - 1] );
    return;
}

/* Clan Roster command written by Dorzak */
void do_roster( CHAR_DATA * ch, char *argument )
{
    ROOM_INDEX_DATA *pRoom;
    CLAN_DATA *clan;
    int i, x;

    i = ( ch->pcdata->clan );

    if ( IS_NPC( ch ) )
        return;

    if ( i < 1 )
    {
        send_to_char( "You must join a clan to check its roster.\n\r", ch );
        return;
    }

    clan = get_clan( i );

    if ( clan == NULL )
    {
        send_to_char( "No such clan.\n\r", ch );
        return;
    }

    pRoom = ( get_room_index( clan->recall_room ) );

    printf_to_char( ch, "Name:   [%s]\n\rLeader: [%12s]\n\rSponsor:[%12s]\n\r",
                    clan->name, clan->leader, clan->god );

    send_to_char( "Members:", ch );

    for ( x = 0; x < MAX_CLAN_MEMBERS; x++ )
    {
        printf_to_char( ch, " %s", clan->members[x] );
    }

    send_to_char( "\n\r", ch );
    printf_to_char( ch, "Recall: %s\n\rClan fund: %d\n\r",
                    pRoom->name, clan->clan_fund );
    printf_to_char( ch, "Clan status: %s\n\r",
                    flag_string( clan_flags, clan->clan_flags ) );

    return;
}
