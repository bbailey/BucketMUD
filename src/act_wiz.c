/**************************************************************************
 * Mudprogram's (Mobprogram, Objprogram and Roomprogram) originaly        *
 * by the SMAUG development team                                          *
 * Ported to EmberMUD by Thanatos and Tyrluk of ToED                      *
 * (Temple of Eternal Death)                                              *
 * Tyrluk   - morn@telmaron.com or dajy@mindspring.com                    *
 * Thanatos - morn@telmaron.com or jonathan_w._rose@ffic.com              * 
 **************************************************************************/
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

#if !defined(WIN32)
#include <sys/time.h>
#endif
#include <time.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include "merc.h"
#include "olc.h"

#include "interp.h"

/* command procedures needed */
DECLARE_DO_FUN( do_remove );
DECLARE_DO_FUN( do_rstat );
DECLARE_DO_FUN( do_mstat );
DECLARE_DO_FUN( do_ostat );
DECLARE_DO_FUN( do_rset );
DECLARE_DO_FUN( do_mset );
DECLARE_DO_FUN( do_oset );
DECLARE_DO_FUN( do_sset );
DECLARE_DO_FUN( do_mfind );
DECLARE_DO_FUN( do_ofind );
DECLARE_DO_FUN( do_rfind );
DECLARE_DO_FUN( do_mpfind );
DECLARE_DO_FUN( do_slookup );
DECLARE_DO_FUN( do_mload );
DECLARE_DO_FUN( do_oload );
DECLARE_DO_FUN( do_force );
DECLARE_DO_FUN( do_quit );
DECLARE_DO_FUN( do_save );
DECLARE_DO_FUN( do_look );
DECLARE_DO_FUN( do_force );
DECLARE_DO_FUN( do_pload );
DECLARE_DO_FUN( do_punload );
DECLARE_DO_FUN( do_wizgrant );
DECLARE_DO_FUN( do_wizrevoke );
DECLARE_DO_FUN( do_olevel );
DECLARE_DO_FUN( do_mlevel );

/*
 * Local functions.
 */
ROOM_INDEX_DATA *find_location args( ( CHAR_DATA * ch, char *arg ) );

extern int control;
extern bool write_to_descriptor( int, char *, int, bool );

#ifdef REQUIRE_EDIT_PERMISSION
void do_setedit( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "That person does not exist.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\rDuh.\n\r", ch );
        return;
    }

    TOGGLE_BIT( victim->act, PLR_CAN_EDIT );
    if ( victim->act & PLR_CAN_EDIT )
    {
        send_to_char( "You may now use OLC.\n\r", victim );
        send_to_char( "OLC enabled!\n\r", ch );
    }
    else
    {
        send_to_char( "OLC disabled.\n\r", ch );
        send_to_char( "You may no longer use OLC.\n\r", victim );
    }
    return;
}
#endif

#ifdef REQUIRE_MUDPROG_PERMISSION
void do_setprog( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "That person does not exist.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\rDuh.\n\r", ch );
        return;
    }

    TOGGLE_BIT( victim->act, PLR_CAN_PROG );
    send_to_char( "Ok.\n\r", ch );
    if ( victim->act & PLR_CAN_PROG )
        send_to_char( "You may now make programs.\n\r", victim );
    else
        send_to_char( "You may no longer make programs.\n\r", victim );
    return;
}
#endif
/* This Permit command is to be used along with the ban site permit
command. It allows certain players to login from banned sites. -Lancelight
*/

void do_permit( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "That person does not exist.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\rDuh.\n\r", ch );
        return;
    }

    TOGGLE_BIT( victim->act, PLR_PERMIT );
    if ( victim->act & PLR_PERMIT )
    {
        send_to_char( "Player may now log in from banned sites.\n\r", ch );
        send_to_char( "You may now log in from banned sites.\n\r", victim );
    }
    else
    {
        send_to_char( "Player may no longer login from banned sites.\n\r", ch );
        send_to_char( "You may no longer login from banned sites.\n\r",
                      victim );
    }
    return;
}

/* I whipped this jail command up after a suggestion came that we should
have some sort of jail system. I'm the meanie type, so I whipped up a
really nasty jail bit. -Lancelight */

void do_jail( CHAR_DATA * ch, char *argument )
{
    int blarg;
    int jtimer;
    char arg1[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
        send_to_char( "Jail whom?\n`RSyntax: `Yjail <player> <time>`w\n", ch );
        return;
    }

    if ( ( victim = get_player_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET( victim->act, PLR_JAILED ) )
    {
        if ( IS_NPC( victim ) )
        {
            blarg = victim->was_in_room->vnum;
        }
        else
        {
            if ( JAIL_RELEASE_RECALL == 1 )
                blarg = victim->pcdata->recall_room->vnum;
            else
                blarg = JAIL_RELEASE_VNUM;
        }

        if ( JAIL_NOSHOUT == 1 )
        {
            REMOVE_BIT( victim->comm, COMM_NOSHOUT );
        }

        if ( JAIL_NOEMOTE == 1 )
        {
            REMOVE_BIT( victim->comm, COMM_NOEMOTE );
        }

        if ( JAIL_NOTELL == 1 )
        {
            REMOVE_BIT( victim->comm, COMM_NOTELL );
        }

        if ( JAIL_NOCHANNEL == 1 )
        {
            REMOVE_BIT( victim->comm, COMM_NOCHANNELS );
        }

        REMOVE_BIT( victim->act, PLR_JAILED );
        send_to_char( "`WYour `Rjail term`W has been `clifted\n\r", victim );

        if ( !IS_NPC( victim ) && ( JAIL_RELEASE_RECALL == 1 ) )
        {
            send_to_char
                ( "`Wand you are being teleported back to `Grecall`W.`w\n\r",
                  victim );
            send_to_char( "Jail Lifted\n\r", ch );
            char_from_room( victim );
            victim->jail_timer = 0;
            char_to_room( victim, get_room_index( blarg ) );
            if ( IS_NPC( victim ) )
                sprintf( buf, "%s frees %s from jail.", ch->name,
                         victim->short_descr );
            else
                sprintf( buf, "%s frees %s from jail.", ch->name,
                         victim->name );
            do_sendinfo( ch, buf );
        }
        else
        {
            send_to_char
                ( "`Wand you are being teleported to the front gates of the jail.`w\n\r",
                  victim );
            send_to_char( "Jail Lifted\n\r", ch );
            char_from_room( victim );
            char_to_room( victim, get_room_index( blarg ) );
            sprintf( buf, "%s frees %s from jail.", ch->name, victim->name );
            do_sendinfo( ch, buf );
        }
    }
    else
    {
        if ( !is_number( arg2 ) )
        {
            send_to_char( "`YPlease enter a time in ticks`w\n", ch );
            return;
        }
        jtimer = atoi( arg2 );
        if ( jtimer <= -1 )
        {
            send_to_char( "`YPositive numbers only brainy.\n", ch );
            return;
        }
        if ( jtimer == 0 )
        {
            jtimer = jtimer - 1;
            victim->jail_timer = jtimer;
        }
        victim->jail_timer = jtimer;
        if ( victim->jail_timer < 0 )
            send_to_char( "`RYou have been Jailed indefinatly!!!!`w\n\r",
                          victim );
        else
            printf_to_char( victim,
                            "`RYou have been Jailed for %d hours!!!!.`w\n\r",
                            victim->jail_timer );
        victim->jail_timer = victim->jail_timer;
        send_to_char( "Jail set.\n\r", ch );
        if ( IS_NPC( victim ) )
            sprintf( buf, "%s has tossed %s in the slammer!", ch->name,
                     victim->short_descr );
        else
            sprintf( buf, "%s has tossed %s in the slammer!", ch->name,
                     victim->name );

        if ( JAIL_NOSHOUT == 1 )
        {
            SET_BIT( victim->comm, COMM_NOSHOUT );
        }

        if ( JAIL_NOEMOTE == 1 )
        {
            SET_BIT( victim->comm, COMM_NOEMOTE );
        }

        if ( JAIL_NOTELL == 1 )
        {
            SET_BIT( victim->comm, COMM_NOTELL );
        }

        if ( JAIL_NOCHANNEL == 1 )
        {
            SET_BIT( victim->comm, COMM_NOCHANNELS );
        }

        SET_BIT( victim->act, PLR_JAILED );

        if ( JAIL_REMOVES_EQ == 1 )
        {
            do_remove( victim, "all" );
        }
        victim->was_in_room = victim->in_room;
        char_from_room( victim );
        char_to_room( victim, get_room_index( JAIL_CELL_VNUM ) );
        do_sendinfo( ch, buf );
    }
    return;
}

 /* equips a character */
void do_outfit( CHAR_DATA * ch, char *argument )
{
    OBJ_DATA *obj;

    if ( ch->level > 5 || IS_NPC( ch ) )
    {
        send_to_char( "Find it yourself!\n\r", ch );
        return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL )
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_BANNER ), 0 );
        obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_LIGHT );
    }

    if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL )
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_VEST ), 0 );
        obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_BODY );
    }

    if ( ( obj = get_eq_char( ch, WEAR_SHIELD ) ) == NULL )
    {
        obj = create_object( get_obj_index( OBJ_VNUM_SCHOOL_SHIELD ), 0 );
        obj->cost = 0;
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_SHIELD );
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
    {
        obj =
            create_object( get_obj_index( class_table[ch->Class].weapon ), 0 );
        obj_to_char( obj, ch );
        equip_char( ch, obj, WEAR_WIELD );
    }

    send_to_char( "You have been equipped by the gods.\n\r", ch );
}

/* do_award function coded by Rindar (Ron Cole) and Raven (Laurie Zenner).  */
void do_award( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    int value;
    int level;
    int levelvi;
    char *maxed = NULL;
    long maxlong = 2147483647;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg3[0] == '\0' || !is_number( arg2 ) )
    {
        send_to_char( "Syntax: award <char> <amount> <type>.\n\r", ch );
        send_to_char( "Valid types: alignment, experience, gold, object.\n\r",
                      ch );
        send_to_char( "NOTE: Substitute the object's VNUM for the amount.\n\r",
                      ch );
        return;
    }

    if ( strncmp( arg3, "alignment", strlen( arg3 ) )
         && strncmp( arg3, "experience", strlen( arg3 ) )
         && strncmp( arg3, "gold", strlen( arg3 ) )
         && strncmp( arg3, "qp", strlen( arg3 ) )
         && strncmp( arg3, "object", strlen( arg3 ) ) )
    {
        send_to_char
            ( "Valid types: alignment, experience, gold, qp, object.\n\r", ch );
        send_to_char( "NOTE: Substitute the object's VNUM for the amount.\n\r",
                      ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "That person is not currently on-line.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    level = get_trust( ch );
    levelvi = get_trust( victim );

    if ( ( level <= levelvi ) && ( ch != victim ) )
    {
        send_to_char( "You can only award those of lesser trust level.", ch );
        return;
    }

    value = atoi( arg2 );

    if ( value == 0 )
    {
        send_to_char( "The value must not be equal to 0.\n\r", ch );
        return;
    }

    if ( arg3[0] == 'a' )       /* alignment */
    {
        if ( value < -2000 || value > 2000 )
        {
            send_to_char
                ( "You can only alter someone's alignment by -2000 to 2000 points.\n\r",
                  ch );
            return;
        }

        if ( ( victim->alignment + value ) > 1000 )
        {
            value = ( 1000 - victim->alignment );
            victim->alignment = 1000;
            maxed = "high";
        }
        else if ( ( victim->alignment + value ) < -1000 )
        {
            value = ( -1000 - victim->alignment );
            victim->alignment = -1000;
            maxed = "low";
        }
        else
            victim->alignment += value;

        if ( value == 0 )
        {
            sprintf( buf, "%s's alignment is already as %s as it can be.\n\r",
                     victim->name, maxed );
            send_to_char( buf, ch );
            return;
        }
        else
        {
            sprintf( buf, "You alter %s's alignment by %d points.\n\r",
                     victim->name, value );
            send_to_char( buf, ch );

            if ( value > 0 )
            {
                sprintf( buf, "Your soul feels lighter and more virtuous!" );
                send_to_char( buf, victim );
                return;
            }
            else
            {
                sprintf( buf, "You shudder deeply as your soul darkens." );
                send_to_char( buf, victim );
                return;
            }
        }
    }

    if ( arg3[0] == 'q' )       /* Quest Points */
    {
        if ( value < -2000 || value > 100 )
        {
            send_to_char
                ( "You can only alter someone's quest points by -2000 to 100 points.\n\r",
                  ch );
            return;
        }

        if ( ( victim->qp + value ) > 2000 )
        {
            value = ( 2000 - victim->qp );
            victim->qp = 2000;
            maxed = "high";
        }
        else if ( ( victim->qp + value ) < -2000 )
        {
            value = ( -2000 - victim->qp );
            victim->qp = -2000;
            maxed = "low";
        }
        else
            victim->qp += value;

        if ( value == 0 )
        {
            sprintf( buf, "%s's qp is already as %s as it can be.\n\r",
                     victim->name, maxed );
            send_to_char( buf, ch );
            return;
        }
        else
        {
            sprintf( buf, "You alter %s's qp by %d points.\n\r",
                     victim->name, value );
            send_to_char( buf, ch );

            if ( value > 0 )
            {
                sprintf( buf,
                         "You receive %d quest points for your efforts!\n\r",
                         value );
                send_to_char( buf, victim );
                return;
            }
            else
            {
                sprintf( buf, "You are drained of %d quest points!\n\r",
                         value );
                send_to_char( buf, victim );
                return;
            }
        }
    }

    if ( arg3[0] == 'e' )       /* experience */
    {
        /* Cannot use the 'gain_exp' function since it won't
           give experience if the player is in the arena and it
           takes con away if they go below minimum experience
           (which could make them self-destruct).   That's just
           too mean to do during an 'award', since it might happen
           by mistake. */

        if ( victim->level >= LEVEL_HERO )
        {
            sprintf( buf, "%s cannot receive experience bonuses",
                     victim->name );
            send_to_char( buf, ch );
            return;
        }

        if ( value < -1000000 || value > 1000000 )
        {
            send_to_char
                ( "You can only award between -1000000 and 1000000 experience.\n\r",
                  ch );
            return;
        }

        if ( victim->exp < 0 && value < 0
             && victim->exp < ( ( -1 * maxlong ) - value ) )
        {
            value = ( -1 * maxlong ) - victim->exp;
            victim->exp = ( -1 * maxlong );
            maxed = "minumum";
        }
        else if ( victim->exp > 0 && value > 0
                  && victim->exp > ( maxlong - value ) )
        {
            value = maxlong - victim->exp;
            victim->exp = maxlong;
            maxed = "maximum";
        }
        else
            victim->exp += value;

        if ( value == 0 )
        {
            sprintf( buf,
                     "%s already has the %s experience points possible.\n\r",
                     victim->name, maxed );
            send_to_char( buf, ch );
            return;
        }
        else
        {
            sprintf( buf, "You award %s %d experience points.\n\r",
                     victim->name, value );
            send_to_char( buf, ch );

            if ( value > 0 )
            {
                sprintf( buf,
                         "You receive %d experience points for your efforts!\n\r",
                         value );
                send_to_char( buf, victim );
                return;
            }
            else
            {
                sprintf( buf,
                         "You are drained of %d experience points!\n\r",
                         value );
                send_to_char( buf, victim );
                return;
            }
        }
    }

    if ( arg3[0] == 'g' )       /* gold */
    {
        if ( value < -9999999 || value > 9999999 )
        {
            send_to_char
                ( "You can only award between -9999999 and 9999999 gold.\n\r",
                  ch );
            return;
        }

        if ( value < 0 && victim->gold < value )
        {
            value = -1 * victim->gold;
            victim->gold = 0;
            maxed = "minumum";
        }
        else if ( value > 0 && victim->gold > ( maxlong - value ) )
        {
            value = maxlong - victim->exp;
            victim->exp = maxlong;
            maxed = "maximum";
        }
        else
            victim->gold += value;

        if ( value == 0 )
        {
            sprintf( buf,
                     "%s already has the %s amount of gold allowed.\n\r",
                     victim->name, maxed );
            send_to_char( buf, ch );
            return;
        }
        else
        {
            sprintf( buf, "You award %s %d gold coins.\n\r", victim->name,
                     value );
            send_to_char( buf, ch );

            if ( value > 0 )
            {
                sprintf( buf,
                         "Your coin pouch grows heavier! You gain %d gold coins!\n\r",
                         value );
                send_to_char( buf, victim );
                return;
            }
            else
            {
                sprintf( buf,
                         "Your coin pouch grows lighter! You lose %d gold coins!\n\r",
                         value );
                send_to_char( buf, victim );
                return;
            }
        }
    }

    if ( arg3[0] == 'o' )       /* objects */
    {
        OBJ_INDEX_DATA *pObjIndex;
        int level = get_trust( ch );
        OBJ_DATA *obj;

        if ( ( pObjIndex = get_obj_index( atoi( arg2 ) ) ) == NULL )
        {
            send_to_char( "There is no object with that vnum.\n\r", ch );
            return;
        }

        obj = create_object( pObjIndex, level );

        if ( victim->carry_number + get_obj_number( obj ) >
             can_carry_n( victim ) )
        {
            sprintf( buf,
                     "Alas, %s is carrying too many items to receive that.\n\r",
                     victim->name );
            send_to_char( buf, ch );
            extract_obj( obj );
            return;
        }

        if ( victim->carry_weight + get_obj_weight( obj ) >
             can_carry_w( victim ) )
        {
            sprintf( buf, "Alas, that is too heavy for %s to carry.\n\r",
                     victim->name );
            send_to_char( buf, ch );
            extract_obj( obj );
            return;
        }

        obj_to_char( obj, victim );
        sprintf( buf, "You award %s item %d.\n\r", victim->name, value );
        send_to_char( buf, ch );

        sprintf( buf, "Your backpack seems heavier!\n\r" );
        send_to_char( buf, victim );
        return;
    }

    return;
}

/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Nochannel whom?", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET( victim->comm, COMM_NOCHANNELS ) )
    {
        REMOVE_BIT( victim->comm, COMM_NOCHANNELS );
        send_to_char( "The gods have restored your channel priviliges.\n\r",
                      victim );
        send_to_char( "NOCHANNELS removed.\n\r", ch );
    }
    else
    {
        SET_BIT( victim->comm, COMM_NOCHANNELS );
        send_to_char( "The gods have revoked your channel priviliges.\n\r",
                      victim );
        send_to_char( "NOCHANNELS set.\n\r", ch );
    }

    return;
}

