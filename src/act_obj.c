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

#if defined(WIN32)
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN( do_split );
DECLARE_DO_FUN( do_yell );
DECLARE_DO_FUN( do_say );
DECLARE_DO_FUN( do_brew );
DECLARE_DO_FUN( do_scribe );
DECLARE_DO_FUN( do_imprint );

extern bool chaos;

/*
 * Local functions.
 */
#define CD CHAR_DATA
bool remove_obj args( ( CHAR_DATA * ch, int iWear, bool fReplace ) );
void wear_obj args( ( CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace ) );
CD *find_keeper args( ( CHAR_DATA * ch ) );
int get_cost
args( ( CHAR_DATA * ch, CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy ) );
#undef  CD

/* RT part of the corpse looting code */

bool can_loot( CHAR_DATA * ch, OBJ_DATA * obj )
{
    CHAR_DATA *owner, *wch;

    if ( IS_IMMORTAL( ch ) )
        return TRUE;

    if ( !obj->owner || obj->owner == NULL )
        return TRUE;

    owner = NULL;
    for ( wch = char_list; wch != NULL; wch = wch->next )
        if ( !str_cmp( wch->name, obj->owner ) )
            owner = wch;

    if ( owner == NULL )
        return TRUE;

    if ( !str_cmp( ch->name, owner->name ) )
        return TRUE;

    if ( !IS_NPC( owner ) && IS_SET( owner->act, PLR_CANLOOT ) )
        return TRUE;

    if ( is_same_group( ch, owner ) )
        return TRUE;

    return FALSE;
}

void get_obj( CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container )
{
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];

    if ( !CAN_WEAR( obj, ITEM_TAKE ) )
    {
        send_to_char( "You can't take that.\n\r", ch );
        return;
    }
    if ( obj->in_room != NULL )
    {
        for ( gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room )
            if ( gch->on == obj )
            {
                act( "$N appears to be using $p.", ch, obj, gch, TO_CHAR );
                return;
            }
    }
    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        act( "$d: you can't carry that many items.",
             ch, NULL, obj->name, TO_CHAR );
        return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
        act( "$d: you can't carry that much weight.",
             ch, NULL, obj->name, TO_CHAR );
        return;
    }

    if ( !can_loot( ch, obj ) )
    {
        act( "Corpse looting is not permitted.", ch, NULL, NULL, TO_CHAR );
        return;
    }

    if ( container != NULL )
    {
        if ( container->pIndexData->vnum == OBJ_VNUM_PIT
             && get_trust( ch ) < obj->level )
        {
            send_to_char( "You are not powerful enough to use it.\n\r", ch );
            return;
        }

        if ( container->pIndexData->vnum == OBJ_VNUM_PIT
             && !CAN_WEAR( container, ITEM_TAKE ) && obj->timer )
            obj->timer = 0;
        act( "You get $p from $P.", ch, obj, container, TO_CHAR );
        act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
        obj_from_obj( obj );
    }
    else
    {
        act( "You get $p.", ch, obj, container, TO_CHAR );
        act( "$n gets $p.", ch, obj, container, TO_ROOM );
        obj_from_room( obj );
    }

    if ( obj->item_type == ITEM_MONEY )
    {
        ch->gold += obj->value[0];
        if ( IS_SET( ch->act, PLR_AUTOSPLIT ) )
        {                       /* AUTOSPLIT code */
            members = 0;
            for ( gch = ch->in_room->people; gch != NULL;
                  gch = gch->next_in_room )
            {
                if ( is_same_group( gch, ch ) )
                    members++;
            }

            if ( members > 1 && obj->value[0] > 1 )
            {
                sprintf( buffer, "%d", obj->value[0] );
                do_split( ch, buffer );
            }
        }

        extract_obj( obj );
    }
    else
    {
        obj_to_char( obj, ch );
    }

    if ( !ch || !obj )
        return;
    oprog_get_trigger( ch, obj );
    return;
}

void do_get( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    bool found;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( !str_cmp( arg2, "from" ) )
        argument = one_argument( argument, arg2 );

    /* Get type. */
    if ( arg[0] == '\0' )
    {
        send_to_char( "Get what?\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
        {
            /* 'get obj' */
            obj = get_obj_list( ch, arg, ch->in_room->contents );
            if ( obj == NULL )
            {
                act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
                return;
            }

            get_obj( ch, obj, NULL );
        }
        else
        {
            /* 'get all' or 'get all.obj' */
            found = FALSE;
            for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
            {
                obj_next = obj->next_content;
                if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
                     && can_see_obj( ch, obj ) )
                {
                    found = TRUE;
                    get_obj( ch, obj, NULL );
                }
            }

            if ( !found )
            {
                if ( arg[3] == '\0' )
                    send_to_char( "I see nothing here.\n\r", ch );
                else
                    act( "I see no $T here.", ch, NULL, &arg[4], TO_CHAR );
            }
        }
    }
    else
    {
        /* 'get ... container' */
        if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }

        if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
        {
            act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
            return;
        }

        switch ( container->item_type )
        {
        default:
            send_to_char( "That's not a container.\n\r", ch );
            return;

        case ITEM_CONTAINER:
        case ITEM_CORPSE_NPC:
            break;

        case ITEM_CORPSE_PC:
            {

                if ( !can_loot( ch, container ) )
                {
                    send_to_char( "You can't do that.\n\r", ch );
                    return;
                }
            }
        }

        if ( IS_SET( container->value[1], CONT_CLOSED ) )
        {
            act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
            return;
        }

        if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
        {
            /* 'get obj container' */
            obj = get_obj_list( ch, arg, container->contains );
            if ( obj == NULL )
            {
                act( "I see nothing like that in the $T.",
                     ch, NULL, arg2, TO_CHAR );
                return;
            }
            get_obj( ch, obj, container );
        }
        else
        {
            /* 'get all container' or 'get all.obj container' */
            found = FALSE;
            for ( obj = container->contains; obj != NULL; obj = obj_next )
            {
                obj_next = obj->next_content;
                if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
                     && can_see_obj( ch, obj ) )
                {
                    found = TRUE;
                    if ( container->pIndexData->vnum == OBJ_VNUM_PIT
                         && !IS_IMMORTAL( ch ) )
                    {
                        send_to_char( "Don't be so greedy!\n\r", ch );
                        return;
                    }
                    get_obj( ch, obj, container );
                }
            }

            if ( !found )
            {
                if ( arg[3] == '\0' )
                    act( "I see nothing in the $T.", ch, NULL, arg2, TO_CHAR );
                else
                    act( "I see nothing like that in the $T.",
                         ch, NULL, arg2, TO_CHAR );
            }
        }
    }

    return;
}

void do_put( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( !str_cmp( arg2, "in" ) )
        argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Put what in what?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) )
    {
        send_to_char( "You can't do that.\n\r", ch );
        return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
    {
        act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
        return;
    }

    if ( container->item_type != ITEM_CONTAINER )
    {
        send_to_char( "That's not a container.\n\r", ch );
        return;
    }

    if ( IS_SET( container->value[1], CONT_CLOSED ) )
    {
        act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
        return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
        /* 'put obj container' */
        if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }

        if ( obj == container )
        {
            send_to_char( "You can't fold it into itself.\n\r", ch );
            return;
        }

        if ( obj->item_type == ITEM_CONTAINER )
        {
            send_to_char( "That wouldn't be a good idea.\n\r", ch );
            return;
        }

        if ( !can_drop_obj( ch, obj ) )
        {
            send_to_char( "You can't let go of it.\n\r", ch );
            return;
        }

        if ( get_obj_weight( obj ) + get_obj_weight( container )
             > container->value[0] )
        {
            send_to_char( "It won't fit.\n\r", ch );
            return;
        }

        if ( container->pIndexData->vnum == OBJ_VNUM_PIT
             && !CAN_WEAR( container, ITEM_TAKE ) )
        {
            if ( obj->timer )
            {
                send_to_char( "Only permanent items may go in the pit.\n\r",
                              ch );
                return;
            }
            else
                obj->timer = number_range( 100, 200 );
        }

        obj_from_char( obj );
        obj_to_obj( obj, container );
        act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
        act( "You put $p in $P.", ch, obj, container, TO_CHAR );
    }
    else
    {
        /* 'put all container' or 'put all.obj container' */
        for ( obj = ch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;

            if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
                 && can_see_obj( ch, obj )
                 && obj->wear_loc == WEAR_NONE
                 && obj != container
                 && can_drop_obj( ch, obj )
                 && get_obj_weight( obj ) + get_obj_weight( container )
                 <= container->value[0] )
            {
                if ( container->pIndexData->vnum == OBJ_VNUM_PIT
                     && !CAN_WEAR( obj, ITEM_TAKE ) )
                {
                    if ( obj->timer )
                        continue;
                    else
                        obj->timer = number_range( 100, 200 );
                }

                obj_from_char( obj );
                obj_to_obj( obj, container );
                act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
                act( "You put $p in $P.", ch, obj, container, TO_CHAR );
            }
        }
    }

    return;
}

void do_donate( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    ROOM_INDEX_DATA *donation, *find_location(  );
    ROOM_INDEX_DATA *was_in_room;
    char *room;
    bool found;

    argument = one_argument( argument, arg );
    room = ROOM_VNUM_DONATE;
    donation = find_location( ch, room );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Donate what?\n\r", ch );
        return;
    }
/* Donating EQ is not a good thing for someone who is in jail. Lets inform
them that they shouldnt be trying to cheat. -Lancelight*/

    if ( IS_SET( ch->act, PLR_JAILED ) && JAIL_CAN_DONATE == 0 )
    {
        send_to_char( "Donating is not allowed in jail.\n\r", ch );
        return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
        /* 'donate obj' */
        if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }

        if ( !can_drop_obj( ch, obj ) )
        {
            send_to_char( "You can't let go of it.\n\r", ch );
            return;
        }

        obj_from_char( obj );
        obj_to_room( obj, donation );
        act( "$n donates $p.", ch, obj, NULL, TO_ROOM );
        act( "You donate $p.", ch, obj, NULL, TO_CHAR );
        was_in_room = ch->in_room;
        ch->in_room = donation;
        sprintf( buf,
                 "A shimmering portal appears in midair, and through it falls\n\r$n's donation: $p" );
        act( buf, ch, obj, NULL, TO_ROOM );
        ch->in_room = was_in_room;
    }
    else
    {
        /* 'donate all' or 'donate all.obj' */
        found = FALSE;
        for ( obj = ch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;

            if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
                 && can_see_obj( ch, obj )
                 && obj->wear_loc == WEAR_NONE && can_drop_obj( ch, obj ) )
            {
                found = TRUE;
                obj_from_char( obj );
                obj_to_room( obj, donation );
                act( "$n donates $p.", ch, obj, NULL, TO_ROOM );
                act( "You donate $p.", ch, obj, NULL, TO_CHAR );
                was_in_room = ch->in_room;
                ch->in_room = donation;
                sprintf( buf,
                         "A shimmering portal appears in midair, and through it falls\n\r$n's donation: $p" );
                act( buf, ch, obj, NULL, TO_ROOM );
                ch->in_room = was_in_room;
            }
        }

        if ( !found )
        {
            if ( arg[3] == '\0' )
                act( "You are not carrying anything.", ch, NULL, arg, TO_CHAR );
            else
                act( "You are not carrying any $T.",
                     ch, NULL, &arg[4], TO_CHAR );
        }
    }

    return;
}

