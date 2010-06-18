/***************************************************************************
 *  This file contains auction code developed by Brian Babey, and any      *
 *  communication regarding it should be sent to [bbabey@iname.com]        *
 *  Web Address: http://www.erols.com/bribe/                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#if defined(WIN32)
#include <windows.h>
#else
#include <sys/types.h>
#endif
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "merc.h"

DECLARE_DO_FUN( do_auction );
DECLARE_SPELL_FUN( spell_identify );
void show_obj_stats( int sn, int level, CHAR_DATA * ch, void *vo );
void auction_channel_bid(  );
void auction_channel_sell(  );
void auction_channel_begin(  );
void auction_channel_remove(  );
void auction_channel_once(  );
void auction_channel_twice(  );

void do_auction( CHAR_DATA * ch, char *argument )
{

#ifdef ALLOW_AFFECTS
    AFFECT_DATA *paf;
#endif
    long gold = 0;
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    argument = one_argument( argument, arg1 );

    if ( ch == NULL || IS_NPC( ch ) )
        return;

    if ( arg1[0] == '\0' )
    {
        if ( IS_SET( ch->comm, COMM_NOAUCTION ) )
        {
            REMOVE_BIT( ch->comm, COMM_NOAUCTION );
            send_to_char( "Auction channel is now ON.\n\r", ch );
            return;
        }

        SET_BIT( ch->comm, COMM_NOAUCTION );
        send_to_char( "Auction channel is now OFF.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "info" ) )
    {
        obj = auction_info.item;

        if ( !obj )
        {
            send_to_char( "There is nothing up for auction right now.\n\r",
                          ch );
            return;
        }

        if ( auction_info.owner == ch )
        {
            sprintf( buf, "\n\rYou are currently auctioning '%s'.\n\r",
                     obj->short_descr );
            send_to_char( buf, ch );
            return;
        }

        else
        {

#ifdef ALLOW_AUCTION_IDENTIFY
            spell_identify( skill_lookup( "identify" ), ch->level, ch, obj );
#endif

#ifdef ALLOW_SHORT_DESCR
            printf_to_char( ch, "`RName:        `C[`W%5s`C]`w\n\r",
                            obj->short_descr );
#endif

#ifdef ALLOW_LEVEL
            printf_to_char( ch, "`RLevel:       `C[`W%5d`C]`w\n\r",
                            obj->level );
#endif

#ifdef ALLOW_TYPE
            printf_to_char( ch, "`RType:        `C[`W%5s`C]`w\n\r",
                            item_type_name( obj ) );
#endif

#ifdef ALLOW_WEAR_BITS
            printf_to_char( ch, "`RWear flags:  `C[`W%5s`C]`w\n\r",
                            wear_bit_name( obj->wear_flags ) );
#endif

#ifdef ALLOW_EXTRA_BITS
            printf_to_char( ch, "`RExtras:      `C[`W%5s`C]`w\n\r",
                            extra_bit_name( obj->extra_flags ) );
#endif

#ifdef ALLOW_WEIGHT
            printf_to_char( ch, "`RWeight:      `C[`W%5d`C]`w\n\r",
                            obj->weight );
#endif

#ifdef ALLOW_COST
            printf_to_char( ch, "`RCost:        `C[`W%5d`C]`w\n\r", obj->cost );
#endif

#ifdef ALLOW_CONDITION
            printf_to_char( ch, "`RCondition:   `C[`W%5d`C]`w\n\r",
                            obj->condition );
#endif

#ifdef ALLOW_TIMER
            printf_to_char( ch, "`RTimer:       `C[`W%5d`C]`w\n\r",
                            obj->timer );
#endif

#ifdef ALLOW_ARMOR_VALUES
            printf_to_char( ch,
                            "`RValues:      `C[`WPierce   Bash  Slash  Magic`C]`w\n\r" );
            printf_to_char( ch,
                            "`RValues:      `C[`W%4d    %4d   %4d   %4d `C]`w\n\r",
                            obj->value[0], obj->value[1], obj->value[2],
                            obj->value[3] );
#endif

#ifdef ALLOW_ITEM_TYPE

            switch ( obj->item_type )
            {
#ifdef ALLOW_SCROLL_POTION_PILL
            case ITEM_SCROLL:
            case ITEM_POTION:
            case ITEM_PILL:
                sprintf( buf, "`RLevel `W%d `Rspells of:`W", obj->value[0] );
                send_to_char( buf, ch );

                if ( obj->value[1] >= 0 && obj->value[1] < MAX_SKILL )
                {
                    send_to_char( " '", ch );
                    send_to_char( skill_table[obj->value[1]].name, ch );
                    send_to_char( "'`", ch );
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

                send_to_char( ".`w\n\r", ch );
                break;
#endif

#ifdef ALLOW_WAND_STAFF
            case ITEM_WAND:
            case ITEM_STAFF:
                sprintf( buf, "`RHas `W%d`C(`W%d`C) `Rcharges of level `W%d",
                         obj->value[1], obj->value[2], obj->value[0] );
                send_to_char( buf, ch );

                if ( obj->value[3] >= 0 && obj->value[3] < MAX_SKILL )
                {
                    send_to_char( " '", ch );
                    send_to_char( skill_table[obj->value[3]].name, ch );
                    send_to_char( "'", ch );
                }

                send_to_char( ".`w\n\r", ch );
                break;
#endif

#ifdef ALLOW_WEAPON
            case ITEM_WEAPON:
#ifdef ALLOW_WEAPON_TYPE
                send_to_char( "`RWeapon type: `C[`W", ch );
                switch ( obj->value[0] )
                {
                case ( WEAPON_EXOTIC ):
                    send_to_char( "exotic`C]`w\n\r", ch );
                    break;
                case ( WEAPON_SWORD ):
                    send_to_char( "sword`C]`w\n\r", ch );
                    break;
                case ( WEAPON_DAGGER ):
                    send_to_char( "dagger`C]`w\n\r", ch );
                    break;
                case ( WEAPON_SPEAR ):
                    send_to_char( "spear/staff`C]`w\n\r", ch );
                    break;
                case ( WEAPON_MACE ):
                    send_to_char( "mace/club`C]`w\n\r", ch );
                    break;
                case ( WEAPON_AXE ):
                    send_to_char( "axe`C]`w\n\r", ch );
                    break;
                case ( WEAPON_FLAIL ):
                    send_to_char( "flail`C]`w\n\r", ch );
                    break;
                case ( WEAPON_WHIP ):
                    send_to_char( "whip`C]`w\n\r", ch );
                    break;
                case ( WEAPON_POLEARM ):
                    send_to_char( "polearm`C]`w\n\r", ch );
                    break;
                default:
                    send_to_char( "unknown`C]`w\n\r", ch );
                    break;
                }

#endif
#ifdef ALLOW_WEAPON_DAMAGE
                sprintf( buf, "`RDamage is `W%dd%d `C(`Raverage `W%d`C)`w\n\r",
                         obj->value[1], obj->value[2],
                         ( 1 + obj->value[2] ) * obj->value[1] / 2 );
                send_to_char( buf, ch );
#endif
#ifdef ALLOW_WEAPON_FLAGS
                if ( obj->value[4] )    /* weapon flags */
                {
                    sprintf( buf, "`RExtra flags: `C[`W%s`C]`w\n\r",
                             weapon_bit_name( obj->value[4] ) );
                    send_to_char( buf, ch );
                }