void do_bamfin( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    if ( !IS_NPC( ch ) )
    {
        smash_tilde( argument );

        if ( argument[0] == '\0' )
        {
            sprintf( buf, "Your poofin is %s\n\r`w", ch->pcdata->bamfin );
            send_to_char( buf, ch );
            return;
        }

        if ( str_str( argument, ch->name ) == NULL )
        {
            send_to_char( "You must include your name.\n\r", ch );
            return;
        }

        free_string( &ch->pcdata->bamfin );
        sprintf( buf, "%s`w", argument );
        ch->pcdata->bamfin = str_dup( buf );

        sprintf( buf, "Your poofin is now %s\n\r`w", ch->pcdata->bamfin );
        send_to_char( buf, ch );
    }
    return;
}

void do_bamfout( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    if ( !IS_NPC( ch ) )
    {
        smash_tilde( argument );

        if ( argument[0] == '\0' )
        {
            sprintf( buf, "Your poofout is %s\n\r`w", ch->pcdata->bamfout );
            send_to_char( buf, ch );
            return;
        }

        if ( str_str( argument, ch->name ) == NULL )
        {
            send_to_char( "You must include your name.\n\r", ch );
            return;
        }

        free_string( &ch->pcdata->bamfout );
        sprintf( buf, "%s`w", argument );
        ch->pcdata->bamfout = str_dup( buf );

        sprintf( buf, "Your poofout is now %s\n\r`w", ch->pcdata->bamfout );
        send_to_char( buf, ch );
    }
    return;
}

void do_deny( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Deny whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    SET_BIT( victim->act, PLR_DENY );
    send_to_char( "You are denied access!\n\r", victim );
    send_to_char( "OK.\n\r", ch );
    save_char_obj( victim );
    do_quit( victim, "" );

    return;
}

void do_disconnect( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Disconnect whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_player_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim->desc == NULL )
    {
        act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
        return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d == victim->desc )
        {
#if defined(cbuilder)
            if ( d->character )
                RemoveUser( d->character );
#endif
            close_socket( d );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}

void do_new_discon( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    int sock_num;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Disconnect whom?\n\r", ch );
        return;
    }

    if ( !is_number( arg ) )
    {
        send_to_char
            ( "Argument must be numeric, use sockets to find number.\n\r", ch );
        return;
    }
    sock_num = atoi( arg );

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->descriptor == sock_num )
        {
#if defined(cbuilder)
            if ( d->character )
                RemoveUser( d->character );
#endif
            close_socket( d );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
    }

    send_to_char( "Descriptor not on list, use sockets.\n\r", ch );
    return;
}

void do_pardon( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: pardon <character> <killer|thief>.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "killer" ) )
    {
        if ( IS_SET( victim->act, PLR_KILLER ) )
        {
            REMOVE_BIT( victim->act, PLR_KILLER );
            send_to_char( "Killer flag removed.\n\r", ch );
            send_to_char( "You are no longer a KILLER.\n\r", victim );
        }
        return;
    }

    if ( !str_cmp( arg2, "thief" ) )
    {
        if ( IS_SET( victim->act, PLR_THIEF ) )
        {
            REMOVE_BIT( victim->act, PLR_THIEF );
            send_to_char( "Thief flag removed.\n\r", ch );
            send_to_char( "You are no longer a THIEF.\n\r", victim );
        }
        return;
    }

    send_to_char( "Syntax: pardon <character> <killer|thief>.\n\r", ch );
    return;
}

void do_echo( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buff[MAX_STRING_LENGTH];
    char buff2[MAX_STRING_LENGTH];

    if ( argument[0] == '\0' )
    {
        send_to_char( "Global echo what?\n\r", ch );
        return;
    }

    sprintf( buff, "%s\n\r", argument );
    sprintf( buff2, "global> %s", buff );

#if defined(cbuilder)
    ChannelMessage( buff2, NULL );
#endif

    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->connected == CON_PLAYING )
        {
            if ( get_trust( d->character ) >= get_trust( ch ) )
                send_to_char( buff2, d->character );
            send_to_char( buff, d->character );
        }
    }

    return;
}

void do_recho( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Local echo what?\n\r", ch );

        return;
    }

    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->connected == CON_PLAYING
             && d->character->in_room == ch->in_room )
        {
            if ( get_trust( d->character ) >= get_trust( ch ) )
                send_to_char( "local> ", d->character );
            send_to_char( argument, d->character );
            send_to_char( "\n\r`w", d->character );
        }
    }

    return;
}

void do_pecho( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( argument[0] == '\0' || arg[0] == '\0' )
    {
        send_to_char( "Personal echo what?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "Target not found.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch )
         && get_trust( ch ) != MAX_LEVEL )
        send_to_char( "personal> ", victim );

    send_to_char( argument, victim );
    send_to_char( "\n\r`w", victim );
    send_to_char( "personal> ", ch );
    send_to_char( argument, ch );
    send_to_char( "\n\r`w", ch );
}

ROOM_INDEX_DATA *find_location( CHAR_DATA * ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number( arg ) && ( atoi( arg ) != ROOM_VNUM_SUPERMOB ) )
        return get_room_index( atoi( arg ) );

    if ( ( ( victim = get_char_world( ch, arg ) ) != NULL ) && ( !IS_NPC( victim )  /* +1 */
                                                                 ||
                                                                 ( IS_NPC
                                                                   ( victim )
                                                                   &&
                                                                   ( victim->
                                                                     pIndexData->
                                                                     vnum !=
                                                                     MOB_VNUM_SUPERMOB ) ) ) )
        return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
        return obj->in_room;

    return NULL;
}

void do_transfer( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Transfer whom (and where)?\n\r", ch );
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
                do_transfer( ch, buf );
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
            send_to_char( "No such location.\n\r", ch );
            return;
        }

        if ( room_is_private( location ) && get_trust( ch ) < MAX_LEVEL )
        {
            send_to_char( "That room is private right now.\n\r", ch );
            return;
        }
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) > get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( victim->in_room == NULL )
    {
        send_to_char( "They are in limbo.\n\r", ch );
        return;
    }

    if ( victim->fighting != NULL )
        stop_fighting( victim, TRUE );
    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
        act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
}

void do_at( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *wch;
    OBJ_DATA *on;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "At where what?\n\r", ch );
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    if ( room_is_private( location ) && get_trust( ch ) < MAX_LEVEL )
    {
        send_to_char( "That room is private right now.\n\r", ch );
        return;
    }

    original = ch->in_room;
    on = ch->on;
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
            ch->on = on;
            break;
        }
    }

    return;
}

void do_repop( CHAR_DATA * ch, char *argument )
{
    AREA_DATA *pArea;
    char buf[MAX_STRING_LENGTH];
    bool found_area = FALSE;
    int vnum;

    if ( argument[0] == 0 )
    {
        reset_area( ch->in_room->area );
        send_to_char( "Area repop!\n\r", ch );
    }

    if ( is_number( argument ) )
    {
        vnum = atoi( argument );
        for ( pArea = area_first; pArea; pArea = pArea->next )
        {
            if ( pArea->vnum == vnum )
            {
                sprintf( buf, "%s has been repoped!\n\r", &pArea->name[8] );
                send_to_char( buf, ch );
                found_area = TRUE;
                reset_area( pArea );
            }
        }
        if ( !found_area )
            send_to_char( "No such area!\n\r", ch );
    }

    if ( !strcmp( argument, "all" ) )
    {
        for ( pArea = area_first; pArea; pArea = pArea->next )
        {
            reset_area( pArea );
        }
        send_to_char( "World repop!\n\r", ch );
    }

    return;
}