void do_drop( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Drop what?\n\r", ch );
        return;
    }

    if ( is_number( arg ) )
    {
        /* 'drop NNNN coins' */
        int amount;

        amount = atoi( arg );
        argument = one_argument( argument, arg );
        if ( amount <= 0
             || ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) &&
                  str_cmp( arg, "gold" ) ) )
        {
            send_to_char( "Sorry, you can't do that.\n\r", ch );
            return;
        }

        if ( ch->gold < amount )
        {
            send_to_char( "You haven't got that many coins.\n\r", ch );
            return;
        }

        ch->gold -= amount;

        for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;

            switch ( obj->pIndexData->vnum )
            {
            case OBJ_VNUM_MONEY_ONE:
                amount += 1;
                extract_obj( obj );
                break;

            case OBJ_VNUM_MONEY_SOME:
                amount += obj->value[0];
                extract_obj( obj );
                break;
            }
        }

        obj_to_room( create_money( amount ), ch->in_room );
        act( "$n drops some gold.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "OK.\n\r", ch );
        return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
        /* 'drop obj' */
        if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }

        if ( !can_drop_obj( ch, obj ) )
        {
            send_to_char( "You can't let go of it.\n\r", ch );
            return;
        }

        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
        act( "You drop $p.", ch, obj, NULL, TO_CHAR );
        oprog_drop_trigger( ch, obj );
        if ( !ch || !obj )
            return;
    }
    else
    {
        /* 'drop all' or 'drop all.obj' */
        found = FALSE;
        for ( obj = ch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;

            if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
                 && can_see_obj( ch, obj )
                 && obj->wear_loc == WEAR_NONE && can_drop_obj( ch, obj ) )
            {
                found = TRUE;
                obj_from_char( obj );
                obj_to_room( obj, ch->in_room );
                act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
                act( "You drop $p.", ch, obj, NULL, TO_CHAR );
                oprog_drop_trigger( ch, obj );
                if ( !ch || !obj )
                    return;
            }
        }

        if ( !found )
        {
            if ( arg[3] == '\0' )
                act( "You are not carrying anything.", ch, NULL, arg, TO_CHAR );
            else
                act( "You are not carrying any $T.",
                     ch, NULL, &arg[4], TO_CHAR );
        }
    }

    return;
}

void do_give( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Give what to whom?\n\r", ch );
        return;
    }

    if ( is_number( arg ) )
    {
        /* 'give NNNN coins victim' */
        int amount;

        amount = atoi( arg );
        if ( amount <= 0
             || ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) &&
                  str_cmp( arg2, "gold" ) ) )
        {
            send_to_char( "Sorry, you can't do that.\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg2 );
        if ( arg2[0] == '\0' )
        {
            send_to_char( "Give what to whom?\n\r", ch );
            return;
        }

        if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }

        if ( ch->gold < amount )
        {
            send_to_char( "You haven't got that much gold.\n\r", ch );
            return;
        }

        ch->gold -= amount;
        victim->gold += amount;
        sprintf( buf, "$n gives you %d gold.", amount );
        act( buf, ch, NULL, victim, TO_VICT );
        act( "$n gives $N some gold.", ch, NULL, victim, TO_NOTVICT );
        sprintf( buf, "You give $N %d gold.", amount );
        act( buf, ch, NULL, victim, TO_CHAR );
        mprog_bribe_trigger( victim, ch, amount );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
        send_to_char( "You must remove it first.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't let go of it.\n\r", ch );
        return;
    }

    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) )
    {
        act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( victim->carry_weight + get_obj_weight( obj ) > can_carry_w( victim ) )
    {
        act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
        act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
        return;
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );
    act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
    act( "$n gives you $p.", ch, obj, victim, TO_VICT );
    act( "You give $p to $N.", ch, obj, victim, TO_CHAR );
    mprog_give_trigger( victim, ch, obj );
    return;
}

void do_fill( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    OBJ_DATA *obj;
    OBJ_DATA *fountain;
    bool found;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Fill what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    found = FALSE;
    for ( fountain = ch->in_room->contents; fountain != NULL;
          fountain = fountain->next_content )
    {
        if ( fountain->item_type == ITEM_FOUNTAIN )
        {
            found = TRUE;
            break;
        }
    }

    if ( !found )
    {
        send_to_char( "There is no fountain here!\n\r", ch );
        return;
    }

    if ( obj->item_type != ITEM_DRINK_CON )
    {
        send_to_char( "You can't fill that.\n\r", ch );
        return;
    }

    if ( obj->value[1] != 0 && obj->value[2] != 0 )
    {
        send_to_char( "There is already another liquid in it.\n\r", ch );
        return;
    }

    if ( obj->value[1] >= obj->value[0] )
    {
        send_to_char( "Your container is full.\n\r", ch );
        return;
    }

    act( "You fill $p.", ch, obj, NULL, TO_CHAR );
    obj->value[2] = 0;
    obj->value[1] = obj->value[0];
    return;
}

void do_pour( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *out, *in;
    CHAR_DATA *vch = NULL;
    int amount;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Pour what into what?\n\r", ch );
        return;
    }

    if ( ( out = get_obj_carry( ch, arg ) ) == NULL )
    {
        send_to_char( "You don't have that item.\n\r", ch );
        return;
    }

    if ( out->item_type != ITEM_DRINK_CON )
    {
        send_to_char( "That's not a drink container.\n\r", ch );
        return;
    }

    if ( !str_cmp( argument, "out" ) )
    {
        if ( out->value[1] == 0 )
        {
            send_to_char( "It's already empty.\n\r", ch );
            return;
        }

        out->value[1] = 0;
        out->value[3] = 0;
        sprintf( buf, "You invert $p, spilling %s all over the ground.",
                 liq_table[out->value[2]].liq_name );
        act( buf, ch, out, NULL, TO_CHAR );

        sprintf( buf, "$n inverts $p, spilling %s all over the ground.",
                 liq_table[out->value[2]].liq_name );
        act( buf, ch, out, NULL, TO_ROOM );
        return;
    }

    if ( ( in = get_obj_here( ch, argument ) ) == NULL )
    {
        vch = get_char_room( ch, argument );

        if ( vch == NULL )
        {
            send_to_char( "Pour into what?\n\r", ch );
            return;
        }

        in = get_eq_char( vch, WEAR_HOLD );

        if ( in == NULL )
        {
            send_to_char( "They aren't holding anything.", ch );
            return;
        }
    }

    if ( in->item_type != ITEM_DRINK_CON )
    {
        send_to_char( "You can only pour into other drink containers.\n\r",
                      ch );
        return;
    }

    if ( in == out )
    {
        send_to_char( "You cannot change the laws of physics!\n\r", ch );
        return;
    }

    if ( in->value[1] != 0 && in->value[2] != out->value[2] )
    {
        send_to_char( "They don't hold the same liquid.\n\r", ch );
        return;
    }

    if ( out->value[1] == 0 )
    {
        act( "There's nothing in $p to pour.", ch, out, NULL, TO_CHAR );
        return;
    }

    if ( in->value[1] >= in->value[0] )
    {
        act( "$p is already filled to the top.", ch, in, NULL, TO_CHAR );
        return;
    }

    amount = UMIN( out->value[1], in->value[0] - in->value[1] );

    in->value[1] += amount;
    out->value[1] -= amount;
    in->value[2] = out->value[2];

    if ( vch == NULL )
    {
        sprintf( buf, "You pour %s from $p into $P.",
                 liq_table[out->value[2]].liq_name );
        act( buf, ch, out, in, TO_CHAR );
        sprintf( buf, "$n pours %s from $p into $P.",
                 liq_table[out->value[2]].liq_name );
        act( buf, ch, out, in, TO_ROOM );
    }
    else
    {
        sprintf( buf, "You pour some %s for $N.",
                 liq_table[out->value[2]].liq_name );
        act( buf, ch, NULL, vch, TO_CHAR );
        sprintf( buf, "$n pours you some %s.",
                 liq_table[out->value[2]].liq_name );
        act( buf, ch, NULL, vch, TO_VICT );
        sprintf( buf, "$n pours some %s for $N.",
                 liq_table[out->value[2]].liq_name );
        act( buf, ch, NULL, vch, TO_NOTVICT );

    }
}

void do_drink( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        for ( obj = ch->in_room->contents; obj; obj = obj->next_content )
        {
            if ( obj->item_type == ITEM_FOUNTAIN )
                break;
        }

        if ( obj == NULL )
        {
            send_to_char( "Drink what?\n\r", ch );
            return;
        }
    }
    else
    {
        if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
        {
            send_to_char( "You can't find it.\n\r", ch );
            return;
        }
    }

    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
    {
        send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
        return;
    }

    switch ( obj->item_type )
    {
    default:
        send_to_char( "You can't drink from that.\n\r", ch );
        break;

    case ITEM_FOUNTAIN:
        if ( !IS_NPC( ch ) )
            ch->pcdata->condition[COND_THIRST] = 48;
        act( "$n drinks from $p.", ch, obj, NULL, TO_ROOM );
        send_to_char( "You are no longer thirsty.\n\r", ch );
        break;

    case ITEM_DRINK_CON:
        if ( obj->value[1] <= 0 )
        {
            send_to_char( "It is already empty.\n\r", ch );
            return;
        }

        if ( ( liquid = obj->value[2] ) >= LIQ_MAX )
        {
            bug( "Do_drink: bad liquid number %d.", liquid );
            liquid = obj->value[2] = 0;
        }

        act( "$n drinks $T from $p.",
             ch, obj, liq_table[liquid].liq_name, TO_ROOM );
        act( "You drink $T from $p.",
             ch, obj, liq_table[liquid].liq_name, TO_CHAR );

        amount = number_range( 3, 10 );
        amount = UMIN( amount, obj->value[1] );

        gain_condition( ch, COND_DRUNK,
                        amount * liq_table[liquid].liq_affect[COND_DRUNK] );
        gain_condition( ch, COND_FULL,
                        amount * liq_table[liquid].liq_affect[COND_FULL] );
        gain_condition( ch, COND_THIRST,
                        amount * liq_table[liquid].liq_affect[COND_THIRST] );

        if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
            send_to_char( "You feel drunk.\n\r", ch );
        if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] > 40 )
            send_to_char( "You are full.\n\r", ch );
        if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_THIRST] > 40 )
            send_to_char( "You do not feel thirsty.\n\r", ch );

        if ( obj->value[3] != 0 )
        {
            /* The shit was poisoned ! */
            AFFECT_DATA af;

            act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You choke and gag.\n\r", ch );
            af.type = gsn_poison;
            af.level = number_fuzzy( amount );
            af.duration = 3 * amount;
            af.location = APPLY_NONE;
            af.modifier = 0;
            af.bitvector = AFF_POISON;
            affect_join( ch, &af );
        }

        obj->value[1] -= amount;
        break;
    }

    return;
}

void do_eat( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Eat what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if ( !IS_IMMORTAL( ch ) )
    {
        if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL )
        {
            send_to_char( "That's not edible.\n\r", ch );
            return;
        }

        if ( !IS_NPC( ch ) && obj->item_type != ITEM_PILL
             && ch->pcdata->condition[COND_FULL] > 40 )
        {
            send_to_char( "You are too full to eat more.\n\r", ch );
            return;
        }
    }

    act( "$n eats $p.", ch, obj, NULL, TO_ROOM );
    act( "You eat $p.", ch, obj, NULL, TO_CHAR );

    switch ( obj->item_type )
    {

    case ITEM_FOOD:
        if ( !IS_NPC( ch ) )
        {
            int condition;

            condition = ch->pcdata->condition[COND_FULL];
            gain_condition( ch, COND_FULL, obj->value[0] );
            if ( condition == 0 && ch->pcdata->condition[COND_FULL] > 0 )
                send_to_char( "You are no longer hungry.\n\r", ch );
            else if ( ch->pcdata->condition[COND_FULL] > 40 )
                send_to_char( "You are full.\n\r", ch );
        }

        if ( obj->value[3] != 0 )
        {
            /* The shit was poisoned! */
            AFFECT_DATA af;

            act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
            send_to_char( "You choke and gag.\n\r", ch );

            af.type = gsn_poison;
            af.level = number_fuzzy( obj->value[0] );
            af.duration = 2 * obj->value[0];
            af.location = APPLY_NONE;
            af.modifier = 0;
            af.bitvector = AFF_POISON;
            affect_join( ch, &af );
        }
        break;

    case ITEM_PILL:
        obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
        obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
        obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
        break;
    }

    extract_obj( obj );
    return;
}

