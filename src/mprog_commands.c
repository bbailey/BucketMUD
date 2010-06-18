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
 *  The MOBprograms have been contributed by N'Atas-ha.  Any support for   *
 *  these routines should not be expected from Merc Industries.  However,  *
 *  under no circumstances should the blame for bugs, etc be placed on     *
 *  Merc Industries.  They are not guaranteed to work on all systems due   *
 *  to their frequent use of strxxx functions.  They are also not the most *
 *  efficient way to perform their tasks, but hopefully should be in the   *
 *  easiest possible way to install and begin using. Documentation for     *
 *  such installation can be found in INSTALL.  Enjoy........    N'Atas-Ha *
 ***************************************************************************/

/***************************************************************************
 * do_mpremember(), do_mpforget(), and do_mptrack() added by Zak of the    *
 * EmberMUD coding team.  EmberMUD improvements on ROM 2.3 base code are   *
 * Copyright 1996 by the EmberMUD development team.  See EmberMUD.license. *
 ***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "interp.h"

/*
 * Local functions.
 */

char *mprog_type_to_name args( ( int type ) );

SOCIALLIST_DATA *RandomSOCIAL = NULL;
void mprog_get_RandomSOCIAL( void );
bool RandomSOCIAL_picked;
/* This routine transfers between alpha and numeric forms of the
 *  mud_prog bitvector types. It allows the words to show up in mpstat to
 *  make it just a hair bit easier to see what a mob should be doing.
 */

char *mprog_type_to_name( int type )
{
    switch ( type )
    {
    case IN_FILE_PROG:
        return "in_file_prog";
    case ACT_PROG:
        return "act_prog";
    case SPEECH_PROG:
        return "speech_prog";
    case RAND_PROG:
        return "rand_prog";
    case FIGHT_PROG:
        return "fight_prog";
    case FIGHTGROUP_PROG:
        return "fightgroup_prog";
    case HIT_PROG:
        return "hit_prog";
    case HITPRCNT_PROG:
        return "hitprcnt_prog";
    case DEATH_PROG:
        return "death_prog";
    case ENTRY_PROG:
        return "entry_prog";
    case GREET_PROG:
        return "greet_prog";
    case ALL_GREET_PROG:
        return "all_greet_prog";
    case GIVE_PROG:
        return "give_prog";
    case BRIBE_PROG:
        return "bribe_prog";
    case LEAVE_PROG:
        return "leave_prog";
    case SLEEP_PROG:
        return "sleep_prog";
    case REST_PROG:
        return "rest_prog";
    case WEAR_PROG:
        return "wear_prog";
    case REMOVE_PROG:
        return "remove_prog";
    case SAC_PROG:
        return "sac_prog";
    case EXA_PROG:
        return "examine_prog";
    case LOOK_PROG:
        return "look_prog";
    case ZAP_PROG:
        return "zap_prog";
    case GET_PROG:
        return "get_prog";
    case DROP_PROG:
        return "drop_prog";
    case USE_PROG:
        return "use_prog";
    case COMMAND_PROG:
        return "command_prog";
    default:
        return "ERROR_PROG";
    }
}

/* do_mobstat: Show the mudprogs on a given mob.
 *             argument can be a vnum or a mob name. */
void do_mobstat( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MPROG_LIST *pList;
    MPROG_LIST *pInnerList;
    MPROG_GROUP_LIST *pGroupList;
    MPROG_DATA *mprg;
    MOB_INDEX_DATA *pMob = NULL;
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim = NULL;
    int iFound;

    *buf = '\0';

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "MudProg stat whom?\n\r", ch );
        return;
    }

    if ( !is_number( arg ) )
    {
        victim = get_char_world( ch, argument );
        if ( victim )
            pMob = victim->pIndexData;
        else
        {
            send_to_char( "No such creature.\n\r", ch );
            return;
        }
    }
    else if ( ( pMob = get_mob_index( atoi( arg ) ) ) == NULL )
    {
        send_to_char( "No such creature.\n\r", ch );
        return;
    }

    if ( pMob )
    {
        if ( !( pMob->progtypes ) )
        {
            send_to_char( "That Mobile has no Programs set.\n\r", ch );
            return;
        }

        sprintf( buf, "`WName: `w%s`w.  `WVnum: `w%d`w.\n\r",
                 pMob->player_name, pMob->vnum );
        strcat( buf,
                "`K------------------------------------------------------`w\n\r" );
        send_to_char( buf, ch );

        for ( pGroupList = pMob->mprog_groups; pGroupList;
              pGroupList = pGroupList->next )
            show_mpgroup( ch, pGroupList->mprog_group );

        for ( pList = pMob->mudprogs; pList; pList = pList->next )
        {
            mprg = pList->mudprog;

            iFound = 0;

            for ( pGroupList = pMob->mprog_groups; pGroupList;
                  pGroupList = pGroupList->next )
                for ( pInnerList = pGroupList->mprog_group->mudprogs;
                      pInnerList; pInnerList = pInnerList->next )
                    if ( pInnerList->mudprog->vnum == mprg->vnum )
                        iFound = 1;

            if ( !iFound )
                show_mprog( ch, mprg );
        }
    }
    else
    {
        send_to_char( "No such creature.\n\r", ch );
        return;
    }
}

