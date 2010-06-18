
#if defined(WIN32)
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "merc.h"
#include "factions.h"
#include "olc.h"

FACTIONLIST_DATA *faction_first = NULL;
FACTIONLIST_DATA *faction_last = NULL;

sh_int faction_count = 0;

const struct olc_cmd_type factionedit_table[] = {
    {"show", factedit_show},
    {"create", factedit_create},
    {"name", factedit_name},
    {"increase", factedit_increase},
    {"decrease", factedit_decrease},
    {"?", show_help},
    {"version", show_version},
    {"commands", show_commands},
    {"", 0,}
};

void load_factions( FILE * fp )
{
    FACTIONLIST_DATA *pFact;
    sh_int vnum;
    char *word;
    char letter;

    for ( ;; )
    {
        word = fread_word( fp );

        /* Exit when "#$" found */
        if ( !str_cmp( word, "#$" ) )
        {
            break;
        }

        /* Start loading when "#FACTIONS" found */
        if ( !str_cmp( word, "#FACTIONS" ) )
        {
            for ( ;; )
            {
                letter = fread_letter( fp );

                /* First thing must be a vnum */
                if ( letter != '#' )
                {
                    bug( "load_factions: # not found while looking for vnum",
                         0 );
                    return;
                }

                vnum = fread_number( fp );

                if ( vnum == 0 )
                {
                    break;
                }

                pFact = get_faction_by_vnum( vnum );

                if ( pFact != NULL )
                {
                    bug( "load_factions: vnum %d duplicated", vnum );
                    return;
                }

                pFact = new_faction(  );

                free_string( &pFact->name );
                free_string( &pFact->increase_msg );
                free_string( &pFact->decrease_msg );

                pFact->vnum = vnum;
                pFact->name = fread_string( fp );
                pFact->increase_msg = fread_string( fp );
                pFact->decrease_msg = fread_string( fp );

                if ( faction_first == NULL )
                {
                    faction_first = pFact;
                    faction_last = pFact;
                }
                else
                {
                    faction_last->next = pFact;
                    faction_last = pFact;
                }
            }

            return;
        }
        else
        {
            bug( "load_factions: bad section name", 0 );
            return;
        }
    }
}

void save_factions( void )
{
    FACTIONLIST_DATA *pFact;
    FILE *fp;
    char buf[1024];

    sprintf( buf, "%s/%s", sysconfig.area_dir, sysconfig.factions_file );

    fp = fopen( buf, "w" );

    if ( !fp )
    {
        bug( "Cannot write to %s !!!", sysconfig.factions_file );
        return;
    }

    fprintf( fp, "#FACTIONS\n\n" );

    for ( pFact = faction_first; pFact != NULL; pFact = pFact->next )
    {
        fprintf( fp, "#%d\n", pFact->vnum );
        fprintf( fp, "%s~\n", pFact->name );
        fprintf( fp, "%s~\n", pFact->increase_msg );
        fprintf( fp, "%s~\n\n", pFact->decrease_msg );
    }

    fprintf( fp, "#0\n" );

    fclose( fp );
}