/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA * ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
        return TRUE;

    if ( !fReplace )
        return FALSE;

    if ( IS_SET( obj->extra_flags, ITEM_NOREMOVE ) )
    {
        act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
        return FALSE;
    }

    unequip_char( ch, obj );
    act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
    act( "You stop using $p.", ch, obj, NULL, TO_CHAR );
    oprog_remove_trigger( ch, obj );
    return TRUE;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->level < obj->level )
    {
        sprintf( buf, "You must be level %d to use this object.\n\r",
                 obj->level );
        send_to_char( buf, ch );
        act( "$n tries to use $p, but is too inexperienced.",
             ch, obj, NULL, TO_ROOM );
        return;
    }

    if ( obj->item_type == ITEM_LIGHT )
    {
        if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
            return;
        if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
        {
            act( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
            act( "You light $p and hold it.", ch, obj, NULL, TO_CHAR );
        }
        equip_char( ch, obj, WEAR_LIGHT );
        oprog_wear_trigger( ch, obj );

        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
        if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
             && get_eq_char( ch, WEAR_FINGER_R ) != NULL
             && !remove_obj( ch, WEAR_FINGER_L, fReplace )
             && !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
            return;

        if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
        {
            if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
            {
                act( "$n wears $p on $s left finger.", ch, obj, NULL, TO_ROOM );
                act( "You wear $p on your left finger.", ch, obj, NULL,
                     TO_CHAR );
            }
            equip_char( ch, obj, WEAR_FINGER_L );
            oprog_wear_trigger( ch, obj );
            return;
        }

        if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
        {
            if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
            {
                act( "$n wears $p on $s right finger.", ch, obj, NULL,
                     TO_ROOM );
                act( "You wear $p on your right finger.", ch, obj, NULL,
                     TO_CHAR );
            }
            equip_char( ch, obj, WEAR_FINGER_R );
            oprog_wear_trigger( ch, obj );
            return;
        }

        bug( "Wear_obj: no free finger.", 0 );
        send_to_char( "You already wear two rings.\n\r", ch );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
        if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
             && get_eq_char( ch, WEAR_NECK_2 ) != NULL
             && !remove_obj( ch, WEAR_NECK_1, fReplace )
             && !remove_obj( ch, WEAR_NECK_2, fReplace ) )
            return;

        if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
        {
            if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
            {
                act( "$n wears $p around $s neck.", ch, obj, NULL, TO_ROOM );
                act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
            }
            equip_char( ch, obj, WEAR_NECK_1 );
            oprog_wear_trigger( ch, obj );
            return;
        }

        if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
        {
            if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
            {
                act( "$n wears $p around $s neck.", ch, obj, NULL, TO_ROOM );
                act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
            }
            equip_char( ch, obj, WEAR_NECK_2 );
            oprog_wear_trigger( ch, obj );
            return;
        }

        bug( "Wear_obj: no free neck.", 0 );
        send_to_char( "You already wear two neck items.\n\r", ch );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
        if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
            return;
        if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
        {
            act( "$n wears $p on $s body.", ch, obj, NULL, TO_ROOM );
            act( "You wear $p on your body.", ch, obj, NULL, TO_CHAR );
        }
        equip_char( ch, obj, WEAR_BODY );
        oprog_wear_trigger( ch, obj );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
        if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
            return;
        if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
        {
            act( "$n wears $p on $s head.", ch, obj, NULL, TO_ROOM );
            act( "You wear $p on your head.", ch, obj, NULL, TO_CHAR );
        }
        equip_char( ch, obj, WEAR_HEAD );
        oprog_wear_trigger( ch, obj );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
        if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
            return;
        if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
        {
            act( "$n wears $p on $s legs.", ch, obj, NULL, TO_ROOM );
            act( "You wear $p on your legs.", ch, obj, NULL, TO_CHAR );
        }
        equip_char( ch, obj, WEAR_LEGS );
        oprog_wear_trigger( ch, obj );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
        if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
            return;
        if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
        {
            act( "$n wears $p on $s feet.", ch, obj, NULL, TO_ROOM );
            act( "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
        }
        equip_char( ch, obj, WEAR_FEET );
        oprog_wear_trigger( ch, obj );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
        if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
            return;
        if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
        {
            act( "$n wears $p on $s hands.", ch, obj, NULL, TO_ROOM );
            act( "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
        }
        equip_char( ch, obj, WEAR_HANDS );
        oprog_wear_trigger( ch, obj );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
        if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
            return;
        if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
        {
            act( "$n wears $p on $s arms.", ch, obj, NULL, TO_ROOM );
            act( "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
        }
        equip_char( ch, obj, WEAR_ARMS );
        oprog_wear_trigger( ch, obj );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
        if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
            return;
        if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
        {
            act( "$n wears $p about $s body.", ch, obj, NULL, TO_ROOM );
            act( "You wear $p about your body.", ch, obj, NULL, TO_CHAR );
        }
        equip_char( ch, obj, WEAR_ABOUT );
        oprog_wear_trigger( ch, obj );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
        if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
            return;
        if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
        {
            act( "$n wears $p about $s waist.", ch, obj, NULL, TO_ROOM );
            act( "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
        }
        equip_char( ch, obj, WEAR_WAIST );
        oprog_wear_trigger( ch, obj );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
        if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
             && get_eq_char( ch, WEAR_WRIST_R ) != NULL
             && !remove_obj( ch, WEAR_WRIST_L, fReplace )
             && !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
            return;

        if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
        {
            if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
            {
                act( "$n wears $p around $s left wrist.",
                     ch, obj, NULL, TO_ROOM );
                act( "You wear $p around your left wrist.",
                     ch, obj, NULL, TO_CHAR );
            }
            equip_char( ch, obj, WEAR_WRIST_L );
            oprog_wear_trigger( ch, obj );
            return;
        }

        if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
        {
            if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
            {
                act( "$n wears $p around $s right wrist.",
                     ch, obj, NULL, TO_ROOM );
                act( "You wear $p around your right wrist.",
                     ch, obj, NULL, TO_CHAR );
            }
            equip_char( ch, obj, WEAR_WRIST_R );
            oprog_wear_trigger( ch, obj );
            return;
        }

        bug( "Wear_obj: no free wrist.", 0 );
        send_to_char( "You already wear two wrist items.\n\r", ch );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
        OBJ_DATA *weapon;
        OBJ_DATA *second_weapon;

        if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
            return;

        weapon = get_eq_char( ch, WEAR_WIELD );
        second_weapon = get_eq_char( ch, WEAR_SECOND_WIELD );
        if ( ( ( weapon != NULL ) && ( ch->size < SIZE_LARGE )
               && IS_WEAPON_STAT( weapon, WEAPON_TWO_HANDS ) )
             || ( weapon != NULL && ( second_weapon != NULL
                                      || get_eq_char( ch,
                                                      WEAR_HOLD ) != NULL ) ) )
        {
            send_to_char( "Your hands are tied up with your weapon!\n\r", ch );
            return;
        }

        if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
        {
            act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
            act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
        }
        equip_char( ch, obj, WEAR_SHIELD );
        oprog_wear_trigger( ch, obj );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
        OBJ_DATA *weapon;
        OBJ_DATA *second_weapon;
        int sn = 0, skill;
        weapon = get_eq_char( ch, WEAR_WIELD );
        second_weapon = get_eq_char( ch, WEAR_SECOND_WIELD );

        if ( !remove_obj( ch, WEAR_SECOND_WIELD, fReplace ) )
            return;

        if ( !IS_NPC( ch )
             && get_obj_weight( obj ) >
             str_app[get_curr_stat( ch, STAT_STR )].wield )
        {
            send_to_char( "It is too heavy for you to wield.\n\r", ch );
            return;
        }

        if ( ( !IS_NPC( ch ) && ch->size < SIZE_LARGE
               && IS_WEAPON_STAT( obj, WEAPON_TWO_HANDS )
               && ( ( get_eq_char( ch, WEAR_SECOND_WIELD ) != NULL
                      || get_eq_char( ch, WEAR_SHIELD ) != NULL )
                    || get_eq_char( ch, WEAR_WIELD ) != NULL ) )
             || ( get_eq_char( ch, WEAR_WIELD ) != NULL
                  && ( get_eq_char( ch, WEAR_SHIELD ) != NULL
                       || get_eq_char( ch, WEAR_SECOND_WIELD ) != NULL
                       || get_eq_char( ch, WEAR_HOLD ) != NULL ) )
             || ( get_eq_char( ch, WEAR_SHIELD ) != NULL
                  && get_eq_char( ch, WEAR_HOLD ) != NULL ) )
        {
            send_to_char( "You'd need another free hand to do that.\n\r", ch );
            if ( get_eq_char( ch, WEAR_WIELD ) != NULL )
                if ( !remove_obj( ch, WEAR_SECOND_WIELD, fReplace ) )
                    return;
            return;
        }
        if ( ( weapon != NULL ) && ( ch->size < SIZE_LARGE )
             && IS_WEAPON_STAT( weapon, WEAPON_TWO_HANDS ) )
        {
            send_to_char( "You'd need another free hand to do that.\n\r", ch );
            return;
        }

        if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
        {
            act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
            act( "You wield $p.", ch, obj, NULL, TO_CHAR );
        }

        if ( get_eq_char( ch, WEAR_WIELD ) == NULL )
        {
            equip_char( ch, obj, WEAR_WIELD );
            sn = get_weapon_sn( ch );
        }
        else if ( get_eq_char( ch, WEAR_SECOND_WIELD ) == NULL )
        {
            equip_char( ch, obj, WEAR_SECOND_WIELD );
            sn = get_second_weapon_sn( ch );
        }

        if ( get_eq_char( ch, WEAR_WIELD ) != NULL )
            oprog_wear_trigger( ch, obj );

        if ( sn == gsn_hand_to_hand )
        {
            act( "You don't even know which end is up on $p.", ch, obj, NULL,
                 TO_CHAR );
            return;
        }

        skill = get_weapon_skill( ch, sn );

        if ( skill >= 100 )
            act( "$p feels like a part of you!", ch, obj, NULL, TO_CHAR );
        else if ( skill > 85 )
            act( "You feel quite confident with $p.", ch, obj, NULL, TO_CHAR );
        else if ( skill > 70 )
            act( "You are skilled with $p.", ch, obj, NULL, TO_CHAR );
        else if ( skill > 50 )
            act( "Your skill with $p is adequate.", ch, obj, NULL, TO_CHAR );
        else if ( skill > 25 )
            act( "$p feels a little clumsy in your hands.", ch, obj, NULL,
                 TO_CHAR );
        else if ( skill > 1 )
            act( "You fumble and almost drop $p.", ch, obj, NULL, TO_CHAR );
        else
            act( "You don't even know which end is up on $p.",
                 ch, obj, NULL, TO_CHAR );

        return;
    }

    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
        if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
            return;
        if ( get_eq_char( ch, WEAR_WIELD ) != NULL
             && ( get_eq_char( ch, WEAR_SECOND_WIELD ) != NULL
                  || get_eq_char( ch, WEAR_SHIELD ) != NULL ) )
        {
            send_to_char( "You need one hand free to hold that.\n\r", ch );
            return;
        }
        if ( !oprog_use_trigger( ch, obj, NULL, NULL ) )
        {
            act( "$n holds $p in $s hands.", ch, obj, NULL, TO_ROOM );
            act( "You hold $p in your hands.", ch, obj, NULL, TO_CHAR );
        }
        equip_char( ch, obj, WEAR_HOLD );
        oprog_wear_trigger( ch, obj );
        return;
    }

    if ( fReplace )
        send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

    return;
}

void do_wear( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Wear, wield, or hold what?\n\r", ch );
        return;
    }

/* This check is more for RP'ish reasons, but I like the idea so its not
commented out. If you dont want this,just commented it out. -Lancelight */
    if ( IS_SET( ch->act, PLR_JAILED ) && JAIL_CAN_WEAR == 0 )
    {
        send_to_char( "You will NOT wear civilian clothes while in jail!", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        OBJ_DATA *obj_next;

        for ( obj = ch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;
            if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
                wear_obj( ch, obj, FALSE );
        }
        return;
    }
    else
    {
        if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }

        wear_obj( ch, obj, TRUE );
    }

    return;
}

void do_remove( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *wield;
    bool found;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Remove what?\n\r", ch );
        return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
        /* 'rem obj' */
        if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
        {
            send_to_char( "You aren't wearing that item.\n\r", ch );
            return;
        }
        if ( get_eq_char( ch, WEAR_WIELD ) == obj
             && get_eq_char( ch, WEAR_SECOND_WIELD ) != NULL )
        {
            remove_obj( ch, obj->wear_loc, TRUE );
            wield = get_eq_char( ch, WEAR_SECOND_WIELD );
            wield->wear_loc = WEAR_WIELD;
        }
        else
            remove_obj( ch, obj->wear_loc, TRUE );
    }
    else
    {
        /* 'rem all' or 'rem all.obj' */
        found = FALSE;
        for ( obj = ch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;

            if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
                 && can_see_obj( ch, obj ) && obj->wear_loc != WEAR_NONE )
            {
                found = TRUE;
                remove_obj( ch, obj->wear_loc, TRUE );
            }
        }

        if ( !found )
        {
            if ( arg[3] == '\0' )
                act( "You are not wearing anything.", ch, NULL, arg, TO_CHAR );
            else
                act( "You are not wearing any $T.",
                     ch, NULL, &arg[4], TO_CHAR );
        }
    }

    return;
}