#endif
                break;
#endif                          /* End of Weapon Type */

                break;
            }

#endif                          /* End of ITEM_TYPE */

#ifdef ALLOW_EXTRA_DESCR_KEYWORDS
            if ( obj->extra_descr != NULL
                 || obj->pIndexData->extra_descr != NULL )
            {
                EXTRA_DESCR_DATA *ed;
                send_to_char( "`RExtra description keywords: `W'", ch );

                for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
                {
                    send_to_char( ed->keyword, ch );
                    if ( ed->next != NULL )
                        send_to_char( " ", ch );
                }

                for ( ed = obj->pIndexData->extra_descr; ed != NULL;
                      ed = ed->next )
                {
                    send_to_char( ed->keyword, ch );
                    if ( ed->next != NULL )
                        send_to_char( " ", ch );
                }

                send_to_char( "'`w\n\r", ch );
            }
#endif

#ifdef ALLOW_AFFECTS
            for ( paf = obj->affected; paf != NULL; paf = paf->next )
            {
                sprintf( buf, "`RAffects `W%s `Rby `W%d, `Rlevel `W%d.`w\n\r",
                         affect_loc_name( paf->location ), paf->modifier,
                         paf->level );
                send_to_char( buf, ch );
            }

            if ( !obj->enchanted )
                for ( paf = obj->pIndexData->affected; paf != NULL; paf =
                      paf->next )
                {
                    sprintf( buf,
                             "`RAffects `W%s `Rby `W%d, `Rlevel `W%d.`w\n\r",
                             affect_loc_name( paf->location ), paf->modifier,
                             paf->level );
                    send_to_char( buf, ch );
                }