FACTIONLIST_DATA *new_faction( void )
{
    FACTIONLIST_DATA *pFact;

    pFact = alloc_perm( sizeof( FACTIONLIST_DATA ) );

    if ( !pFact )
    {
        bug( "new_faction: Call to alloc_perm failed!", 0 );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    faction_count++;

    pFact->next = NULL;
    pFact->vnum = faction_count;
    pFact->name = str_dup( "Unnamed" );
    pFact->increase_msg = str_dup( "Your unnamed faction has increased" );
    pFact->decrease_msg = str_dup( "Your unnamed faction has decreased" );

    return pFact;
}

FACTIONLIST_DATA *get_faction_by_vnum( sh_int vnum )
{
    FACTIONLIST_DATA *pFact;

    for ( pFact = faction_first; pFact != NULL; pFact = pFact->next )
    {
        if ( pFact->vnum == vnum )
            break;
    }

    return pFact;
}

void do_factionedit( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    FACTIONLIST_DATA *pFact;
    sh_int vnum;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "NPCs can't do that!\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( is_number( arg ) )
    {
        vnum = atoi( arg );

        pFact = get_faction_by_vnum( vnum );

        if ( !pFact )
        {
            send_to_char( "That faction number does not exist!\n\r", ch );
            return;
        }

        ch->desc->pEdit = ( void * ) pFact;
        ch->desc->editor = ED_FACTION;
        SET_BIT( ch->act, PLR_BUILDING );
        act( "$n has entered the Factions Editor.", ch, NULL, NULL, TO_ROOM );

        return;
    }
    else
    {
        if ( !str_cmp( arg, "create" ) )
        {
            if ( factedit_create( ch, argument ) )
            {
                ch->desc->editor = ED_FACTION;
                SET_BIT( ch->act, PLR_BUILDING );
                act( "$n has entered the Factions Editor.", ch, NULL, NULL,
                     TO_ROOM );

            }

            return;
        }
    }

    send_to_char( "There is no default faction to edit.\n\r", ch );
    return;
}

void factionedit( CHAR_DATA * ch, char *argument )
{
    FACTIONLIST_DATA *pFact;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde( argument );
    strncpy( arg, argument, sizeof( arg ) - 1 );
    arg[sizeof( arg )] = '\0';
    argument = one_argument( argument, command );

    pFact = ( FACTIONLIST_DATA * ) ch->desc->pEdit;

    if ( !str_cmp( command, "done" ) )
    {
        edit_done( ch );
        return;
    }

    if ( get_trust( ch ) < FACTION_EDIT_LEVEL )
    {
        send_to_char( "Insufficient security to modify factions.\n\r", ch );
        interpret( ch, arg );
        return;
    }

    if ( command[0] == '\0' )
    {
        factedit_show( ch, argument );
        return;
    }

    if ( !pFact )
    {
        send_to_char( "You are not currently editing a faction.\n\r", ch );
        interpret( ch, arg );
        return;
    }

    /* Search table and dispatch command */
    for ( cmd = 0; factionedit_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( !str_prefix( command, factionedit_table[cmd].name ) )
        {
            if ( ( *factionedit_table[cmd].olc_fun ) ( ch, argument ) )
            {
                return;
            }
            else
            {
                return;
            }
        }
    }

    /* If command not found, default to the standard interpreter. */
    interpret( ch, arg );
    return;
}

bool factedit_show( CHAR_DATA * ch, char *argument )
{
    FACTIONLIST_DATA *pFact;

    pFact = ( FACTIONLIST_DATA * ) ch->desc->pEdit;

    if ( !pFact )
    {
        send_to_char( "You are not currently editing a faction.\n\r", ch );
    }
    else
    {
        printf_to_char( ch, "VNum:             [%-5d]\n\r", pFact->vnum );
        printf_to_char( ch, "Name:             %s\n\r", pFact->name );
        printf_to_char( ch, "Increase Message: %s\n\r", pFact->increase_msg );
        printf_to_char( ch, "Decrease Message: %s\n\r", pFact->decrease_msg );
    }

    return FALSE;
}

bool factedit_create( CHAR_DATA * ch, char *argument )
{
    FACTIONLIST_DATA *pFact = new_faction(  );

    if ( !pFact )
    {
        send_to_char( "ERROR!  Could not create a new faction!\n\r", ch );
        return FALSE;
    }

    if ( faction_first == NULL )
    {
        faction_first = pFact;
        faction_last = pFact;
    }
    else
    {
        faction_last->next = pFact;
        faction_last = pFact;
    }

    send_to_char( "Faction created.\n\r", ch );

    ch->desc->pEdit = ( void * ) pFact;

    return TRUE;
}

bool factedit_name( CHAR_DATA * ch, char *argument )
{
    FACTIONLIST_DATA *pFact;

    pFact = ( FACTIONLIST_DATA * ) ch->desc->pEdit;

    if ( !pFact )
    {
        send_to_char( "You are not currently editing a faction.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  name <faction name>\n\r", ch );
        return FALSE;
    }

    if ( strlen( argument ) != str_len( argument ) )
    {
        send_to_char
            ( "ERROR!  Color codes are not allowed in faction names.\n\r", ch );
        return FALSE;
    }

    if ( strlen( argument ) > 20 )
    {
        send_to_char
            ( "ERROR!  Faction names must be 20 characters or less.\n\r", ch );
        return FALSE;
    }

    free_string( &pFact->name );
    pFact->name = str_dup( argument );

    send_to_char( "Faction name set.\n\r", ch );

    return TRUE;
}

bool factedit_increase( CHAR_DATA * ch, char *argument )
{
    FACTIONLIST_DATA *pFact;

    pFact = ( FACTIONLIST_DATA * ) ch->desc->pEdit;

    if ( !pFact )
    {
        send_to_char( "You are not currently editing a faction.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  increase <increase message>\n\r", ch );
        return FALSE;
    }

    free_string( &pFact->increase_msg );
    pFact->increase_msg = str_dup( argument );

    send_to_char( "Faction increase message set.\n\r", ch );

    return TRUE;
}

bool factedit_decrease( CHAR_DATA * ch, char *argument )
{
    FACTIONLIST_DATA *pFact;

    pFact = ( FACTIONLIST_DATA * ) ch->desc->pEdit;

    if ( !pFact )
    {
        send_to_char( "You are not currently editing a faction.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  decrease <decrease message>\n\r", ch );
        return FALSE;
    }

    free_string( &pFact->decrease_msg );
    pFact->decrease_msg = str_dup( argument );

    send_to_char( "Faction decrease message set.\n\r", ch );

    return TRUE;
}

void load_factionaffs( FILE * fp )
{
    FACTIONAFF_DATA *pFactAff;
    FACTIONLIST_DATA *pFact;

    if ( !area_last )
    {
        bug( "Load_factionaffs: no #AREA seen yet.", 0 );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    for ( ;; )
    {
        MOB_INDEX_DATA *pMob;
        sh_int number;

        number = fread_number( fp );

        if ( number == 0 )
            break;

        pMob = get_mob_index( number );

        if ( !pMob )
        {
            bug( "Load_factionaffs: Bad mobile vnum %d in #FACTIONAFFS",
                 number );
#if defined(cbuilder)
            return -1;
#else
            exit( 1 );
#endif
        }

        number = fread_number( fp );

        pFact = get_faction_by_vnum( number );

        if ( !pFact )
        {
            bug( "Load_factionaffs: Bad faction vnum %d in #FACTIONAFFS",
                 number );
#if defined(cbuilder)
            return -1;
#else
            exit( 1 );
#endif
        }

        if ( pMob->faction_affs == NULL )
        {
            pMob->faction_affs = alloc_mem( sizeof( FACTIONAFF_DATA ) );
            pMob->faction_affs->faction = pFact;
            pMob->faction_affs->change = fread_number( fp );
            pMob->faction_affs->next = NULL;
        }
        else
        {
            for ( pFactAff = pMob->faction_affs; pFactAff->next != NULL;
                  pFactAff = pFactAff->next );
            pFactAff->next = alloc_mem( sizeof( FACTIONAFF_DATA ) );
            pFactAff->next->faction = pFact;
            pFactAff->next->change = fread_number( fp );
            pFactAff->next->next = NULL;
        }

        fread_to_eol( fp );
    }
}

void affect_factions( CHAR_DATA * ch, CHAR_DATA * victim )
{
    FACTIONAFF_DATA *pFactAff;
    FACTIONPC_DATA *pFactPC;
    CHAR_DATA *group_leader, *group_ch;

    if ( IS_NPC( ch ) )
    {
        bug( "affect_factions: ch is an NPC?  Doing nothing.", 0 );
        return;
    }

    if ( !IS_NPC( victim ) )
    {
        bug( "affect_factions: victim isn't an NPC?  Doing nothing.", 0 );
        return;
    }

    for ( pFactAff = victim->pIndexData->faction_affs;
          pFactAff != NULL; pFactAff = pFactAff->next )
    {
        group_leader = ( ch->leader != NULL ) ? ch->leader : ch;

        for ( group_ch = player_list; group_ch != NULL;
              group_ch = group_ch->next )
        {
            if ( is_same_group( group_ch, ch ) )
            {
                for ( pFactPC = group_ch->pcdata->faction_standings;
                      pFactPC != NULL; pFactPC = pFactPC->next )
                {
                    if ( pFactPC->faction == pFactAff->faction )
                        break;
                }

                if ( !pFactPC )
                {
                    pFactPC = alloc_mem( sizeof( FACTIONPC_DATA ) );
                    pFactPC->faction = pFactAff->faction;
                    pFactPC->value = CFG_FACTION_INITIAL_VALUE;
                    pFactPC->next = group_ch->pcdata->faction_standings;
                    group_ch->pcdata->faction_standings = pFactPC;
                }

                pFactPC->value += pFactAff->change;

                if ( pFactPC->value > CFG_FACTION_MAX_VALUE )
                {
                    pFactPC->value = CFG_FACTION_MAX_VALUE;
                }

                if ( pFactPC->value < CFG_FACTION_MIN_VALUE )
                {
                    pFactPC->value = CFG_FACTION_MIN_VALUE;
                }

#ifdef FACTION_SHOW_CHANGE_AMOUNT
                printf_to_char( group_ch, "%s [%d]\n\r",
                                ( pFactAff->change >
                                  0 ) ? pFactAff->faction->
                                increase_msg : pFactAff->faction->decrease_msg,
                                pFactAff->change );
#else
                printf_to_char( group_ch, "%s\n\r",
                                ( pFactAff->change >
                                  0 ) ? pFactAff->faction->
                                increase_msg : pFactAff->faction->
                                decrease_msg );
#endif
            }
        }
    }
}

void fread_faction_standings( CHAR_DATA * ch, FILE * fp )
{
    FACTIONLIST_DATA *pFact;
    FACTIONPC_DATA *pFactPC;
    sh_int number;

    if ( IS_NPC( ch ) )
    {
        return;
    }

    for ( ;; )
    {
        number = feof( fp ) ? 0 : fread_number( fp );

        if ( number == 0 )
        {
            break;
        }

        pFact = get_faction_by_vnum( number );

        if ( !pFact )
        {
            bug( "fread_faction_standings: Unknown faction vnum %d found in pfile!", number );
#if defined(cbuilder)
            return -1;
#else
            exit( 1 );
#endif
        }

        pFactPC = alloc_mem( sizeof( FACTIONPC_DATA ) );

        if ( !pFactPC )
        {
            bug( "fread_faction_standings: call to alloc_mem failed!", 0 );
#if defined(cbuilder)
            return -1;
#else
            exit( 1 );
#endif
        }

        pFactPC->faction = pFact;
        pFactPC->value = fread_number( fp );
        pFactPC->next = NULL;

        if ( ch->pcdata->faction_standings == NULL )
        {
            ch->pcdata->faction_standings = pFactPC;
        }
        else
        {
            pFactPC->next = ch->pcdata->faction_standings;
            ch->pcdata->faction_standings = pFactPC;
        }
    }
}

void fwrite_faction_standings( CHAR_DATA * ch, FILE * fp )
{
    FACTIONPC_DATA *pFactPC;
    sh_int count = 0;

    if ( IS_NPC( ch ) )
    {
        return;
    }

    fprintf( fp, "#FACTIONS\n" );

    for ( pFactPC = ch->pcdata->faction_standings;
          pFactPC != NULL; pFactPC = pFactPC->next )
    {
        count++;

        fprintf( fp, "%5d %5d ", pFactPC->faction->vnum, pFactPC->value );

        if ( count >= 5 )
        {
            fprintf( fp, "\n" );
            count = 0;
        }
    }

    fprintf( fp, "\n0\n\n" );
}

void free_faction_standings( FACTIONPC_DATA * pFactPC )
{
    FACTIONPC_DATA *tmp;

    while ( pFactPC )
    {
        tmp = pFactPC;
        pFactPC = pFactPC->next;

        free_mem( &tmp );
    }
}

char *add_to_buf( char *buf, char *txt )
{
    char *tmp;

    if ( buf == NULL )
    {
        buf = malloc( strlen( txt ) + 1 );
        tmp = buf;
    }
    else
    {
        buf = realloc( buf, strlen( buf ) + strlen( txt ) + 1 );
        tmp = buf + strlen( buf );
    }

    while ( *txt != '\0' )
    {
        *tmp = *txt;
        tmp++;
        txt++;
    }

    *tmp = '\0';

    return buf;
}

void show_faction_standings( CHAR_DATA * ch, char *argument )
{
    FACTIONPC_DATA *pFactPC;
    char *buffer = NULL;
    char buf[MAX_STRING_LENGTH];

    if ( !ch->pcdata->faction_standings )
    {
        send_to_char( "You have no factions!\n\r", ch );
        return;
    }

    buffer = add_to_buf( buffer, "`WFaction                Value\n\r" );
    buffer =
        add_to_buf( buffer,
                    "`K----------------------------------------------------------------------\n\r" );

    for ( pFactPC = ch->pcdata->faction_standings;
          pFactPC != NULL; pFactPC = pFactPC->next )
    {
        sprintf( buf, "`w%-20s   ", pFactPC->faction->name );
        buffer = add_to_buf( buffer, buf );

        if ( CFG_FACTION_MESSAGES < 2 )
        {
            sprintf( buf, "%s ", faction_con_msg( pFactPC->value ) );
            buffer = add_to_buf( buffer, buf );
        }
        if ( CFG_FACTION_MESSAGES > 0 )
        {
            sprintf( buf, "[%d%%]", faction_percentage( pFactPC->value ) );
            buffer = add_to_buf( buffer, buf );
        }

        buffer = add_to_buf( buffer, "\n\r" );
    }

    page_to_char( buffer, ch );
}

char *faction_con_msg( sh_int value )
{
    sh_int percentage;

    percentage = faction_percentage( value );

    if ( percentage > 0 )
    {
        if ( percentage <= 15 )
            return CFG_FACTION_CON_15MAX;
        if ( percentage <= 30 )
            return CFG_FACTION_CON_30MAX;
        if ( percentage <= 45 )
            return CFG_FACTION_CON_45MAX;
        if ( percentage <= 60 )
            return CFG_FACTION_CON_60MAX;
        if ( percentage <= 75 )
            return CFG_FACTION_CON_75MAX;
        if ( percentage <= 90 )
            return CFG_FACTION_CON_90MAX;
        return CFG_FACTION_CON_MAX;
    }

    if ( percentage < 0 )
    {
        if ( percentage >= -15 )
            return CFG_FACTION_CON_15MIN;
        if ( percentage >= -30 )
            return CFG_FACTION_CON_30MIN;
        if ( percentage >= -45 )
            return CFG_FACTION_CON_45MIN;
        if ( percentage >= -60 )
            return CFG_FACTION_CON_60MIN;
        if ( percentage >= -75 )
            return CFG_FACTION_CON_75MIN;
        if ( percentage >= -90 )
            return CFG_FACTION_CON_90MIN;
        return CFG_FACTION_CON_MIN;
    }

    return CFG_FACTION_CON_0;
}

sh_int faction_percentage( sh_int value )
{
    sh_int percentage;

    if ( value == 0 )
    {
        percentage = 0;
    }
    else if ( value > 0 )
    {
        percentage =
            ( sh_int ) ( 100 * ( double ) value /
                         ( double ) CFG_FACTION_MAX_VALUE );
    }
    else
    {
        percentage =
            ( sh_int ) ( 100 * ( double ) value /
                         fabs( ( double ) CFG_FACTION_MIN_VALUE ) );
    }

    return percentage;
}

sh_int consider_factions( CHAR_DATA * ch, CHAR_DATA * victim, bool show )
{
    FACTIONAFF_DATA *pFactAff;
    FACTIONPC_DATA *pFactPC;
    sh_int value = 0;
    sh_int count = 0;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
        return 0;
    if ( !IS_NPC( victim ) )
        return 0;

    if ( !can_see( victim, ch ) )
    {
        if ( show )
        {
            sprintf( buf, "$N %s", faction_con_msg( 0 ) );
            act( buf, ch, NULL, victim, TO_CHAR );
        }

        return 0;
    }

    /* Loop through all faction decreases on the victim and average 
     * the matching values on the ch */
    for ( pFactAff = victim->pIndexData->faction_affs;
          pFactAff != NULL; pFactAff = pFactAff->next )
    {
        if ( pFactAff->change < 0 )
        {
            for ( pFactPC = ch->pcdata->faction_standings;
                  pFactPC != NULL; pFactPC = pFactPC->next )
            {
                if ( pFactPC->faction == pFactAff->faction )
                {
                    count++;
                    value = value + pFactPC->value;
                }
            }
        }
    }

    if ( count != 0 )
    {
        value = value / count;
    }

    if ( show )
    {
        sprintf( buf, "$N %s", faction_con_msg( value ) );
        act( buf, ch, NULL, victim, TO_CHAR );
    }

    return value;
}

void set_faction( CHAR_DATA * ch, CHAR_DATA * victim, sh_int vnum,
                  sh_int value )
{
    FACTIONPC_DATA *pFactPC;
    FACTIONLIST_DATA *pFact;

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Cannot set faction on a mobile.\n\r", ch );
        return;
    }

    pFact = get_faction_by_vnum( vnum );

    if ( !pFact )
    {
        send_to_char( "No such faction.\n\r", ch );
        return;
    }

    if ( ( value > CFG_FACTION_MAX_VALUE )
         || ( value < CFG_FACTION_MIN_VALUE ) )
    {
        printf_to_char( ch, "Faction values must be in the range %d to %d\n\r",
                        CFG_FACTION_MIN_VALUE, CFG_FACTION_MAX_VALUE );
        return;
    }

    for ( pFactPC = victim->pcdata->faction_standings;
          pFactPC != NULL; pFactPC = pFactPC->next )
    {
        if ( pFactPC->faction == pFact )
        {
#ifdef FACTION_SHOW_CHANGE_AMOUNT
            printf_to_char( victim, "%s [%d]\n\r",
                            ( pFactPC->value <
                              value ) ? pFact->increase_msg : pFact->
                            decrease_msg, value - pFactPC->value );
#else
            printf_to_char( victim, "%s\n\r",
                            ( pFactPC->value <
                              value ) ? pFact->increase_msg : pFact->
                            decrease_msg );
#endif
            printf_to_char( ch, "%s %s's faction #%d (%s) to %d\n\r",
                            ( pFactPC->value <
                              value ) ? "Increased" : "Decreased", victim->name,
                            vnum, pFact->name, value );

            pFactPC->value = value;

            return;
        }
    }

    printf_to_char( ch, "%s has no faction #%d.  Creating...\n\r", victim->name,
                    vnum );

    pFactPC = alloc_mem( sizeof( FACTIONPC_DATA ) );
    pFactPC->faction = pFact;
    pFactPC->value = CFG_FACTION_INITIAL_VALUE;
    pFactPC->next = victim->pcdata->faction_standings;
    victim->pcdata->faction_standings = pFactPC;

#ifdef FACTION_SHOW_CHANGE_AMOUNT
    printf_to_char( victim, "%s [%d]\n\r",
                    ( pFactPC->value <
                      value ) ? pFact->increase_msg : pFact->decrease_msg,
                    value - pFactPC->value );
#else
    printf_to_char( victim, "%s\n\r",
                    ( pFactPC->value <
                      value ) ? pFact->increase_msg : pFact->decrease_msg );
#endif
    printf_to_char( ch, "%s %s's faction #%d (%s) to %d\n\r",
                    ( pFactPC->value < value ) ? "Increased" : "Decreased",
                    victim->name, vnum, pFact->name, value );

    pFactPC->value = value;

    return;
}

void faction_stat( CHAR_DATA * ch, CHAR_DATA * victim )
{
    FACTIONAFF_DATA *pFactAff;
    FACTIONPC_DATA *pFactPC;
    char *buffer = NULL;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( victim ) )
    {
        sprintf( buf, "\n\rKilling %s will affect the following factions:\n\r",
                 victim->name );
        buffer = add_to_buf( buffer, buf );

        for ( pFactAff = victim->pIndexData->faction_affs;
              pFactAff != NULL; pFactAff = pFactAff->next )
        {
            sprintf( buf, "%s will be %s by %d points\n\r",
                     pFactAff->faction->name,
                     ( pFactAff->change > 0 ) ? "increased" : "decreased",
                     abs( pFactAff->change ) );
            buffer = add_to_buf( buffer, buf );
        }
    }
    else
    {
        buffer = add_to_buf( buffer, "\n\r`WFaction                Value\n\r" );
        buffer =
            add_to_buf( buffer,
                        "`K----------------------------------------------------------------------\n\r" );

        for ( pFactPC = victim->pcdata->faction_standings;
              pFactPC != NULL; pFactPC = pFactPC->next )
        {
            sprintf( buf, "%-20s   %s [%d]\n\r", pFactPC->faction->name,
                     faction_con_msg( pFactPC->value ), pFactPC->value );
            buffer = add_to_buf( buffer, buf );
        }
    }

    page_to_char( buffer, ch );
}