void do_sacrifice( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int gold;
    OBJ_DATA *obj;
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
        act( "$n offers $mself to the gods, but they graciously decline.",
             ch, NULL, NULL, TO_ROOM );
        send_to_char
            ( "The gods appreciate your offer and may accept it later.\n\r",
              ch );
        return;
    }
    obj = get_obj_list( ch, arg, ch->in_room->contents );
    if ( obj == NULL )
    {
        send_to_char( "You can't find it.\n\r", ch );
        return;
    }
    if ( obj->in_room != NULL )
    {
        for ( gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room )
            if ( gch->on == obj )
            {
                act( "$N appears to be using $p.", ch, obj, gch, TO_CHAR );
                return;
            }
    }

/* Again, this is sorta for RP, but it can also stop people from getting
rit of items they know that they arent suppose to have while in jail.
-Lancelight */

    if ( IS_SET( ch->act, PLR_JAILED ) )
    {
        send_to_char( "You will NOT get rid of any evidance!\n\r", ch );
        return;
    }

    if ( obj->item_type == ITEM_CORPSE_PC )
    {
        if ( obj->contains )
        {
            send_to_char( "The gods wouldn't like that.\n\r", ch );
            return;
        }
    }

    if ( !CAN_WEAR( obj, ITEM_TAKE ) )
    {
        act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
        return;
    }

    gold = UMAX( 1, obj->level * 2 );

    if ( obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC )
        gold = UMIN( gold, obj->cost );

    if ( gold == 1 )
        send_to_char( "The gods give you one gold coin for your sacrifice.\n\r",
                      ch );
    else
    {
        sprintf( buf, "The gods give you %d gold coins for your sacrifice.\n\r",
                 gold );
        send_to_char( buf, ch );
    }

    ch->gold += gold;

    if ( IS_SET( ch->act, PLR_AUTOSPLIT ) )
    {                           /* AUTOSPLIT code */
        members = 0;
        for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
        {
            if ( is_same_group( gch, ch ) )
                members++;
        }

        if ( members > 1 && gold > 1 )
        {
            sprintf( buffer, "%d", gold );
            do_split( ch, buffer );
        }
    }

    act( "$n sacrifices $p to the gods.", ch, obj, NULL, TO_ROOM );
    oprog_sac_trigger( ch, obj );
    extract_obj( obj );
    return;
}

void do_quaff( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Quaff what?\n\r", ch );
        return;
    }

/* I suggest leaving this check in place, just incase your mud has potions
that can damage the player. We dont want them breaking outa jail now do we
;) -Lancelight */

    if ( IS_SET( ch->act, PLR_JAILED ) )
    {
        send_to_char( "No need to quaff while in jail.\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
        send_to_char( "You do not have that potion.\n\r", ch );
        return;
    }

    if ( obj->item_type != ITEM_POTION )
    {
        send_to_char( "You can quaff only potions.\n\r", ch );
        return;
    }

    if ( ch->level < obj->level )
    {
        send_to_char( "This liquid is too powerful for you to drink.\n\r", ch );
        return;
    }

    act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
    act( "You quaff $p.", ch, obj, NULL, TO_CHAR );

    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );

    extract_obj( obj );
    return;
}

void do_recite( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *scroll;
    OBJ_DATA *obj;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( ( scroll = get_obj_carry( ch, arg ) ) == NULL )
    {
        send_to_char( "You do not have that scroll.\n\r", ch );
        return;
    }

/* Again, leave this in place unless u want jail breakers running around.
-Lancelight */
    if ( IS_SET( ch->act, PLR_JAILED ) )
    {
        send_to_char( "No need to recite anything while in jail.\n\r", ch );
        return;
    }

    if ( scroll->item_type != ITEM_SCROLL )
    {
        send_to_char( "You can recite only scrolls.\n\r", ch );
        return;
    }

    if ( ch->level < scroll->level )
    {
        send_to_char( "This scroll is too complex for you to comprehend.\n\r",
                      ch );
        return;
    }

    obj = NULL;
    if ( arg2[0] == '\0' )
    {
        victim = ch;
    }
    else
    {
        if ( ( victim = get_char_room( ch, arg2 ) ) == NULL
             && ( obj = get_obj_here( ch, arg2 ) ) == NULL )
        {
            send_to_char( "You can't find it.\n\r", ch );
            return;
        }
    }

    act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
    act( "You recite $p.", ch, scroll, NULL, TO_CHAR );

    if ( number_percent(  ) >= 20 + get_skill( ch, gsn_scrolls ) * 4 / 5 )
    {
        send_to_char( "You mispronounce a syllable.\n\r", ch );
        check_improve( ch, gsn_scrolls, FALSE, 2 );
    }

    else
    {
        obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
        obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
        obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
        check_improve( ch, gsn_scrolls, TRUE, 2 );
    }

    extract_obj( scroll );
    return;
}

void do_brandish( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    int sn;

    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
        send_to_char( "You hold nothing in your hand.\n\r", ch );
        return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
        send_to_char( "You can brandish only with a staff.\n\r", ch );
        return;
    }

/* Leave thgis in place unless u want jail breakers. -Lancelight */
    if ( IS_SET( ch->act, PLR_JAILED ) )
    {
        send_to_char( "No staves allowed in jail.\n\r", ch );
        return;
    }

    if ( ( sn = staff->value[3] ) < 0
         || sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
        bug( "Do_brandish: bad sn %d.", sn );
        return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
        if ( !oprog_use_trigger( ch, staff, NULL, NULL ) )
        {
            act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
            act( "You brandish $p.", ch, staff, NULL, TO_CHAR );
        }
        if ( ch->level < staff->level
             || number_percent(  ) >= 20 + get_skill( ch, gsn_staves ) * 4 / 5 )
        {
            act( "You fail to invoke $p.", ch, staff, NULL, TO_CHAR );
            act( "...and nothing happens.", ch, NULL, NULL, TO_ROOM );
            check_improve( ch, gsn_staves, FALSE, 2 );
        }

        else
            for ( vch = ch->in_room->people; vch; vch = vch_next )
            {
                vch_next = vch->next_in_room;

                switch ( skill_table[sn].target )
                {
                default:
                    bug( "Do_brandish: bad target for sn %d.", sn );
                    return;

                case TAR_IGNORE:
                    if ( vch != ch )
                        continue;
                    break;

                case TAR_CHAR_OFFENSIVE:
                    if ( IS_NPC( ch ) ? IS_NPC( vch ) : !IS_NPC( vch ) )
                        continue;
                    break;

                case TAR_CHAR_DEFENSIVE:
                    if ( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) )
                        continue;
                    break;

                case TAR_CHAR_SELF:
                    if ( vch != ch )
                        continue;
                    break;
                }

                obj_cast_spell( staff->value[3], staff->value[0], ch, vch,
                                NULL );
                check_improve( ch, gsn_staves, TRUE, 2 );
            }
    }

    if ( --staff->value[2] <= 0 )
    {
        act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
        act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
        extract_obj( staff );
    }

    return;
}

void do_zap( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;

    one_argument( argument, arg );
    if ( arg[0] == '\0' && ch->fighting == NULL )
    {
        send_to_char( "Zap whom or what?\n\r", ch );
        return;
    }

    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
        send_to_char( "You hold nothing in your hand.\n\r", ch );
        return;
    }

    /* Jail breakers? HA I think not. -Lancelight */
    if ( IS_SET( ch->act, PLR_JAILED ) )
    {
        send_to_char( "No using wands in jail bubz.\n\r", ch );
        return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
        send_to_char( "You can zap only with a wand.\n\r", ch );
        return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
        if ( ch->fighting != NULL )
        {
            victim = ch->fighting;
        }
        else
        {
            send_to_char( "Zap whom or what?\n\r", ch );
            return;
        }
    }
    else
    {
        if ( ( victim = get_char_room( ch, arg ) ) == NULL
             && ( obj = get_obj_here( ch, arg ) ) == NULL )
        {
            send_to_char( "You can't find it.\n\r", ch );
            return;
        }
    }

    if ( victim != NULL && !IS_NPC( victim ) && !IS_NPC( ch ) ) /*make sure its a char first */
    {
        if ( !chaos )
        {
            if ( !IS_SET( victim->act, PLR_KILLER )
                 || !IS_SET( ch->act, PLR_KILLER ) )
            {
                send_to_char( "You can only kill other player killers.\n\r",
                              ch );
                return;
            }
        }
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( wand->value[2] > 0 )
    {
        if ( victim != NULL )
        {
            if ( !oprog_use_trigger( ch, wand, NULL, NULL ) )
            {
                act( "$n zaps $N with $p.", ch, wand, victim, TO_ROOM );
                act( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
            }
        }
        else
        {
            if ( !oprog_use_trigger( ch, wand, NULL, NULL ) )
            {
                act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
                act( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
            }
        }

        if ( ch->level < wand->level
             || number_percent(  ) >= 20 + get_skill( ch, gsn_wands ) * 4 / 5 )
        {
            act( "Your efforts with $p produce only smoke and sparks.",
                 ch, wand, NULL, TO_CHAR );
            act( "$n's efforts with $p produce only smoke and sparks.",
                 ch, wand, NULL, TO_ROOM );
            check_improve( ch, gsn_wands, FALSE, 2 );
        }
        else
        {
            obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
            check_improve( ch, gsn_wands, TRUE, 2 );
        }
    }

    if ( --wand->value[2] <= 0 )
    {
        act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
        act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
        extract_obj( wand );
    }

    return;
}

void do_steal( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    int percent;

    char *name;

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Steal what from whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "That's pointless.\n\r", ch );
        return;
    }

    if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_NO_STEAL ) )
    {
        send_to_char( "Oops.\n\r", ch );

        name = IS_NPC( ch ) ? ch->short_descr : ch->name;

        switch ( number_range( 0, 3 ) )
        {
        case 0:
            sprintf( buf, "%s is a lousy thief!", name );
            break;
        case 1:
            sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
                     name, ( ch->sex == 2 ) ? "her" : "his" );
            break;
        case 2:
            sprintf( buf, "%s tried to rob me!", name );
            break;
        case 3:
            sprintf( buf, "Keep your hands out of there, %s!", name );
            break;
        }

        do_yell( victim, buf );
        log_string( buf );

        if ( !IS_SET( ch->act, PLR_THIEF ) )
        {
            SET_BIT( ch->act, PLR_THIEF );
            send_to_char( "*** You are now a THIEF!! ***\n\r", ch );
            save_char_obj( ch );
        }

        return;
    }

#ifdef SHOPKEEPERS_NOSTEAL
    if ( IS_NPC( victim ) && ( victim->pIndexData->pShop != NULL ) )
    {
        send_to_char( "The shopkeeper is wary of thieves.\n\r", ch );
        name = IS_NPC( ch ) ? ch->short_descr : ch->name;

        switch ( number_range( 0, 3 ) )
        {
        case 0:
            sprintf( buf, "%s is a lousy thief!", name );
            break;
        case 1:
            sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
                     name, ( ch->sex == 2 ) ? "her" : "his" );
            break;
        case 2:
            sprintf( buf, "%s tried to rob me!", name );
            break;
        case 3:
            sprintf( buf, "Keep your hands out of there, %s!", name );
            break;
        }

        do_yell( victim, buf );
        log_string( buf );

        if ( !IS_SET( ch->act, PLR_THIEF ) )
        {
            SET_BIT( ch->act, PLR_THIEF );
            send_to_char( "*** You are now a THIEF!! ***\n\r", ch );
            save_char_obj( ch );
        }

        return;
    }