#endif

        }

        return;
    }

    if ( !str_cmp( arg1, "bid" ) )
    {
        long bid;
        obj = auction_info.item;

        if ( !obj )
        {
            send_to_char( "There is nothing up for auction right now.\n\r",
                          ch );
            return;
        }

        if ( argument[0] == '\0' )
        {
            send_to_char( "You must enter an amount to bid.\n\r", ch );
            return;
        }

        bid = atol( argument );

        if ( bid <= auction_info.current_bid )
        {
            sprintf( buf, "You must bid above the current bid of %ld gold.\n\r",
                     auction_info.current_bid );
            return;
        }

        if ( bid < MINIMUM_BID )
        {
            sprintf( buf, "The minimum bid is %d gold.\n\r", MINIMUM_BID );
            send_to_char( buf, ch );
            return;
        }

        if ( ( ch->gold ) < bid )
        {
            send_to_char( "You can't cover that bid.\n\r", ch );
            return;
        }

        if ( auction_info.high_bidder != NULL )
        {
            auction_info.high_bidder->gold += auction_info.gold_held;
        }

        gold = UMIN( ch->gold, bid );

        ch->gold -= gold;

        auction_info.gold_held = gold;
        auction_info.high_bidder = ch;
        auction_info.current_bid = bid;
        auction_info.status = 0;

        auction_channel_bid(  );

        return;
    }

    if ( auction_info.item != NULL )
    {
        send_to_char( "There is already another item up for bid.\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
        send_to_char( "You aren't carrying that item.\n\r", ch );
        return;
    }

    if ( IS_OBJ_STAT( obj, ITEM_NODROP ) )
    {
        send_to_char( "You can't let go of that item.\n\r", ch );
        return;
    }

    auction_info.owner = ch;
    auction_info.item = obj;
    auction_info.current_bid = 0;
    auction_info.status = 0;

    if ( auction_info.item->timer > 0 )
    {
        auction_info.oldtimer = auction_info.item->timer;
        auction_info.item->timer += 16000;
    }

    auction_channel_begin(  );
    obj_from_char( obj );
    return;
}

void auction_update(  )
{
    if ( auction_info.item == NULL )
        return;

    auction_info.status++;

    if ( auction_info.status == AUCTION_LENGTH )
    {
        if ( auction_info.item->timer > 0 )
        {
            auction_info.item->timer = auction_info.oldtimer;
            auction_info.oldtimer = 0;
        }

        auction_channel_sell(  );

        return;
    }

    if ( auction_info.status == AUCTION_LENGTH - 1 )
    {
        auction_channel_twice(  );
        return;
    }

    if ( auction_info.status == AUCTION_LENGTH - 2 )
    {
        if ( auction_info.current_bid == 0 )
        {
            auction_channel_remove(  );
            return;
        }

        auction_channel_once(  );
        return;
    }

    return;
}

void auction_channel_once(  )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, CFG_AUC_ONCE, auction_info.current_bid );

    /* Send to everyone that should hear it */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->original ? d->original : d->character;

        if ( d->connected == CON_PLAYING &&
             !IS_SET( victim->comm, COMM_NOAUCTION ) &&
             !IS_SET( victim->comm, COMM_QUIET ) )
        {
            act_new( buf, auction_info.high_bidder, auction_info.item, victim,
                     TO_VICT, MIN_POS_AUCTION );
        }
    }

    /* Send to the bidder */
    act_new( buf, auction_info.high_bidder, auction_info.item,
             auction_info.high_bidder, TO_CHAR, MIN_POS_AUCTION );
}

void auction_channel_twice(  )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, CFG_AUC_TWICE, auction_info.current_bid );

    /* Send to everyone that should hear it */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->original ? d->original : d->character;

        if ( d->connected == CON_PLAYING &&
             !IS_SET( victim->comm, COMM_NOAUCTION ) &&
             !IS_SET( victim->comm, COMM_QUIET ) )
        {
            act_new( buf, auction_info.high_bidder, auction_info.item, victim,
                     TO_VICT, MIN_POS_AUCTION );
        }
    }

    /* Send to the bidder */
    act_new( buf, auction_info.high_bidder, auction_info.item,
             auction_info.high_bidder, TO_CHAR, MIN_POS_AUCTION );
}

void auction_channel_bid(  )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, CFG_AUC_BID, auction_info.current_bid );

    /* Send to everyone that should hear it */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->original ? d->original : d->character;

        if ( d->connected == CON_PLAYING &&
             !IS_SET( victim->comm, COMM_NOAUCTION ) &&
             !IS_SET( victim->comm, COMM_QUIET ) )
        {
            act_new( buf, auction_info.high_bidder, auction_info.item, victim,
                     TO_VICT, MIN_POS_AUCTION );
        }
    }

    /* Send to the bidder */
    act_new( buf, auction_info.high_bidder, auction_info.item,
             auction_info.high_bidder, TO_CHAR, MIN_POS_AUCTION );
}