void do_roomstat( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MPROG_DATA *mprg;
    MPROG_LIST *pList;
    MPROG_LIST *pInnerList;
    MPROG_GROUP_LIST *pGroupList;
    ROOM_INDEX_DATA *location = NULL;
    int iFound;

    one_argument( argument, arg );

    location = ( !arg[0] ) ? ch->in_room : find_location( ch, arg );

    if ( !location )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    if ( ch->in_room != location
         && room_is_private( location ) && get_trust( ch ) < MAX_LEVEL )
    {
        send_to_char( "That room is private right now.\n\r", ch );
        return;
    }

    if ( !location->progtypes )
    {
        send_to_char( "That Room has no Programs set.\n\r", ch );
        return;
    }

    sprintf( buf, "`WName: `w%s`w.\n\r`WArea: `w%s.\n\r",
             location->name, location->area->name );
    send_to_char( buf, ch );

    sprintf( buf, "`WVnum: `w%d. `WSector: `w%d. `WLight: `w%d.\n\r",
             location->vnum, location->sector_type, location->light );
    send_to_char( buf, ch );

    sprintf( buf, "`WRoom Flags: `w%d.\n\r", location->room_flags );
    send_to_char( buf, ch );

    send_to_char
        ( "`K-------------------------------------------------------------------------------`w\n\r",
          ch );
    *buf = '\0';

    for ( pGroupList = location->mprog_groups; pGroupList;
          pGroupList = pGroupList->next )
        show_mpgroup( ch, pGroupList->mprog_group );

    for ( pList = location->mudprogs; pList; pList = pList->next )
    {
        mprg = pList->mudprog;

        iFound = 0;

        for ( pGroupList = location->mprog_groups; pGroupList;
              pGroupList = pGroupList->next )
            for ( pInnerList = pGroupList->mprog_group->mudprogs; pInnerList;
                  pInnerList = pInnerList->next )
                if ( pInnerList->mudprog->vnum == mprg->vnum )
                    iFound = 1;

        if ( !iFound )
            show_mprog( ch, mprg );

    }

    return;
}

void do_objstat( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MPROG_DATA *mprg;
    MPROG_LIST *pList;
    MPROG_LIST *pInnerList;
    MPROG_GROUP_LIST *pGroupList;
    OBJ_DATA *obj = NULL;
    OBJ_INDEX_DATA *oIndex = NULL;
    int iFound;

    *buf = '\0';

    one_argument( argument, arg );

    if ( !arg[0] )
    {
        send_to_char( "OProg stat what?\n\r", ch );
        return;
    }

    if ( !is_number( arg ) )
    {
        obj = get_obj_world( ch, argument );
        if ( obj )
            oIndex = obj->pIndexData;
        else
        {
            send_to_char( "No such object.\n\r", ch );
            return;
        }
    }
    else
    {
        if ( ( oIndex = get_obj_index( atoi( arg ) ) ) == NULL )
        {
            send_to_char( "No such object.\n\r", ch );
            return;
        }
    }

    if ( oIndex )
    {
        if ( !oIndex->progtypes )
        {
            send_to_char( "That object has no programs set.\n\r", ch );
            return;
        }

        sprintf( buf, "`WName: `w%s`w.  `WVnum: `w%d`w.\n\r",
                 oIndex->name, oIndex->vnum );
        strcat( buf,
                "`K--------------------------------------------------------------------------------`w\n\r" );
        send_to_char( buf, ch );

        for ( pGroupList = oIndex->mprog_groups; pGroupList;
              pGroupList = pGroupList->next )
            show_mpgroup( ch, pGroupList->mprog_group );

        for ( pList = oIndex->mudprogs; pList; pList = pList->next )
        {
            mprg = pList->mudprog;

            iFound = 0;

            for ( pGroupList = oIndex->mprog_groups; pGroupList;
                  pGroupList = pGroupList->next )
                for ( pInnerList = pGroupList->mprog_group->mudprogs;
                      pInnerList; pInnerList = pInnerList->next )
                    if ( pInnerList->mudprog->vnum == mprg->vnum )
                        iFound = 1;

            if ( !iFound )
                show_mprog( ch, mprg );
        }
    }
    else
    {
        send_to_char( "No such object.\n\r", ch );
        return;
    }
}