#endif

    if ( is_safe( ch, victim ) )
        return;

    if ( victim->position == POS_FIGHTING )
    {
        send_to_char( "You'd better not -- you might get hit.\n\r", ch );
        return;
    }

    WAIT_STATE( ch, skill_table[gsn_steal].beats );
    percent = number_percent(  ) + ( IS_AWAKE( victim ) ? 10 : -50 );

    if ( ch->level + 5 < victim->level || victim->position == POS_FIGHTING
#ifdef NO_PLAYER_STEALING
         || ( !IS_NPC( ch ) && !IS_NPC( victim ) )
#endif
         || ( IS_NPC( ch ) && !IS_NPC( victim )
              && percent > ( victim->level - ch->level ) * 20 )
         || ( !IS_NPC( ch ) && percent > ch->pcdata->learned[gsn_steal] ) )
    {
        /*
         * Failure.
         */
        send_to_char( "Oops.\n\r", ch );
        act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT );
        act( "$n tried to steal from $N.\n\r", ch, NULL, victim, TO_NOTVICT );

        name = IS_NPC( ch ) ? ch->short_descr : ch->name;

        switch ( number_range( 0, 3 ) )
        {
        case 0:
            sprintf( buf, "%s is a lousy thief!", name );
            break;
        case 1:
            sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
                     name, ( ch->sex == 2 ) ? "her" : "his" );
            break;
        case 2:
            sprintf( buf, "%s tried to rob me!", name );
            break;
        case 3:
            sprintf( buf, "Keep your hands out of there, %s!", name );
            break;
        }

        do_yell( victim, buf );

        if ( !IS_NPC( ch ) )
        {
            if ( IS_NPC( victim ) )
            {
                check_improve( ch, gsn_steal, FALSE, 2 );
                multi_hit( victim, ch, TYPE_UNDEFINED );
            }
            if ( IS_SET( victim->act, PLR_KILLER ) )
            {
                check_improve( ch, gsn_steal, FALSE, 2 );
                multi_hit( victim, ch, TYPE_UNDEFINED );
            }

            else
            {
                log_string( buf );
                if ( !IS_SET( ch->act, PLR_THIEF ) )
                {
                    SET_BIT( ch->act, PLR_THIEF );
                    send_to_char( "*** You are now a THIEF!! ***\n\r", ch );
                    save_char_obj( ch );
                }
            }
        }

        return;
    }

    if ( !str_cmp( arg, "coin" )
         || !str_cmp( arg, "coins" ) || !str_cmp( arg, "gold" ) )
    {
        int amount;

        amount = victim->gold * number_range( 1, 10 ) / 100;
        if ( amount <= 0 )
        {
            send_to_char( "You couldn't get any gold.\n\r", ch );
            return;
        }

        ch->gold += amount;
        victim->gold -= amount;
        sprintf( buf, "Bingo!  You got %d gold coins.\n\r", amount );
        send_to_char( buf, ch );
        check_improve( ch, gsn_steal, TRUE, 2 );
        return;
    }

    if ( ( obj = get_obj_carry( victim, arg ) ) == NULL )
    {
        send_to_char( "You can't find it.\n\r", ch );
        return;
    }

    if ( !can_drop_obj( ch, obj )
         || IS_SET( obj->extra_flags, ITEM_INVENTORY )
         || obj->level > ch->level )
    {
        send_to_char( "You can't pry it away.\n\r", ch );
        return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
        send_to_char( "You have your hands full.\n\r", ch );
        return;
    }

    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
    {
        send_to_char( "You can't carry that much weight.\n\r", ch );
        return;
    }

    obj_from_char( obj );
    obj_to_char( obj, ch );
    check_improve( ch, gsn_steal, TRUE, 2 );
    return;
}

/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA * ch )
{
    char buf[MAX_STRING_LENGTH];

    CHAR_DATA *keeper;
    SHOP_DATA *pShop;

    pShop = NULL;
    for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room )
    {
        if ( IS_NPC( keeper ) && ( pShop = keeper->pIndexData->pShop ) != NULL )
            break;
    }

    if ( pShop == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return NULL;
    }

    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_THIEF ) )
    {
        do_say( keeper, "Thieves are not welcome!" );
        sprintf( buf, "%s the THIEF is over here!\n\r", ch->name );
        do_yell( keeper, buf );
        return NULL;
    }

    /*
     * Shop hours.
     */
    if ( time_info.hour < pShop->open_hour )
    {
        do_say( keeper, "Sorry, I am closed. Come back later." );
        return NULL;
    }

    if ( time_info.hour > pShop->close_hour )
    {
        do_say( keeper, "Sorry, I am closed. Come back tomorrow." );
        return NULL;
    }

    /*
     * Invisible or hidden people.
     */
    if ( !can_see( keeper, ch ) )
    {
        do_say( keeper, "I don't trade with folks I can't see." );
        return NULL;
    }

    return keeper;
}

int get_cost( CHAR_DATA * ch, CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy )
{
    SHOP_DATA *pShop;
    int cost;

    if ( obj == NULL || ( pShop = keeper->pIndexData->pShop ) == NULL )
        return 0;

    if ( fBuy )
    {
        cost =
            ( int ) ( obj->cost * pShop->profit_buy *
                      faction_cost_multiplier( ch, keeper, TRUE ) / 100 );
    }
    else
    {
        OBJ_DATA *obj2;
        int itype;

        cost = 0;
        for ( itype = 0; itype < MAX_TRADE; itype++ )
        {
            if ( obj->item_type == pShop->buy_type[itype] )
            {
                cost =
                    ( int ) ( obj->cost * pShop->profit_sell *
                              faction_cost_multiplier( ch, keeper,
                                                       FALSE ) / 100 );
                break;
            }
        }

        for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content )
        {
            if ( obj->pIndexData == obj2->pIndexData )
            {
                cost = ( int ) ( cost * MORE_COST_MULTIPLIER );
                break;
            }
        }
    }

    if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND )
    {
        if ( obj->value[1] == 0 )
            cost = cost / 4;
        else
            cost = cost * obj->value[2] / obj->value[1];
    }
    return cost;
}

void do_buy( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int cost, roll;
    bool fHaggle = FALSE;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Buy what?\n\r", ch );
        return;
    }

    smash_tilde( argument );

    if ( IS_SET( ch->in_room->room_flags, ROOM_PET_SHOP ) )
    {
        CHAR_DATA *pet;
        ROOM_INDEX_DATA *pRoomIndexNext;
        ROOM_INDEX_DATA *in_room;

        if ( IS_NPC( ch ) )
            return;

        argument = one_argument( argument, arg );

        pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
        if ( pRoomIndexNext == NULL )
        {
            bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
            send_to_char( "Sorry, you can't buy that here.\n\r", ch );
            return;
        }

        in_room = ch->in_room;
        ch->in_room = pRoomIndexNext;
        pet = get_char_room( ch, arg );
        ch->in_room = in_room;

        if ( pet == NULL || !IS_SET( pet->act, ACT_PET ) )
        {
            send_to_char( "Sorry, you can't buy that here.\n\r", ch );
            return;
        }

        if ( ch->pet != NULL )
        {
            send_to_char( "You already own a pet.\n\r", ch );
            return;
        }

        cost = 10 * pet->level * pet->level;

        if ( ch->gold < cost )
        {
            send_to_char( "You can't afford it.\n\r", ch );
            return;
        }

        if ( ch->level < pet->level )
        {
            send_to_char( "You're not powerful enough to master this pet.\n\r",
                          ch );
            return;
        }

        /* haggle */
        roll = number_percent(  );
        if ( !IS_NPC( ch ) && roll < ch->pcdata->learned[gsn_haggle] )
        {
            cost -= cost / 2 * roll / 100;
            sprintf( buf, "You haggle the price down to %d coins.\n\r", cost );
            send_to_char( buf, ch );
            check_improve( ch, gsn_haggle, TRUE, 4 );

        }

        if ( !IS_NPC( ch ) && ch->pcdata->learned[gsn_haggle] > 0
             && roll > ch->pcdata->learned[gsn_haggle] )
        {
            send_to_char( "You fail to haggle the price down.\n\r", ch );
            check_improve( ch, gsn_haggle, TRUE, 4 );
        }

        ch->gold -= cost;
        pet = create_mobile( pet->pIndexData );
        SET_BIT( ch->act, PLR_BOUGHT_PET );
        SET_BIT( pet->act, ACT_PET );
        SET_BIT( pet->affected_by, AFF_CHARM );
        pet->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;

        argument = one_argument( argument, arg );
        if ( arg[0] != '\0' )
        {
            sprintf( buf, "%s %s", pet->name, arg );
            free_string( &pet->name );
            pet->name = str_dup( buf );
        }

        sprintf( buf, "%sA neck tag says 'I belong to %s'.\n\r",
                 pet->description, ch->name );
        free_string( &pet->description );
        pet->description = str_dup( buf );

        char_to_room( pet, ch->in_room );
        add_follower( pet, ch );
        pet->leader = ch;
        ch->pet = pet;
        send_to_char( "Enjoy your pet.\n\r", ch );
        act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
        return;
    }
