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
#include "merc.h"
#include "mud_progs.h"

extern void parse_command_var( char var, char *outbuf );

/*
 * MudProgs Command table.
 *
 * Format:  { <proc name>,    <proc pointer>,    <arg type> },
 * 
 * Argtype can be one of:
 *
 *     'C'  - Character Pointer (CHAR_DATA *)
 *     'O'  - Object Pointer (OBJ_DATA *)
 *     'I'  - Numeric Value
 *     'S'  - String (char *)
 *     'N'  - No args
 * 
 * Execproc guarantees that the proper argument type is passed to avoid
 * crashes.  If you specify the wrong argtype here and expect a different argtype
 * in your proc then you will crash.  All arguments passed to these procs are of
 * type (void *) and need to be cast to the proper type before used.
 *
 * -Zane
 */

const struct mprog_cmd_type mprog_cmd_table[] = {
    {"alignment", mprog_alignment, 'C'},
    {"clan", mprog_clan, 'C'},
    {"class", mprog_class, 'C'},
    {"crimethief", mprog_crimethief, 'C'},
    {"fightinroom", mprog_fightinroom, 'N'},
    {"getrand", mprog_getrand, 'I'},
    {"goldamount", mprog_goldamount, 'C'},
    {"hasmemory", mprog_hasmemory, 'N'},
    {"hitpercent", mprog_hitpercent, 'C'},
    {"hour", mprog_hour, 'N'},
    {"immune", mprog_immune, 'C'},
    {"isawake", mprog_isawake, 'C'},
    {"ischarmed", mprog_ischarmed, 'C'},
    {"isfight", mprog_isfight, 'C'},
    {"isfollow", mprog_isfollow, 'C'},
    {"isimmort", mprog_isimmort, 'C'},
    {"isgood", mprog_isgood, 'C'},
    {"isname", mprog_isname, 'S'},
    {"isnpc", mprog_isnpc, 'C'},
    {"ispc", mprog_ispc, 'C'},
    {"level", mprog_level, 'C'},
    {"memory", mprog_memory, 'C'},
    {"mobvnum", mprog_mobvnum, 'C'},
    {"objtype", mprog_objtype, 'O'},
    {"objval0", mprog_objval0, 'O'},
    {"objval1", mprog_objval1, 'O'},
    {"objval2", mprog_objval2, 'O'},
    {"objval3", mprog_objval3, 'O'},
    {"objvnum", mprog_objvnum, 'O'},
    {"position", mprog_position, 'C'},
    {"rand", mprog_rand, 'I'},
    {"roomvnum", mprog_roomvnum, 'N'},
    {"sex", mprog_sex, 'C'},
    {"sgetrand", mprog_sgetrand, 'I'},
    {"sreset", mprog_sreset, 'N'},
    {"faction", mprog_faction, 'S'},
    {"isequal", mprog_isequal, 'S'},
    /*
     * End of list.
     */
    {"", 0, 0,}
};

int mprog_alignment( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( ( CHAR_DATA * ) vo )->alignment;
}

int mprog_clan( void *vo )
{
    CHAR_DATA *ch = ( CHAR_DATA * ) vo;

    if ( !ch )
        return FALSE;

    if ( IS_NPC( ch ) )
        return FALSE;

    return ch->pcdata->clan;
}

int mprog_class( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( ( CHAR_DATA * ) vo )->Class;
}

int mprog_crimethief( void *vo )
{
    if ( !vo )
        return FALSE;

    return IS_SET( ( ( CHAR_DATA * ) vo )->act, PLR_THIEF );
}

int mprog_fightinroom( void *vo )
{
    CHAR_DATA *ch;

    if ( !ProgSource->in_room )
        return FALSE;

    for ( ch = ProgSource->in_room->people; ch; ch = ch->next_in_room )
        if ( ch->fighting )
            return TRUE;

    return FALSE;
}

int mprog_getrand( void *vo )
{
    if ( !vo )
        return FALSE;

    return number_range( 1, *( ( int * ) vo ) );
}

int mprog_goldamount( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( ( CHAR_DATA * ) vo )->gold;
}

int mprog_hasmemory( void *vo )
{
    if ( ProgSource->memory )
        return TRUE;
    else
        return FALSE;
}

int mprog_hitpercent( void *vo )
{
    CHAR_DATA *ch = ( CHAR_DATA * ) vo;

    if ( !vo )
        return FALSE;

    return ( ch->hit / ch->max_hit ) * 100;
}

int mprog_hour( void *vo )
{
    return time_info.hour;
}

int mprog_immune( void *vo )
{
    if ( !vo )
        return FALSE;

    return check_immune( ( CHAR_DATA * ) vo,
                         attack_table[ProgObjectSource->value[3]].damage );
}

int mprog_isawake( void *vo )
{
    if ( !vo )
        return FALSE;

    return IS_AWAKE( ( ( CHAR_DATA * ) vo ) );
}

int mprog_ischarmed( void *vo )
{
    if ( !vo )
        return FALSE;

    return IS_SET( ( ( CHAR_DATA * ) vo )->affected_by, AFF_CHARM );
}

int mprog_isfight( void *vo )
{
    if ( !vo )
        return FALSE;

    if ( ( ( CHAR_DATA * ) vo )->fighting )
        return TRUE;

    return FALSE;
}

int mprog_isfollow( void *vo )
{
    CHAR_DATA *ch = ( CHAR_DATA * ) vo;

    if ( !vo )
        return FALSE;

    return ( ch->master != NULL && ch->master->in_room == ch->in_room );
}