void do_mpstat( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *string;
    OBJ_DATA *obj = NULL;
    ROOM_INDEX_DATA *location = NULL;
    CHAR_DATA *victim = NULL;

    string = one_argument( argument, arg );

    if ( !arg[0] )
    {
        send_to_char( "Syntax:\n\r", ch );
        send_to_char( "    mpstat obj <name>   OR   mpstat obj  <vnum>\n\r",
                      ch );
        send_to_char( "    mpstat mob <name>   OR   mpstat mob  <vnum>\n\r",
                      ch );
        send_to_char( "    mpstat room         OR   mpstat room <vnum>\n\r",
                      ch );
        return;
    }

    /* view a mob prog */
    if ( !str_cmp( arg, "mob" ) )
    {
        do_mobstat( ch, string );
        return;
    }

    /* view a room prog */
    if ( !str_cmp( arg, "room" ) )
    {
        do_roomstat( ch, string );
        return;
    }

    /* view an obj prog */
    if ( !str_cmp( arg, "obj" ) )
    {
        do_objstat( ch, string );
        return;
    }

    /* do it the old way */

    victim = get_char_world( ch, argument );
    if ( victim != NULL )
    {
        do_mobstat( ch, argument );
        return;
    }

    obj = get_obj_world( ch, argument );
    if ( obj != NULL )
    {
        do_objstat( ch, argument );
        return;
    }

    location = find_location( ch, argument );
    if ( location != NULL )
    {
        do_roomstat( ch, argument );
        return;
    }

    send_to_char( "Nothing by that name found anywhere.\n\r", ch );
}

/* prints the argument to all the rooms aroud the mobile */

void do_mpasound( CHAR_DATA * ch, char *argument )
{

    ROOM_INDEX_DATA *was_in_room;
    int door;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        bug( "Mpasound - No argument: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
        EXIT_DATA *pexit;

        if ( ( pexit = was_in_room->exit[door] ) != NULL
             && pexit->u1.to_room != NULL && pexit->u1.to_room != was_in_room )
        {
            ch->in_room = pexit->u1.to_room;
            MOBtrigger = FALSE;
            act( argument, ch, NULL, NULL, TO_ROOM );
        }
    }

    ch->in_room = was_in_room;
    return;

}

/* lets the mobile kill any player or mobile without murder*/