/* Multiple object buy routine

  This code was written by Erwin Andreasen, 4u2@aarhues.dk
  Please mail me if you decide to use this code, or if you have any
  additions, bug reports, ideas or just comments.
  Please do not remove my name from the files.

*/
    else                        /* object purchase code begins HERE */
    {
        CHAR_DATA *keeper;
        OBJ_DATA *obj;
        int cost;
        char arg2[MAX_INPUT_LENGTH];    /* 2nd argument */
        int item_count = 1;     /* default: buy only 1 item */

        argument = one_argument( argument, arg );   /* get first argument */
        argument = one_argument( argument, arg2 );  /* get another argument, if any */

        if ( ( keeper = find_keeper( ch ) ) == NULL )   /* is there a shopkeeper here? */
            return;

        /* If the character is fighting, kick them out */
        if ( ch->fighting )
        {
            if ( shopkeeper_kick_ch
                 ( ch, keeper, "$t kicks you out of the store for fighting!" ) )
            {
                do_look( ch, "" );
            }
            else
            {
                send_to_char( "Not while you're fighting.\n\r", ch );
            }

            return;
        }

        /* If the character fails faction check, kick them out */
        if ( faction_percentage( consider_factions( ch, keeper, FALSE ) ) <=
             CFG_FACTION_SHOPKEEPER_ABSOLUTE_MIN )
        {
            if ( shopkeeper_kick_ch
                 ( ch, keeper,
                   "$t says 'Get out of my store, you piece of filth!'" ) )
            {
                do_look( ch, "" );
            }

            return;
        }

        if ( arg2[0] )          /* more than one argument specified? then arg2[0] <> 0 */
        {
            /* check if first of the 2 args really IS a number */

            if ( !is_number( arg ) )
            {
                send_to_char
                    ( "Syntax for BUY is: BUY [number] <item>\n\r\"number\" is an optional number of items to buy.\n\r",
                      ch );
                return;
            }

            if ( strlen( arg ) > 4 )
            {
                act( "$n tells you 'You're joking! I don't have that much in stock!'.", keeper, NULL, ch, TO_VICT );
                ch->reply = keeper;
                return;
            }

            item_count = atoi( arg );   /* first argument is the optional count */
            strcpy( arg, arg2 );    /* copy the item name to its right spot */
        }

/* find the pointer to the object */
        obj = get_obj_carry( keeper, arg );
        cost = get_cost( ch, keeper, obj, TRUE );

        if ( cost <= 0 || !can_see_obj( ch, obj ) ) /* cant buy what you cant see */
        {
            act( "$n tells you 'I don't sell that -- try 'list''.", keeper,
                 NULL, ch, TO_VICT );
            ch->reply = keeper;
            return;
        }

/* check for valid positive numeric value entered */
        if ( !( item_count > 0 ) )
        {
            send_to_char( "Buy how many? Number must be positive!\n\r", ch );
            return;
        }

        /* can the character afford it ? */
        if ( ch->gold < ( cost * item_count ) )
        {
            if ( item_count == 1 )  /* he only wanted to buy one */
            {
                act( "$n tells you 'You can't afford to buy $p'.", keeper, obj,
                     ch, TO_VICT );
            }
            else
            {
                if ( ( ch->gold / cost ) > 0 )  /* how many CAN he afford? */
                    sprintf( buf,
                             "$n tells you 'You can only afford %ld of those!",
                             ( ch->gold / cost ) );
                else            /* not even a single one! what a bum! */
                    sprintf( buf,
                             "$n tells you '%s? You must be kidding - you can't even afford a single one, let alone %d!'",
                             capitalize( obj->short_descr ), item_count );

                act( buf, keeper, obj, ch, TO_VICT );
                ch->reply = keeper; /* like the character really would reply to the shopkeeper... */
                return;
            }

            ch->reply = keeper; /* like the character really would reply to the shopkeeper... */
            return;
        }

        /* haggle */
        roll = number_percent(  );
        if ( !IS_NPC( ch ) && roll < ch->pcdata->learned[gsn_haggle] )
        {
            cost -= obj->cost / 2 * roll / 100;
            sprintf( buf, "You haggle the price down to %d coins.\n\r", cost );
            send_to_char( buf, ch );
            fHaggle = TRUE;
        }

        if ( !IS_NPC( ch ) && ch->pcdata->learned[gsn_haggle] > 0
             && roll > ch->pcdata->learned[gsn_haggle] )

        {
            send_to_char( "You fail to haggle the price down.\n\r", ch );
            check_improve( ch, gsn_haggle, TRUE, 4 );

        }

        /* Can the character use the item at all ? */
        if ( obj->level > ch->level )
        {
            act( "$n tells you 'You can't use $p yet'.",
                 keeper, obj, ch, TO_VICT );
            ch->reply = keeper;
            return;
        }
/* can the character carry more items? */
        if ( ch->carry_number + ( get_obj_number( obj ) * item_count ) >
             can_carry_n( ch ) )
        {
            send_to_char( "You can't carry that many items.\n\r", ch );
            return;
        }

/* can the character carry more weight? */
        if ( ch->carry_weight + item_count * get_obj_weight( obj ) >
             can_carry_w( ch ) )
        {
            send_to_char( "You can't carry that much weight.\n\r", ch );
            return;
        }

/* check for objects sold to the keeper */
        if ( ( item_count > 1 ) && !IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
        {
            act( "$n tells you 'Sorry - $p is something I have only one of'.",
                 keeper, obj, ch, TO_VICT );
            ch->reply = keeper;
            return;
        }

/* change this to reflect multiple items bought */
        if ( item_count == 1 )
        {
            act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
            act( "You buy $p.", ch, obj, NULL, TO_CHAR );
        }
        else                    /* inform of multiple item purchase */
        {
/* "buys 5 * a piece of bread" seems to be the easiest and least gramatically incorerct solution. */
            sprintf( buf, "$n buys %d * $p.", item_count );
            act( buf, ch, obj, NULL, TO_ROOM ); /* to char self */
            sprintf( buf, "You buy %d * $p.", item_count );
            act( buf, ch, obj, NULL, TO_CHAR ); /* to others */
        }

        ch->gold -= cost * item_count;
        keeper->gold += cost * item_count;

        /* haggle */
        if ( fHaggle )
            check_improve( ch, gsn_haggle, TRUE, 4 );

        if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )   /* 'permanent' item */
        {
            /* item_count of items */
            for ( ; item_count > 0; item_count-- )  /* create item_count objects */
            {
                obj = create_object( obj->pIndexData, obj->level );
                obj_to_char( obj, ch );
            }
        }
        else                    /* single item */
        {
            obj_from_char( obj );
            obj_to_char( obj, ch );
        }
        return;
    }                           /* else */
}                               /* do_buy */

void do_heal( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    int cost, sn;
    SPELL_FUN *spell;
    char *words;

    /* check for healer */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC( mob ) && IS_SET( mob->act, ACT_IS_HEALER ) )
            break;
    }

    if ( mob == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        /* display price list */
        act( "$N says 'I offer the following spells:'", ch, NULL, mob,
             TO_CHAR );
        send_to_char( "  light: cure light wounds      100 gold\n\r", ch );
        send_to_char( "  serious: cure serious wounds  150 gold\n\r", ch );
        send_to_char( "  critic: cure critical wounds  250 gold\n\r", ch );
        send_to_char( "  heal: healing spell	       500 gold\n\r", ch );
        send_to_char( "  blind: cure blindness         200 gold\n\r", ch );
        send_to_char( "  disease: cure disease         150 gold\n\r", ch );
        send_to_char( "  poison:  cure poison	       250 gold\n\r", ch );
        send_to_char( "  uncurse: remove curse	       500 gold\n\r", ch );
        send_to_char( "  refresh: restore movement      50 gold\n\r", ch );
        send_to_char( "  mana:  restore mana	       100 gold\n\r", ch );
        send_to_char( "  cancel:  cancel spells	       500 gold\n\r", ch );
        send_to_char( " Type heal <type> to be healed.\n\r", ch );
        return;
    }

    if ( !str_prefix( arg, "light" ) )
    {
        spell = spell_cure_light;
        sn = skill_lookup( "cure light" );
        words = "judicandus dies";
        cost = 100;
    }

    else if ( !str_prefix( arg, "serious" ) )
    {
        spell = spell_cure_serious;
        sn = skill_lookup( "cure serious" );
        words = "judicandus gzfuajg";
        cost = 160;
    }

    else if ( !str_prefix( arg, "critical" ) )
    {
        spell = spell_cure_critical;
        sn = skill_lookup( "cure critical" );
        words = "judicandus qfuhuqar";
        cost = 250;
    }

    else if ( !str_prefix( arg, "heal" ) )
    {
        spell = spell_heal;
        sn = skill_lookup( "heal" );
        words = "pzar";
        cost = 500;
    }

    else if ( !str_prefix( arg, "blindness" ) )
    {
        spell = spell_cure_blindness;
        sn = skill_lookup( "cure blindness" );
        words = "judicandus noselacri";
        cost = 200;
    }

    else if ( !str_prefix( arg, "disease" ) )
    {
        spell = spell_cure_disease;
        sn = skill_lookup( "cure disease" );
        words = "judicandus eugzagz";
        cost = 150;
    }

    else if ( !str_prefix( arg, "poison" ) )
    {
        spell = spell_cure_poison;
        sn = skill_lookup( "cure poison" );
        words = "judicandus sausabru";
        cost = 250;
    }

    else if ( !str_prefix( arg, "uncurse" ) || !str_prefix( arg, "curse" ) )
    {
        spell = spell_remove_curse;
        sn = skill_lookup( "remove curse" );
        words = "candussido judifgz";
        cost = 500;
    }

    else if ( !str_prefix( arg, "mana" ) || !str_prefix( arg, "energize" ) )
    {
        spell = NULL;
        sn = -1;
        words = "energizer";
        cost = 100;
    }

    else if ( !str_prefix( arg, "refresh" ) || !str_prefix( arg, "moves" ) )
    {
        spell = spell_refresh;
        sn = skill_lookup( "refresh" );
        words = "candusima";
        cost = 50;
    }

    else if ( !str_prefix( arg, "cancel" ) )
    {
        spell = spell_cancellation;
        sn = skill_lookup( "cancellation" );
        words = "niotallecanc";
        cost = 500;
    }

    else
    {
        act( "$N says 'Type 'heal' for a list of spells.'",
             ch, NULL, mob, TO_CHAR );
        return;
    }

    if ( cost > ( ch->gold ) )
    {
        act( "$N says 'You do not have enough gold for my services.'",
             ch, NULL, mob, TO_CHAR );
        return;
    }

    WAIT_STATE( ch, PULSE_VIOLENCE );

    ch->gold -= cost;
    mob->gold += cost;
    act( "$n utters the words '$T'.", mob, NULL, words, TO_ROOM );

    if ( spell == NULL )        /* restore mana trap...kinda hackish */
    {
        ch->mana += dice( 2, 8 ) + mob->level / 3;
        ch->mana = UMIN( ch->mana, ch->max_mana );
        send_to_char( "A warm glow passes through you.\n\r", ch );
        return;
    }

    if ( sn == -1 )
        return;

    ( *skill_table[sn].spell_fun ) ( sn, mob->level, mob, ch );

    /* spell(sn,mob->level,mob,ch,TARGET_CHAR); */
}

void do_list( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET( ch->in_room->room_flags, ROOM_PET_SHOP ) )
    {
        ROOM_INDEX_DATA *pRoomIndexNext;
        CHAR_DATA *pet;
        bool found;

        pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );

        if ( pRoomIndexNext == NULL )
        {
            bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
            send_to_char( "You can't do that here.\n\r", ch );
            return;
        }

        found = FALSE;

        for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room )
        {
            if ( IS_SET( pet->act, ACT_PET ) )
            {
                if ( !found )
                {
                    found = TRUE;
                    send_to_char( "Pets for sale:\n\r", ch );
                }

                sprintf( buf, "[%2d] %8d - %s\n\r",
                         pet->level,
                         10 * pet->level * pet->level, pet->short_descr );

                send_to_char( buf, ch );
            }
        }

        if ( !found )
        {
            send_to_char( "Sorry, we're out of pets right now.\n\r", ch );
        }

        return;
    }
    else
    {
        CHAR_DATA *keeper;
        OBJ_DATA *obj;
        int cost;
        bool found;

        if ( ( keeper = find_keeper( ch ) ) == NULL )
        {
            return;
        }

        /* If the character is fighting, kick them out */
        if ( ch->fighting )
        {
            if ( shopkeeper_kick_ch
                 ( ch, keeper, "$t kicks you out of the store for fighting!" ) )
            {
                do_look( ch, "" );
            }
            else
            {
                send_to_char( "Not while you're fighting.\n\r", ch );
            }

            return;
        }

        /* If the character fails faction check, kick them out */
        if ( faction_percentage( consider_factions( ch, keeper, FALSE ) ) <=
             CFG_FACTION_SHOPKEEPER_ABSOLUTE_MIN )
        {
            if ( shopkeeper_kick_ch
                 ( ch, keeper,
                   "$t says 'Get out of my store, you piece of filth!'" ) )
            {
                do_look( ch, "" );
            }

            return;
        }

        one_argument( argument, arg );

        found = FALSE;

        for ( obj = keeper->carrying; obj; obj = obj->next_content )
        {
            if ( obj->wear_loc == WEAR_NONE
                 && can_see_obj( ch, obj )
                 && ( cost = get_cost( ch, keeper, obj, TRUE ) ) > 0
                 && ( arg[0] == '\0' || is_name( arg, obj->name ) ) )
            {
                if ( !found )
                {
                    found = TRUE;

                    send_to_char( "[Lv Price] Item\n\r", ch );
                }

                sprintf( buf, "[%2d %5d] %s.\n\r",
                         obj->level, cost, obj->short_descr );

                send_to_char( buf, ch );
            }
        }

        if ( !found )
            send_to_char( "You can't buy anything here.\n\r", ch );

        return;
    }
}