void do_goto( CHAR_DATA * ch, char *argument )
{
    ROOM_INDEX_DATA *location;
    CHAR_DATA *rch;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Goto where?\n\r", ch );
        return;
    }

    if ( ( location = find_location( ch, argument ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    if ( room_is_private( location ) && get_trust( ch ) < MAX_LEVEL )
    {
        send_to_char( "That room is private right now.\n\r", ch );
        return;
    }

    if ( !can_see_room( ch, location ) && get_trust( ch ) < MAX_LEVEL )
    {
        send_to_char( "That room is private right now.\n\r", ch );
        return;
    }

    if ( ch->fighting != NULL )
        stop_fighting( ch, TRUE );

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if ( get_trust( rch ) >= ch->invis_level )
        {
            if ( ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0' )
                act( "$t", ch, ch->pcdata->bamfout, rch, TO_VICT );
            else
                act( "$n leaves in a swirling mist.", ch, NULL, rch, TO_VICT );
        }
    }

    char_from_room( ch );
    char_to_room( ch, location );

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if ( get_trust( rch ) >= ch->invis_level )
        {
            if ( ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0' )
                act( "$t", ch, ch->pcdata->bamfin, rch, TO_VICT );
            else
                act( "$n appears in a swirling mist.", ch, NULL, rch, TO_VICT );
        }
    }

    do_look( ch, "auto" );
    return;
}

/* RT to replace the 3 stat commands */

void do_stat( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *string;
    OBJ_DATA *obj;
    ROOM_INDEX_DATA *location;
    CHAR_DATA *victim;

    string = one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax:\n\r", ch );
        send_to_char( "  stat <name>\n\r", ch );
        send_to_char( "  stat obj <name>\n\r", ch );
        send_to_char( "  stat mob <name>\n\r", ch );
        send_to_char( "  stat room <number>\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "room" ) )
    {
        do_rstat( ch, string );
        return;
    }

    if ( !str_cmp( arg, "obj" ) )
    {
        do_ostat( ch, string );
        return;
    }

    if ( !str_cmp( arg, "char" ) || !str_cmp( arg, "mob" ) )
    {
        do_mstat( ch, string );
        return;
    }

    /* do it the old way */

    obj = get_obj_world( ch, argument );
    if ( obj != NULL )
    {
        do_ostat( ch, argument );
        return;
    }

    victim = get_char_world( ch, argument );
    if ( victim != NULL )
    {
        do_mstat( ch, argument );
        return;
    }

    location = find_location( ch, argument );
    if ( location != NULL )
    {
        do_rstat( ch, argument );
        return;
    }

    send_to_char( "Nothing by that name found anywhere.\n\r", ch );
}

void do_rstat( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    if ( ch->in_room != location && room_is_private( location ) &&
         get_trust( ch ) < MAX_LEVEL )
    {
        send_to_char( "That room is private right now.\n\r", ch );
        return;
    }

    sprintf( buf, "Name: '%s.'\n\rArea: '%s'.\n\r",
             location->name, location->area->name );
    send_to_char( buf, ch );

    sprintf( buf,
             "Vnum: %d.  Sector: %d.  Light: %d.\n\r",
             location->vnum, location->sector_type, location->light );
    send_to_char( buf, ch );

    sprintf( buf,
             "Room flags: %d.\n\rDescription:\n\r%s",
             location->room_flags, location->description );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
        EXTRA_DESCR_DATA *ed;

        send_to_char( "Extra description keywords: '", ch );
        for ( ed = location->extra_descr; ed; ed = ed->next )
        {
            send_to_char( ed->keyword, ch );
            if ( ed->next != NULL )
                send_to_char( " ", ch );
        }
        send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
        if ( can_see( ch, rch ) )
        {
            send_to_char( " ", ch );
            one_argument( rch->name, buf );
            send_to_char( buf, ch );
        }
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
        send_to_char( " ", ch );
        one_argument( obj->name, buf );
        send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( door = 0; door <= 5; door++ )
    {
        EXIT_DATA *pexit;

        if ( ( pexit = location->exit[door] ) != NULL )
        {
            sprintf( buf,
                     "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",
                     door,
                     ( pexit->u1.to_room ==
                       NULL ? -1 : pexit->u1.to_room->vnum ), pexit->key,
                     pexit->exit_info, pexit->keyword,
                     pexit->description[0] !=
                     '\0' ? pexit->description : "(none).\n\r" );
            send_to_char( buf, ch );
        }
    }

    return;
}

void do_repeat( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int count = 0;

    argument = one_argument( argument, buf );

    if ( strlen( buf ) < 1 || ( count = atoi( buf ) ) < 1
         || strlen( argument ) < 1 )
    {
        send_to_char( "Syntax:  repeat <count> <command string>\r\n", ch );
        return;
    }

    for ( ; count > 0; count-- )
        interpret( ch, argument );
}

void do_ostat( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Stat what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_world( ch, argument ) ) == NULL )
    {
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
        return;
    }

    sprintf( buf, "Name(s): %s\n\r", obj->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d  Type: %s  Resets: %d\n\r",
             obj->pIndexData->vnum, item_type_name( obj ),
             obj->pIndexData->reset_num );
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
             obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\r",
             wear_bit_name( obj->wear_flags ),
             extra_bit_name( obj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Number: %d/%d  Weight: %d/%d\n\r",
             1, get_obj_number( obj ), obj->weight, get_obj_weight( obj ) );
    send_to_char( buf, ch );

    sprintf( buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
             obj->level, obj->cost, obj->condition, obj->timer );
    send_to_char( buf, ch );

    sprintf( buf,
             "In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
             obj->in_room == NULL ? 0 : obj->in_room->vnum,
             obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr,
             obj->carried_by == NULL ? "(none)" :
             can_see( ch, obj->carried_by ) ? obj->carried_by->name
             : "someone", obj->wear_loc );
    send_to_char( buf, ch );

    sprintf( buf, "Values: %d %d %d %d %d\n\r",
             obj->value[0], obj->value[1], obj->value[2], obj->value[3],
             obj->value[4] );
    send_to_char( buf, ch );

    /* now give out vital statistics as per identify */

    switch ( obj->item_type )
    {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
        sprintf( buf, "Level %d spells of:", obj->value[0] );
        send_to_char( buf, ch );

        if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
        {
            send_to_char( " '", ch );
            send_to_char( skill_table[obj->value[1]].name, ch );
            send_to_char( "'", ch );
        }

        if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL )
        {
            send_to_char( " '", ch );
            send_to_char( skill_table[obj->value[2]].name, ch );
            send_to_char( "'", ch );
        }

        if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
        {
            send_to_char( " '", ch );
            send_to_char( skill_table[obj->value[3]].name, ch );
            send_to_char( "'", ch );
        }

        send_to_char( ".\n\r", ch );
        break;

    case ITEM_WAND:
    case ITEM_STAFF:
        sprintf( buf, "Has %d(%d) charges of level %d",
                 obj->value[1], obj->value[2], obj->value[0] );
        send_to_char( buf, ch );

        if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
        {
            send_to_char( " '", ch );
            send_to_char( skill_table[obj->value[3]].name, ch );
            send_to_char( "'", ch );
        }

        send_to_char( ".\n\r", ch );
        break;

    case ITEM_WEAPON:
        send_to_char( "Weapon type is ", ch );
        switch ( obj->value[0] )
        {
        case ( WEAPON_EXOTIC ):
            send_to_char( "exotic\n\r", ch );
            break;
        case ( WEAPON_SWORD ):
            send_to_char( "sword\n\r", ch );
            break;
        case ( WEAPON_DAGGER ):
            send_to_char( "dagger\n\r", ch );
            break;
        case ( WEAPON_SPEAR ):
            send_to_char( "spear/staff\n\r", ch );
            break;
        case ( WEAPON_MACE ):
            send_to_char( "mace/club\n\r", ch );
            break;
        case ( WEAPON_AXE ):
            send_to_char( "axe\n\r", ch );
            break;
        case ( WEAPON_FLAIL ):
            send_to_char( "flail\n\r", ch );
            break;
        case ( WEAPON_WHIP ):
            send_to_char( "whip\n\r", ch );
            break;
        case ( WEAPON_POLEARM ):
            send_to_char( "polearm\n\r", ch );
            break;
        default:
            send_to_char( "unknown\n\r", ch );
            break;
        }
        sprintf( buf, "Damage is %dd%d (average %d)\n\r",
                 obj->value[1], obj->value[2],
                 ( 1 + obj->value[2] ) * obj->value[1] / 2 );
        send_to_char( buf, ch );

        if ( obj->value[4] )    /* weapon flags */
        {
            sprintf( buf, "Weapons flags: %s\n\r",
                     weapon_bit_name( obj->value[4] ) );
            send_to_char( buf, ch );
        }
        break;

    case ITEM_ARMOR:
        sprintf( buf,
                 "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
                 obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
        send_to_char( buf, ch );
        break;
    }

    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
        EXTRA_DESCR_DATA *ed;

        send_to_char( "Extra description keywords: '", ch );

        for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
        {
            send_to_char( ed->keyword, ch );
            if ( ed->next != NULL )
                send_to_char( " ", ch );
        }

        for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
        {
            send_to_char( ed->keyword, ch );
            if ( ed->next != NULL )
                send_to_char( " ", ch );
        }

        send_to_char( "'\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
        sprintf( buf, "Affects %s by %d, level %d.\n\r",
                 affect_loc_name( paf->location ), paf->modifier, paf->level );
        send_to_char( buf, ch );
    }

    if ( !obj->enchanted )
        for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
        {
            sprintf( buf, "Affects %s by %d, level %d.\n\r",
                     affect_loc_name( paf->location ), paf->modifier,
                     paf->level );
            send_to_char( buf, ch );
        }

    return;
}

void do_mstat( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    NEWAFFECT_DATA *npaf;
    CHAR_DATA *victim;
    int x;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Stat whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, argument ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    sprintf( buf, "Name: %s.\n\r", victim->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d  Format: %s  Race: %s  Sex: %s  Room: %d\n\r",
             IS_NPC( victim ) ? victim->pIndexData->vnum : 0,
             IS_NPC( victim ) ? "mob" : "pc",
             race_table[victim->race].name,
             victim->sex == SEX_MALE ? "male" :
             victim->sex == SEX_FEMALE ? "female" : "neutral",
             victim->in_room == NULL ? 0 : victim->in_room->vnum );
    send_to_char( buf, ch );

    if ( IS_NPC( victim ) )
    {
        sprintf( buf, "Count: %d  Killed: %d\n\r",
                 victim->pIndexData->count, victim->pIndexData->killed );
        send_to_char( buf, ch );
    }

    sprintf( buf,
             "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
             victim->perm_stat[STAT_STR],
             get_curr_stat( victim, STAT_STR ),
             victim->perm_stat[STAT_INT],
             get_curr_stat( victim, STAT_INT ),
             victim->perm_stat[STAT_WIS],
             get_curr_stat( victim, STAT_WIS ),
             victim->perm_stat[STAT_DEX],
             get_curr_stat( victim, STAT_DEX ),
             victim->perm_stat[STAT_CON], get_curr_stat( victim, STAT_CON ) );
    send_to_char( buf, ch );

    sprintf( buf, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d\n\r",
             victim->hit, victim->max_hit,
             victim->mana, victim->max_mana,
             victim->move, victim->max_move,
             IS_NPC( ch ) ? 0 : victim->practice );
    send_to_char( buf, ch );

    sprintf( buf,
             "Lv: %d  Class: %s  Align: %d  Gold: %ld  Exp: %ld\n\r",
             victim->level,
             IS_NPC( victim ) ? "mobile" : class_table[victim->Class].name,
             victim->alignment, victim->gold, victim->exp );
    send_to_char( buf, ch );

    sprintf( buf, "Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
             GET_AC( victim, AC_PIERCE ), GET_AC( victim, AC_BASH ),
             GET_AC( victim, AC_SLASH ), GET_AC( victim, AC_EXOTIC ) );
    send_to_char( buf, ch );

    sprintf( buf,
             "Hit: %d  Dam: %d  Saves: %d  Position: %d  Wimpy: %d\n\r",
             GET_HITROLL( victim ), GET_DAMROLL( victim ),
             victim->saving_throw, victim->position, victim->wimpy );
    send_to_char( buf, ch );

    if ( IS_NPC( victim ) )
    {
        sprintf( buf, "Damage: %dd%d  Message:  %s\n\r",
                 victim->damage[DICE_NUMBER], victim->damage[DICE_TYPE],
                 attack_table[victim->dam_type].name );
        send_to_char( buf, ch );
    }

    sprintf( buf, "Fighting: %s\n\r",
             victim->fighting ? victim->fighting->name : "(none)" );
    send_to_char( buf, ch );

    if ( !IS_NPC( victim ) )
    {
        sprintf( buf,
                 "Thirst: %d  Full: %d  Drunk: %d\n\r",
                 victim->pcdata->condition[COND_THIRST],
                 victim->pcdata->condition[COND_FULL],
                 victim->pcdata->condition[COND_DRUNK] );
        send_to_char( buf, ch );
    }

    sprintf( buf, "Carry number: %d  Carry weight: %d\n\r",
             victim->carry_number, victim->carry_weight );
    send_to_char( buf, ch );

    if ( !IS_NPC( victim ) )
    {
        sprintf( buf,
                 "Age: %d  Played: %d  Last Level: %d  Timer: %d\n\r",
                 get_age( victim ),
                 ( int ) ( victim->played + current_time -
                           victim->logon ) / 3600,
                 victim->pcdata->last_level, victim->timer );
        send_to_char( buf, ch );
    }

    if ( !IS_NPC( victim ) )
    {
        sprintf( buf, "Recall: %d  Incarnations: %d\n\r ",
                 victim->pcdata->recall_room->vnum, victim->incarnations );
        send_to_char( buf, ch );
    }

    sprintf( buf, "Act: %s\n\r", act_bit_name( victim->act ) );
    send_to_char( buf, ch );
    printf_to_char( ch, "Jail Timer: %d\n\r", victim->jail_timer );

    if ( victim->comm )
    {
        sprintf( buf, "Comm: %s\n\r", comm_bit_name( victim->comm ) );
        send_to_char( buf, ch );
    }

    if ( IS_NPC( victim ) && victim->off_flags )
    {
        sprintf( buf, "Offense: %s\n\r", off_bit_name( victim->off_flags ) );
        send_to_char( buf, ch );
    }

    if ( victim->imm_flags )
    {
        sprintf( buf, "Immune: %s\n\r", imm_bit_name( victim->imm_flags ) );
        send_to_char( buf, ch );
    }

    if ( victim->res_flags )
    {
        sprintf( buf, "Resist: %s\n\r", imm_bit_name( victim->res_flags ) );
        send_to_char( buf, ch );
    }

    if ( victim->vuln_flags )
    {
        sprintf( buf, "Vulnerable: %s\n\r",
                 imm_bit_name( victim->vuln_flags ) );
        send_to_char( buf, ch );
    }

    sprintf( buf, "Form: %s\n\r", form_bit_name( victim->form ) );
    send_to_char( buf, ch );

    sprintf( buf, "Parts: %s\n\r", part_bit_name( victim->parts ) );
    send_to_char( buf, ch );

    if ( victim->affected_by )
    {
        sprintf( buf, "Affected by %s\n\r",
                 affect_bit_name( victim->affected_by ) );
        send_to_char( buf, ch );
    }

    if ( victim->newaffected )
    {
        send_to_char( "NewAffected by: ", ch );
        for ( x = 0; x < MAX_NEWAFF_BIT; x++ )
        {
            if ( IS_NEWAFF_SET( victim->newaff, x ) )
            {
                printf_to_char( ch, "In is new aff set" );
                printf_to_char( ch, "%s ", newaff_table[x].name );
            }
        }
        send_to_char( "\n\r", ch );
    }

    sprintf( buf, "Master: %s  Leader: %s  Pet: %s\n\r",
             victim->master ? victim->master->name : "(none)",
             victim->leader ? victim->leader->name : "(none)",
             victim->pet ? victim->pet->name : "(none)" );
    send_to_char( buf, ch );

    if ( !IS_NPC( victim ) )
    {
        sprintf( buf, "Security: %d.\n\r", victim->pcdata->security );  /* OLC */
        send_to_char( buf, ch );    /* OLC */
    }

#ifdef AUTO_HATE
    sprintf( buf, "Hating: %s\n\r",
             victim->hate ? victim->hate->name : "(none)" );
    send_to_char( buf, ch );
#endif

    sprintf( buf, "Short description: %s\n\rLong  description: %s",
             victim->short_descr,
             victim->long_descr[0] !=
             '\0' ? victim->long_descr : "(none)\n\r" );
    send_to_char( buf, ch );

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
        sprintf( buf,
                 "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
                 skill_table[( int ) paf->type].name,
                 affect_loc_name( paf->location ),
                 paf->modifier,
                 paf->duration, affect_bit_name( paf->bitvector ), paf->level );
        send_to_char( buf, ch );
    }

    for ( npaf = victim->newaffected; npaf != NULL; npaf = npaf->next )
    {
        sprintf( buf,
                 "NewSpell: '%s' modifies %s by %d for %d hours at level %d with bits ",
                 skill_table[( int ) npaf->type].name,
                 affect_loc_name( npaf->location ),
                 npaf->modifier, npaf->duration, npaf->level );
        send_to_char( buf, ch );
        for ( x = 1; x < MAX_NEWAFF_BIT; x++ )
        {
            if ( IS_NEWAFF_SET( victim->newaff, x ) )
            {
                printf_to_char( ch, "%s.", newaff_table[x].name );
            }
        }
        send_to_char( "\n\r", ch );

    }

    if ( !IS_NPC( victim ) )
    {
        sprintf( buf,
                 "E-Mail Addres: %s\n\rComment: %s\n\rSpouse: %s\n\rNemesis: %s\n\rPK Deaths: %d PK Kills: %d\n\rPoints: %d\n\r",
                 victim->pcdata->email, victim->pcdata->comment,
                 victim->pcdata->spouse, victim->pcdata->nemesis,
                 victim->pcdata->pk_deaths, victim->pcdata->pk_kills,
                 victim->pcdata->points );
        send_to_char( buf, ch );
    }

    faction_stat( ch, victim );

    return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax:\n\r", ch );
        send_to_char( "  vnum obj <name>\n\r", ch );
        send_to_char( "  vnum mob <name>\n\r", ch );
        send_to_char( "  vnum room <name>\n\r", ch );
        send_to_char( "  vnum skill <skill or spell>\n\r", ch );
        send_to_char( "  vnum mprog <name>\n\r", ch );
        send_to_char( "  vnum faction <name> or all\n\r", ch );
        return;
    }

    if ( !str_prefix( arg, "obj" ) )
    {
        do_ofind( ch, string );
        return;
    }

    if ( !str_prefix( arg, "mob" ) || !str_prefix( arg, "char" ) )
    {
        do_mfind( ch, string );
        return;
    }

    if ( !str_prefix( arg, "room" ) )
    {
        do_rfind( ch, string );
        return;
    }

    if ( !str_prefix( arg, "skill" ) || !str_prefix( arg, "spell" ) )
    {
        do_slookup( ch, string );
        return;
    }

    if ( !str_prefix( arg, "mprog" ) || !str_prefix( arg, "prog" ) )
    {
        do_mpfind( ch, string );
        return;
    }

    if ( !str_prefix( arg, "faction" ) )
    {
        do_factionfind( ch, string );
        return;
    }

    /* do all */
    do_mfind( ch, argument );
    do_ofind( ch, argument );
    do_rfind( ch, argument );
}

void do_mfind( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    extern int top_mob_index;
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Find whom?\n\r", ch );
        return;
    }

    fAll = FALSE;               /* !str_cmp( arg, "all" ); */
    found = FALSE;
    nMatch = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */

    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
        if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
        {
            nMatch++;

            if ( fAll || is_name( argument, pMobIndex->player_name ) )
            {
                found = TRUE;
                sprintf( buf, "[M:%5d] %s\n\r",
                         pMobIndex->vnum, pMobIndex->short_descr );
                send_to_char( buf, ch );
            }
        }
    }

    if ( !found )
        send_to_char( "No mobiles by that name.\n\r", ch );

    return;
}

void do_ofind( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    extern int top_obj_index;
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Find what?\n\r", ch );
        return;
    }

    fAll = FALSE;
    found = FALSE;
    nMatch = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
        if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
        {
            nMatch++;
            if ( fAll || is_name( argument, pObjIndex->name ) )
            {
                found = TRUE;
                sprintf( buf, "[O:%5d] %s\n\r",
                         pObjIndex->vnum, pObjIndex->short_descr );
                send_to_char( buf, ch );
            }
        }
    }

    if ( !found )
        send_to_char( "No objects by that name.\n\r", ch );

    return;
}

void do_rfind( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    extern int top_room;
    ROOM_INDEX_DATA *pRoomIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Find where?\n\r", ch );
        return;
    }

    fAll = FALSE;               /* !str_cmp( arg, "all" ); */
    found = FALSE;
    nMatch = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_room; vnum++ )
    {
        if ( ( pRoomIndex = get_room_index( vnum ) ) != NULL )
        {
            nMatch++;
            if ( fAll || is_name( argument, pRoomIndex->name ) )
            {
                if ( pRoomIndex->area )
                {
                    found = TRUE;
                    sprintf( buf, "[R:%5d] %-28s [Area %3d] %s\n\r",
                             pRoomIndex->vnum, pRoomIndex->name,
                             pRoomIndex->area->vnum, pRoomIndex->area->name );
                    send_to_char( buf, ch );
                }
                else
                {
                    found = TRUE;
                    sprintf( buf, "[R:%5d] %s\n\r",
                             pRoomIndex->vnum, pRoomIndex->name );
                    send_to_char( buf, ch );
                }
            }
        }
    }

    if ( !found )
        send_to_char( "No rooms by that name.\n\r", ch );

    return;
}