void mpchangefaction( CHAR_DATA * ch, char *argument, bool silent )
{
    FACTIONLIST_DATA *pFact;
    FACTIONPC_DATA *pFactPC;
    CHAR_DATA *victim;
    sh_int value;
    char arg1[MAX_INPUT_LENGTH];    /* victim name  */
    char arg2[MAX_INPUT_LENGTH];    /* faction vnum */

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 )
         || !is_number( argument ) )
    {
        bug( "mpchangefaction: incorrect syntax" );
        return;
    }

    victim = get_char_world( ch, arg1 );

    if ( !victim )
    {
        return;
    }

    if ( IS_NPC( victim ) )
    {
        return;
    }

    pFact = get_faction_by_vnum( ( sh_int ) atoi( arg2 ) );
    value = atoi( argument );

    if ( !pFact )
    {
        return;
    }

    for ( pFactPC = victim->pcdata->faction_standings;
          pFactPC != NULL; pFactPC = pFactPC->next )
    {
        if ( pFactPC->faction == pFact )
            break;
    }

    if ( !pFactPC )
    {
        pFactPC = alloc_mem( sizeof( FACTIONPC_DATA ) );
        pFactPC->faction = pFact;
        pFactPC->value = CFG_FACTION_INITIAL_VALUE;
        pFactPC->next = victim->pcdata->faction_standings;
        victim->pcdata->faction_standings = pFactPC;
    }

    pFactPC->value += value;

    if ( pFactPC->value > CFG_FACTION_MAX_VALUE )
    {
        pFactPC->value = CFG_FACTION_MAX_VALUE;
    }

    if ( pFactPC->value < CFG_FACTION_MIN_VALUE )
    {
        pFactPC->value = CFG_FACTION_MIN_VALUE;
    }

    if ( !silent )
    {
#ifdef FACTION_SHOW_CHANGE_AMOUNT
        printf_to_char( victim, "%s [%d]\n\r",
                        ( value >
                          0 ) ? pFact->increase_msg : pFact->decrease_msg,
                        value );
#else
        printf_to_char( victim, "%s\n\r",
                        ( value >
                          0 ) ? pFact->increase_msg : pFact->decrease_msg );
#endif
    }
}