void do_sell( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost, roll;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Sell what?\n\r", ch );
        return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
        return;

    /* If the character is fighting, kick them out */
    if ( ch->fighting )
    {
        if ( shopkeeper_kick_ch
             ( ch, keeper, "$t kicks you out of the store for fighting!" ) )
        {
            do_look( ch, "" );
        }
        else
        {
            send_to_char( "Not while you're fighting.\n\r", ch );
        }

        return;
    }

    /* If the character fails faction check, kick them out */
    if ( faction_percentage( consider_factions( ch, keeper, FALSE ) ) <=
         CFG_FACTION_SHOPKEEPER_ABSOLUTE_MIN )
    {
        if ( shopkeeper_kick_ch
             ( ch, keeper,
               "$t says 'Get out of my store, you piece of filth!'" ) )
        {
            do_look( ch, "" );
        }

        return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
        act( "$n tells you 'You don't have that item'.",
             keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't let go of it.\n\r", ch );
        return;
    }

    if ( !can_see_obj( keeper, obj ) )
    {
        act( "$n doesn't see what you are offering.", keeper, NULL, ch,
             TO_VICT );
        return;
    }

    /* won't buy rotting goods */
    if ( obj->timer || ( cost = get_cost( ch, keeper, obj, FALSE ) ) <= 0 )
    {
        act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
        return;
    }

    if ( obj->clan != 0 )
    {
        act( "$n refuses to buy clan gear.", keeper, NULL, ch, TO_VICT );
        return;
    }

    if ( cost > keeper->gold )
    {
        act( "$n tells you 'I'm afraid I don't have enough gold to buy $p.",
             keeper, obj, ch, TO_VICT );
        return;
    }

    act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
    /* haggle */
    roll = number_percent(  );
    if ( !IS_NPC( ch ) && roll < ch->pcdata->learned[gsn_haggle] )
    {
        send_to_char( "You haggle with the shopkeeper.\n\r", ch );
        cost += obj->cost / 2 * roll / 100;
        cost = UMIN( cost, 95 * get_cost( ch, keeper, obj, TRUE ) / 100 );
        cost = UMIN( cost, keeper->gold );
        check_improve( ch, gsn_haggle, TRUE, 4 );
    }
    if ( !IS_NPC( ch ) && ch->pcdata->learned[gsn_haggle] > 0
         && roll > ch->pcdata->learned[gsn_haggle] )

    {
        send_to_char( "You fail to haggle the price up.\n\r", ch );
        check_improve( ch, gsn_haggle, TRUE, 4 );

    }

    sprintf( buf, "You sell $p for %d gold piece%s.",
             cost, cost == 1 ? "" : "s" );
    act( buf, ch, obj, NULL, TO_CHAR );
    ch->gold += cost;
    keeper->gold -= cost;
    if ( keeper->gold < 0 )
        keeper->gold = 0;

    if ( obj->item_type == ITEM_TRASH )
    {
        extract_obj( obj );
    }
    else
    {
        obj_from_char( obj );
/* This was causing items sold to a shopkeeper and then purchased to rot, 
 * discouraging any kind of economy.
 */
/*        obj->timer = number_range( 50, 100 ); */
        obj_to_char( obj, keeper );
    }

    return;
}

void do_value( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Value what?\n\r", ch );
        return;
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL )
        return;

    /* If the character is fighting, kick them out */
    if ( ch->fighting )
    {
        if ( shopkeeper_kick_ch
             ( ch, keeper, "$t kicks you out of the store for fighting!" ) )
        {
            do_look( ch, "" );
        }
        else
        {
            send_to_char( "Not while you're fighting.\n\r", ch );
        }

        return;
    }

    /* If the character fails faction check, kick them out */
    if ( faction_percentage( consider_factions( ch, keeper, FALSE ) ) <=
         CFG_FACTION_SHOPKEEPER_ABSOLUTE_MIN )
    {
        if ( shopkeeper_kick_ch
             ( ch, keeper,
               "$t says 'Get out of my store, you piece of filth!'" ) )
        {
            do_look( ch, "" );
        }

        return;
    }

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
        act( "$n tells you 'You don't have that item'.",
             keeper, NULL, ch, TO_VICT );
        ch->reply = keeper;
        return;
    }

    if ( !can_see_obj( keeper, obj ) )
    {
        act( "$n doesn't see what you are offering.", keeper, NULL, ch,
             TO_VICT );
        return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
        send_to_char( "You can't let go of it.\n\r", ch );
        return;
    }

    if ( ( cost = get_cost( ch, keeper, obj, FALSE ) ) <= 0 )
    {
        act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
        return;
    }

    sprintf( buf, "$n tells you 'I'll give you %d gold coins for $p'.", cost );
    act( buf, keeper, obj, ch, TO_VICT );
    ch->reply = keeper;

    return;
}

void do_junk( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;
    int gold;

    one_argument( argument, arg );
    gold = 0;
    if ( arg[0] == '\0' )
    {
        send_to_char( "What do you wish to junk?\n\r", ch );
        return;
    }
/* Let not let them throw away evidence now ;)  -Lancelight */
    if ( IS_SET( ch->act, PLR_JAILED ) )
    {
        send_to_char( "You will NOT get rid of any evidance!\n\r", ch );
        return;
    }

    if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) )
    {
        /* 'junk obj' */
        if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
        {
            send_to_char( "You do not have that item.\n\r", ch );
            return;
        }

        if ( !can_drop_obj( ch, obj ) )
        {
            send_to_char( "You can't let go of it.\n\r", ch );
            return;
        }

        if ( obj->item_type == ITEM_CORPSE_PC )
        {
            if ( obj->contains )
            {
                send_to_char( "The gods wouldn't like that.\n\r", ch );
                return;
            }
        }

        gold = UMAX( 1, obj->level * 2 );

        if ( obj->item_type != ITEM_CORPSE_NPC
             && obj->item_type != ITEM_CORPSE_PC )
            gold = UMIN( gold, obj->cost );

        if ( gold == 1 )
            send_to_char
                ( "The gods give you one gold coin for your sacrifice.\n\r",
                  ch );
        else
        {
            sprintf( buf,
                     "The gods give you %d gold coins for your sacrifice.\n\r",
                     gold );
            send_to_char( buf, ch );
        }

        ch->gold += gold;
        act( "$n sacrifices $p to the gods.", ch, obj, NULL, TO_ROOM );
        extract_obj( obj );
        return;
    }
    else
    {
        /* 'donate all' or 'donate all.obj' */
        found = FALSE;
        for ( obj = ch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;

            if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
                 && can_see_obj( ch, obj )
                 && obj->wear_loc == WEAR_NONE && can_drop_obj( ch, obj ) )
            {
                found = TRUE;
                if ( !can_drop_obj( ch, obj ) )
                {
                    send_to_char( "You can't let go of it.\n\r", ch );
                    break;
                }

                if ( obj->item_type == ITEM_CORPSE_PC )
                {
                    if ( obj->contains )
                    {
                        send_to_char( "The gods wouldn't like that.\n\r", ch );
                        break;
                    }
                }
                gold = UMAX( 1, obj->level * 2 );

                if ( obj->item_type != ITEM_CORPSE_NPC
                     && obj->item_type != ITEM_CORPSE_PC )
                    gold = UMIN( gold, obj->cost );

                if ( gold == 1 )
                    send_to_char
                        ( "The gods give you one gold coin for your sacrifice.\n\r",
                          ch );
                else
                {
                    sprintf( buf,
                             "The gods give you %d gold coins for your sacrifice.\n\r",
                             gold );
                    send_to_char( buf, ch );
                }

                ch->gold += gold;
                act( "$n sacrifices $p to the gods.", ch, obj, NULL, TO_ROOM );
                extract_obj( obj );
            }
        }
        if ( !found )
        {
            if ( arg[3] == '\0' )
                act( "You are not carrying anything.", ch, NULL, arg, TO_CHAR );
            else
                act( "You are not carrying any $T.",
                     ch, NULL, &arg[4], TO_CHAR );
        }
    }

    return;
}

/*
brew.c by Jason Huang (huangjac@netcom.com) 
*/
/* Original Code by Todd Lair.                                        */
/* Improvements and Modification by Jason Huang (huangjac@netcom.com).*/
/* Permission to use this code is granted provided this header is     */
/* retained and unaltered.                                            */

void do_brew( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int sn;

    if ( !IS_NPC( ch )
         && ch->level < skill_table[gsn_brew].skill_level[ch->Class] )
    {
        send_to_char( "You do not know how to brew potions.\n\r", ch );
        return;
    }
/* Brewing can damage players, so lets not let them get outa jail free ;)
-Lancelight */
    if ( IS_SET( ch->act, PLR_JAILED ) )
    {
        send_to_char( "Brewing is not allowed in jail.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Brew what spell?\n\r", ch );
        return;
    }

    /* Do we have a vial to brew potions? */
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
        if ( obj->pIndexData->vnum == OBJ_VNUM_EMPTY_VIAL
             && obj->wear_loc == WEAR_HOLD )
            break;
    }

    /* Interesting ... Most scrolls/potions in the mud have no hold
       flag; so, the problem with players running around making scrolls 
       with 3 heals or 3 gas breath from pre-existing scrolls has been 
       severely reduced. Still, I like the idea of 80% success rate for  
       first spell imprint, 25% for 2nd, and 10% for 3rd. I don't like the
       idea of a scroll with 3 ultrablast spells; although, I have limited
       its applicability when I reduced the spell->level to 1/3 and 1/4 of 
       ch->level for scrolls and potions respectively. --- JH */

    /* I will just then make two items, an empty vial and a parchment available
       in midgaard shops with holdable flags and -1 for each of the 3 spell
       slots. Need to update the midgaard.are files --- JH */

    if ( !obj )
    {
        send_to_char( "You are not holding a vial.\n\r", ch );
        return;
    }

    if ( ( sn = skill_lookup( arg ) ) < 0 )
    {
        send_to_char( "You don't know any spells by that name.\n\r", ch );
        return;
    }

    /* preventing potions of gas breath, acid blast, etc.; doesn't make sense
       when you quaff a gas breath potion, and then the mobs in the room are
       hurt. Those TAR_IGNORE spells are a mixed blessing. - JH */

    if ( ( skill_table[sn].target != TAR_CHAR_DEFENSIVE ) &&
         ( skill_table[sn].target != TAR_CHAR_SELF ) )
    {
        send_to_char( "You cannot brew that spell.\n\r", ch );
        return;
    }

    act( "$n begins preparing a potion.", ch, obj, NULL, TO_ROOM );
    WAIT_STATE( ch, skill_table[gsn_brew].beats );

    /* Check the skill percentage, fcn(wis,int,skill) */
    if ( !IS_NPC( ch )
         && ( number_percent(  ) > ch->pcdata->learned[gsn_brew] ||
              number_percent(  ) >
              ( ( get_curr_stat( ch, STAT_INT ) - 13 ) * 5 +
                ( get_curr_stat( ch, STAT_WIS ) - 13 ) * 3 ) ) )
    {
        act( "$p explodes violently!", ch, obj, NULL, TO_CHAR );
        act( "$p explodes violently!", ch, obj, NULL, TO_ROOM );
        act( "A blast of acid erupts from $p, burning your skin!", ch, obj,
             NULL, TO_CHAR );
        act( "A blast of acid erupts from $p, burning $n's skin!", ch, obj,
             NULL, TO_ROOM );
        new_damage( ch, ch, NULL, dice( UMIN( ch->level, 45 ), 10 ),
                    skill_lookup( "acid blast" ), DAM_ACID, FALSE );

        extract_obj( obj );
        return;
    }

    /* took this outside of imprint codes, so I can make do_brew differs from
       do_scribe; basically, setting potion level and spell level --- JH */

    obj->level = ( sh_int ) ( ch->level * BREW_SCRIBE_USE_LEVEL );
    obj->value[0] = ( int ) ( ch->level * BREW_SCRIBE_LEVEL );
    spell_imprint( sn, ch->level, ch, obj );

}

void do_scribe( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int sn, dam;

    if ( !IS_NPC( ch )
         && ch->level < skill_table[gsn_scribe].skill_level[ch->Class] )
    {
        send_to_char( "You do not know how to scribe scrolls.\n\r", ch );
        return;
    }

/* Writting scrolls can hurt players. Lets not let them die to get outa
jail. */
    if ( IS_SET( ch->act, PLR_JAILED ) )
    {
        send_to_char( "You cant read or write while in jail, its to dark.\n\r",
                      ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Scribe what spell?\n\r", ch );
        return;
    }

    /* Do we have a parchment to scribe spells? */
    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
        if ( obj->pIndexData->vnum == OBJ_VNUM_BLANK_SCROLL
             && obj->wear_loc == WEAR_HOLD )
            break;
    }
    if ( !obj )
    {
        send_to_char( "You are not holding a parchment.\n\r", ch );
        return;
    }

    if ( ( sn = skill_lookup( arg ) ) < 0 )
    {
        send_to_char( "You don't know any spells by that name.\n\r", ch );
        return;
    }

    act( "$n begins writing a scroll.", ch, obj, NULL, TO_ROOM );
    WAIT_STATE( ch, skill_table[gsn_scribe].beats );

    /* Check the skill percentage, fcn(int,wis,skill) */
    if ( !IS_NPC( ch )
         && ( number_percent(  ) > ch->pcdata->learned[gsn_brew] ||
              number_percent(  ) >
              ( ( get_curr_stat( ch, STAT_INT ) - 13 ) * 5 +
                ( get_curr_stat( ch, STAT_WIS ) - 13 ) * 3 ) ) )
    {
        act( "$p bursts in flames!", ch, obj, NULL, TO_CHAR );
        act( "$p bursts in flames!", ch, obj, NULL, TO_ROOM );
        act( "$n is enveloped in flames!", ch, NULL, NULL, TO_ROOM );
        act( "You are enveloped in flames!", ch, NULL, NULL, TO_CHAR );

        dam = dice( UMIN( ch->level, 45 ), 7 );
        new_damage( ch, ch, NULL, dam, skill_lookup( "fireball" ), DAM_FIRE,
                    FALSE );
        extract_obj( obj );
        return;
    }

    /* basically, making scrolls more potent than potions; also, scrolls
       are not limited in the choice of spells, i.e. scroll of enchant weapon
       has no analogs in potion forms --- JH */

    obj->level = ( sh_int ) ( ch->level * BREW_SCRIBE_USE_LEVEL );
    obj->value[0] = ( int ) ( ch->level * BREW_SCRIBE_LEVEL );
    spell_imprint( sn, ch->level, ch, obj );

}