void do_mpkill( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        bug( "MpKill - no argument: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        bug( "MpKill - Victim not in room: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( victim == ch )
    {
        bug( "MpKill - Bad victim to attack: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
        bug( "MpKill - Charmed mob attacking master: vnum %d.",
             ch->pIndexData->vnum );
        return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        bug( "MpKill - Already fighting: vnum %d", ch->pIndexData->vnum );
        return;
    }

    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}

/* lets the mobile destroy an object in its inventory
   it can also destroy a worn object and it can destroy 
   items using all.xxxxx or just plain all of them */

void do_mpjunk( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        bug( "Mpjunk - No argument: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
        if ( ( obj = get_obj_wear( ch, arg ) ) != NULL )
        {
            unequip_char( ch, obj );
            extract_obj( obj );
            return;
        }
        if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
            return;
        extract_obj( obj );
    }
    else
        for ( obj = ch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;
            if ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
            {
                if ( obj->wear_loc != WEAR_NONE )
                    unequip_char( ch, obj );
                extract_obj( obj );
            }
        }

    return;

}

/* prints the message to everyone in the room other than the mob and victim */

void do_mpechoaround( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        bug( "Mpechoaround - No argument:  vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
        bug( "Mpechoaround - victim does not exist: vnum %d.",
             ch->pIndexData->vnum );
        return;
    }

    act( argument, ch, NULL, victim, TO_NOTVICT );
    return;
}

/* prints the message to only the victim */

void do_mpechoat( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        bug( "Mpechoat - No argument:  vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) )
    {
        bug( "Mpechoat - victim does not exist: vnum %d.",
             ch->pIndexData->vnum );
        return;
    }

    act( argument, ch, NULL, victim, TO_VICT );
    return;
}

/* prints the message to the room at large */

void do_mpecho( CHAR_DATA * ch, char *argument )
{
    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        bug( "Mpecho - called w/o argument: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    act( argument, ch, NULL, NULL, TO_ROOM );
    return;

}

/* lets the mobile load an item or mobile.  All items
are loaded into inventory.  you can specify a level with
the load object portion as well. */

void do_mpmload( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number( arg ) )
    {
        bug( "Mpmload - Bad vnum as arg: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
        bug( "Mpmload - Bad mob vnum: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    return;
}

void do_mpoload( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' || !is_number( arg ) )
    {
        bug( "Mpoload - Bad syntax: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        level = get_trust( ch );
    }
    else
    {
        /*
         * New feature from Alander.
         */
        if ( !is_number( arg2 ) )
        {
            bug( "Mpoload - Bad syntax: vnum %d.", ch->pIndexData->vnum );
            return;
        }
        level = atoi( arg2 );
        if ( level < 0 || level > get_trust( ch ) )
        {
            bug( "Mpoload - Bad level: vnum %d.", ch->pIndexData->vnum );
            return;
        }
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg ) ) ) == NULL )
    {
        bug( "Mpoload - Bad vnum arg: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR( obj, ITEM_TAKE ) )
    {
        obj_to_char( obj, ch );
    }
    else
    {
        obj_to_room( obj, ch->in_room );
    }

    return;
}

/* lets the mobile purge all objects and other npcs in the room,
   or purge a specified object or mob in the room.  It can purge
   itself, but this had best be the last command in the MudProg
   otherwise ugly stuff will happen */

void do_mppurge( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        CHAR_DATA *vnext;
        OBJ_DATA *obj_next;

        for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
        {
            vnext = victim->next_in_room;
            if ( IS_NPC( victim ) && victim != ch )
                extract_char( victim, TRUE );
        }

        for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;
            extract_obj( obj );
        }

        return;
    }

    if ( !( victim = get_char_room( ch, arg ) ) == '\0' )
    {
        if ( ( obj = get_obj_here( ch, arg ) ) )
        {
            extract_obj( obj );
        }
        else
        {
            bug( "Mppurge - Bad argument: vnum %d.", ch->pIndexData->vnum );
        }
        return;
    }

    if ( !IS_NPC( victim ) )
    {
        bug( "Mppurge - Purging a PC: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    extract_char( victim, TRUE );
    return;
}

/* Allow mobiles to go wizinvis with programs -- SB */

void do_mpinvis( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    sh_int level;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );
    if ( arg && arg[0] != '\0' )
    {
        if ( !is_number( arg ) )
        {
            bug( "Mpinvis - Non numeric argument ", ch );
            return;
        }
        level = atoi( arg );
        if ( level < 2 || level > MAX_LEVEL )
        {
            bug( "MPinvis - Invalid level ", ch );
            return;
        }

        ch->mobinvis = level;
        printf_to_char( ch, "Mobinvis level set to %d.\n\r", level );
        return;
    }

    if ( ch->mobinvis < 2 )
        ch->mobinvis = ch->level;

    if ( IS_SET( ch->act, ACT_MOBINVIS ) )
    {
        REMOVE_BIT( ch->act, ACT_MOBINVIS );
        act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly fade back into existence.\n\r", ch );
    }
    else
    {
        SET_BIT( ch->act, ACT_MOBINVIS );
        act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly vanish into thin air.\n\r", ch );
    }
    return;
}

/* lets the mobile goto any location it wishes that is not private */

void do_mpgoto( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        bug( "Mpgoto - No argument: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        bug( "Mpgoto - No such location: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( ch->fighting != NULL )
        stop_fighting( ch, TRUE );

    char_from_room( ch );
    char_to_room( ch, location );

    return;
}

/* lets the mobile do a command at another location. Very useful */

void do_mpat( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *wch;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        bug( "Mpat - Bad argument: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        bug( "Mpat - No such location: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
        if ( wch == ch )
        {
            char_from_room( ch );
            char_to_room( ch, original );
            break;
        }
    }

    return;
}

/* lets the mobile transfer people.  the all argument transfers
   everyone in the current room to the specified location */

void do_mptransfer( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
        bug( "Mptransfer - Bad syntax: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->connected == CON_PLAYING
                 && d->character != ch
                 && d->character->in_room != NULL
                 && can_see( ch, d->character ) )
            {
                sprintf( buf, "%s %s", d->character->name, arg2 );
                do_mptransfer( ch, buf );
            }
        }
        return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
        location = ch->in_room;
    }
    else
    {
        if ( ( location = find_location( ch, arg2 ) ) == NULL )
        {
            bug( "Mptransfer - No such location: vnum %d.",
                 ch->pIndexData->vnum );
            return;
        }

        if ( room_is_private( location ) )
        {
            bug( "Mptransfer - Private room: vnum %d.", ch->pIndexData->vnum );
            return;
        }
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        bug( "Mptransfer - No such person: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( victim->in_room == NULL )
    {
        bug( "Mptransfer - Victim in Limbo: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( victim->fighting != NULL )
        stop_fighting( victim, TRUE );

    char_from_room( victim );
    char_to_room( victim, location );

    return;
}

/* Adds get_char_world(argument) to the mob's memory for later use in
 * do_mptrack, etc. */
void do_mpremember( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    CHAR_DATA *vict;

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( ( vict = get_char_world( ch, arg ) ) == NULL )
    {
        sprintf( buf,
                 "mpremember - %s doesn't exist! Called by mobile vnum %d.",
                 arg, ch->pIndexData->vnum );
        bug( buf, 0 );
        return;
    }
    else
        ch->memory = vict;
    return;
}

/* Erases a mob's memory. */
void do_mpforget( CHAR_DATA * ch, char *argument )
{
    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    ch->memory = NULL;
    return;
}

/* lets the mobile force someone to do something.  must be mortal level
   and the all argument only affects those in the room with the mobile */

void do_mpforce( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        bug( "Mpforce - Bad syntax: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;

            if ( vch->in_room == ch->in_room
                 && get_trust( vch ) < get_trust( ch ) && can_see( ch, vch ) )
            {
                interpret( vch, argument );
            }
        }
    }
    else
    {
        CHAR_DATA *victim;

        if ( ( victim = get_char_room( ch, arg ) ) == NULL )
        {
            bug( "Mpforce - No such victim: vnum %d.", ch->pIndexData->vnum );
            return;
        }

        if ( victim == ch )
        {
            bug( "Mpforce - Forcing oneself: vnum %d.", ch->pIndexData->vnum );
            return;
        }

        interpret( victim, argument );
    }

    return;
}

void do_mpsilentforce( CHAR_DATA * ch, char *argument )
{
    extern bool silentmode;
    char arg[MAX_INPUT_LENGTH];
    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        bug( "Mpsilentforce - Bad syntax: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;
        struct cmd_type *cmd;

        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;

            /* Don't allow forcing imm commands */
            for ( cmd = ( struct cmd_type * ) cmd_table; *cmd->name; cmd++ )
            {
                if ( ( *argument == *cmd->name )
                     && ( !str_prefix( argument, cmd->name ) ) )
                {
                    if ( cmd->imm )
                    {
                        bug( "mpsilentforce - mob vnum %d",
                             ch->pIndexData->vnum );
                        bug( "mpsilentforce - cannot force wiz commands" );
                        return;
                    }
                }
            }

            if ( vch->in_room == ch->in_room
                 && get_trust( vch ) < get_trust( ch ) && can_see( ch, vch ) )
            {
                silentmode = TRUE;
                interpret( vch, argument );
                silentmode = FALSE;
            }
        }
    }
    else
    {
        CHAR_DATA *victim;
        struct cmd_type *cmd;

        if ( ( victim = get_char_room( ch, arg ) ) == NULL )
        {
            bug( "Mpsilentforce - No such victim: vnum %d.",
                 ch->pIndexData->vnum );
            return;
        }

        if ( victim == ch )
        {
            bug( "Mpsilentforce - Forcing oneself: vnum %d.",
                 ch->pIndexData->vnum );
            return;
        }

        /* Don't allow forcing wiz commands */
        for ( cmd = ( struct cmd_type * ) cmd_table; *cmd->name; cmd++ )
        {
            if ( ( *argument == *cmd->name )
                 && !str_prefix( argument, cmd->name ) )
            {
                if ( cmd->imm )
                {
                    bug( "mpsilentforce - mob vnum %d", ch->pIndexData->vnum );
                    bug( "mpsilentforce - cannot force wiz commands" );
                    return;
                }
            }
        }

        silentmode = TRUE;
        interpret( victim, argument );
        silentmode = FALSE;
    }

    return;
}

void do_mpdosilent( CHAR_DATA * ch, char *argument )
{
    extern bool silentmode;
    silentmode = TRUE;
    interpret( ch, argument );
    silentmode = FALSE;
    return;
}

void do_mpdefault( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    if ( !IS_NPC( ch ) )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        bug( "Mpdefault - Bad syntax: vnum %d.", ch->pIndexData->vnum );
        return;
    }
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        bug( "Mpdefault - No such victim: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( victim == ch )
    {
        bug( "Mpdefault - to oneself: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        bug( "Mpdefault - npc victim: vnum %d.", ch->pIndexData->vnum );
        return;
    }

    mpinterpret( victim, argument );

    return;
}

/* one_argumnet ruins the case so I do some funky stuff.  -Zane */
void do_mpfollowpath( CHAR_DATA * ch, char *argument )
{
    MOB_INDEX_DATA *pMob;
    char arg[MAX_STRING_LENGTH];
    char *arg2;

    if ( ( pMob = ch->pIndexData ) == NULL )
        return;

    arg2 = one_argument( argument, arg );

    if ( strlen( arg ) < 1 )
        return;

    if ( !str_cmp( arg2, "restart" ) && !pMob->path_move )
    {
        pMob->path_move = TRUE;
        pMob->path_pos = 0;
    }

    /* If we're fighting, unconcious(or worse) or not moving at all just exit */
    if ( ch->fighting != NULL || !pMob->path_move
         || ch->position < POS_SLEEPING )
        return;

    switch ( argument[pMob->path_pos] )
    {
    case '0':
    case '1':
    case '2':
    case '3':
        move_char( ch, argument[pMob->path_pos] - '0', FALSE );
        break;

    case 'W':
        ch->position = POS_STANDING;
        act( "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM );
        break;

    case 'S':
        ch->position = POS_SLEEPING;
        act( "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
        break;

    case 'a':
        act( "$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM );
        break;

    case 'b':
        act( "$n says 'What a view!  I must do something about that dump!'",
             ch, NULL, NULL, TO_ROOM );
        break;

    case 'c':
        act( "$n says 'Vandals!  Youngsters have no respect for anything!'",
             ch, NULL, NULL, TO_ROOM );
        break;

    case 'd':
        act( "$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM );
        break;

    case 'e':
        act( "$n says 'I hereby declare the city of Midgaard open!'",
             ch, NULL, NULL, TO_ROOM );
        break;

    case 'E':
        act( "$n says 'I hereby declare the city of Midgaard closed!'",
             ch, NULL, NULL, TO_ROOM );
        break;

    case 'O':
/*		do_unlock( ch, "gate" ); */
        do_open( ch, "gate" );
        break;

    case 'C':
        do_close( ch, "gate" );
/*		do_lock( ch, "gate" ); */
        break;

    case '.':
        pMob->path_move = FALSE;
        break;
    }

    pMob->path_pos++;

    if ( pMob->path_pos >= strlen( arg ) )
    {
        pMob->path_move = FALSE;
        pMob->path_pos = 0;
    }
    return;
}

void do_mpeatcorpse( CHAR_DATA * ch, char *argument )
{
    OBJ_DATA *corpse;
    OBJ_DATA *c_next;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !IS_AWAKE( ch ) )
        return;

    for ( corpse = ch->in_room->contents; corpse != NULL; corpse = c_next )
    {
        c_next = corpse->next_content;
        if ( corpse->item_type != ITEM_CORPSE_NPC )
            continue;

        act( "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );
        for ( obj = corpse->contains; obj; obj = obj_next )
        {
            obj_next = obj->next_content;
            obj_from_obj( obj );
            obj_to_room( obj, ch->in_room );
        }
        extract_obj( corpse );
        return;
    }

    return;
}

void do_mpclean( CHAR_DATA * ch, char *argument )
{
    OBJ_DATA *trash;
    OBJ_DATA *trash_next;
    int value;

    value = atoi( argument );

    if ( value < 1 || strlen( argument ) == 0 )
        value = 10;

    if ( !IS_AWAKE( ch ) )
        return;

    for ( trash = ch->in_room->contents; trash != NULL; trash = trash_next )
    {
        trash_next = trash->next_content;
        if ( !IS_SET( trash->wear_flags, ITEM_TAKE ) || !can_loot( ch, trash ) )
            continue;

        if ( trash->item_type == ITEM_DRINK_CON
             || trash->item_type == ITEM_TRASH || trash->cost < value )
        {
            act( "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
            obj_from_room( trash );
            obj_to_char( trash, ch );
            return;
        }
    }

    return;
}

/* RT does socials */

void do_mprandomsocial( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA *victim = NULL;
    SOCIALLIST_DATA *social = NULL;
    int count;
    int random;
    random = 0;
    count = 0;
    /* Get the number of socials available */
    for ( social = social_first; social != NULL; social = social->next )
        if ( social->next != NULL )
            count++;

    random = number_range( 1, count );
    count = 0;

    /* get a random social from the compiled list */
    for ( social = social_first; social != NULL; social = social->next )
        if ( ( social->next != NULL ) && ++count == random )
        {
            printf_to_char( ch, "2nd:%s\n\r", social->name );
            RandomSOCIAL = social;
            RandomSOCIAL_picked = TRUE;
        }

/* use to be argument */
    if ( argument == '\0' )
    {
        if ( str_cmp( RandomSOCIAL->others_no_arg, "none" ) )
        {
            act( RandomSOCIAL->others_no_arg, ch, NULL, victim, TO_ROOM );
        }
        if ( str_cmp( RandomSOCIAL->char_no_arg, "none" ) )
        {
            act( RandomSOCIAL->char_no_arg, ch, NULL, victim, TO_CHAR );
        }
    }
    else if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    {
        if ( str_cmp( RandomSOCIAL->char_not_found, "none" ) )
        {
            act( RandomSOCIAL->char_not_found, ch, NULL, victim, TO_CHAR );
        }
        else
            send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
        if ( str_cmp( RandomSOCIAL->others_auto, "none" ) )
        {
            act( RandomSOCIAL->others_auto, ch, NULL, victim, TO_ROOM );
        }
        if ( str_cmp( RandomSOCIAL->char_auto, "none" ) )
        {
            act( RandomSOCIAL->char_auto, ch, NULL, victim, TO_CHAR );
        }
    }
    else
    {
        if ( str_cmp( RandomSOCIAL->others_found, "none" ) )
        {
            act( RandomSOCIAL->others_found, ch, NULL, victim, TO_NOTVICT );
        }
        if ( str_cmp( RandomSOCIAL->char_found, "none" ) )
        {
            act( RandomSOCIAL->char_found, ch, NULL, victim, TO_CHAR );
        }
        if ( str_cmp( RandomSOCIAL->vict_found, "none" ) )
        {
            act( RandomSOCIAL->vict_found, ch, NULL, victim, TO_VICT );
        }

        if ( IS_NPC( victim )
             && !IS_AFFECTED( victim, AFF_CHARM )
             && IS_AWAKE( victim ) && victim->desc == NULL )
        {
            switch ( number_bits( 4 ) )
            {
            case 0:

            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                if ( str_cmp( RandomSOCIAL->others_found, "none" ) )
                {
                    act( RandomSOCIAL->others_found,
                         victim, NULL, ch, TO_NOTVICT );
                }
                if ( str_cmp( RandomSOCIAL->char_found, "none" ) )
                {
                    act( RandomSOCIAL->char_found, victim, NULL, ch, TO_CHAR );
                }
                if ( str_cmp( RandomSOCIAL->vict_found, "none" ) )
                {
                    act( RandomSOCIAL->vict_found, victim, NULL, ch, TO_VICT );
                }
                break;

            case 9:
            case 10:
            case 11:
            case 12:
                act( "$n slaps $N.", victim, NULL, ch, TO_NOTVICT );
                act( "You slap $N.", victim, NULL, ch, TO_CHAR );
                act( "$n slaps you.", victim, NULL, ch, TO_VICT );
                break;
            }
        }
    }
}