void do_mpfind( CHAR_DATA * ch, char *argument )
{
    MPROG_DATA *pMudProg;
    MPROG_GROUP *pMprogGroup;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char progtype = 'M';
    bool found;
    bool fAll;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Find what?\n\r", ch );
        return;
    }

    fAll = FALSE;
    found = FALSE;
    *buf = '\0';

    if ( !str_cmp( arg, "all" ) )
    {
        fAll = TRUE;
    }

    for ( pMprogGroup = mprog_group_first; pMprogGroup;
          pMprogGroup = pMprogGroup->next )
    {
        if ( fAll || is_name( argument, pMprogGroup->name ) )
        {
            found = TRUE;

            switch ( pMprogGroup->prog_type )
            {
            case MOB_PROG:
                progtype = 'M';
                break;
            case OBJ_PROG:
                progtype = 'O';
                break;
            case ROOM_PROG:
                progtype = 'R';
                break;
            }

            sprintf( buf2, "[%cG:%4d] %s\n\r", progtype, pMprogGroup->vnum,
                     pMprogGroup->name );

            if ( strlen( buf ) + strlen( buf2 ) >= sizeof( buf ) )
            {
                page_to_char( buf, ch );
                *buf = '\0';
            }
            strcat( buf, buf2 );
        }
    }

    for ( pMudProg = mudprog_first; pMudProg; pMudProg = pMudProg->next )
    {
        if ( fAll || is_name( argument, pMudProg->name ) )
        {
            found = TRUE;

            switch ( pMudProg->prog_type )
            {
            case MOB_PROG:
                progtype = 'M';
                break;
            case OBJ_PROG:
                progtype = 'O';
                break;
            case ROOM_PROG:
                progtype = 'R';
                break;
            }

            sprintf( buf2, "[%cP:%4d] %s\n\r", progtype, pMudProg->vnum,
                     pMudProg->name );

            if ( strlen( buf ) + strlen( buf2 ) >= sizeof( buf ) )
            {
                page_to_char( buf, ch );
                *buf = '\0';
            }
            strcat( buf, buf2 );
        }
    }

    if ( !found )
        send_to_char( "No MudProgs or MudProg groups by that name.\n\r", ch );
    else
        page_to_char( buf, ch );

    return;
}

void do_mwhere( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buffer[8 * MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    bool found;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Mwhere whom?\n\r", ch );
        return;
    }

    found = FALSE;
    buffer[0] = '\0';
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
        if ( IS_NPC( victim )
             && victim->in_room != NULL && is_name( argument, victim->name ) )
        {
            found = TRUE;
            sprintf( buf, "[%5d] %-28s [%5d] %s\n\r",
                     victim->pIndexData->vnum,
                     victim->short_descr,
                     victim->in_room->vnum, victim->in_room->name );
            strcat( buffer, buf );
        }
    }

    if ( !found )
        act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    else if ( ch->lines )
        page_to_char( buffer, ch );
    else
        send_to_char( buffer, ch );

    return;
}

void do_owhere( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buffer[8 * MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    bool found;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Owhere what?\n\r", ch );
        return;
    }

    found = FALSE;
    buffer[0] = '\0';
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( ( obj->in_room || obj->in_obj || obj->carried_by )
             && is_name( argument, obj->name ) )
        {
            if ( obj->in_room )
            {
                found = TRUE;
                sprintf( buf, "[%5d] %-28s [R:%5d] %s\n\r",
                         obj->pIndexData->vnum,
                         obj->short_descr,
                         obj->in_room->vnum, obj->in_room->name );
                strcat( buffer, buf );
            }
            else if ( obj->in_obj )
            {
                found = TRUE;
                sprintf( buf, "[%5d] %-28s [O:%5d] %s\n\r",
                         obj->pIndexData->vnum,
                         obj->short_descr,
                         obj->in_obj->pIndexData->vnum,
                         obj->in_obj->short_descr );
                strcat( buffer, buf );
            }
            else if ( obj->carried_by )
            {
                if ( IS_NPC( obj->carried_by ) )
                {
                    found = TRUE;
                    sprintf( buf, "[%5d] %-28s [M:%5d] %s\n\r",
                             obj->pIndexData->vnum,
                             obj->short_descr,
                             obj->carried_by->pIndexData->vnum,
                             obj->carried_by->name );
                    strcat( buffer, buf );
                }
                else
                {
                    found = TRUE;
                    sprintf( buf, "[%5d] %-28s [Player] %s\n\r",
                             obj->pIndexData->vnum,
                             obj->short_descr, obj->carried_by->name );
                    strcat( buffer, buf );
                }
            }
        }
    }

    if ( !found )
        act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
    else if ( ch->lines )
        page_to_char( buffer, ch );
    else
        send_to_char( buffer, ch );

    return;
}

void do_reboo( CHAR_DATA * ch, char *argument )
{
    send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
    return;
}

void do_reboot( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d, *d_next;
    extern bool chaos;
#if defined(cbuilder)
    extern bool Reboot;
#endif

    if ( ( chaos ) && ( ch->Class != 4 ) )
    {
        send_to_char
            ( "Please remove `rC`RH`YA`RO`rS`w before rebooting.\n\r", ch );
        return;
    }
    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mobs dont need to be rebooting the mud.\n", ch );
        return;
    }
    if ( !IS_SET( ch->act, PLR_WIZINVIS ) )
    {
        sprintf( buf, "Reboot by %s.", ch->name );
        do_echo( ch, buf );
    }
    else
    {
        sprintf( buf, "Rebooting..." );
        do_echo( ch, buf );
    }
    do_force( ch, "all save" );
    if ( ch->Class != 4 )
        do_save( ch, "" );
    do_asave( ch, "changed" );
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
#if defined(cbuilder)
        if ( d->character )
            RemoveUser( d->character );
#endif
        d_next = d->next;
        close_socket( d );
    }

#if defined(cbuilder)
    Reboot = TRUE;
#endif

    return;
}

/* Added to prevent my son, age 5, from hotbooting the mud by
 * typing hot 
 */
void do_hotboo( CHAR_DATA * ch, char *argument )
{
    send_to_char( "Type it all out if you want to hotboot the mud.\n\r", ch );
    return;
}

void do_shutdow( CHAR_DATA * ch, char *argument )
{
    send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
    return;
}

void do_shutdown( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char strPath[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d, *d_next;
    extern bool chaos;

    if ( ( chaos ) && ( ch->Class != 4 ) )
    {
        send_to_char
            ( "Please remove `rC`RH`YA`RO`rS`w before shutting down.\n\r", ch );
        return;
    }
    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mobs dont need to be rebooting the mud.\n", ch );
        return;
    }

    if ( !IS_SET( ch->act, PLR_WIZINVIS ) )
        sprintf( buf, "Shutdown by %s.", ch->name );
    else
        sprintf( buf, "Shutting down." );

    sprintf( strPath, "%s/%s", sysconfig.area_dir, sysconfig.shutdown_file );
    append_file( ch, strPath, buf );

    strcat( buf, "\n\r" );
    do_echo( ch, buf );
    do_force( ch, "all save" );
    if ( ch->Class != 4 )
        do_save( ch, "" );
    do_asave( ch, "changed" );
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
#if defined(cbuilder)
        if ( d->character )
            RemoveUser( d->character );
#endif
        d_next = d->next;
        close_socket( d );
    }
    return;
}

void do_snoop( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Snoop whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim->desc == NULL )
    {
        send_to_char( "No descriptor to snoop.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "Cancelling all snoops.\n\r", ch );
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->snoop_by == ch->desc )
                d->snoop_by = NULL;
        }
        return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
        send_to_char( "Busy already.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( ch->desc != NULL )
    {
        for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
        {
            if ( d->character == victim || d->original == victim )
            {
                send_to_char( "No snoop loops.\n\r", ch );
                return;
            }
        }
    }

    victim->desc->snoop_by = ch->desc;
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_switch( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Switch into whom?\n\r", ch );
        return;
    }

    if ( ch->desc == NULL )
        return;

    if ( ch->desc->original != NULL )
    {
        send_to_char( "You are already switched.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_SET( victim->act, PLR_JAILED ) )
    {
        send_to_char( "Cannot switch into jailed mobs", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) )
    {
        send_to_char( "You can only switch into mobiles.\n\r", ch );
        return;
    }

    if ( victim->desc != NULL )
    {
        send_to_char( "Character in use.\n\r", ch );
        return;
    }

    if ( ch->pcdata->tick > 0 )
    {
        send_to_char
            ( "Mobs cannot use the tick option, so it has been turned off.\n\r",
              ch );
    }

    ch->pcdata->tick = 0;
    ch->desc->character = victim;
    ch->desc->original = ch;
    victim->desc = ch->desc;
    ch->desc = NULL;
    /* change communications to match */
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    send_to_char( "Ok.\n\r", victim );
    return;
}

void do_return( CHAR_DATA * ch, char *argument )
{
    if ( ch->desc == NULL )
        return;

    if ( ch->desc->original == NULL )
    {
        send_to_char( "You aren't switched.\n\r", ch );
        return;
    }

    if ( IS_SET( ch->act, PLR_JAILED ) )
    {
        send_to_char
            ( "You have been jailed and will remain so until your jail sentence as a mob has been completed\n\r",
              ch );
        return;
    }

    send_to_char( "You return to your original body.\n\r", ch );
    ch->desc->character = ch->desc->original;
    ch->desc->original = NULL;
    ch->desc->character->desc = ch->desc;
    ch->desc = NULL;
    return;
}

/* trust levels for load and clone */
bool obj_check( CHAR_DATA * ch, OBJ_DATA * obj )
{
    if ( IS_TRUSTED( ch, GOD )
         || ( IS_TRUSTED( ch, IMMORTAL ) && obj->level <= 20
              && obj->cost <= 1000 ) || ( IS_TRUSTED( ch, DEMI )
                                          && obj->level <= 10
                                          && obj->cost <= 500 )
         || ( IS_TRUSTED( ch, ANGEL ) && obj->level <= 5
              && obj->cost <= 250 ) || ( IS_TRUSTED( ch, AVATAR )
                                         && obj->level == 0
                                         && obj->cost <= 100 ) )
        return TRUE;
    else
        return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone( CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * clone )
{
    OBJ_DATA *c_obj, *t_obj;

    for ( c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content )
    {
        if ( obj_check( ch, c_obj ) )
        {
            t_obj = create_object( c_obj->pIndexData, 0 );
            clone_object( c_obj, t_obj );
            obj_to_obj( t_obj, clone );
            recursive_clone( ch, c_obj, t_obj );
        }
    }
}

/* command that is similar to load */
void do_clone( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char *rest;
    CHAR_DATA *mob;
    OBJ_DATA *obj;

    rest = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Clone what?\n\r", ch );
        return;
    }

    if ( !str_prefix( arg, "object" ) )
    {
        mob = NULL;
        obj = get_obj_here( ch, rest );
        if ( obj == NULL )
        {
            send_to_char( "You don't see that here.\n\r", ch );
            return;
        }
    }
    else if ( !str_prefix( arg, "mobile" ) || !str_prefix( arg, "character" ) )
    {
        obj = NULL;
        mob = get_char_room( ch, rest );
        if ( mob == NULL )
        {
            send_to_char( "You don't see that here.\n\r", ch );
            return;
        }
    }
    else                        /* find both */
    {
        mob = get_char_room( ch, argument );
        obj = get_obj_here( ch, argument );
        if ( mob == NULL && obj == NULL )
        {
            send_to_char( "You don't see that here.\n\r", ch );
            return;
        }
    }

    /* clone an object */
    if ( obj != NULL )
    {
        OBJ_DATA *clone;

        if ( !obj_check( ch, obj ) )
        {
            send_to_char
                ( "Your powers are not great enough for such a task.\n\r", ch );
            return;
        }

        clone = create_object( obj->pIndexData, 0 );
        clone_object( obj, clone );
        if ( obj->carried_by != NULL )
            obj_to_char( clone, ch );
        else
            obj_to_room( clone, ch->in_room );
        recursive_clone( ch, obj, clone );

        act( "$n has created $p.", ch, clone, NULL, TO_ROOM );
        act( "You clone $p.", ch, clone, NULL, TO_CHAR );
        return;
    }
    else if ( mob != NULL )
    {
        CHAR_DATA *clone;
        OBJ_DATA *new_obj;

        if ( !IS_NPC( mob ) )
        {
            send_to_char( "You can only clone mobiles.\n\r", ch );
            return;
        }

        if ( ( mob->level > 20 && !IS_TRUSTED( ch, GOD ) )
             || ( mob->level > 10 && !IS_TRUSTED( ch, IMMORTAL ) )
             || ( mob->level > 5 && !IS_TRUSTED( ch, DEMI ) )
             || ( mob->level > 0 && !IS_TRUSTED( ch, ANGEL ) )
             || !IS_TRUSTED( ch, AVATAR ) )
        {
            send_to_char
                ( "Your powers are not great enough for such a task.\n\r", ch );
            return;
        }

        clone = create_mobile( mob->pIndexData );
        clone_mobile( mob, clone );

        for ( obj = mob->carrying; obj != NULL; obj = obj->next_content )
        {
            if ( obj_check( ch, obj ) )
            {
                new_obj = create_object( obj->pIndexData, 0 );
                clone_object( obj, new_obj );
                recursive_clone( ch, obj, new_obj );
                obj_to_char( new_obj, clone );
                new_obj->wear_loc = obj->wear_loc;
            }
        }
        char_to_room( clone, ch->in_room );
        act( "$n has created $N.", ch, NULL, clone, TO_ROOM );
        act( "You clone $N.", ch, NULL, clone, TO_CHAR );
        return;
    }
}

/* RT to replace the two load commands */

void do_load( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax:\n\r", ch );
        send_to_char( "  load mob <vnum>\n\r", ch );
        send_to_char( "  load obj <vnum> <level>\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "mob" ) || !str_cmp( arg, "char" ) )
    {
        do_mload( ch, argument );
        return;
    }

    if ( !str_cmp( arg, "obj" ) )
    {
        do_oload( ch, argument );
        return;
    }
    /* echo syntax */
    do_load( ch, "" );
}

void do_mload( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number( arg ) )
    {
        send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
        return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
        send_to_char( "No mob has that vnum.\n\r", ch );
        return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_oload( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;

    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );

    if ( arg[0] == '\0' || !is_number( arg ) )
    {
        send_to_char( "Syntax: load obj <vnum> <level>.\n\r", ch );
        return;
    }

    level = get_trust( ch );    /* default */

    if ( arg2[0] != '\0' )      /* load with a level */
    {
        if ( !is_number( arg2 ) )
        {
            send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
            return;
        }
        level = atoi( arg2 );
        if ( level < 0 || level > get_trust( ch ) )
        {
            send_to_char( "Level must be be between 0 and your level.\n\r",
                          ch );
            return;
        }
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg ) ) ) == NULL )
    {
        send_to_char( "No object has that vnum.\n\r", ch );
        return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR( obj, ITEM_TAKE ) )
        obj_to_char( obj, ch );
    else
        obj_to_room( obj, ch->in_room );
    act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_purge( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        CHAR_DATA *vnext;
        OBJ_DATA *obj_next;

        for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
        {
            vnext = victim->next_in_room;

            if ( IS_NPC( victim ) && !IS_SET( victim->act, ACT_NOPURGE )
                 && victim != ch /* safety precaution */  )
            {
                extract_char( victim, TRUE );
            }
        }

        for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;

            if ( !IS_OBJ_STAT( obj, ITEM_NOPURGE ) )
            {
                extract_obj( obj );
            }
        }

        act( "`mYou are surrounded by purple flames as $n purges the room.`w",
             ch, NULL, NULL, TO_ROOM );
        send_to_char( "`mYou are surrounded in all consuming flames.\n\r`w",
                      ch );
        return;
    }

    if ( ( obj = get_obj_list( ch, arg, ch->in_room->contents ) ) != NULL )
    {
        act( "You disintegrate $p.", ch, obj, NULL, TO_CHAR );
        act( "$n disintegrates $p.", ch, obj, NULL, TO_ROOM );
        extract_obj( obj );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "Nothing like that in heaven or hell.\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) )
    {

        if ( ch == victim )
        {
            send_to_char( "Ho ho ho.\n\r", ch );
            return;
        }

        if ( get_trust( ch ) <= get_trust( victim ) )
        {
            send_to_char( "Maybe that wasn't a good idea...\n\r", ch );
            sprintf( buf, "%s tried to purge you!\n\r", ch->name );
            send_to_char( buf, victim );
            return;
        }

        act( "$n disintegrates $N.", ch, 0, victim, TO_NOTVICT );
        act( "You disintegrate $N.", ch, 0, victim, TO_CHAR );

        save_char_obj( victim );

        d = victim->desc;
        extract_char( victim, TRUE );

        if ( d != NULL )
        {
#if defined(cbuilder)
            if ( d->character )
            {
                RemoveUser( d->character );
            }
#endif
            close_socket( d );
        }

        return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    act( "You disintegrate $N.", ch, 0, victim, TO_CHAR );
    extract_char( victim, TRUE );
    return;
}

void do_advance( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buff[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int level;
    int iLevel;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
        send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL )
    {
        sprintf( buff, "Level must be 1 to %d.\n\r", MAX_LEVEL );
        send_to_char( buff, ch );
        return;
    }

    if ( level > get_trust( ch ) )
    {
        send_to_char( "Limited to your trust level.\n\r", ch );
        return;
    }

    if ( victim->level > ch->level )
    {
        printf_to_char( ch,
                        "You're not powerful enough to do that to %s.\n\r",
                        victim->name );
        printf_to_char( victim,
                        "%s just tried to lower your level with the advance command.",
                        ch->name );
        return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= victim->level )
    {
        int temp_prac;

        send_to_char( "Lowering a player's level!\n\r", ch );
        send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim );
        temp_prac = victim->practice;
        victim->level = 1;
        victim->exp = 0;
        victim->max_hit = 10;
        victim->max_mana = 100;
        victim->max_move = 100;
        victim->pcdata->perm_hit = 10;
        victim->pcdata->perm_mana = 100;
        victim->pcdata->perm_move = 100;
        victim->practice = 0;
        victim->hit = victim->max_hit;
        victim->mana = victim->max_mana;
        victim->move = victim->max_move;
        advance_level( victim );
        victim->practice = temp_prac;
    }
    else
    {
        send_to_char( "Raising a player's level!\n\r", ch );
        send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
    }

    for ( iLevel = victim->level; iLevel < level; iLevel++ )
    {
        send_to_char( "You raise a level!!  ", victim );
        victim->level += 1;
        advance_level( victim );
    }
    victim->exp = 0;
    victim->trust = 0;
    save_char_obj( victim );
    return;
}