/*
 * The lore skill.  Basically compare the object's level against your level.
 * The difference in levels (and your lore skill level) determines wether or not
 * you can identify the object. Also, partial identification can occur based on
 * how well you succeed.  -Zane
 */
void do_lore( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj = NULL;
    AFFECT_DATA *paf;
    int chance = 0, tmpchance = 0, affectcount = 0;

    argument = one_argument( argument, arg );

    if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
    {
        if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
        {
            send_to_char( "You are not carrying or wearing that.\n\r", ch );
            return;
        }
    }

    if ( ( chance = get_skill( ch, gsn_lore ) ) == 0
         || ( !IS_NPC( ch )
              && ch->level < skill_table[gsn_lore].skill_level[ch->Class] ) )
    {
        sprintf( buf, "It looks like a %s to you.\n\r", obj->short_descr );
        send_to_char( buf, ch );
        return;
    }

    /*
     * If the item is the same level as the character or higher don't let them identify it.
     * We don't want this skill to be a cheap replacement of the identify spell. -Zane
     */
    if ( obj->level >= ch->level )
    {
        send_to_char
            ( "You lack the lore required to properly identify this item.\n\r",
              ch );
        return;
    }

    /* Modify the level of information by the level of the item as opposed to it's user */
    chance =
        ( int ) ( chance *
                  ( 1 - ( obj->level >= 1 ? obj->level : 1 ) / ch->level ) );

    sprintf( buf,
             "Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\r",
             obj->name, item_type_name( obj ),
             extra_bit_name( obj->extra_flags ), obj->weight, obj->cost,
             obj->level );
    send_to_char( buf, ch );

    switch ( obj->item_type )
    {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
        if ( chance >= 30 )
        {
            sprintf( buf, "Level %d spells of:", obj->value[0] );
            send_to_char( buf, ch );

            if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[1]].name, ch );
                send_to_char( "'", ch );
            }

            if ( obj->value[2] >= 0 && obj->value[2] < MAX_SKILL
                 && chance >= 50 )
            {
                if ( chance >= 65 )
                {
                    send_to_char( " '", ch );
                    send_to_char( skill_table[obj->value[2]].name, ch );
                    send_to_char( "'", ch );
                }
                else
                {
                    send_to_char
                        ( ".\n\rYour lore holds no clue as to what further magics lie within this item",
                          ch );
                }
            }

            if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL
                 && chance >= 85 )
            {
                if ( chance >= 93 )
                {
                    send_to_char( " '", ch );
                    send_to_char( skill_table[obj->value[3]].name, ch );
                    send_to_char( "'", ch );
                }
                else
                {
                    send_to_char
                        ( ".\n\r Your lore holds no clue as to what further magics lie within this item",
                          ch );
                }
            }

            send_to_char( ".\n\r", ch );
        }

        else if ( chance >= 15 )
        {
            send_to_char
                ( "You detect some magic contained within this item.\n\r", ch );
        }

        break;

    case ITEM_WAND:
    case ITEM_STAFF:
        if ( chance >= 80 )
        {
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
        }
        else if ( chance >= 50 )
        {
            sprintf( buf, "Casts level %d", obj->value[0] );
            send_to_char( buf, ch );

            if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
            {
                send_to_char( " '", ch );
                send_to_char( skill_table[obj->value[3]].name, ch );
                send_to_char( "'", ch );
            }

            send_to_char( ".\n\r", ch );
        }
        else if ( chance >= 30 )
        {
            send_to_char
                ( "This item contains magic, but you cannot ascertain it's purpose.\n\r",
                  ch );
        }

        break;

    case ITEM_WEAPON:
        if ( chance >= 75 )
        {
            send_to_char( "Weapon type is ", ch );

            switch ( obj->value[0] )
            {
            case ( WEAPON_EXOTIC ):
                send_to_char( "exotic.\n\r", ch );
                break;
            case ( WEAPON_SWORD ):
                send_to_char( "sword.\n\r", ch );
                break;
            case ( WEAPON_DAGGER ):
                send_to_char( "dagger.\n\r", ch );
                break;
            case ( WEAPON_SPEAR ):
                send_to_char( "spear/staff.\n\r", ch );
                break;
            case ( WEAPON_MACE ):
                send_to_char( "mace/club.\n\r", ch );
                break;
            case ( WEAPON_AXE ):
                send_to_char( "axe.\n\r", ch );
                break;
            case ( WEAPON_FLAIL ):
                send_to_char( "flail.\n\r", ch );
                break;
            case ( WEAPON_WHIP ):
                send_to_char( "whip.\n\r", ch );
                break;
            case ( WEAPON_POLEARM ):
                send_to_char( "polearm.\n\r", ch );
                break;
            default:
                send_to_char( "unknown.\n\r", ch );
                break;
            }

            sprintf( buf, "Damage is %dd%d (average %d).\n\r", obj->value[1],
                     obj->value[2], ( 1 + obj->value[2] ) * obj->value[1] / 2 );
            send_to_char( buf, ch );
        }
        else
        {
            send_to_char( "Weapon type is ", ch );

            switch ( obj->value[0] )
            {
            case ( WEAPON_EXOTIC ):
                send_to_char( "exotic.\n\r", ch );
                break;
            case ( WEAPON_SWORD ):
                send_to_char( "sword.\n\r", ch );
                break;
            case ( WEAPON_DAGGER ):
                send_to_char( "dagger.\n\r", ch );
                break;
            case ( WEAPON_SPEAR ):
                send_to_char( "spear/staff.\n\r", ch );
                break;
            case ( WEAPON_MACE ):
                send_to_char( "mace/club.\n\r", ch );
                break;
            case ( WEAPON_AXE ):
                send_to_char( "axe.\n\r", ch );
                break;
            case ( WEAPON_FLAIL ):
                send_to_char( "flail.\n\r", ch );
                break;
            case ( WEAPON_WHIP ):
                send_to_char( "whip.\n\r", ch );
                break;
            case ( WEAPON_POLEARM ):
                send_to_char( "polearm.\n\r", ch );
                break;
            default:
                send_to_char( "unknown.\n\r", ch );
                break;
            }
        }

        break;

    case ITEM_ARMOR:
        if ( chance >= 75 )
        {
            sprintf( buf,
                     "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r",
                     obj->value[0], obj->value[1], obj->value[2],
                     obj->value[3] );
            send_to_char( buf, ch );
        }

        break;
    }

    tmpchance = chance;

    if ( !obj->enchanted )
    {
        for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
        {
            if ( paf->location != APPLY_NONE && paf->modifier != 0 )
            {
                if ( tmpchance >= 77 )
                {
                    sprintf( buf, "Affects %s by %d.\n\r",
                             affect_loc_name( paf->location ), paf->modifier );
                    send_to_char( buf, ch );
                }
                else if ( tmpchance >= 40 )
                {
                    affectcount++;
                }

                tmpchance -= 2;
            }
        }
    }

    tmpchance = chance;

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
        if ( paf->location != APPLY_NONE && paf->modifier != 0 )
        {
            if ( tmpchance >= 77 )
            {
                sprintf( buf, "Affects %s by %d.\n\r",
                         affect_loc_name( paf->location ), paf->modifier );
                send_to_char( buf, ch );
            }
            else if ( tmpchance >= 40 )
            {
                affectcount++;
            }

            tmpchance -= 2;
        }
    }

    if ( affectcount > 0 )
    {
        send_to_char
            ( "You detect additional affects but are unable to discern their purpose.\n\r",
              ch );
    }

    check_improve( ch, gsn_lore, TRUE, 1 );

    return;
}

bool shopkeeper_kick_ch( CHAR_DATA * ch, CHAR_DATA * keeper, char *actstr )
{
    ROOM_INDEX_DATA *was_in;
    EXIT_DATA *pExit;
    int attempt, door;
    CHAR_DATA *tmpch;

    /* Shopkeeper won't kick someone out when he's fighting */
    if ( keeper->fighting )
        return FALSE;

    was_in = ch->in_room;

    for ( attempt = 0; attempt != 6; attempt++ )
    {
        door = number_door(  );

        if ( ( pExit = was_in->exit[door] ) == 0    /* No exit that direction */
             || pExit->u1.to_room == NULL   /* No room that direction */
             || IS_SET( pExit->exit_info, EX_CLOSED )   /* Closed door */
             || ( IS_NPC( ch ) && IS_SET( pExit->u1.to_room->room_flags, ROOM_NO_MOB ) )    /* Dest is NO_MOB */
             || IS_SET( pExit->exit_info, EX_HIDDEN )   /* Hidden exit */
             || room_is_private( pExit->u1.to_room )    /* Private room */
             || pExit->u1.to_room->sector_type == SECT_WATER_NOSWIM /* Water room */
             || pExit->u1.to_room->sector_type == SECT_AIR /* Air room */  )
        {
            continue;
        }

        act_new( actstr, ch, keeper->short_descr, NULL, TO_CHAR, POS_RESTING );
        act_new( "$t kicks you out of the store!", ch, keeper->short_descr,
                 NULL, TO_CHAR, POS_RESTING );
        act_new( "$n is kicked out of the store!", ch, NULL, NULL, TO_NOTVICT,
                 POS_RESTING );

        if ( ch->fighting )
        {
            for ( tmpch = ch->in_room->people; tmpch != NULL;
                  tmpch = tmpch->next )
            {
                if ( tmpch->fighting )
                {
                    act_new( "$t kicks you out of the store!", tmpch,
                             keeper->short_descr, NULL, TO_CHAR, POS_RESTING );
                    act_new( "$n is kicked out of the store!", tmpch, NULL,
                             NULL, TO_NOTVICT, POS_RESTING );
                    char_from_room( tmpch );
                    char_to_room( tmpch, pExit->u1.to_room );
                }
            }
        }

        char_from_room( ch );
        char_to_room( ch, pExit->u1.to_room );

        return TRUE;
    }

    return FALSE;
}

void do_portal( CHAR_DATA * ch, OBJ_DATA * portal )
{
    ROOM_INDEX_DATA *pRoom;

    if ( portal == NULL )
    {
        send_to_char( "Trust me, it won't take you anywhere.\n\r", ch );
        return;
    }

    if ( portal->item_type != ITEM_PORTAL )
    {
        send_to_char( "Trust me, it won't take you anywhere.\n\r", ch );
        return;
    }

    if ( portal->value[0] == 0 )
    {
        send_to_char( "It doesn't seem to go anywhere...\n\r", ch );
        return;
    }

    if ( ( pRoom = get_room_index( portal->value[0] ) ) == NULL )
    {
        send_to_char( "It doesn't seem to go anywhere...\n\r", ch );
        return;
    }

    act( "$n steps into a shimmering portal and disappears.", ch, NULL,
         portal->short_descr, TO_ROOM );
    act( "You step into a shimmering portal and find yourself in new surroundings.\n\r", ch, NULL, NULL, TO_CHAR );
    char_from_room( ch );
    char_to_room( ch, get_room_index( portal->value[0] ) );
    act( "$n steps out of a shimmering portal.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );
    return;
}

void do_enter( CHAR_DATA * ch, char *argument )
{
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: Enter <portal>\n\r", ch );
        return;
    }
    if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
    {
        send_to_char( "You do not see that here.\n\r", ch );
        return;
    }

    switch ( obj->item_type )
    {
    default:
        send_to_char( "Trust me, it won't take you anywhere.\n\r", ch );
        break;

    case ITEM_PORTAL:
        do_portal( ch, obj );
        break;
    }

    return;
}