int mprog_isimmort( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( get_trust( ( CHAR_DATA * ) vo ) >= LEVEL_IMMORTAL );
}

int mprog_isgood( void *vo )
{
    if ( !vo )
        return FALSE;

    return IS_GOOD( ( ( CHAR_DATA * ) vo ) );
}

int mprog_isnpc( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( IS_NPC( ( CHAR_DATA * ) vo ) );
}

int mprog_ispc( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( !IS_NPC( ( CHAR_DATA * ) vo ) );
}

int mprog_level( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( ( CHAR_DATA * ) vo )->level;
}

int mprog_memory( void *vo )
{
    if ( !vo )
        return FALSE;

    if ( !ProgSource->memory || ProgSource->memory == ProgSource )
        return FALSE;

    return ( ProgSource->memory == vo );
}

int mprog_mobvnum( void *vo )
{
    CHAR_DATA *ch = ( CHAR_DATA * ) vo;

    if ( !vo )
        return FALSE;

    if ( IS_NPC( ch ) )
        return ch->pIndexData->vnum;

    return FALSE;
}

int mprog_objvnum( void *vo )
{
    OBJ_DATA *obj = ( OBJ_DATA * ) vo;

    if ( !vo )
        return FALSE;

    return obj->pIndexData->vnum;
}

int mprog_objtype( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( ( OBJ_DATA * ) vo )->item_type;
}

int mprog_objval0( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( ( OBJ_DATA * ) vo )->value[0];
}

int mprog_objval1( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( ( OBJ_DATA * ) vo )->value[1];
}

int mprog_objval2( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( ( OBJ_DATA * ) vo )->value[2];
}

int mprog_objval3( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( ( OBJ_DATA * ) vo )->value[3];
}

int mprog_position( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( ( CHAR_DATA * ) vo )->position;
}

int mprog_rand( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( number_percent(  ) <= *( ( int * ) vo ) );
}

int mprog_roomvnum( void *vo )
{
    if ( !ProgSource->in_room )
        return FALSE;

    return ProgSource->in_room->vnum;
}

int mprog_sex( void *vo )
{
    if ( !vo )
        return FALSE;

    return ( ( CHAR_DATA * ) vo )->sex;
}

int mprog_sgetrand( void *vo )
{
    static int lastarg = 0;
    static int lastrand = 0;
    static int lastprogid = 0;
    int arg = *( ( int * ) vo );

    if ( !vo )
        return FALSE;

    if ( arg != lastarg || lastprogid != ExecMudProgID )
    {
        lastrand = number_range( 1, arg );
        lastprogid = ExecMudProgID;
        lastarg = arg;
    }

    return lastrand;
}

int mprog_sreset( void *vo )
{
    ExecMudProgID++;

    return TRUE;
}

int mprog_isequal( void *vo )
{
    char *argument = ( char * ) vo;
    char left[MAX_INPUT_LENGTH];
    char *right;

    if ( argument == NULL )
        return FALSE;

    /* The left argument must be a variable... */
    if ( *argument == '$' )
        parse_command_var( *( ++argument ), left );
    else
        return FALSE;

    /* The right argument should be a string... */
    strtok( argument, "," );
    right = strtok( NULL, "," );

#ifdef WIN32
    return !stricmp( left, right );
#else
    return !strcasecmp( left, right );
#endif
}

int mprog_faction( void *vo )
{
    FACTIONLIST_DATA *pFact;
    FACTIONPC_DATA *pFactPC;
    CHAR_DATA *victim;
    char *argument = ( char * ) vo;
    int vnum;

    if ( *argument == '$' )
    {
        victim = mprog_get_actor( strtok( argument, "," ), 'C' );
        if ( !victim )
        {
            bug( "mprog_faction: Couldn't find victim %s", argument );
            return 0;
        }
    }
    else
    {
        victim = get_char_world( ProgSource, strtok( argument, "," ) );
        if ( !victim )
        {
            bug( "mprog_faction: Couldn't find victim %s", argument );
            return 0;
        }
    }

    argument += strlen( argument ) + 1;

    if ( !is_number( argument ) )
    {
        bug( "mprog_faction: bad argument syntax '%s'.", argument );
        return 0;
    }

    vnum = atoi( argument );

    if ( IS_NPC( victim ) )
        return 0;

    pFact = get_faction_by_vnum( ( sh_int ) vnum );

    if ( !pFact )
    {
        bug( "mprog_faction: No such faction vnum %s", argument );
        return 0;
    }

    for ( pFactPC = victim->pcdata->faction_standings;
          pFactPC != NULL; pFactPC = pFactPC->next )
    {
        if ( pFactPC->faction == pFact )
            break;
    }

    if ( !pFactPC )
    {
        return CFG_FACTION_INITIAL_VALUE;
    }

    return pFactPC->value;
}

int mprog_isname( void *vo )
{
    CHAR_DATA *victim;
    char *argument = ( char * ) vo;

    if ( *argument == '$' )
    {
        victim = mprog_get_actor( strtok( argument, "," ), 'C' );
        if ( !victim )
        {
            bug( "mprog_isname: Couldn't find victim %s", argument );
            return 0;
        }
    }
    else
    {
        victim = get_char_world( ProgSource, strtok( argument, "," ) );
        if ( !victim )
        {
            bug( "mprog_isname: Couldn't find victim %s", argument );
            return 0;
        }
    }

    argument += strlen( argument ) + 1;

    if ( !( *argument ) )
    {
        bug( "mprog_isname: Bad argument syntax." );
        return 0;
    }

    return is_name( argument, victim->name );
}