void do_mpsilentchangefaction( CHAR_DATA * ch, char *argument )
{
    mpchangefaction( ch, argument, TRUE );
}

void do_mpchangefaction( CHAR_DATA * ch, char *argument )
{
    mpchangefaction( ch, argument, FALSE );
}

void do_factionfind( CHAR_DATA * ch, char *argument )
{
    FACTIONLIST_DATA *pFact;
    char *buffer = NULL;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Find what?\n\r", ch );
        return;
    }

    buffer = add_to_buf( buffer, "`Wvnum  name\n\r" );
    buffer = add_to_buf( buffer, "`K---------------------------`w\n\r" );

    for ( pFact = faction_first; pFact != NULL; pFact = pFact->next )
    {
        if ( ( is_name( argument, pFact->name ) )
             || ( !str_cmp( argument, "all" ) ) )
        {
            sprintf( buf, "%5d %-s\n\r", pFact->vnum, pFact->name );
            buffer = add_to_buf( buffer, buf );

            found = TRUE;
        }
    }

    if ( !found )
    {
        send_to_char( "No factions found by that name.\n\r", ch );
    }
    else
    {
        page_to_char( buffer, ch );
    }
}

bool medit_faction( CHAR_DATA * ch, char *argument )
{
    FACTIONLIST_DATA *pFact;
    FACTIONAFF_DATA *pFactAff;
    FACTIONAFF_DATA *tmp;
    MOB_INDEX_DATA *pMob;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax:\n\r", ch );
        send_to_char( "  faction add <vnum> <change>\n\r", ch );
        send_to_char( "  faction delete <vnum>\n\r", ch );
        return FALSE;
    }

    if ( ( arg2[0] == '\0' ) || ( !is_number( arg2 ) ) )
    {
        send_to_char( "What faction?\n\r", ch );
        return FALSE;
    }

    pFact = get_faction_by_vnum( ( sh_int ) atoi( arg2 ) );

    if ( !pFact )
    {
        send_to_char( "No such faction.\n\r", ch );
        return FALSE;
    }

    EDIT_MOB( ch, pMob );

    for ( pFactAff = pMob->faction_affs;
          pFactAff != NULL; pFactAff = pFactAff->next )
    {
        if ( pFactAff->faction == pFact )
            break;
    }

    if ( !str_prefix( arg1, "add" ) )
    {
        if ( pFactAff != NULL )
        {
            send_to_char( "The mobile already has that faction.\n\r", ch );
            return FALSE;
        }

        if ( !is_number( argument ) )
        {
            send_to_char( "The change amount must be numeric.\n\r", ch );
            return FALSE;
        }

        pFactAff = alloc_mem( sizeof( FACTIONAFF_DATA ) );
        pFactAff->faction = pFact;
        pFactAff->change = atoi( argument );

        if ( pMob->faction_affs == NULL )
        {
            pFactAff->next = NULL;
            pMob->faction_affs = pFactAff;
        }
        else
        {
            pFactAff->next = pMob->faction_affs;
            pMob->faction_affs = pFactAff;
        }

        printf_to_char( ch, "Faction #%d added.\n\r", atoi( arg2 ) );

        return TRUE;
    }

    if ( !str_prefix( arg1, "delete" ) )
    {
        if ( !pFactAff )
        {
            send_to_char( "The mobile doesn't appear to have that faction.\n\r",
                          ch );
            return FALSE;
        }

        if ( pMob->faction_affs == pFactAff )
        {
            free_mem( &pMob->faction_affs );
            pMob->faction_affs = NULL;

            printf_to_char( ch, "Faction #%d removed.\n\r", atoi( arg2 ) );

            return TRUE;
        }

        for ( tmp = pMob->faction_affs; tmp->next != pFactAff;
              tmp = tmp->next );

        tmp->next = pFactAff->next;

        free_mem( &pFactAff );

        printf_to_char( ch, "Faction #%d removed.\n\r", atoi( arg2 ) );

        return TRUE;
    }

    //UNREACHED
    return TRUE;
}