void do_trust( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buff[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int level;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
        send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch );
        return;
    }

    if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
    {
        sprintf( buff, "Level must be 0 (reset) or 1 to %d.\n\r", MAX_LEVEL );
        send_to_char( buff, ch );
        return;
    }

    if ( get_trust( ch ) <= get_trust( victim ) )
    {
        send_to_char( "Nice try.\n\r", ch );
        return;
    }

    if ( level > get_trust( ch ) )
    {
        send_to_char( "Limited to your trust.\n\r", ch );
        return;
    }

    victim->trust = level;
    return;
}

void do_restore( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );
    if ( arg[0] == '\0' || !str_cmp( arg, "room" ) )
    {
        /* cure room */

        for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
        {
            affect_strip( vch, gsn_plague );
            affect_strip( vch, gsn_poison );
            affect_strip( vch, gsn_blindness );
            affect_strip( vch, gsn_sleep );
            affect_strip( vch, gsn_curse );

            vch->hit = vch->max_hit;
            vch->mana = vch->max_mana;
            vch->move = vch->max_move;
            update_pos( vch );
            act( "$n has restored you.", ch, NULL, vch, TO_VICT );
        }

        send_to_char( "Room restored.\n\r", ch );
        return;

    }

    if ( get_trust( ch ) >= MAX_LEVEL && !str_cmp( arg, "all" ) )
    {
        /* cure all */

        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            victim = d->character;

            if ( victim == NULL || IS_NPC( victim ) )
                continue;

            affect_strip( victim, gsn_plague );
            affect_strip( victim, gsn_poison );
            affect_strip( victim, gsn_blindness );
            affect_strip( victim, gsn_sleep );
            affect_strip( victim, gsn_curse );

            victim->hit = victim->max_hit;
            victim->mana = victim->max_mana;
            victim->move = victim->max_move;
            update_pos( victim );
            if ( victim->in_room != NULL )
                act( "$n has restored you.", ch, NULL, victim, TO_VICT );
        }
        send_to_char( "All active players restored.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    affect_strip( victim, gsn_plague );
    affect_strip( victim, gsn_poison );
    affect_strip( victim, gsn_blindness );
    affect_strip( victim, gsn_sleep );
    affect_strip( victim, gsn_curse );
    victim->hit = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );
    act( "$n has restored you.", ch, NULL, victim, TO_VICT );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_freeze( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Freeze whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET( victim->act, PLR_FREEZE ) )
    {
        REMOVE_BIT( victim->act, PLR_FREEZE );
        send_to_char( "You can play again.\n\r", victim );
        send_to_char( "FREEZE removed.\n\r", ch );
    }
    else
    {
        SET_BIT( victim->act, PLR_FREEZE );
        send_to_char( "You can't do ANYthing!\n\r", victim );
        send_to_char( "FREEZE set.\n\r", ch );
    }

    save_char_obj( victim );

    return;
}

void do_log( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Log whom?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        if ( fLogAll )
        {
            fLogAll = FALSE;
            send_to_char( "Log ALL off.\n\r", ch );
        }
        else
        {
            fLogAll = TRUE;
            send_to_char( "Log ALL on.\n\r", ch );
        }
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET( victim->act, PLR_LOG ) )
    {
        REMOVE_BIT( victim->act, PLR_LOG );
        send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
        SET_BIT( victim->act, PLR_LOG );
        send_to_char( "LOG set.\n\r", ch );
    }

    return;
}

void do_noemote( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Noemote whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET( victim->comm, COMM_NOEMOTE ) )
    {
        REMOVE_BIT( victim->comm, COMM_NOEMOTE );
        send_to_char( "You can emote again.\n\r", victim );
        send_to_char( "NOEMOTE removed.\n\r", ch );
    }
    else
    {
        SET_BIT( victim->comm, COMM_NOEMOTE );
        send_to_char( "You can't emote!\n\r", victim );
        send_to_char( "NOEMOTE set.\n\r", ch );
    }

    return;
}

void do_noshout( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Noshout whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET( victim->comm, COMM_NOSHOUT ) )
    {
        REMOVE_BIT( victim->comm, COMM_NOSHOUT );
        send_to_char( "You can shout again.\n\r", victim );
        send_to_char( "NOSHOUT removed.\n\r", ch );
    }
    else
    {
        SET_BIT( victim->comm, COMM_NOSHOUT );
        send_to_char( "You can't shout!\n\r", victim );
        send_to_char( "NOSHOUT set.\n\r", ch );
    }

    return;
}

void do_notell( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Notell whom?", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET( victim->comm, COMM_NOTELL ) )
    {
        REMOVE_BIT( victim->comm, COMM_NOTELL );
        send_to_char( "You can tell again.\n\r", victim );
        send_to_char( "NOTELL removed.\n\r", ch );
    }
    else
    {
        SET_BIT( victim->comm, COMM_NOTELL );
        send_to_char( "You can't tell!\n\r", victim );
        send_to_char( "NOTELL set.\n\r", ch );
    }

    return;
}

void do_peace( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA *rch;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if ( rch->fighting != NULL )
            stop_fighting( rch, TRUE );
        if ( IS_NPC( rch ) && IS_SET( rch->act, ACT_AGGRESSIVE ) )
            REMOVE_BIT( rch->act, ACT_AGGRESSIVE );
#ifdef AUTO_HATE
        stop_hating( rch );
#endif
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

/* The ban functions that use to be here have been ripped out to make room
for the new ban and permban. -Lancelight */

void do_wizlock( CHAR_DATA * ch, char *argument )
{
    extern bool wizlock;
    wizlock = !wizlock;

    if ( wizlock )
        send_to_char( "Game wizlocked.\n\r", ch );
    else
        send_to_char( "Game un-wizlocked.\n\r", ch );

    return;
}

/* RT anti-newbie code */

void do_newlock( CHAR_DATA * ch, char *argument )
{
    extern bool newlock;
    newlock = !newlock;

    if ( newlock )
        send_to_char( "New characters have been locked out.\n\r", ch );
    else
        send_to_char( "Newlock removed.\n\r", ch );

    return;
}

void do_slookup( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int sn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Lookup which skill or spell?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        for ( sn = 0; sn < MAX_SKILL; sn++ )
        {
            if ( skill_table[sn].name == NULL )
                break;
            sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
                     sn, skill_table[sn].slot, skill_table[sn].name );
            send_to_char( buf, ch );
        }
    }
    else
    {
        if ( ( sn = skill_lookup( arg ) ) < 0 )
        {
            send_to_char( "No such skill or spell.\n\r", ch );
            return;
        }

        sprintf( buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
                 sn, skill_table[sn].slot, skill_table[sn].name );
        send_to_char( buf, ch );
    }

    return;
}

/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax:\n\r", ch );
        send_to_char( "  set char  <name> <field> <value>\n\r", ch );
        send_to_char( "  set mob   <name> <field> <value>\n\r", ch );
        send_to_char( "  set obj   <name> <field> <value>\n\r", ch );
        send_to_char( "  set room  <room> <field> <value>\n\r", ch );
        send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch );
        return;
    }

    if ( !str_prefix( arg, "mobile" ) || !str_prefix( arg, "character" ) )
    {
        do_mset( ch, argument );
        return;
    }

    if ( !str_prefix( arg, "skill" ) || !str_prefix( arg, "spell" ) )
    {
        do_sset( ch, argument );
        return;
    }

    if ( !str_prefix( arg, "object" ) )
    {
        do_oset( ch, argument );
        return;
    }

    if ( !str_prefix( arg, "room" ) )
    {
        do_rset( ch, argument );
        return;
    }
    /* echo syntax */
    do_set( ch, "" );
}

void do_sset( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char( "Syntax:\n\r", ch );
        send_to_char( "  set skill <name> <spell or skill> <value>\n\r", ch );
        send_to_char( "  set skill <name> all <value>\n\r", ch );
        send_to_char( "   (use the name of the skill, not the number)\n\r",
                      ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
        send_to_char( "No such skill or spell.\n\r", ch );
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
        send_to_char( "Value must be numeric.\n\r", ch );
        return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
        send_to_char( "Value range is 0 to 100.\n\r", ch );
        return;
    }

    if ( fAll )
    {
        for ( sn = 0; sn < MAX_SKILL; sn++ )
        {
            if ( skill_table[sn].name != NULL )
                victim->pcdata->learned[sn] = value;
        }
    }
    else
    {
        victim->pcdata->learned[sn] = value;
    }

    return;
}