void auction_channel_begin(  )
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->original ? d->original : d->character;

        if ( d->connected == CON_PLAYING &&
             !IS_SET( victim->comm, COMM_NOAUCTION ) &&
             !IS_SET( victim->comm, COMM_QUIET ) )
        {
            act_new( CFG_AUC_BEGIN, auction_info.owner, auction_info.item,
                     victim, TO_VICT, MIN_POS_AUCTION );
        }
    }

    /* Send to the seller */
    act_new( CFG_AUC_BEGIN, auction_info.owner, auction_info.item,
             auction_info.owner, TO_CHAR, MIN_POS_AUCTION );
}

void auction_channel_sell(  )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, CFG_AUC_SELL, auction_info.current_bid );

    /* Send to everyone that should hear it */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->original ? d->original : d->character;

        if ( victim != auction_info.high_bidder &&
             d->connected == CON_PLAYING &&
             !IS_SET( victim->comm, COMM_NOAUCTION ) &&
             !IS_SET( victim->comm, COMM_QUIET ) )
        {
            act_new( buf, auction_info.high_bidder, auction_info.item, victim,
                     TO_VICT, MIN_POS_AUCTION );
        }
    }

    /* Give the item to the winner */
    auction_info.owner->gold += auction_info.gold_held;
    obj_to_char( auction_info.item, auction_info.high_bidder );
    act_new( CFG_AUC_TOWINNER, auction_info.high_bidder, auction_info.item,
             auction_info.high_bidder, TO_CHAR, MIN_POS_AUCTION );

    /* Reset the auction */
    auction_info.item = NULL;
    auction_info.owner = NULL;
    auction_info.high_bidder = NULL;
    auction_info.current_bid = 0;
    auction_info.status = 0;
    auction_info.gold_held = 0;
}

void auction_channel_remove(  )
{
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->original ? d->original : d->character;

        if ( d->connected == CON_PLAYING &&
             !IS_SET( victim->comm, COMM_NOAUCTION ) &&
             !IS_SET( victim->comm, COMM_QUIET ) )
        {
            act_new( CFG_AUC_REMOVE, auction_info.owner, auction_info.item,
                     victim, TO_VICT, MIN_POS_AUCTION );
        }
    }

    obj_to_char( auction_info.item, auction_info.owner );
    act_new( CFG_AUC_REMOVE, auction_info.owner, auction_info.item,
             auction_info.owner, TO_CHAR, MIN_POS_AUCTION );
    act_new( CFG_AUC_RETURNED, auction_info.owner, auction_info.item, NULL,
             TO_CHAR, MIN_POS_AUCTION );

    auction_info.item = NULL;
    auction_info.owner = NULL;
    auction_info.current_bid = 0;
    auction_info.status = 0;
}

/*
 * Show_obj_stats: code taken from stock identify spell (-Brian)
 */
void show_obj_stats( int sn, int level, CHAR_DATA * ch, void *vo )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj = ( OBJ_DATA * ) vo;
    AFFECT_DATA *paf;

    sprintf( buf,
             "Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\r",
             obj->name,
             item_type_name( obj ), extra_bit_name( obj->extra_flags ),
/*	extra2_bit_name( obj->extra2_flags ),*//* To be added later.-Lancelight */
/*	extra3_bit_name( obj->extra3_flags ),*/
             obj->weight, obj->cost, obj->level );
    send_to_char( buf, ch );

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
        sprintf( buf, "Damage is %dd%d (average %d).\n\r",
                 obj->value[1], obj->value[2],
                 ( 1 + obj->value[2] ) * obj->value[1] / 2 );
        send_to_char( buf, ch );
        break;

    case ITEM_ARMOR:
        sprintf( buf,
                 "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r",
                 obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
        send_to_char( buf, ch );
        break;
    }

    if ( !obj->enchanted )
        for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
        {
            if ( paf->location != APPLY_NONE && paf->modifier != 0 )
            {
                sprintf( buf, "Affects %s by %d.\n\r",
                         affect_loc_name( paf->location ), paf->modifier );
                send_to_char( buf, ch );
            }
        }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
        if ( paf->location != APPLY_NONE && paf->modifier != 0 )
        {
            sprintf( buf, "Affects %s by %d.\n\r",
                     affect_loc_name( paf->location ), paf->modifier );
            send_to_char( buf, ch );
        }
    }

    return;
}