double faction_cost_multiplier( CHAR_DATA * ch, CHAR_DATA * keeper, bool buy )
{
    int percentage;

    percentage = faction_percentage( consider_factions( ch, keeper, FALSE ) );

    if ( buy )
    {
        if ( percentage > 0 )
        {
            if ( percentage <= 15 )
                return CFG_FACTION_SHOPKEEPER_15MAX;
            if ( percentage <= 30 )
                return CFG_FACTION_SHOPKEEPER_30MAX;
            if ( percentage <= 45 )
                return CFG_FACTION_SHOPKEEPER_45MAX;
            if ( percentage <= 60 )
                return CFG_FACTION_SHOPKEEPER_60MAX;
            if ( percentage <= 75 )
                return CFG_FACTION_SHOPKEEPER_75MAX;
            if ( percentage <= 90 )
                return CFG_FACTION_SHOPKEEPER_90MAX;
            return CFG_FACTION_SHOPKEEPER_MAX;
        }

        if ( percentage < 0 )
        {
            if ( percentage >= -15 )
                return CFG_FACTION_SHOPKEEPER_15MIN;
            if ( percentage >= -30 )
                return CFG_FACTION_SHOPKEEPER_30MIN;
            if ( percentage >= -45 )
                return CFG_FACTION_SHOPKEEPER_45MIN;
            if ( percentage >= -60 )
                return CFG_FACTION_SHOPKEEPER_60MIN;
            if ( percentage >= -75 )
                return CFG_FACTION_SHOPKEEPER_75MIN;
            if ( percentage >= -90 )
                return CFG_FACTION_SHOPKEEPER_90MIN;
            return CFG_FACTION_SHOPKEEPER_MIN;
        }
    }

    if ( !buy )
    {
        if ( percentage > 0 )
        {
            if ( percentage <= 15 )
                return ( ( 1 - CFG_FACTION_SHOPKEEPER_15MAX ) + 1 );
            if ( percentage <= 30 )
                return ( ( 1 - CFG_FACTION_SHOPKEEPER_30MAX ) + 1 );
            if ( percentage <= 45 )
                return ( ( 1 - CFG_FACTION_SHOPKEEPER_45MAX ) + 1 );
            if ( percentage <= 60 )
                return ( ( 1 - CFG_FACTION_SHOPKEEPER_60MAX ) + 1 );
            if ( percentage <= 75 )
                return ( ( 1 - CFG_FACTION_SHOPKEEPER_75MAX ) + 1 );
            if ( percentage <= 90 )
                return ( ( 1 - CFG_FACTION_SHOPKEEPER_90MAX ) + 1 );
            return ( ( 1 - CFG_FACTION_SHOPKEEPER_MAX ) + 1 );
        }

        if ( percentage < 0 )
        {
            if ( percentage >= -15 )
                return ( 1 - ( CFG_FACTION_SHOPKEEPER_15MIN - 1 ) ) <
                    0 ? 0 : ( 1 - ( CFG_FACTION_SHOPKEEPER_15MIN - 1 ) );
            if ( percentage >= -30 )
                return ( 1 - ( CFG_FACTION_SHOPKEEPER_30MIN - 1 ) ) <
                    0 ? 0 : ( 1 - ( CFG_FACTION_SHOPKEEPER_30MIN - 1 ) );
            if ( percentage >= -45 )
                return ( 1 - ( CFG_FACTION_SHOPKEEPER_45MIN - 1 ) ) <
                    0 ? 0 : ( 1 - ( CFG_FACTION_SHOPKEEPER_45MIN - 1 ) );
            if ( percentage >= -60 )
                return ( 1 - ( CFG_FACTION_SHOPKEEPER_60MIN - 1 ) ) <
                    0 ? 0 : ( 1 - ( CFG_FACTION_SHOPKEEPER_60MIN - 1 ) );
            if ( percentage >= -75 )
                return ( 1 - ( CFG_FACTION_SHOPKEEPER_75MIN - 1 ) ) <
                    0 ? 0 : ( 1 - ( CFG_FACTION_SHOPKEEPER_75MIN - 1 ) );
            if ( percentage >= -90 )
                return ( 1 - ( CFG_FACTION_SHOPKEEPER_90MIN - 1 ) ) <
                    0 ? 0 : ( 1 - ( CFG_FACTION_SHOPKEEPER_90MIN - 1 ) );
            return ( 1 - ( CFG_FACTION_SHOPKEEPER_MIN - 1 ) ) <
                0 ? 0 : ( 1 - ( CFG_FACTION_SHOPKEEPER_MIN - 1 ) );
        }
    }

    return 1;
}

void save_factionaffs( FILE * fp, AREA_DATA * pArea )
{
    FACTIONAFF_DATA *f;
    MOB_INDEX_DATA *mob;
    int iHash;

    fprintf( fp, "#FACTIONAFFS\n" );

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( mob = mob_index_hash[iHash]; mob; mob = mob->next )
        {
            if ( mob && mob->area == pArea && mob->faction_affs )
            {
                for ( f = mob->faction_affs; f != NULL; f = f->next )
                {
                    fprintf( fp, "%d %d %d\n",
                             mob->vnum, f->faction->vnum, f->change );
                }
            }
        }
    }

    fprintf( fp, "0\n\n\n\n" );
    return;
}