void do_mset( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );
    argument = one_argument( argument, arg4 );
    argument = one_argument( argument, arg4 );

    if ( ( arg[0] == '\0' ) || ( arg2[0] == '\0' ) || ( arg3[0] == '\0' ) )
    {
        send_to_char( "Syntax:\n\r", ch );
        send_to_char( "  set char <name> <field> <value>\n\r", ch );
        send_to_char( "  set char <name> faction <vnum> <value>\n\r\n\r", ch );
        send_to_char( "Field being one of:\n\r", ch );
        send_to_char( "  str int wis dex con sex class level\n\r", ch );
        send_to_char( "  race gold hp mana move practice align\n\r", ch );
        send_to_char( "  train thirst drunk full security recall\n\r", ch );
        send_to_char( "  vlower vupper whorace whoprefix\n\r", ch );
/*      send_to_char( "    clan\n\r",ch ); */
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ( victim != ch && !IS_NPC( victim ) )
         && ( get_trust( ch ) <= get_trust( victim ) ) )
    {
        send_to_char
            ( "You cannot modify a character of equal or higher level.\n\r",
              ch );
        return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set Faction.
     */
    if ( !str_prefix( arg2, "faction" ) )
    {
        one_argument( arg3, arg3 );
        value = is_number( arg3 ) ? atoi( arg3 ) : -1;

        if ( value < 1 )
        {
            send_to_char( "Invalid faction vnum\n\r", ch );
            return;
        }

        if ( arg4[0] == '\0' )
        {
            send_to_char( "Invalid faction value\n\r", ch );
            return;
        }

        set_faction( ch, victim, ( sh_int ) value, ( sh_int ) atoi( arg4 ) );

        return;
    }

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "str" ) )
    {
        if ( value < 3 || value > get_max_train( victim, STAT_STR ) )
        {
            sprintf( buf,
                     "Strength range is 3 to %d\n\r.",
                     get_max_train( victim, STAT_STR ) );
            send_to_char( buf, ch );
            return;
        }

        victim->perm_stat[STAT_STR] = value;
        return;
    }

    if ( !str_prefix( arg2, "vlower" ) )
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > victim->pcdata->vnum_range[1] )
        {
            send_to_char( "Syntex Error:\n\r", ch );
            send_to_char( "1) vlower has to be less than vupper\n\r", ch );
            send_to_char( "2) you have not set the vupper range yet\n\r", ch );
            return;
        }

        victim->pcdata->vnum_range[0] = value;
        printf_to_char( ch, "%s's Lower Vnum has been set to %d\n\r",
                        victim->name, value );
        return;
    }

    if ( !str_prefix( arg2, "vupper" ) )
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < victim->pcdata->vnum_range[0] )
        {
            send_to_char
                ( "Invalid VNUM.  The Vnum must be greater then the lower vnum.\n\r",
                  ch );
            return;
        }

        victim->pcdata->vnum_range[1] = value;
        printf_to_char( ch, "%s's Upper Vnum has been set to %d\n\r",
                        victim->name, value );
        return;
    }

    if ( !str_prefix( arg2, "int" ) )
    {
        if ( value < 3 || value > get_max_train( victim, STAT_INT ) )
        {
            sprintf( buf,
                     "Intelligence range is 3 to %d.\n\r",
                     get_max_train( victim, STAT_INT ) );
            send_to_char( buf, ch );
            return;
        }

        victim->perm_stat[STAT_INT] = value;
        return;
    }

    if ( !str_prefix( arg2, "wis" ) )
    {
        if ( value < 3 || value > get_max_train( victim, STAT_WIS ) )
        {
            sprintf( buf,
                     "Wisdom range is 3 to %d.\n\r", get_max_train( victim,
                                                                    STAT_WIS ) );
            send_to_char( buf, ch );
            return;
        }

        victim->perm_stat[STAT_WIS] = value;
        return;
    }

    if ( !str_prefix( arg2, "dex" ) )
    {
        if ( value < 3 || value > get_max_train( victim, STAT_DEX ) )
        {
            sprintf( buf,
                     "Dexterity ranges is 3 to %d.\n\r",
                     get_max_train( victim, STAT_DEX ) );
            send_to_char( buf, ch );
            return;
        }

        victim->perm_stat[STAT_DEX] = value;
        return;
    }

    if ( !str_prefix( arg2, "con" ) )
    {
        if ( value < 3 || value > get_max_train( victim, STAT_CON ) )
        {
            sprintf( buf,
                     "Constitution range is 3 to %d.\n\r",
                     get_max_train( victim, STAT_CON ) );
            send_to_char( buf, ch );
            return;
        }

        victim->perm_stat[STAT_CON] = value;
        return;
    }

    if ( !str_prefix( arg2, "sex" ) )
    {
        if ( value < 0 || value > 2 )
        {
            send_to_char( "Sex range is 0 to 2.\n\r", ch );
            return;
        }
        victim->sex = value;
        if ( !IS_NPC( victim ) )
            victim->pcdata->true_sex = value;
        return;
    }

    if ( !str_prefix( arg2, "class" ) )
    {
        int Class;

        if ( IS_NPC( victim ) )
        {
            send_to_char( "Mobiles have no class.\n\r", ch );
            return;
        }

        Class = class_lookup( arg3 );
        if ( Class == -1 )
        {
            strcpy( buf, "Possible classes are: " );
            for ( Class = 0; Class < MAX_CLASS; Class++ )
            {
                if ( Class > 0 )
                    strcat( buf, " " );
                strcat( buf, class_table[Class].name );
            }
            strcat( buf, ".\n\r" );

            send_to_char( buf, ch );
            return;
        }

        victim->Class = Class;
        return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
        if ( !IS_NPC( victim ) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > MAX_LEVEL )
        {
            sprintf( buf, "Level range is 0 to %d.\n\r", MAX_LEVEL );
            send_to_char( buf, ch );
            return;
        }
        victim->level = value;
        return;
    }
    if ( !str_prefix( arg2, "gold" ) )
    {
        victim->gold = value;
        return;
    }

    if ( !str_prefix( arg2, "hp" ) )
    {
        if ( value < -10 || value > 30000 )
        {
            send_to_char( "Hp range is -10 to 30,000 hit points.\n\r", ch );
            return;
        }
        victim->max_hit = value;
        if ( !IS_NPC( victim ) )
            victim->pcdata->perm_hit = value;
        return;
    }

    if ( !str_prefix( arg2, "mana" ) )
    {
        if ( value < 0 || value > 30000 )
        {
            send_to_char( "Mana range is 0 to 30,000 mana points.\n\r", ch );
            return;
        }
        victim->max_mana = value;
        if ( !IS_NPC( victim ) )
            victim->pcdata->perm_mana = value;
        return;
    }

    if ( !str_prefix( arg2, "move" ) )
    {
        if ( value < 0 || value > 30000 )
        {
            send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
            return;
        }
        victim->max_move = value;
        if ( !IS_NPC( victim ) )
            victim->pcdata->perm_move = value;
        return;
    }

    if ( !str_prefix( arg2, "practice" ) )
    {
        if ( value < 0 || value > 250 )
        {
            send_to_char( "Practice range is 0 to 250 sessions.\n\r", ch );
            return;
        }
        victim->practice = value;
        return;
    }

    if ( !str_prefix( arg2, "train" ) )
    {
        if ( value < 0 || value > 50 )
        {
            send_to_char( "Training session range is 0 to 50 sessions.\n\r",
                          ch );
            return;
        }
        victim->train = value;
        return;
    }

    if ( !str_prefix( arg2, "align" ) )
    {
        if ( value < -1000 || value > 1000 )
        {
            send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
            return;
        }
        victim->alignment = value;
        return;
    }

    if ( !str_prefix( arg2, "thirst" ) )
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < -1 || value > 100 )
        {
            send_to_char( "Thirst range is -1 to 100.\n\r", ch );
            return;
        }

        victim->pcdata->condition[COND_THIRST] = value;
        return;
    }

    if ( !str_prefix( arg2, "drunk" ) )
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < -1 || value > 100 )
        {
            send_to_char( "Drunk range is -1 to 100.\n\r", ch );
            return;
        }

        victim->pcdata->condition[COND_DRUNK] = value;
        return;
    }

    if ( !str_prefix( arg2, "full" ) )
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < -1 || value > 100 )
        {
            send_to_char( "Full range is -1 to 100.\n\r", ch );
            return;
        }

        victim->pcdata->condition[COND_FULL] = value;
        return;
    }

    if ( !str_prefix( arg2, "race" ) )
    {
        int race;

        race = race_lookup( arg3 );

        if ( race == 0 )
        {
            send_to_char( "That is not a valid race.\n\r", ch );
            return;
        }

        if ( !IS_NPC( victim ) && !race_table[race].pc_race )
        {
            send_to_char( "That is not a valid player race.\n\r", ch );
            return;
        }

        victim->race = race;
        return;
    }

    if ( !str_prefix( arg2, "security" ) )  /* OLC */
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
        /* Why should IMPs have to have security 9 to change anything?
           Changed by Kyle to be less annoying to new IMPs.  */
        if ( ( value > ch->pcdata->security || value < 0 )
             && get_trust( ch ) < MAX_LEVEL )
        {
            if ( ch->pcdata->security != 0 )
            {
                sprintf( buf, "Valid security is 0-%d.\n\r",
                         ch->pcdata->security );
                send_to_char( buf, ch );
            }
            else
            {
                send_to_char( "Valid security is 0 only.\n\r", ch );
            }
            return;
        }
        else if ( value > 9 || value < 0 )
        {
            send_to_char( "Valid security is 0-9.\n\r", ch );
            return;
        }

        victim->pcdata->security = value;
        return;
    }

    if ( !str_prefix( arg2, "recall" ) )    /* Thexder */
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
        if ( ( location = find_location( ch, arg3 ) ) == NULL )
        {
            send_to_char( "That is an invalid location.\n\r", ch );
            return;
        }
        else
        {
            victim->pcdata->recall_room = location;
            send_to_char( "Recall set.\n\r", ch );
            return;
        }
    }

    if ( !str_prefix( arg2, "whorace" ) )   /* By Kyle */
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg3, "normal" )
             || !str_cmp( arg3, "default" ) || !str_cmp( arg3, "none" ) )
        {
            free_string( &victim->pcdata->who_race );
            send_to_char( "Race now shows the actual race.\n\r", ch );
            return;
        }
        if ( str_len( arg3 ) > 10 )
        {
            send_to_char
                ( "Too long, length is limited to 10 characters not counting color.\n\r",
                  ch );
            return;
        }
        if ( str_len( arg3 ) < 10 )
        {
            int numpads = 0;
            int extrapad = 0;
            int x = 0;

            extrapad = ( 10 - str_len( arg3 ) ) % 2;

            numpads = ( 10 - str_len( arg3 ) - extrapad ) / 2;

            if ( numpads > 0 )
            {
                sprintf( buf, "%*s", numpads + str_len( arg3 ), arg3 );
                strcpy( arg3, buf );
            }

            for ( x = 0; x < numpads + extrapad; x++ )
                strcat( arg3, " " );
        }
        free_string( &victim->pcdata->who_race );
        smash_tilde( arg3 );
        victim->pcdata->who_race = str_dup( arg3 );
        send_to_char( "Whorace set.\n\r", ch );
        return;
    }

    if ( !str_prefix( arg2, "whoprefix" ) ) /* By Kyle */
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
        if ( !str_cmp( arg3, "normal" )
             || !str_cmp( arg3, "default" ) || !str_cmp( arg3, "none" ) )
        {
            free_string( &victim->pcdata->who_prefix );
            send_to_char( "Prefix removed.\n\r", ch );
            return;
        }
        if ( str_len( arg3 ) > 20 )
        {
            send_to_char
                ( "Too long, length is limited to 20 characters not counting color.\n\r",
                  ch );
            return;
        }
        free_string( &victim->pcdata->who_prefix );
        smash_tilde( arg3 );
        victim->pcdata->who_prefix = str_dup( arg3 );
        send_to_char( "Whoprefix set.\n\r", ch );
        return;
    }

    /*
     * Generate usage message.
     */
    do_mset( ch, "" );
    return;
}

void do_string( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char type[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    smash_tilde( argument );
    argument = one_argument( argument, type );
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( type[0] == '\0' || arg[0] == '\0' || arg2[0] == '\0'
         || arg3[0] == '\0' )
    {
        send_to_char( "Syntax:\n\r", ch );
        send_to_char( "  string char <name> <field> <string>\n\r", ch );
        send_to_char( "    fields: name short long desc title spec\n\r", ch );
        send_to_char( "  string obj  <name> <field> <string>\n\r", ch );
        send_to_char( "    fields: name short long extended\n\r", ch );
        send_to_char( "  For stringing extended descriptions:\n\r", ch );
        send_to_char
            ( "    string obj <name> extended <keyword> <string>\n\r", ch );
        return;
    }

    if ( !str_prefix( type, "character" ) || !str_prefix( type, "mobile" ) )
    {
        if ( ( victim = get_char_world( ch, arg ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }

        /* string something */

        if ( !str_prefix( arg2, "name" ) )
        {
            if ( !IS_NPC( victim ) )
            {
                send_to_char( "Not on PC's.\n\r", ch );
                return;
            }

            free_string( &victim->name );
            victim->name = str_dup( arg3 );
            return;
        }

        if ( !str_prefix( arg2, "description" ) )
        {
            free_string( &victim->description );
            string_append( ch, &victim->description );
            return;
        }

        if ( !str_prefix( arg2, "short" ) )
        {
            free_string( &victim->short_descr );
            victim->short_descr = str_dup( arg3 );
            return;
        }

        if ( !str_prefix( arg2, "long" ) )
        {
            free_string( &victim->long_descr );
            strcat( arg3, "\n\r" );
            victim->long_descr = str_dup( arg3 );
            return;
        }

        if ( !str_prefix( arg2, "title" ) )
        {
            if ( IS_NPC( victim ) )
            {
                send_to_char( "Not on NPC's.\n\r", ch );
                return;
            }

            set_title( victim, arg3 );
            return;
        }

    }

    if ( !str_prefix( type, "object" ) )
    {
        /* string an obj */

        if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
        {
            send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
            return;
        }

        if ( !str_prefix( arg2, "name" ) )
        {
            free_string( &obj->name );
            obj->name = str_dup( arg3 );
            return;
        }

        if ( !str_prefix( arg2, "short" ) )
        {
            free_string( &obj->short_descr );
            obj->short_descr = str_dup( arg3 );
            return;
        }

        if ( !str_prefix( arg2, "long" ) )
        {
            free_string( &obj->description );
            obj->description = str_dup( arg3 );
            return;
        }

        if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended" ) )
        {
            EXTRA_DESCR_DATA *ed;

            argument = one_argument( argument, arg3 );
            if ( argument == NULL )
            {
                send_to_char
                    ( "Syntax: oset <object> ed <keyword> <string>\n\r", ch );
                return;
            }

            sprintf( buf, "%s\n\r", argument );

            if ( extra_descr_free == NULL )
            {
                ed = alloc_perm( sizeof( *ed ) );
            }
            else
            {
                ed = extra_descr_free;
                extra_descr_free = ed->next;
            }

            ed->keyword = str_dup( arg3 );
            ed->description = str_dup( buf );
            ed->next = obj->extra_descr;
            obj->extra_descr = ed;
            return;
        }
    }

    /* echo bad use message */
    do_string( ch, "" );
}

void do_oset( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char( "Syntax:\n\r", ch );
        send_to_char( "  set obj <object> <field> <value>\n\r", ch );
        send_to_char( "  Field being one of:\n\r", ch );
        send_to_char( "    value0 value1 value2 value3 value4 (v1-v4)\n\r",
                      ch );
        send_to_char( "    extra wear level weight cost timer\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
    {
        send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
        return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
        obj->value[0] = UMIN( 50, value );
        return;
    }

    if ( !str_prefix( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
        obj->value[1] = value;
        return;
    }

    if ( !str_prefix( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
        obj->value[2] = value;
        return;
    }

    if ( !str_prefix( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
        obj->value[3] = value;
        return;
    }

    if ( !str_prefix( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
    {
        obj->value[3] = value;
        return;
    }

    if ( !str_prefix( arg2, "extra" ) )
    {
        obj->extra_flags = value;
        return;
    }

    if ( !str_prefix( arg2, "wear" ) )
    {
        obj->wear_flags = value;
        return;
    }

    if ( !str_prefix( arg2, "level" ) )
    {
        obj->level = value;
        return;
    }

    if ( !str_prefix( arg2, "weight" ) )
    {
        obj->weight = value;
        return;
    }

    if ( !str_prefix( arg2, "cost" ) )
    {
        obj->cost = value;
        return;
    }

    if ( !str_prefix( arg2, "timer" ) )
    {
        obj->timer = value;
        return;
    }

    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}

void do_rset( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char( "Syntax:\n\r", ch );
        send_to_char( "  set room <location> <field> <value>\n\r", ch );
        send_to_char( "  Field being one of:\n\r", ch );
        send_to_char( "    flags sector\n\r", ch );
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
        send_to_char( "Value must be numeric.\n\r", ch );
        return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_prefix( arg2, "flags" ) )
    {
        location->room_flags = value;
        return;
    }

    if ( !str_prefix( arg2, "sector" ) )
    {
        location->sector_type = value;
        return;
    }

    /*
     * Generate usage message.
     */
    do_rset( ch, "" );
    return;
}

/* Written by Stimpy, ported to rom2.4 by Silverhand 3/12
 *
 *      Added the other COMM_ stuff that wasn't defined before 4/16 -Silverhand
 *
 * Donated to EmberMUD by Dorzak.
 */
void do_sockets( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int count;
    char *st;
    char s[100];
    char idle[10];

    count = 0;
    buf[0] = '\0';
    buf2[0] = '\0';

    strcat( buf2,
            "\n\r[Num Connected_State  Login@ Idle] Player Name Host\n\r" );
    strcat( buf2,
            "--------------------------------------------------------------------------\n\r" );
    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->character && can_see( ch, d->character ) )
        {
            /* NB: You may need to edit the CON_ values */
            switch ( d->connected )
            {
            case CON_PLAYING:
                st = "    PLAYING    ";
                break;
            case CON_GET_NAME:
                st = "   Get Name    ";
                break;
            case CON_GET_OLD_PASSWORD:
                st = "Get Old Passwd ";
                break;
            case CON_CONFIRM_NEW_NAME:
                st = " Confirm Name  ";
                break;
            case CON_GET_NEW_PASSWORD:
                st = "Get New Passwd ";
                break;
            case CON_CONFIRM_NEW_PASSWORD:
                st = "Confirm Passwd ";
                break;
            case CON_GET_NEW_RACE:
                st = "  Get New Race ";
                break;
            case CON_GET_NEW_SEX:
                st = "  Get New Sex  ";
                break;
            case CON_GET_NEW_CLASS:
                st = " Get New Class ";
                break;
            case CON_GET_ALIGNMENT:
                st = " Get New Align ";
                break;
            case CON_DEFAULT_CHOICE:
                st = " Choosing Cust ";
                break;
            case CON_GEN_GROUPS:
                st = " Customization ";
                break;
            case CON_PICK_WEAPON:
                st = " Picking Weapon";
                break;
            case CON_READ_IMOTD:
                st = " Reading IMOTD ";
                break;
            case CON_BREAK_CONNECT:
                st = "   LINKDEAD    ";
                break;
            case CON_READ_MOTD:
                st = "  Reading MOTD ";
                break;
            case CON_GET_STATS:
                st = " Rolling Stats ";
                break;
            case CON_GET_ANSI:
                st = " Getting COLOR ";
                break;
            case CON_COPYOVER_RECOVER:
                st = "HOTBOOT RECOVER";
                break;
            case CON_NOTE_TO:
                st = "   Note  To:   ";
                break;
            case CON_NOTE_SUBJECT:
                st = " Note Subject: ";
                break;
            case CON_NOTE_EXPIRE:
                st = " Note  Expire: ";
                break;
            case CON_NOTE_TEXT:
                st = " Typing Notes  ";
                break;
            case CON_NOTE_FINISH:
                st = "Finishing Note ";
                break;
            case CON_SHELL:
                st = " Crash Hat On? ";
                break;
            default:
                st = "   !UNKNOWN!   ";
                break;
            }
            count++;

            /* Format "login" value... */
            vch = d->original ? d->original : d->character;
            strftime( s, 100, "%I:%M%p", localtime( &vch->logon ) );

            if ( vch->timer > 0 )
                sprintf( idle, "%-3d", vch->timer );
            else
                sprintf( idle, "  " );

            sprintf( buf, "[%3d %s %7s %3s] %-12s %-32.32s\n\r",
                     d->descriptor,
                     st,
                     s,
                     idle,
                     ( d->original ) ? d->original->name
                     : ( d->character ) ? d->character->name
                     : "(None!)", d->host );

            strcat( buf2, buf );

        }
    }

    sprintf( buf, "\n\r%d user%s\n\r", count, count == 1 ? "" : "s" );
    strcat( buf2, buf );
    send_to_char( buf2, ch );
    return;
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA * ch, char *argument )
{
    char buf2[MAX_STRING_LENGTH];
    char arg_target[MAX_INPUT_LENGTH];
    char arg_name[MAX_INPUT_LENGTH];
    char arg_command[MAX_INPUT_LENGTH];
    enum Targets {
        mob,
        player,
        all,
        players,
        gods
    } etarget;

    argument = one_argument( argument, arg_target );

    if ( arg_target[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char
            ( "Syntax:  force [mob|char] [name] [command]\n\r         force [all|players|gods] [command]\n\r",
              ch );
        return;
    }

    if ( !str_cmp( arg_target, "mob" ) )
    {
        etarget = mob;

        argument = one_argument( argument, arg_name );

        if ( arg_name[0] == '\0' || argument[0] == '\0' )
        {
            send_to_char
                ( "Syntax:  force [mob|char] [name] [command]\n\r         force [all|players|gods] [command]\n\r",
                  ch );
            return;
        }
    }
    else if ( !str_cmp( arg_target, "char" ) )
    {
        etarget = player;

        argument = one_argument( argument, arg_name );

        if ( arg_name[0] == '\0' || argument[0] == '\0' )
        {
            send_to_char
                ( "Syntax:  force [mob|char] [name] [command]\n\r         force [all|players|gods] [command]\n\r",
                  ch );
            return;
        }
    }
    else if ( !str_cmp( arg_target, "all" ) )
        etarget = all;
    else if ( !str_cmp( arg_target, "players" ) )
        etarget = players;
    else if ( !str_cmp( arg_target, "gods" ) )
        etarget = gods;
    else
    {
        send_to_char
            ( "Syntax:  force [mob|char] [name] [command]\n\r         force [all|players|gods] [command]\n\r",
              ch );
        return;
    }

    one_argument( argument, arg_command );

    if ( !str_cmp( arg_command, "delete" ) )
    {
        send_to_char( "That will NOT be done.\n\r", ch );
        return;
    }

    if ( is_immcmd( arg_command ) )
    {
        send_to_char( "Cannot force wiz commands.\n\r", ch );
        return;
    }

    sprintf( buf2, "$n forces you to '%s'.", argument );

    if ( etarget == all )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        if ( ( get_trust( ch ) < MAX_LEVEL - 3 ) && ( ch->Class != 4 ) )
        {
            send_to_char( "Not at your level!\n\r", ch );
            return;
        }

        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !IS_NPC( vch ) && get_trust( vch ) < get_trust( ch ) )
            {
                act( buf2, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else if ( etarget == players )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        if ( get_trust( ch ) < MAX_LEVEL - 2 )
        {
            send_to_char( "Not at your level!\n\r", ch );
            return;
        }

        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !IS_NPC( vch ) && get_trust( vch ) < get_trust( ch )
                 && vch->level < LEVEL_HERO )
            {
                act( buf2, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else if ( etarget == gods )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        if ( get_trust( ch ) < MAX_LEVEL - 2 )
        {
            send_to_char( "Not at your level!\n\r", ch );
            return;
        }

        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !IS_NPC( vch ) && get_trust( vch ) < get_trust( ch )
                 && vch->level >= LEVEL_HERO )
            {
                act( buf2, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
            }
        }
    }
    else
    {
        CHAR_DATA *victim;

        if ( etarget == player )
            victim = get_player_world( ch, arg_name );
        else
            victim = get_mob_world( ch, arg_name );

        if ( !victim )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }

        if ( victim == ch )
        {
            send_to_char( "Aye aye, right away!\n\r", ch );
            return;
        }

        if ( get_trust( victim ) >= get_trust( ch ) )
        {
            send_to_char( "Do it yourself!\n\r", ch );
            return;
        }

        if ( !IS_NPC( victim ) && get_trust( ch ) < MAX_LEVEL - 3 )
        {
            send_to_char( "Not at your level!\n\r", ch );
            return;
        }

        act( buf2, ch, NULL, victim, TO_VICT );
        interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    int level;

    if ( IS_NPC( ch ) )
        return;

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
        /* take the default path */

        if ( IS_SET( ch->act, PLR_WIZINVIS ) )
        {
            REMOVE_BIT( ch->act, PLR_WIZINVIS );
            ch->invis_level = 0;
            act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You slowly fade back into existence.\n\r", ch );
        }
        else
        {
            SET_BIT( ch->act, PLR_WIZINVIS );
            ch->invis_level = get_trust( ch );
            act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You slowly vanish into thin air.\n\r", ch );
        }
    else
        /* do the level thing */
    {
        level = atoi( arg );
        if ( level < 2 || level > get_trust( ch ) )
        {
            send_to_char
                ( "Invis level must be between 2 and your level.\n\r", ch );
            return;
        }
        else
        {
            ch->reply = NULL;
            SET_BIT( ch->act, PLR_WIZINVIS );
            ch->invis_level = level;
            act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You slowly vanish into thin air.\n\r", ch );
        }
    }

    return;
}

void do_holylight( CHAR_DATA * ch, char *argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( IS_SET( ch->act, PLR_HOLYLIGHT ) )
    {
        REMOVE_BIT( ch->act, PLR_HOLYLIGHT );
        send_to_char( "Holy light mode off.\n\r", ch );
    }
    else
    {
        SET_BIT( ch->act, PLR_HOLYLIGHT );
        send_to_char( "Holy light mode on.\n\r", ch );
    }

    return;
}

void do_chaos( CHAR_DATA * ch, char *argument )
{
    extern bool chaos;

    if ( !chaos )
    {
        do_force( ch, "all save" );
        do_save( ch, "" );
    }

    chaos = !chaos;

    if ( chaos )
    {
        send_to_char( "Chaos now set.\n\r", ch );
        do_sendinfo( ch, "`rC`RH`YA`RO`rS`R has begun!" );
    }
    else
    {
        send_to_char( "Chaos cancelled.\n\r", ch );
        do_sendinfo( ch, "`rC`RH`YA`RO`rS`R has been cancelled." );
        do_force( ch, "all quit" );
    }

    return;
}

void do_rlist( CHAR_DATA * ch, char *argument )
{
    ROOM_INDEX_DATA *pRoomIndex;
    AREA_DATA *pArea;
    char *dupl;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char *buffer = NULL;
    bool found;
    int vnum;
    int avnum;
    int col = 0;

    one_argument( argument, arg );
    found = FALSE;

    if ( arg[0] == '\0' )
    {
        pArea = ch->in_room->area;

        for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
        {
            if ( ( pRoomIndex = get_room_index( vnum ) ) )
            {
                found = TRUE;
                dupl = remove_color( pRoomIndex->name );
                sprintf( buf, "`B [`K%5d`B] `w%-30.29s`w",
                         pRoomIndex->vnum, dupl );
                buffer = add_to_buf( buffer, buf );
                if ( ++col % 2 == 0 )
                    buffer = add_to_buf( buffer, "\n\r" );
            }
        }

        if ( !found )
        {
            send_to_char( "No rooms found in this area.\n\r", ch );
            return;
        }

        if ( col % 2 != 0 )
            buffer = add_to_buf( buffer, "\n\r" );

        page_to_char( buffer, ch );
        return;
    }

    else if ( is_number( argument ) )

    {
        avnum = atoi( argument );
        for ( pArea = area_first; pArea; pArea = pArea->next )
        {
            if ( pArea->vnum == avnum )
            {
                found = TRUE;
                break;
            }
        }

        if ( !found )
        {
            send_to_char( "No such area!\n\r", ch );
            return;
        }

        found = FALSE;

        for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
        {
            if ( ( pRoomIndex = get_room_index( vnum ) ) )
            {
                found = TRUE;
                sprintf( buf, "[%5d] %-32.31s`w",
                         pRoomIndex->vnum, capitalize( pRoomIndex->name ) );
                buffer = add_to_buf( buffer, buf );
                if ( ++col % 2 == 0 )
                    buffer = add_to_buf( buffer, "\n\r" );
            }
        }

        if ( col % 2 != 0 )
            buffer = add_to_buf( buffer, "\n\r" );

        page_to_char( buffer, ch );
        return;
    }
}

/*
Object checking function, by Kyle Boyd.
This cycles through a given set of vnums and identifies problems
in the objects.  Useful for quickly debugging objects in new areas.
*/
void do_objcheck( CHAR_DATA * ch, char *argument )
{
    OBJ_INDEX_DATA *obj;
    char arg[MAX_INPUT_LENGTH];
    int lvnum, hvnum, i = 0, x = 0;

    argument = one_argument( argument, arg );
    if ( argument[0] == '\0' ||
         arg[0] == '\0' || !is_number( arg ) || !is_number( argument ) )
    {
        send_to_char( "Syntax: objcheck <low vnum> <high vnum>\n\r", ch );
        return;
    }

    lvnum = atoi( arg );
    hvnum = atoi( argument );

    if ( lvnum > hvnum )
    {
        x = lvnum;
        lvnum = hvnum;
        hvnum = x;
        x = 0;
    }

    for ( x = lvnum; x < hvnum; x++ )
    {
        obj = get_obj_index( x );
        if ( obj == NULL )
            continue;
        if ( i > 20 )
            return;

        if ( obj->weight <= 0 )
        {
            i++;
            printf_to_char( ch, "[%5d] Weight = %d\n\r", obj->vnum,
                            obj->weight );
        }

        if ( obj->cost <= 0 )
        {
            i++;
            printf_to_char( ch, "[%5d] Cost = %d\n\r", obj->vnum, obj->cost );
        }

        if ( !str_cmp( obj->name, "no name" ) || obj->name[0] == '\0' )
        {
            i++;
            printf_to_char( ch, "[%5d] Keywords not set", obj->vnum );
        }

        if ( !str_cmp( obj->short_descr, "(no short description)" ) ||
             obj->short_descr[0] == '\0' )
        {
            printf_to_char( ch, "[%5d] No short description\n\r", obj->vnum );
            i++;
        }

        if ( !str_cmp( obj->description, "(no description)" ) ||
             obj->description[0] == '\0' )
        {
            printf_to_char( ch, "[%5d] No long description\n\r", obj->vnum );
            i++;
        }

        if ( !( obj->wear_flags & ITEM_TAKE )
             && obj->item_type != ITEM_FURNITURE
             && obj->item_type != ITEM_FOUNTAIN
             && obj->item_type != ITEM_CONTAINER )
        {
            printf_to_char( ch, "[%5d] No take flag!\n\r", obj->vnum );
            i++;
        }

        if ( obj->item_type == ITEM_ARMOR
             && ( obj->wear_flags == 0 || obj->wear_flags == ITEM_TAKE ) )
        {
            printf_to_char( ch, "[%5d] Unwearable armor\n\r", obj->vnum );
            i++;
        }

        if ( obj->item_type == ITEM_WEAPON
             && ( obj->wear_flags == 0 || obj->wear_flags == ITEM_TAKE ) )
        {
            printf_to_char( ch, "[%5d] Unwieldable weapon\n\r", obj->vnum );
            i++;
        }

        if ( obj->item_type == ITEM_WEAPON
             && obj->value[1] * obj->value[2] >= 150 )
        {
            printf_to_char( ch, "[%5d] Huge damage dice, way too big.\n\r",
                            obj->vnum );
            i++;
        }

    }
    return;
}

void do_aexits( CHAR_DATA * ch, char *argument )
{
    ROOM_INDEX_DATA *rid;
    AREA_DATA *area;
    EXIT_DATA *exit;
    char arg[MAX_INPUT_LENGTH];
    bool header_printed = FALSE;
    long avnum, room, door;

    argument = one_argument( argument, arg );

    /* If user specifies no area vnum, use current area */
    if ( arg[0] == '\0' )
    {
        area = ch->in_room->area;
    }
    /* Else, find the specified area */
    else
    {
        avnum = atoi( arg );

        if ( avnum == 0 )
        {
            send_to_char( "No such area!\n\r", ch );
            return;
        }

        for ( area = area_first; area != NULL; area = area->next )
        {
            if ( area->vnum == avnum )
                break;
        }
    }

    /* If area was not found */
    if ( area == NULL )
    {
        send_to_char( "No such area!\n\r", ch );
        return;
    }

    /* Loop through all room vnums in the area */
    for ( room = area->lvnum; room <= area->uvnum; room++ )
    {
        rid = get_room_index( room );

        if ( rid == NULL )
            continue;

        /* Loop through all doors in the room */
        for ( door = 0; door <= 5; door++ )
        {
            exit = rid->exit[door];

            if ( exit == NULL )
                continue;

            /* If the to_room for the exit is in a different area */
            /* then print out the information                     */
            if ( exit->u1.to_room->area != area )
            {
                if ( !header_printed )
                {
                    printf_to_char( ch,
                                    "\n\r`wExits for area: `W%s`w\n\r\n\r"
                                    "`K[ `wvnum `K] `wRoom in `W%-25.25s `K[ `wvnum `K] `wRoom in other area\n\r"
                                    "`K-------------------------------------------------------------------------------`w\n\r",
                                    area->name, area->filename );

                    header_printed = TRUE;
                }

                printf_to_char( ch,
                                "`K[`w%6d`K] `w%-25.25s %4.4s to `K[`w%6d`K] `w%-25.25s in `W%12.12s`w\n\r",
                                room, rid->name, dir_name[door],
                                exit->u1.to_room->vnum,
                                exit->u1.to_room->name,
                                exit->u1.to_room->area->filename );
            }
        }
    }

    return;
}

void do_aentrances( CHAR_DATA * ch, char *argument )
{
    ROOM_INDEX_DATA *rid;
    AREA_DATA *area, *other_area;
    EXIT_DATA *exit;
    bool header_printed = FALSE;
    char arg[MAX_INPUT_LENGTH];
    long avnum, room, door;

    argument = one_argument( argument, arg );

    /* If user specifies no area vnum, use current area */
    if ( arg[0] == '\0' )
    {
        area = ch->in_room->area;
    }
    /* Else, find the specified area */
    else
    {
        avnum = atoi( arg );

        if ( avnum == 0 )
        {
            send_to_char( "No such area!\n\r", ch );
            return;
        }

        for ( area = area_first; area != NULL; area = area->next )
        {
            if ( avnum == area->vnum )
                break;
        }
    }
    /* If area was not found */
    if ( area == NULL )
    {
        send_to_char( "No such area!\n\r", ch );
        return;
    }

    /* Loop through all areas EXCEPT area */
    for ( other_area = area_first; other_area != NULL;
          other_area = other_area->next )
    {
        if ( other_area == area )
            continue;

        /* Loop through all rooms in other_area */
        for ( room = other_area->lvnum; room <= other_area->uvnum; room++ )
        {
            rid = get_room_index( room );

            if ( rid == NULL )
                continue;

            for ( door = 0; door <= 5; door++ )
            {
                exit = rid->exit[door];

                if ( exit == NULL )
                    continue;

                /* If the to_room for the exit is in area */
                /* then print out the information         */
                if ( exit->u1.to_room->area == area )
                {
                    if ( !header_printed )
                    {
                        printf_to_char( ch,
                                        "\n\r`wEntrances to area: `W%s`w\n\r\n\r"
                                        "`K-------------------------------------------------------------------------------`w\n\r",
                                        area->name, area->filename );

                        header_printed = TRUE;
                    }

                    printf_to_char( ch,
                                    "`K[`w%6d`K] `w%5.5s from `K[`w%6d`K] `w%-25.25s in `W%12.12s`w\n\r",
                                    exit->u1.to_room->vnum, dir_name[door],
                                    rid->vnum, rid->name, rid->area->filename );
                }
            }
        }
    }

    return;
}

/* Player load and unload added by Lancelight */

void do_pload( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA *c;
    DESCRIPTOR_DATA d;
    bool isChar = FALSE;
    char name[MAX_INPUT_LENGTH];

    if ( argument[0] == '\0' )
    {
        send_to_char( "Load who?\n\r", ch );
        return;
    }

    argument[0] = UPPER( argument[0] );
    argument = one_argument( argument, name );

    /* Don't want to load a second copy of a player who's already online! */
    for ( c = player_list; c != NULL; c = c->next_player )
    {
        if ( is_exact_name( name, c->name ) )
        {
            send_to_char( "That person is already connected!\n\r", ch );
            return;
        }
    }

    isChar = load_char_obj( &d, name ); /* char pfile exists? */

    if ( !isChar )
    {
        send_to_char
            ( "Load Who? Are you sure? I can't seem to find them.\n\r", ch );
        return;
    }

    d.character->desc = NULL;
    d.character->next = char_list;
    char_list = d.character;
    d.character->pcdata->ticks = 1;
    d.character->next_player = player_list;

    player_list = d.character;
    d.connected = CON_PLAYING;
    reset_char( d.character );

    /* bring player to imm */
    if ( d.character->in_room != NULL )
    {
        if ( d.character->was_in_room == NULL )
            d.character->was_in_room = d.character->in_room;

        char_from_room( d.character );
        char_to_room( d.character, ch->in_room );
    }

    printf_to_char( ch, "You have pulled %s from the pattern!\n\r",
                    d.character->name );

    act( "$n has pulled $N from the pattern!", ch, NULL, d.character, TO_ROOM );

    if ( d.character->pet != NULL )
    {
        char_to_room( d.character->pet, d.character->in_room );
        act( "$n has entered the game.", d.character->pet, NULL, NULL,
             TO_ROOM );
    }
}

void do_punload( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA *victim;
    char who[MAX_INPUT_LENGTH];

    argument = one_argument( argument, who );

    if ( ( victim = get_player_world( ch, who ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

  /** Person is legitimatly logged on... was not ploaded.
   */
    if ( victim->desc != NULL )
    {
        send_to_char( "I don't think that would be a good idea...\n\r", ch );
        return;
    }

    if ( victim->was_in_room != NULL )  /* return player and pet to orig room */
    {
        char_from_room( victim );
        char_to_room( victim, victim->was_in_room );

        if ( victim->pet != NULL )
        {
            char_from_room( victim->pet );
            char_to_room( victim->pet, victim->was_in_room );
        }
        ch->was_in_room = NULL;
    }

    act( "You have released $N back to the Pattern.",
         ch, NULL, victim, TO_CHAR );
    act( "$n has released $N back to the Pattern.", ch, NULL, victim, TO_ROOM );

    do_quit( victim, "" );

}

void do_wizgrant( CHAR_DATA * ch, char *argument )
{
    struct cmd_type *cmd;
    IMMCMD_TYPE *tmp;
    CHAR_DATA *victim;
    bool valid = FALSE;
    char buf[MAX_INPUT_LENGTH];

    if ( argument == NULL )
    {
        send_to_char( "Syntax:\n\r"
                      "  wizgrant <name> <command>\n\r"
                      "  wizgrant <name> all\n\r"
                      "\n\r"
                      "Note: The 'all' option will grant access to the wiz-commands that YOU\n\r"
                      "      currently have access to.\n\r", ch );
        return;
    }

    argument = one_argument( argument, buf );

    /* Is argument a valid imm command? */
    for ( cmd = ( struct cmd_type * ) cmd_table; *cmd->name; cmd++ )
    {
        if ( !str_cmp( argument, cmd->name ) )
        {
            if ( !cmd->imm )
            {
                send_to_char( "That is not a valid wiz command!\n\r", ch );
                return;
            }
            else
            {
                valid = TRUE;
                break;
            }
        }
    }

    if ( str_cmp( argument, "all" ) )
    {
        if ( !valid )
        {
            send_to_char( "That is not a valid wiz command!\n\r", ch );
            return;
        }

        /* Does grantor have access to the command themselves? */
        if ( !can_do_immcmd( ch, argument ) )
        {
            send_to_char
                ( "You do not have access to grant that command!\n\r", ch );
            return;
        }
    }

    victim = get_player_world( ch, buf );

    if ( victim == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    /* Does victim already have access to the command? */
    if ( can_do_immcmd( victim, argument ) )
    {
        send_to_char( "They already have access to that command!\n\r", ch );
        return;
    }

    /* Grant access to all commands */
    if ( !str_cmp( argument, "all" ) )
    {
        IMMCMD_TYPE *orig;

        /* Copy all of ch's commands to victim */
        for ( orig = ch->pcdata->immcmdlist; orig != NULL; orig = orig->next )
        {
            if ( can_do_immcmd( victim, orig->cmd ) )
                continue;

            tmp = malloc( sizeof( IMMCMD_TYPE ) );

            tmp->cmd = str_dup( orig->cmd );
            tmp->next = victim->pcdata->immcmdlist;
            victim->pcdata->immcmdlist = tmp;

            sprintf( buf, "%s now has access to the %s command.\n\r",
                     victim->name, tmp->cmd );

            send_to_char( buf, ch );

            sprintf( buf,
                     "%s has granted you access to the %s command!\n\r",
                     ch->name, tmp->cmd );

            send_to_char( buf, victim );
        }

        return;
    }

    /* Grant access to a single command */
    tmp = malloc( sizeof( IMMCMD_TYPE ) );

    tmp->cmd = str_dup( argument );
    tmp->next = victim->pcdata->immcmdlist;
    victim->pcdata->immcmdlist = tmp;

    sprintf( buf, "%s now has access to the %s command.\n\r",
             victim->name, tmp->cmd );

    send_to_char( buf, ch );

    sprintf( buf, "%s has granted you access to the %s command!\n\r",
             ch->name, tmp->cmd );

    send_to_char( buf, victim );

    return;
}

void do_wizrevoke( CHAR_DATA * ch, char *argument )
{
    IMMCMD_TYPE *tmp, *tmp2;
    CHAR_DATA *victim;
    char buf[MAX_INPUT_LENGTH];

    if ( argument == NULL )
    {
        send_to_char( "Syntax:\n\r " "  wizrevoke <name> <command>\n\r", ch );
        return;
    }

    argument = one_argument( argument, buf );

    if ( argument == NULL )
    {
        send_to_char( "Syntax:\n\r " "  wizrevoke <name> <command>\n\r", ch );
        return;
    }

    victim = get_player_world( ch, buf );

    if ( victim == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You cannot revoke access to yourself.\n\r", ch );
        return;
    }

    if ( !can_do_immcmd( victim, argument ) )
    {
        send_to_char
            ( "You cannot revoke access to a command that they don't have access to.\n\r",
              ch );
        return;
    }

    if ( get_trust( ch ) <= get_trust( victim ) )
    {
        send_to_char
            ( "That person is too powerful for you to wizrevoke!\n\r", ch );
        return;
    }

    /* Revoke access to the command */

    /* If the command is their only command... */
    if ( victim->pcdata->immcmdlist->next == NULL )
    {
        free_string( &victim->pcdata->immcmdlist->cmd );
        free( victim->pcdata->immcmdlist );

        sprintf( buf, "%s no longer has access to the %s command.\n\r",
                 victim->name, argument );
        send_to_char( buf, ch );

        sprintf( buf, "Your access to the %s command has been revoked!\n\r",
                 argument );
        send_to_char( buf, victim );

        victim->pcdata->immcmdlist = NULL;
        return;
    }

    tmp2 = NULL;

    for ( tmp = victim->pcdata->immcmdlist; tmp != NULL; tmp = tmp->next )
    {
        if ( !str_cmp( argument, tmp->cmd ) )
        {
            if ( tmp2 == NULL )
            {
                victim->pcdata->immcmdlist = tmp->next;
                free_string( &tmp->cmd );
                free( tmp );
                break;
            }
            else
            {
                tmp2->next = tmp->next;
                free_string( &tmp->cmd );
                free( tmp );
                break;
            }
        }

        tmp2 = tmp;
    }

    sprintf( buf, "%s no longer has access to the %s command.\n\r",
             victim->name, argument );
    send_to_char( buf, ch );

    sprintf( buf, "Your access to the %s command has been revoked!\n\r",
             argument );
    send_to_char( buf, victim );

    return;
}
void do_olevel( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    char level[MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 200;

    buffer = new_buf(  );

    argument = one_argument( argument, level );
    if ( level[0] == '\0' )
    {
        send_to_char( "Syntax: olevel <level>\n\r", ch );
        send_to_char( "        olevel <level> <name>\n\r", ch );
        return;
    }

    argument = one_argument( argument, name );
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( obj->level != atoi( level ) )
            continue;

        if ( name[0] != '\0' && !is_name( name, obj->name ) )
            continue;

        found = TRUE;
        number++;

        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );

        if ( in_obj->carried_by != NULL && can_see( ch, in_obj->carried_by )
             && in_obj->carried_by->in_room != NULL )
            sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
                     number, obj->short_descr, PERS( in_obj->carried_by,
                                                     ch ),
                     in_obj->carried_by->in_room->vnum );
        else if ( in_obj->in_room != NULL
                  && can_see_room( ch, in_obj->in_room ) )
            sprintf( buf, "%3d) %s is in %s [Room %d]\n\r", number,
                     obj->short_descr, in_obj->in_room->name,
                     in_obj->in_room->vnum );
        else
            sprintf( buf, "%3d) %s is somewhere\n\r", number,
                     obj->short_descr );

        buf[0] = UPPER( buf[0] );
        add_buf( buffer, buf );

        if ( number >= max_found )
            break;
    }

    if ( !found )
        send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else
        page_to_char( buf_string( buffer ), ch );

    free_buf( buffer );
}

void do_mlevel( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    BUFFER *buffer;
    CHAR_DATA *victim;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax: mlevel <level>\n\r", ch );
        return;
    }

    found = FALSE;
    buffer = new_buf(  );
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
        if ( victim->in_room != NULL && atoi( argument ) == victim->level )
        {
            found = TRUE;
            count++;
            sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
                     IS_NPC( victim ) ? victim->pIndexData->vnum : 0,
                     IS_NPC( victim ) ? victim->short_descr : victim->name,
                     victim->in_room->vnum, victim->in_room->name );
            add_buf( buffer, buf );
        }
    }

    if ( !found )
        act( "You didn't find any mob of level $T.", ch, NULL, argument,
             TO_CHAR );
    else
        page_to_char( buf_string( buffer ), ch );

    free_buf( buffer );

    return;
}

#ifdef CFG_SHELL_ENABLED
#if defined(WIN32)
void do_shell( CHAR_DATA * ch, char *argument )
{
    send_to_char
        ( "Sorry -- shell is not currently available under Windows.\n\r", ch );
    return;
}
#else
void do_shell( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *tmpch;
    pid_t pid;
    char ptyname[12];
    char ttyname[12];
    int tmp_fds[2];
    int master_fd, slave_fd;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Not for NPCs\n\r", ch );
        return;
    }

#ifdef CFG_SHELL_LEVEL
    if ( ch->level < CFG_SHELL_LEVEL )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }
#endif

    /* Remove the char from player_list and char_list and add them to
     * shell_char_list */
    if ( ch == char_list )
    {
        char_list = ch->next;
    }
    else
    {
        for ( tmpch = char_list; tmpch && tmpch->next != ch;
              tmpch = tmpch->next );

        if ( tmpch )
        {
            tmpch->next = ch->next;
        }
        else
        {
            bug( "do_shell: current char not found in char_list!", 0 );
            return;
        }
    }

    if ( ch == player_list )
    {
        player_list = ch->next_player;
    }
    else
    {
        for ( tmpch = player_list; tmpch && tmpch->next_player != ch;
              tmpch = tmpch->next );

        if ( tmpch )
        {
            tmpch->next_player = ch->next_player;
        }
        else
        {
            bug( "do_shell: current char not found in the player_list!", 0 );
            return;
        }
    }

    ch->next_in_shell = shell_char_list;
    shell_char_list = ch;
    ch->next_player = NULL;
    ch->next = NULL;

    /* Set their connected state to CON_SHELL */
    ch->desc->connected = CON_SHELL;

    /* Create a pipe and associate it with ch so that when the
     * pipe is closed after the shell terminates, we can put them
     * back into the game. */
    if ( pipe( tmp_fds ) < 0 )
    {
        perror( "do_shell: pipe: " );
        return;
    }

    ch->fdpair[0] = tmp_fds[0];
    ch->fdpair[1] = tmp_fds[1];

    fcntl( ch->fdpair[0], F_SETFL, O_NONBLOCK );

    /* fork */
    if ( ( pid = fork(  ) ) > 0 )
    {
        /* Parent - close the "out" end of the pipe and return
         * to normal MUD business. */
        close( ch->fdpair[1] );
        fcntl( ch->fdpair[0], F_SETFL, O_NONBLOCK );
        return;
    }
    else if ( pid < 0 )
    {
        /* Error - close both ends of the pipe so that the MUD
         * will stick the player back into the game. */
        perror( "do_shell: fork:" );
        send_to_char( "system error forking shell.\n\r", ch );
        close( ch->fdpair[1] );
        close( ch->fdpair[0] );
        return;
    }

    /* Child */

    /* Close all the open descriptors that we're not using */
    close( control );

    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d == ch->desc )
            continue;
        close( d->descriptor );
    }

    /* Close the "in" end of the pipe. */
    close( ch->fdpair[0] );

    /* Open the master pty. */
    master_fd = master_pty( ptyname );

    if ( master_fd < 0 )
    {
        write_to_descriptor( ch->desc->descriptor,
                             "Failed to open pty master for shell.\n\r", 28,
                             FALSE );
        close( ch->fdpair[1] );
        return;
    }

    /* fork again */
    pid = fork(  );

    /* Child */
    if ( pid == 0 )
    {
        /* Remove the sigchld handler - let the main MUD handle that */
        struct sigaction sa;
        sa.sa_flags = SA_RESETHAND;
        sa.sa_handler = 0;
        if ( sigaction( SIGCHLD, &sa, 0 ) < 0 )
        {
            perror( "do_shell: sigaction: can't reset SIGCHLD" );
            close( ch->fdpair[1] );
            exit( 0 );
        }

        /* Open slave end of the pty. */
        slave_fd = slave_tty( ptyname, ttyname );

        if ( slave_fd < 0 )
        {
            perror( "do_shell: slave_tty:" );
            close( ch->fdpair[1] );
            exit( 0 );
        }

        /* Close the master pty. */
        close( master_fd );

        /* Close stdin, stdout, and stderr */
        close( 0 );
        close( 1 );
        close( 2 );

        /* dupage */
        if ( dup2( slave_fd, STDIN_FILENO ) != STDIN_FILENO
             || dup2( slave_fd, STDOUT_FILENO ) != STDOUT_FILENO
             || dup2( slave_fd, STDERR_FILENO ) != STDERR_FILENO )
        {
            fprintf( stderr, "do_shell: dup2 failed!\n" );
            close( ch->fdpair[1] );
            exit( 0 );
        }

        /* Close the slave tty <shrug> */
        close( slave_fd );

        /* Exec the shell!  YES! */
        execl( SHELL_PATH, "MUDSHELL", ( char * ) 0 );

        /* If we get here, something went wrong with the exec. */
        close( ch->fdpair[1] );
        exit( 0 );
    }
    else if ( pid < 0 )
    {
        /* Error */
    }

    /* Parent */

    /* Set the pty and socket to non-block */
    fcntl( master_fd, F_SETFL, O_NONBLOCK );
    fcntl( ch->desc->descriptor, F_SETFL, O_NONBLOCK );

    /* Set the socket to non-local-echo and char-by-char-mode. */
    write( ch->desc->descriptor, echo_off_str, 3 );
    write( ch->desc->descriptor, will_suppress_ga_str, 3 );

    /* Route I/O between the socket and the pty. */
    route_io( ch->desc->descriptor, master_fd );

    /* Set the socket back to local echo and line-mode. */
    write( ch->desc->descriptor, echo_on_str, 3 );
    write( ch->desc->descriptor, wont_suppress_ga_str, 3 );

    /* Close the "out" end of the pipe to notify the MUD of
     * who to put back in the game. */
    close( ch->fdpair[1] );

    /* Exit this process.  MUD will take the user over from here. */
    exit( 0 );
}
#endif
#endif
