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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

#define MAX_DAMAGE_MESSAGE 35

/* command procedures needed */
DECLARE_DO_FUN( do_emote );
DECLARE_DO_FUN( do_berserk );
DECLARE_DO_FUN( do_bash );
DECLARE_DO_FUN( do_trip );
DECLARE_DO_FUN( do_dirt );
DECLARE_DO_FUN( do_flee );
DECLARE_DO_FUN( do_kick );
DECLARE_DO_FUN( do_blackjack );
DECLARE_DO_FUN( do_disarm );
DECLARE_DO_FUN( do_get );
DECLARE_DO_FUN( do_recall );
DECLARE_DO_FUN( do_yell );
DECLARE_DO_FUN( do_sacrifice );

/*
 * Local functions.
 */
void check_assist args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool check_block args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool check_dodge args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void check_killer args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool check_parry args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void dam_message args( ( CHAR_DATA * ch, CHAR_DATA * victim, int dam,
                         int dt, bool immune ) );
void death_cry args( ( CHAR_DATA * ch ) );
void group_gain args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
int xp_compute args( ( CHAR_DATA * gch, CHAR_DATA * victim,
                       int total_levels, int members ) );
int hit_xp_compute args( ( CHAR_DATA * gch, CHAR_DATA * victim,
                           int total_levels, int members, int dam ) );
int cast_xp_compute args( ( CHAR_DATA * gch, CHAR_DATA * victim,
                            int total_levels, int members, int dam ) );
bool is_safe args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void make_corpse args( ( CHAR_DATA * ch ) );
void make_pk_corpse args( ( CHAR_DATA * ch ) );
void one_hit
args( ( CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * weapon, int dt ) );
void mob_hit args( ( CHAR_DATA * ch, CHAR_DATA * victim, int dt ) );
void chaos_kill args( ( CHAR_DATA * victim ) );
void pk_kill args( ( CHAR_DATA * victim ) );
void raw_kill args( ( CHAR_DATA * victim ) );
void set_fighting args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void disarm
args( ( CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * target_weapon ) );
void chaos_log args( ( CHAR_DATA * ch, char *argument ) );
bool vorpal_kill( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
                  int dam_type );
extern bool chaos;
extern bool gsilentdamage;
extern bool can_use( CHAR_DATA * ch, long sn );

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *victim;

    for ( ch = char_list; ch != NULL; ch = ch->next )
    {
        ch_next = ch->next;

        if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
            continue;

        if ( IS_AWAKE( ch ) && ch->in_room == victim->in_room )
            multi_hit( ch, victim, TYPE_UNDEFINED );
        else
        {
            stop_fighting( ch, FALSE );
        }

        if ( ( victim = ch->fighting ) == NULL )
            continue;

        rprog_rfight_trigger( ch );
        mprog_hitprcnt_trigger( ch, victim );
        mprog_fightgroup_trigger( ch );
        mprog_fight_trigger( ch, victim );

        /*
         * Fun for the whole family!
         */
        check_assist( ch, victim );
    }

    return;
}

/* for auto assisting */
void check_assist( CHAR_DATA * ch, CHAR_DATA * victim )
{
    CHAR_DATA *rch, *rch_next;

    for ( rch = ch->in_room->people; rch != NULL; rch = rch_next )
    {
        rch_next = rch->next_in_room;

        if ( IS_AWAKE( rch ) && rch->fighting == NULL )
        {
            /* quick check for ASSIST_PLAYER */
            if ( !IS_NPC( ch ) && IS_NPC( rch )
                 && IS_SET( rch->off_flags, ASSIST_PLAYERS )
                 && rch->level + 6 > victim->level
                 && ( !chaos && !IS_SET( ch->act, PLR_KILLER ) ) )
            {
                do_emote( rch, "screams and attacks!" );
                multi_hit( rch, victim, TYPE_UNDEFINED );
                continue;
            }

            /* PCs next */
            if ( ( !IS_NPC( ch ) && IS_NPC( victim ) )
                 || ( IS_AFFECTED( ch, AFF_CHARM ) ) )
            {
                if ( ( ( !IS_NPC( rch ) && IS_SET( rch->act, PLR_AUTOASSIST ) )
                       || IS_AFFECTED( rch, AFF_CHARM ) )
                     && is_same_group( ch, rch ) )
                    multi_hit( rch, victim, TYPE_UNDEFINED );

                continue;
            }

            /* now check the NPC cases */

            if ( IS_NPC( ch ) && !IS_AFFECTED( ch, AFF_CHARM ) )

            {
                if ( ( IS_NPC( rch ) && IS_SET( rch->off_flags, ASSIST_ALL ) )
                     || ( IS_NPC( rch ) && rch->race == ch->race
                          && IS_SET( rch->off_flags, ASSIST_RACE ) )
                     || ( IS_NPC( rch )
                          && IS_SET( rch->off_flags, ASSIST_ALIGN )
                          && ( ( IS_GOOD( rch ) && IS_GOOD( ch ) )
                               || ( IS_EVIL( rch ) && IS_EVIL( ch ) )
                               || ( IS_NEUTRAL( rch ) && IS_NEUTRAL( ch ) ) ) )
                     || ( rch->pIndexData == ch->pIndexData
                          && IS_SET( rch->off_flags, ASSIST_VNUM ) ) )

                {
                    CHAR_DATA *vch;
                    CHAR_DATA *target;
                    int number;

                    if ( number_bits( 1 ) == 0 )
                        continue;

                    target = NULL;
                    number = 0;
                    for ( vch = ch->in_room->people; vch; vch = vch->next )
                    {
                        if ( can_see( rch, vch )
                             && is_same_group( vch, victim )
                             && number_range( 0, number ) == 0 )
                        {
                            target = vch;
                            number++;
                        }
                    }

                    if ( target != NULL )
                    {
                        do_emote( rch, "screams and attacks!" );
                        multi_hit( rch, target, TYPE_UNDEFINED );
                    }
                }
            }
        }
    }
}

/* AUTO_HATE - The following three functions are only called
   if you have defined AUTO_HATE on in the config.h file
*/

bool is_hating( CHAR_DATA * ch, CHAR_DATA * victim )
{
    if ( !ch->hate || ch->hate->who != victim )
        return FALSE;
    return TRUE;
}

void stop_hating( CHAR_DATA * ch )
{
    if ( ch->hate )
    {
        free_string( &ch->hate->name );
        free_mem( &ch->hate );
        ch->hate = NULL;
    }
    return;
}

void start_hating( CHAR_DATA * ch, CHAR_DATA * victim )
{
    if ( ch == victim )
        return;

    if ( ch->hate )
        stop_hating( ch );

    ch->hate = alloc_mem( sizeof( HATE_DATA ) );
    ch->hate->name = str_dup( victim->name );
    ch->hate->who = victim;
    return;
}

/* End Hatred Items */

/*
 * Do one group of attacks.
 */
void multi_hit( CHAR_DATA * ch, CHAR_DATA * victim, int dt )
{
    OBJ_DATA *weapon;
    OBJ_DATA *second_weapon;
    int chance;

    /* decrement the wait */
    if ( ch->desc == NULL )
        ch->wait = UMAX( 0, ch->wait - PULSE_VIOLENCE );

    /* no attacks for stunnies -- just a check */
    if ( ch->position < POS_RESTING )
        return;

    if ( IS_NPC( ch ) )
    {
        mob_hit( ch, victim, dt );
        return;
    }

    weapon = get_eq_char( ch, WEAR_WIELD );
    one_hit( ch, victim, weapon, dt );
    if ( ( second_weapon = get_eq_char( ch, WEAR_SECOND_WIELD ) ) != NULL )
        one_hit( ch, victim, second_weapon, dt );

    if ( ch->fighting != victim )
        return;

    if ( IS_AFFECTED( ch, AFF_HASTE ) )
        one_hit( ch, victim, weapon, dt );

    if ( ch->fighting != victim || dt == gsn_backstab )
        return;

    if ( ch->fighting != victim || dt == gsn_circle )
        return;

    chance = get_skill( ch, gsn_second_attack );
    if ( number_percent(  ) < chance )
    {
        one_hit( ch, victim, weapon, dt );
        check_improve( ch, gsn_second_attack, TRUE, 5 );
        if ( ch->fighting != victim )
            return;
    }

    chance = get_skill( ch, gsn_third_attack ) / 2;
    if ( number_percent(  ) < chance )
    {
        one_hit( ch, victim, weapon, dt );
        check_improve( ch, gsn_third_attack, TRUE, 6 );
        if ( ch->fighting != victim )
            return;
    }
#ifdef FOURTH_ATTACK
    chance = get_skill( ch, gsn_fourth_attack ) / 4;
    if ( number_percent(  ) < chance )
    {
        one_hit( ch, victim, weapon, dt );
        check_improve( ch, gsn_fourth_attack, TRUE, 6 );
        if ( ch->fighting != victim )
            return;
    }
#endif
    return;
}

/* procedure for all mobile attacks */
void mob_hit( CHAR_DATA * ch, CHAR_DATA * victim, int dt )
{
    OBJ_DATA *weapon;
    OBJ_DATA *second_weapon;
    int chance, number;
    CHAR_DATA *vch, *vch_next;

    weapon = get_eq_char( ch, WEAR_WIELD );
    one_hit( ch, victim, weapon, dt );
    if ( ( second_weapon = get_eq_char( ch, WEAR_SECOND_WIELD ) ) != NULL )
        one_hit( ch, victim, second_weapon, dt );

    if ( ch->fighting != victim )
        return;

    /* Area attack -- BALLS nasty! */

    if ( IS_SET( ch->off_flags, OFF_AREA_ATTACK ) )
    {
        for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;
            if ( ( vch != victim && vch->fighting == ch ) )
                one_hit( ch, vch, weapon, dt );
        }
    }

    if ( IS_AFFECTED( ch, AFF_HASTE ) || IS_SET( ch->off_flags, OFF_FAST ) )
        one_hit( ch, victim, weapon, dt );

    if ( ch->fighting != victim || dt == gsn_backstab )
        return;

    if ( ch->fighting != victim || dt == gsn_circle )
        return;

    chance = get_skill( ch, gsn_second_attack );
    if ( number_percent(  ) < chance )
    {
        one_hit( ch, victim, weapon, dt );
        if ( ch->fighting != victim )
            return;
    }

    chance = get_skill( ch, gsn_third_attack ) / 2;
    if ( number_percent(  ) < chance )
    {
        one_hit( ch, victim, weapon, dt );
        if ( ch->fighting != victim )
            return;
    }

    /* oh boy!  Fun stuff! */

    if ( ch->wait > 0 )
        return;

    number = number_range( 0, 2 );

    /*if (number == 1 && IS_SET(ch->act,ACT_MAGE))
       { mob_cast_mage(ch,victim); return; }; */

    /*if (number == 2 && IS_SET(ch->act,ACT_CLERIC))
       { mob_cast_cleric(ch,victim); return; }; */

    /* now for the skills */

    number = number_range( 0, 7 );

    switch ( number )
    {
    case ( 0 ):
        if ( IS_SET( ch->off_flags, OFF_BASH ) )
            do_bash( ch, "" );
        break;

    case ( 1 ):
        if ( IS_SET( ch->off_flags, OFF_BERSERK )
             && !IS_AFFECTED( ch, AFF_BERSERK ) )
            do_berserk( ch, "" );
        break;

    case ( 2 ):
        if ( IS_SET( ch->off_flags, OFF_DISARM )
             || ( get_weapon_sn( ch ) != gsn_hand_to_hand
                  && ( IS_SET( ch->act, ACT_WARRIOR )
                       || IS_SET( ch->act, ACT_THIEF ) ) ) )
            do_disarm( ch, "" );
        break;

    case ( 3 ):
        if ( IS_SET( ch->off_flags, OFF_KICK ) )
            do_kick( ch, "" );
        break;

    case ( 4 ):
        if ( IS_SET( ch->off_flags, OFF_KICK_DIRT ) )
            do_dirt( ch, "" );
        break;

        /*case (5) :
           if (IS_SET(ch->off_flags,OFF_TAIL))
           do_tail(ch,"");
           break; */

    case ( 6 ):
        if ( IS_SET( ch->off_flags, OFF_TRIP ) )
            do_trip( ch, "" );
        break;

        /*case (7) :
           if (IS_SET(ch->off_flags,OFF_CRUSH))
           do_crush(ch,"");
           break; */
    }
}

/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * weapon, int dt )
{
    bool counter;
    bool damaged = FALSE;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int dam;
    int diceroll;
    int sn, skill;
    int dam_type;
    int gsnnum;
    sn = -1;
    gsnnum = 0;

    /* just in case */
    if ( victim == ch || ch == NULL || victim == NULL )
        return;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
        return;

    /*
     * Figure out the type of damage message.
     */
    if ( dt == TYPE_UNDEFINED )
    {
        dt = TYPE_HIT;
        if ( weapon != NULL && weapon->item_type == ITEM_WEAPON )
            dt += weapon->value[3];
        else
            dt += ch->dam_type;
    }

    if ( dt < TYPE_HIT )
        if ( weapon != NULL )
            dam_type = attack_table[weapon->value[3]].damage;
        else
            dam_type = attack_table[ch->dam_type].damage;
    else
        dam_type = attack_table[dt - TYPE_HIT].damage;

    if ( dam_type == -1 )
        dam_type = DAM_BASH;

    /* get the weapon skill */
    sn = get_weapon_sn( ch );
    skill = 20 + get_weapon_skill( ch, sn );

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC( ch ) )
    {
        thac0_00 = 20;
        thac0_32 = -4;          /* as good as a thief */
        if ( IS_SET( ch->act, ACT_WARRIOR ) )
            thac0_32 = -10;
        else if ( IS_SET( ch->act, ACT_THIEF ) )
            thac0_32 = -4;
        else if ( IS_SET( ch->act, ACT_CLERIC ) )
            thac0_32 = 2;
        else if ( IS_SET( ch->act, ACT_MAGE ) )
            thac0_32 = 6;
    }
    else
    {
        thac0_00 = class_table[ch->Class].thac0_00;
        thac0_32 = class_table[ch->Class].thac0_32;
    }

    thac0 = interpolate( ch->level, thac0_00, thac0_32 );

    thac0 -= GET_HITROLL( ch ) * skill / 100;
    thac0 += 5 * ( 100 - skill ) / 100;

    if ( dt == gsn_backstab )
        thac0 -= 10 * ( 100 - get_skill( ch, gsn_backstab ) );

    if ( dt == gsn_circle )
        thac0 -= 10 * ( 100 - get_skill( ch, gsn_circle ) );

    switch ( dam_type )
    {
    case ( DAM_PIERCE ):
        victim_ac = GET_AC( victim, AC_PIERCE ) / 10;
        break;
    case ( DAM_BASH ):
        victim_ac = GET_AC( victim, AC_BASH ) / 10;
        break;
    case ( DAM_SLASH ):
        victim_ac = GET_AC( victim, AC_SLASH ) / 10;
        break;
    default:
        victim_ac = GET_AC( victim, AC_EXOTIC ) / 10;
        break;
    };

    if ( victim_ac < -15 )
        victim_ac = ( victim_ac + 15 ) / 5 - 15;

    if ( !can_see( ch, victim ) )
    {
        if ( ch->level > skill_table[gsn_blind_fighting].skill_level[ch->Class]
             && number_percent(  ) < get_skill( ch, gsn_blind_fighting ) )
        {
            check_improve( ch, gsn_blind_fighting, TRUE, 16 );
        }
        else
            victim_ac -= 4;
    }

    if ( victim->position < POS_FIGHTING )
        victim_ac += 4;

    if ( victim->position < POS_RESTING )
        victim_ac += 6;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
        ;

    if ( diceroll == 0 || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
        /* Miss. */
        damage( ch, victim, NULL, 0, dt, dam_type );
        tail_chain(  );
        return;
    }

    /*
     * Hit.
     * Calc damage.
     */
    if ( IS_NPC( ch ) && ( weapon == NULL ) )
        dam = dice( ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE] );

    else
    {
        if ( sn != -1 )
            check_improve( ch, sn, TRUE, 5 );
        if ( weapon != NULL )
        {
            dam = dice( weapon->value[1], weapon->value[2] ) * skill / 100;

            if ( get_eq_char( ch, WEAR_SHIELD ) == NULL )   /* no shield = more */
                dam = dam * 21 / 20;
        }
        else
            dam =
                number_range( 1 + 4 * skill / 100,
                              2 * ch->level / 3 * skill / 100 );
    }

    /*
     * Bonuses.
     */
    if ( get_skill( ch, gsn_enhanced_damage ) > 0 )
    {
        diceroll = number_percent(  );
        if ( diceroll <= get_skill( ch, gsn_enhanced_damage ) )
        {
            check_improve( ch, gsn_enhanced_damage, TRUE, 6 );
            dam += dam * diceroll / 100;
        }
    }

    if ( get_skill( victim, gsn_counter ) > 0 )
    {
        gsnnum = get_skill( victim, gsn_counter );

        /*
         * At most counter will only happen MAX_COUNTER_PERCENTAGE
         */
        if ( number_percent(  ) <= gsnnum
             && number_percent(  ) <= UMIN( MAX_COUNTER_PERCENTAGE, 99 ) )
            counter = TRUE;
        else
        {
            counter = FALSE;
            /* Every 20th failed counter check and see if counter improves */
            if ( number_percent(  ) <= 5 )
                check_improve( victim, gsn_counter, FALSE, 1 );
        }
    }
    else
        counter = FALSE;

    if ( !IS_AWAKE( victim ) )
        dam *= 2;
    else if ( victim->position < POS_FIGHTING )
        dam = dam * 3 / 2;

    if ( dt == gsn_backstab && weapon != NULL )
    {
        if ( weapon->value[0] != 2 )
            dam *= 2 + ch->level / 10;
        else
            dam *= 2 + ch->level / 8;
    }

    if ( dt == gsn_circle && weapon != NULL )
    {
        if ( weapon->value[0] != 2 )
            dam *= 2 + ch->level / 10;
        else
            dam *= 2 + ch->level / 8;
    }

    dam += GET_DAMROLL( ch ) * UMIN( 100, skill ) / 100;

    if ( dam <= 0 )
        dam = 1;

    if ( counter == TRUE )
    {
        OBJ_DATA *counter_weapon;
        OBJ_DATA *counter_second_weapon;
        act( "$n counters your attack!", victim, NULL, ch, TO_VICT );
        act( "You counter $N's attack!", victim, NULL, ch, TO_CHAR );
        act( "$n counters $N's attack!", victim, NULL, ch, TO_NOTVICT );
        counter_weapon = get_eq_char( victim, WEAR_WIELD );
        one_hit( victim, ch, counter_weapon, TYPE_UNDEFINED );
        if ( ( counter_second_weapon =
               get_eq_char( victim, WEAR_SECOND_WIELD ) ) != NULL )
            one_hit( victim, ch, counter_second_weapon, TYPE_UNDEFINED );
        check_improve( victim, gsn_counter, TRUE, 1 );
    }
    else
    {
        damaged = damage( ch, victim, weapon, dam, dt, dam_type );

        if ( damaged && ( weapon != NULL && ch->fighting == victim ) )
        {

            if ( IS_WEAPON_STAT( weapon, WEAPON_VAMPIRIC ) )
            {
                dam = number_range( 1, weapon->level / 5 + 1 );
                act( "$p draws life from $n.", victim, weapon, NULL, TO_ROOM );
                act( "You feel $p drawing your life away.",
                     victim, weapon, NULL, TO_CHAR );
                damage( ch, victim, NULL, dam, 1033, DAM_NEGATIVE );
                ch->alignment = UMAX( -1000, ch->alignment - 1 );
                ch->hit += dam / 2;
            }

            if ( IS_WEAPON_STAT( weapon, WEAPON_FLAMING ) )
            {
                dam = number_range( 1, weapon->level / 4 + 1 );
                act( "$n is burned by $p.", victim, weapon, NULL, TO_ROOM );
                act( "$p sears your flesh.", victim, weapon, NULL, TO_CHAR );
                damage( ch, victim, NULL, dam, 1034, DAM_FIRE );
            }

            if ( IS_WEAPON_STAT( weapon, WEAPON_FROST ) )
            {
                dam = number_range( 1, weapon->level / 6 + 2 );
                act( "$p freezes $n.", victim, weapon, NULL, TO_ROOM );
                act( "The cold touch of $p surrounds you with ice.",
                     victim, weapon, NULL, TO_CHAR );
                damage( ch, victim, NULL, dam, 1035, DAM_COLD );
            }
            if ( IS_WEAPON_STAT( weapon, WEAPON_VORPAL )
                 && number_range( 1,
                                  UMAX( 50,
                                        ( 100 + MAX_LEVEL - ( 2 * ch->level ) +
                                          victim->level ) ) ) == 1
                 && IS_SET( victim->parts, PART_HEAD )
                 && check_immune( victim, dam_type ) != IS_IMMUNE
                 && !IS_IMMORTAL( victim ) && ch != victim
                 && !is_safe( ch, victim ) )
            {
                char buf[MAX_STRING_LENGTH];
                char buf2[MAX_STRING_LENGTH];
                OBJ_DATA *obj;
                char *name;
                int parts_buf;

                /* used a modified version of death_cry to make a severed head */

                name = IS_NPC( victim ) ? victim->short_descr : victim->name;
                obj =
                    create_object( get_obj_index( OBJ_VNUM_SEVERED_HEAD ), 0 );
                obj->timer = number_range( 20, 30 );

                sprintf( buf, obj->short_descr, name );
                free_string( &obj->short_descr );
                obj->short_descr = str_dup( buf );

                sprintf( buf, obj->description, name );
                free_string( &obj->description );
                obj->description = str_dup( buf );

                /* yummy... severed head for dinner! */

                if ( obj->item_type == ITEM_FOOD )
                {
                    if ( IS_SET( victim->form, FORM_POISON ) )
                        obj->value[3] = 1;
                    else if ( !IS_SET( victim->form, FORM_EDIBLE ) )
                        obj->item_type = ITEM_TRASH;
                }

                obj_to_room( obj, ch->in_room );
                /* change this if you want a more or less gory death message! */
                act( "$n's head is severed from $s body by $p!", victim, weapon,
                     NULL, TO_ROOM );
                sprintf( buf2,
                         "Your last feeling before you die is %s's weapon severing your head.",
                         ( IS_NPC( ch ) ? ch->short_descr : ch->name ) );
                send_to_char( buf, victim );    /* send_to_char used to prevent a crash */
                /* parts_buf is used to remove all parts of the body so the death_cry 
                   function does not create any other body parts */

                parts_buf = victim->parts;
                victim->parts = 0;
                stop_hating( ch );
                vorpal_kill( ch, victim, dam, dt, dam_type );
                victim->parts = parts_buf;
            }

        }
    }

    tail_chain(  );
    return;
}

/*
 * Inflict damage from a hit.
 */
bool damage( CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * weapon, int dam,
             int dt, int dam_type )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    bool immune;
    extern bool chaos;
    int chaos_points;

    if ( victim->position == POS_DEAD )
        return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > MAX_MORTAL_WEAPON_DAMAGE && !IS_IMMORTAL( ch ) && !IS_NPC( ch ) )
    {
        bug( "Damage: %s: more than %d points!", ch->name,
             MAX_MORTAL_WEAPON_DAMAGE );
        dam = 0;

        if ( weapon )
            extract_obj( weapon );

        send_to_char( "You really shouldn't cheat.\n\r", ch );
        return FALSE;
    }

    if ( victim != ch )
    {
        /*
         * Certain attacks are forbidden.
         * Most other attacks are returned.
         */
        if ( is_safe( ch, victim ) )
            return FALSE;
        check_killer( ch, victim );

        if ( victim->position > POS_STUNNED )
        {
            if ( victim->fighting == NULL )
                set_fighting( victim, ch );
            if ( victim->timer <= 4 )
                victim->position = POS_FIGHTING;
        }

        if ( victim->position > POS_STUNNED )
        {
            if ( ch->fighting == NULL )
                set_fighting( ch, victim );

            /*
             * If victim is charmed, ch might attack victim's master.
             */
            if ( IS_NPC( ch )
                 && IS_NPC( victim )
                 && IS_AFFECTED( victim, AFF_CHARM )
                 && victim->master != NULL
                 && victim->master->in_room == ch->in_room
                 && number_bits( 3 ) == 0 )
            {
                stop_fighting( ch, FALSE );
                multi_hit( ch, victim->master, TYPE_UNDEFINED );
                return FALSE;
            }
        }

        /*
         * More charm stuff.
         */
        if ( victim->master == ch )
            stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */
    if ( IS_AFFECTED( ch, AFF_INVISIBLE ) )
    {
        affect_strip( ch, gsn_invis );
        affect_strip( ch, gsn_mass_invis );
        REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
        act( "`K$n fades into existence.`w", ch, NULL, NULL, TO_ROOM );
    }
#ifdef AUTO_HATE
    if ( victim->hate && ( ch != victim ) )
    {
        free_string( &victim->hate->name );
        victim->hate->name = str_dup( ch->name );
        victim->hate->who = ch;
    }
    else
        start_hating( victim, ch );

#endif

    /*
     * Damage modifiers.
     */
    if ( IS_AFFECTED( victim, AFF_SANCTUARY ) )
        dam /= 2;

    if ( IS_AFFECTED( victim, AFF_PROTECT ) && IS_EVIL( ch ) )
        dam -= dam / 4;

    immune = FALSE;

    /*
     * Check for parry, and dodge.
     */
    if ( dt >= TYPE_HIT && ch != victim )
    {
        if ( check_block( ch, victim ) )
            return FALSE;
        if ( check_parry( ch, victim ) )
            return FALSE;
        if ( check_dodge( ch, victim ) )
            return FALSE;
    }

    if ( weapon && dam > 0 )
        oprog_hit_trigger( ch, victim, weapon );

    switch ( check_immune( victim, dam_type ) )
    {
    case ( IS_IMMUNE ):
        immune = TRUE;
        dam = 0;
        break;
    case ( IS_RESISTANT ):
        dam -= dam / 3;
        break;
    case ( IS_VULNERABLE ):
        dam += dam / 2;
        break;
    }

    if ( !gsilentdamage )
        dam_message( ch, victim, dam, dt, immune );

    if ( dam == 0 )
        return FALSE;

    /* Ok, give the ch xp for his hit and add to ch->exp_stack */
    if ( !IS_NPC( ch ) )
    {
        int xp = 0;
        int members = 0;
        int group_levels = 0;
        CHAR_DATA *gch;

        for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
        {
            if ( is_same_group( gch, ch ) )
            {
                members++;
                group_levels += gch->level;
            }
        }

        xp = hit_xp_compute( ch, victim, group_levels, members,
                             UMIN( dam, victim->hit + 20 ) );
        ch->exp_stack += xp;
        gain_exp( ch, xp );
    }

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC( victim )
         && victim->level >= LEVEL_IMMORTAL && victim->hit < 1 )
        victim->hit = 1;
    update_pos( victim );

    switch ( victim->position )
    {
    case POS_MORTAL:
        act( "`R$n is mortally wounded, and will die soon, if not aided.`w",
             victim, NULL, NULL, TO_ROOM );
        send_to_char
            ( "`RYou are mortally wounded, and will die soon, if not aided.\n\r`w",
              victim );
        break;

    case POS_INCAP:
        act( "`R$n is incapacitated and will slowly die, if not aided.`w",
             victim, NULL, NULL, TO_ROOM );
        send_to_char
            ( "`RYou are incapacitated and will slowly die, if not aided.\n\r`w",
              victim );
        break;

    case POS_STUNNED:
        act( "`R$n is stunned, but will probably recover.`w",
             victim, NULL, NULL, TO_ROOM );
        send_to_char( "`RYou are stunned, but will probably recover.\n\r`w",
                      victim );
        break;

    case POS_DEAD:
        rprog_death_trigger( victim );
        mprog_death_trigger( victim );
        act( "`R$n is DEAD!!`w", victim, 0, 0, TO_ROOM );
        send_to_char( "`RYou have been KILLED!!\n\r\n\r`w", victim );
        break;

    default:
        if ( dam > victim->max_hit / 4 )
            send_to_char( "`RThat really did HURT`R!\n\r`w", victim );
        if ( victim->hit < victim->max_hit / 4 )
            send_to_char( "`RYou sure are BLEEDING`R!\n\r`w", victim );
        break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE( victim ) )
        stop_fighting( victim, FALSE );

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
        group_gain( ch, victim );

        if ( !IS_NPC( victim ) )
        {
            sprintf( log_buf, "%s killed by %s at %d",
                     victim->name,
                     ( IS_NPC( ch ) ? ch->short_descr : ch->name ),
                     victim->in_room->vnum );
            log_string( log_buf );
            if ( !IS_IMMORTAL( ch ) )
            {
                free_string( &victim->pcdata->nemesis );
                victim->pcdata->nemesis =
                    str_dup( IS_NPC( ch ) ? ch->short_descr : ch->name );
            }

            /*
             * Dying penalty:
             * 1/2 way back to previous level.
             */
            if ( !chaos && victim->exp > 0 && IS_NPC( ch ) )
                gain_exp( victim, -1 * ( victim->exp / 2 ) );
        }
        if ( chaos )
        {
            chaos_points = 0;

            if ( ch->level < victim->level )
                chaos_points = 2 * ( victim->level - ch->level );

            chaos_points = chaos_points + victim->level;
            ch->pcdata->chaos_score = chaos_points;
        }

#ifdef AUTO_HATE
        if ( !IS_NPC( victim ) && IS_NPC( ch ) )
            stop_hating( ch );
#endif

        if ( !IS_NPC( victim ) )
        {
            sprintf( buf, "%s has been slain by %s!", victim->name,
                     IS_NPC( ch ) ? ch->short_descr : ch->name );
            do_sendinfo( ch, buf );
        }

        if ( chaos && !IS_NPC( victim ) )
            chaos_kill( victim );
        else if ( !( !IS_NPC( victim ) && !IS_NPC( ch ) ) )
        {
            affect_factions( ch, victim );
            raw_kill( victim );
        }
        else
        {
            pk_kill( victim );

            if ( !IS_IMMORTAL( ch ) )
            {
                victim->pcdata->pk_deaths++;
                ch->pcdata->pk_kills++;
            }
        }
        /* RT new auto commands */

        if ( !IS_NPC( ch ) && IS_NPC( victim ) )
        {
            corpse = get_obj_list( ch, "corpse", ch->in_room->contents );

            if ( IS_SET( ch->act, PLR_AUTOLOOT ) && corpse && corpse->contains )    /* exists and not empty */
                do_get( ch, "all corpse" );

            if ( IS_SET( ch->act, PLR_AUTOGOLD ) && corpse && corpse->contains &&   /* exists and not empty */
                 !IS_SET( ch->act, PLR_AUTOLOOT ) )
                do_get( ch, "all.gold corpse" );

            if ( IS_SET( ch->act, PLR_AUTOSAC ) )
            {
                if ( IS_SET( ch->act, PLR_AUTOLOOT ) && corpse
                     && corpse->contains )
                    return TRUE;    /* leave if corpse has treasure */
                else
                    do_sacrifice( ch, "corpse" );
            }
        }

        return TRUE;
    }

    if ( victim == ch )
        return TRUE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC( victim ) && victim->desc == NULL )
    {
        if ( number_range( 0, victim->wait ) == 0 )
        {
            do_recall( victim, "" );
            return TRUE;
        }
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC( victim ) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2 )
    {
        if ( ( IS_SET( victim->act, ACT_WIMPY ) && number_bits( 2 ) == 0
               && victim->hit < victim->max_hit / 5 )
             || ( IS_AFFECTED( victim, AFF_CHARM ) && victim->master != NULL
                  && victim->master->in_room != victim->in_room ) )
            do_flee( victim, "" );
    }

    if ( !IS_NPC( victim )
         && victim->hit > 0
         && victim->hit <= victim->wimpy && victim->wait < PULSE_VIOLENCE / 2 )
        do_flee( victim, "" );

    tail_chain(  );
    return TRUE;
}

bool new_damage( CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * weapon, int dam,
                 int dt, int dam_type, bool show )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    bool immune;
    extern bool chaos;
    int chaos_points;

    if ( victim->position == POS_DEAD )
        return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if ( dam > MAX_MORTAL_WEAPON_DAMAGE && !IS_IMMORTAL( ch ) && !IS_NPC( ch ) )
    {
        bug( "Damage: %s: more than %d points!", ch->name,
             MAX_MORTAL_WEAPON_DAMAGE );
        dam = 0;

        if ( weapon )
            extract_obj( weapon );

        send_to_char( "You really shouldn't cheat.\n\r", ch );
        return FALSE;
    }

    if ( victim != ch )
    {
        /*
         * Certain attacks are forbidden.
         * Most other attacks are returned.
         */
        if ( is_safe( ch, victim ) )
            return FALSE;
        check_killer( ch, victim );

        if ( victim->position > POS_STUNNED )
        {
            if ( victim->fighting == NULL )
                set_fighting( victim, ch );
            if ( victim->timer <= 4 )
                victim->position = POS_FIGHTING;
        }

        if ( victim->position > POS_STUNNED )
        {
            if ( ch->fighting == NULL )
                set_fighting( ch, victim );

            /*
             * If victim is charmed, ch might attack victim's master.
             */
            if ( IS_NPC( ch )
                 && IS_NPC( victim )
                 && IS_AFFECTED( victim, AFF_CHARM )
                 && victim->master != NULL
                 && victim->master->in_room == ch->in_room
                 && number_bits( 3 ) == 0 )
            {
                stop_fighting( ch, FALSE );
                multi_hit( ch, victim->master, TYPE_UNDEFINED );
                return FALSE;
            }
        }

        /*
         * More charm stuff.
         */
        if ( victim->master == ch )
            stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */
    if ( IS_AFFECTED( ch, AFF_INVISIBLE ) )
    {
        affect_strip( ch, gsn_invis );
        affect_strip( ch, gsn_mass_invis );
        REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
        act( "`K$n fades into existence.`w", ch, NULL, NULL, TO_ROOM );
    }

    /*
     * Damage modifiers.
     */
    if ( IS_AFFECTED( victim, AFF_SANCTUARY ) )
        dam /= 2;

    if ( IS_AFFECTED( victim, AFF_PROTECT ) && IS_EVIL( ch ) )
        dam -= dam / 4;

    immune = FALSE;

    /*
     * Check for parry, and dodge.
     */
    if ( dt >= TYPE_HIT && ch != victim )
    {
        if ( check_block( ch, victim ) )
            return FALSE;
        if ( check_parry( ch, victim ) )
            return FALSE;
        if ( check_dodge( ch, victim ) )
            return FALSE;
    }

    if ( weapon && dam > 0 )
        oprog_hit_trigger( ch, victim, weapon );

    switch ( check_immune( victim, dam_type ) )
    {
    case ( IS_IMMUNE ):
        immune = TRUE;
        dam = 0;
        break;
    case ( IS_RESISTANT ):
        dam -= dam / 3;
        break;
    case ( IS_VULNERABLE ):
        dam += dam / 2;
        break;
    }

    if ( show )
        dam_message( ch, victim, dam, dt, immune );

    if ( dam == 0 )
        return FALSE;

    /* Ok, give the ch xp for his hit and add to ch->exp_stack */
    if ( ( !IS_NPC( ch ) ) && ( victim != ch ) )    /* not NPCs, and no xp for hitting self */
    {
        int xp = 0;
        int members = 0;
        int group_levels = 0;
        CHAR_DATA *gch;

        for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
        {
            if ( is_same_group( gch, ch ) )
            {
                members++;
                group_levels += gch->level;
            }
        }

        xp = hit_xp_compute( ch, victim, group_levels, members,
                             UMIN( dam, victim->hit + 20 ) );
        ch->exp_stack += xp;
        gain_exp( ch, xp );
    }

    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;
    if ( !IS_NPC( victim )
         && victim->level >= LEVEL_IMMORTAL && victim->hit < 1 )
        victim->hit = 1;
    update_pos( victim );

    switch ( victim->position )
    {
    case POS_MORTAL:
        act( "`R$n is mortally wounded, and will die soon, if not aided.`w",
             victim, NULL, NULL, TO_ROOM );
        send_to_char
            ( "`RYou are mortally wounded, and will die soon, if not aided.\n\r`w",
              victim );
        break;

    case POS_INCAP:
        act( "`R$n is incapacitated and will slowly die, if not aided.`w",
             victim, NULL, NULL, TO_ROOM );
        send_to_char
            ( "`RYou are incapacitated and will slowly die, if not aided.\n\r`w",
              victim );
        break;

    case POS_STUNNED:
        act( "`R$n is stunned, but will probably recover.`w",
             victim, NULL, NULL, TO_ROOM );
        send_to_char( "`RYou are stunned, but will probably recover.\n\r`w",
                      victim );
        break;

    case POS_DEAD:
        rprog_death_trigger( victim );
        mprog_death_trigger( victim );
        act( "`R$n is DEAD!!`w", victim, 0, 0, TO_ROOM );
        send_to_char( "`RYou have been KILLED!!\n\r\n\r`w", victim );
        break;

    default:
        if ( dam > victim->max_hit / 4 )
            send_to_char( "`RThat really did HURT`R!\n\r`w", victim );
        if ( victim->hit < victim->max_hit / 4 )
            send_to_char( "`RYou sure are BLEEDING`R!\n\r`w", victim );
        break;
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE( victim ) )
        stop_fighting( victim, FALSE );

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
        group_gain( ch, victim );

        if ( !IS_NPC( victim ) )
        {
            sprintf( log_buf, "%s killed by %s at %d",
                     victim->name,
                     ( IS_NPC( ch ) ? ch->short_descr : ch->name ),
                     victim->in_room->vnum );
            log_string( log_buf );
            if ( !IS_IMMORTAL( ch ) )
            {
                free_string( &victim->pcdata->nemesis );
                victim->pcdata->nemesis =
                    str_dup( IS_NPC( ch ) ? ch->short_descr : ch->name );
            }

            /*
             * Dying penalty:
             * 1/2 way back to previous level.
             */
            if ( !chaos && victim->exp > 0 && IS_NPC( ch ) )
                gain_exp( victim, -1 * ( victim->exp / 2 ) );
        }
        if ( chaos )
        {
            chaos_points = 0;

            if ( ch->level < victim->level )
                chaos_points = 2 * ( victim->level - ch->level );

            chaos_points = chaos_points + victim->level;
            ch->pcdata->chaos_score = chaos_points;
        }

        if ( !IS_NPC( victim ) )
        {
            sprintf( buf, "%s has been slain by %s!", victim->name,
                     IS_NPC( ch ) ? ch->short_descr : ch->name );
            do_sendinfo( ch, buf );
        }

        if ( chaos && !IS_NPC( victim ) )
            chaos_kill( victim );
        else if ( !( !IS_NPC( victim ) && !IS_NPC( ch ) ) )
            raw_kill( victim );
        else
        {
            pk_kill( victim );
            if ( !IS_IMMORTAL( ch ) )
            {
                victim->pcdata->pk_deaths++;
                ch->pcdata->pk_kills++;
            }
        }

        /* RT new auto commands */

        if ( !IS_NPC( ch ) && IS_NPC( victim ) )
        {
            corpse = get_obj_list( ch, "corpse", ch->in_room->contents );

            if ( IS_SET( ch->act, PLR_AUTOLOOT ) && corpse && corpse->contains )    /* exists and not empty */
                do_get( ch, "all corpse" );

            if ( IS_SET( ch->act, PLR_AUTOGOLD ) && corpse && corpse->contains &&   /* exists and not empty */
                 !IS_SET( ch->act, PLR_AUTOLOOT ) )
                do_get( ch, "all.gold corpse" );

            if ( IS_SET( ch->act, PLR_AUTOSAC ) )
            {
                if ( IS_SET( ch->act, PLR_AUTOLOOT ) && corpse
                     && corpse->contains )
                    return TRUE;    /* leave if corpse has treasure */
                else
                    do_sacrifice( ch, "corpse" );
            }
        }

        return TRUE;
    }

    if ( victim == ch )
        return TRUE;

    /*
     * Take care of link dead people.
     */
    if ( !IS_NPC( victim ) && victim->desc == NULL )
    {
        if ( number_range( 0, victim->wait ) == 0 )
        {
            do_recall( victim, "" );
            return TRUE;
        }
    }

    /*
     * Wimp out?
     */
    if ( IS_NPC( victim ) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2 )
    {
        if ( ( IS_SET( victim->act, ACT_WIMPY ) && number_bits( 2 ) == 0
               && victim->hit < victim->max_hit / 5 )
             || ( IS_AFFECTED( victim, AFF_CHARM ) && victim->master != NULL
                  && victim->master->in_room != victim->in_room ) )
            do_flee( victim, "" );
    }

    if ( !IS_NPC( victim )
         && victim->hit > 0
         && victim->hit <= victim->wimpy && victim->wait < PULSE_VIOLENCE / 2 )
        do_flee( victim, "" );

    tail_chain(  );
    return TRUE;
}

bool vorpal_kill( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
                  int dam_type )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    extern bool chaos;
    int chaos_points;

    if ( victim->position == POS_DEAD )
        return FALSE;

    /*
     * Stop up any residual loopholes.
     */
    if ( victim == ch )
    {
        log_string( "BUG: victim == ch in vorpal_kill" );
        return FALSE;
    }

    if ( victim != ch )
    {
        /*
         * Certain attacks are forbidden.
         * Most other attacks are returned.
         */
        if ( is_safe( ch, victim ) )
            return FALSE;
        check_killer( ch, victim );

        if ( victim->position > POS_STUNNED )
        {
            if ( victim->fighting == NULL )
                set_fighting( victim, ch );
            if ( victim->timer <= 4 )
                victim->position = POS_FIGHTING;
        }

        if ( victim->position > POS_STUNNED )
        {
            if ( ch->fighting == NULL )
                set_fighting( ch, victim );

            /*
             * If victim is charmed, ch might attack victim's master.
             */
            if ( IS_NPC( ch )
                 && IS_NPC( victim )
                 && IS_AFFECTED( victim, AFF_CHARM )
                 && victim->master != NULL
                 && victim->master->in_room == ch->in_room
                 && number_bits( 3 ) == 0 )
            {
                stop_fighting( ch, FALSE );
                multi_hit( ch, victim->master, TYPE_UNDEFINED );
                return FALSE;
            }
        }

        /*
         * More charm stuff.
         */
        if ( victim->master == ch )
            stop_follower( victim );
    }

    /*
     * Inviso attacks ... not.
     */
    if ( IS_AFFECTED( ch, AFF_INVISIBLE ) )
    {
        affect_strip( ch, gsn_invis );
        affect_strip( ch, gsn_mass_invis );
        REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
        act( "`K$n fades into existence.`w", ch, NULL, NULL, TO_ROOM );
    }
    /* dam changed to max_hit to get a neat damage message */
    dam = victim->max_hit;
    dam_message( ch, victim, dam, dt, FALSE );

    if ( dam == 0 )
        return FALSE;

    /* Ok, give the ch xp for his hit and add to ch->exp_stack */
    if ( !IS_NPC( ch ) )
    {
        int xp = 0;
        int members = 0;
        int group_levels = 0;
        CHAR_DATA *gch;

        for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
        {
            if ( is_same_group( gch, ch ) )
            {
                members++;
                group_levels += gch->level;
            }
        }

        xp = hit_xp_compute( ch, victim, group_levels, members,
                             UMIN( dam, victim->hit + 20 ) );
        /* extra exp given for the vorpal kill.  dam being really big doesn't give lots of exp */
        xp *= 3;
        ch->exp_stack += xp;
        gain_exp( ch, xp );
    }

    /*
     * KILL the victim.
     * Inform the victim of his new state.
     */
    victim->hit = -20;
    update_pos( victim );

    rprog_death_trigger( victim );
    mprog_death_trigger( victim );
    act( "`R$n is DEAD!!`w", victim, 0, 0, TO_ROOM );
    send_to_char( "`RYou have been KILLED!!\n\r\n\r`w", victim );

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE( victim ) )
        stop_fighting( victim, FALSE );

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
        group_gain( ch, victim );

        if ( !IS_NPC( victim ) )
        {
            sprintf( log_buf, "%s decapitated by %s at %d",
                     victim->name,
                     ( IS_NPC( ch ) ? ch->short_descr : ch->name ),
                     victim->in_room->vnum );
            log_string( log_buf );
            if ( !IS_IMMORTAL( ch ) )
            {
                free_string( &victim->pcdata->nemesis );
                victim->pcdata->nemesis =
                    str_dup( IS_NPC( ch ) ? ch->short_descr : ch->name );
            }

            /*
             * Dying penalty:
             * 1/2 way back to previous level.
             */
            if ( !chaos && victim->exp > 0 && IS_NPC( ch ) )
                gain_exp( victim, -1 * ( victim->exp / 2 ) );
        }
        if ( chaos )
        {
            chaos_points = 0;

            if ( ch->level < victim->level )
                chaos_points = 2 * ( victim->level - ch->level );

            chaos_points = chaos_points + victim->level;
            ch->pcdata->chaos_score += chaos_points;
        }

        if ( !IS_NPC( victim ) )
        {
            sprintf( buf, "%s has been brutally decapitated by %s!",
                     victim->name, IS_NPC( ch ) ? ch->short_descr : ch->name );
            do_sendinfo( ch, buf );
        }

        if ( chaos && !IS_NPC( victim ) )
            chaos_kill( victim );
        else if ( !( !IS_NPC( victim ) && !IS_NPC( ch ) ) )
            raw_kill( victim );
        else
        {
            pk_kill( victim );

            if ( !IS_IMMORTAL( ch ) )
            {
                victim->pcdata->pk_deaths++;
                ch->pcdata->pk_kills++;
            }
        }
        /* RT new auto commands */

        if ( !IS_NPC( ch ) && IS_NPC( victim ) )
        {
            corpse = get_obj_list( ch, "corpse", ch->in_room->contents );

            if ( IS_SET( ch->act, PLR_AUTOLOOT ) && corpse && corpse->contains )    /* exists and not empty */
                do_get( ch, "all corpse" );

            if ( IS_SET( ch->act, PLR_AUTOGOLD ) && corpse && corpse->contains &&   /* exists and not empty */
                 !IS_SET( ch->act, PLR_AUTOLOOT ) )
                do_get( ch, "all.gold corpse" );

            if ( IS_SET( ch->act, PLR_AUTOSAC ) )
            {
                if ( IS_SET( ch->act, PLR_AUTOLOOT ) && corpse
                     && corpse->contains )
                    return TRUE;    /* leave if corpse has treasure */
                else
                    do_sacrifice( ch, "corpse" );
            }
        }
        return TRUE;
    }

    tail_chain(  );
    return TRUE;
}

bool is_safe( CHAR_DATA * ch, CHAR_DATA * victim )
{

/* no killing NPCs with ACT_NO_KILL */

    if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_NO_KILL ) )
    {
        send_to_char( "I don't think the gods would approve.\n\r", ch );
        return TRUE;
    }

    /* no fighting in safe rooms */
    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
    {
        send_to_char( "Not in this room.\n\r", ch );
        return TRUE;
    }

    if ( victim->fighting == ch )
        return FALSE;

    if ( IS_NPC( ch ) )
    {
        /* charmed mobs and pets can only attack PK players */
        if ( !IS_NPC( victim ) &&
             IS_NPC( ch ) &&
             ( IS_AFFECTED( ch, AFF_CHARM ) || IS_SET( ch->act, ACT_PET ) ) )
        {
            if ( !IS_SET( ch->master->act, PLR_KILLER ) ||
                 ( !chaos && !IS_SET( victim->act, PLR_KILLER ) ) )
            {
                return TRUE;
            }
        }

        return FALSE;
    }
    else                        /* Not NPC */
    {
        if ( IS_IMMORTAL( ch ) )
            return FALSE;

        /* no pets */
        if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PET ) )
        {
            act( "But $N looks so cute and cuddly...", ch, NULL, victim,
                 TO_CHAR );
            return TRUE;
        }

        /* no charmed mobs unless char is the the owner */
        if ( IS_AFFECTED( victim, AFF_CHARM ) && ch != victim->master )
        {
            send_to_char( "You don't own that monster.\n\r", ch );
            return TRUE;
        }

        return FALSE;
    }
}

bool is_safe_spell( CHAR_DATA * ch, CHAR_DATA * victim, bool area )
{
    /* can't zap self (crash bug) */
    if ( ch == victim )
        return TRUE;

    /* immortals not hurt in area attacks */
    if ( IS_IMMORTAL( victim ) && area )
        return TRUE;

    /* no killing NO_KILL mobiles */
    if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_NO_KILL ) )
        return TRUE;

    /* no fighting in safe rooms */
    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE ) )
        return TRUE;

    if ( victim->fighting == ch )
        return FALSE;

    if ( IS_NPC( ch ) )
    {
        /* charmed mobs and pets cannot attack players */
        if ( !IS_NPC( victim ) && ( IS_AFFECTED( ch, AFF_CHARM )
                                    || IS_SET( ch->act, ACT_PET ) ) )
            return TRUE;

        /* area affects don't hit other mobiles */
        if ( IS_NPC( victim ) && area )
            return TRUE;

        return FALSE;
    }

    else                        /* Not NPC */
    {
        if ( IS_IMMORTAL( ch ) && !area )
            return FALSE;

        /* no pets */
        if ( IS_NPC( victim ) && IS_SET( victim->act, ACT_PET ) )
            return TRUE;

        /* no charmed mobs unless char is the the owner */
        if ( IS_AFFECTED( victim, AFF_CHARM ) && ch != victim->master )
            return TRUE;

        /* no player killing */
        if ( !IS_NPC( victim ) && !IS_NPC( ch )
             && ( ( !chaos && !IS_SET( victim->act, PLR_KILLER ) )
                  || ( !chaos && !IS_SET( ch->act, PLR_KILLER ) ) ) )
        {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return TRUE;
        }

        /* cannot use spells if not in same group */
        if ( victim->fighting != NULL
             && !is_same_group( ch, victim->fighting ) )
            return TRUE;

        return FALSE;
    }
}

/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA * ch, CHAR_DATA * victim )
{
    char buf[MAX_STRING_LENGTH];
/*
     * Follow charm thread to responsible character.
     * Attacking someone's charmed char is hostile!
     */
    while ( IS_AFFECTED( victim, AFF_CHARM ) && victim->master != NULL )
        victim = victim->master;

    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if ( IS_NPC( victim )
         || IS_SET( victim->act, PLR_KILLER )
         || IS_SET( victim->act, PLR_THIEF ) )
        return;

    /*
     * Charm-o-rama.
     */
    if ( IS_SET( ch->affected_by, AFF_CHARM ) )
    {
        if ( ch->master == NULL )
        {
            sprintf( buf, "Check_killer: %s bad AFF_CHARM",
                     IS_NPC( ch ) ? ch->short_descr : ch->name );
            bug( buf, 0 );
            affect_strip( ch, gsn_charm_person );
            REMOVE_BIT( ch->affected_by, AFF_CHARM );
            return;
        }

        stop_follower( ch );
        return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     *
     * You can't be fighting a Hero unless he attacked you first or
     * you're both PK.  So if you're not PK and fighting an IMM
     * don't set your PK flag cuz the Hero probably started it. :)
     */
    if ( IS_NPC( ch )
         || ch == victim
         || ch->level >= LEVEL_HERO
         || IS_SET( ch->act, PLR_KILLER ) || victim->level >= LEVEL_HERO
         || chaos )
        return;

    send_to_char( "*** You are now a KILLER!! ***\n\r", ch );
    SET_BIT( ch->act, PLR_KILLER );
    save_char_obj( ch );
    return;
}

/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA * victim )
{
    if ( victim->hit > 0 )
    {
        if ( victim->position <= POS_STUNNED )
            victim->position = POS_STANDING;
        return;
    }

    if ( IS_NPC( victim ) && victim->hit < 1 )
    {
        victim->position = POS_DEAD;
        return;
    }

    if ( victim->hit <= -11 )
    {
        victim->position = POS_DEAD;
        return;
    }

    if ( victim->hit <= -6 )
        victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 )
        victim->position = POS_INCAP;
    else
        victim->position = POS_STUNNED;

    return;
}

/*
 * Start fights.
 */
void set_fighting( CHAR_DATA * ch, CHAR_DATA * victim )
{
    if ( ch->fighting != NULL )
    {
        bug( "Set_fighting: already fighting", 0 );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_SLEEP ) )
        affect_strip( ch, gsn_sleep );

    ch->fighting = victim;
    ch->position = POS_FIGHTING;

    return;
}

/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA * ch, bool fBoth )
{
    CHAR_DATA *fch;
    char buf[MAX_STRING_LENGTH];

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
        if ( fch == ch || ( fBoth && fch->fighting == ch ) )
        {
            if ( fch->exp_stack == 0 && ( fch->position != POS_FIGHTING
                                          && fch->fighting == NULL ) )
                continue;

            fch->fighting = NULL;
            fch->position = IS_NPC( fch ) ? ch->default_pos : POS_STANDING;
            if ( fch->exp_stack > 0 )
                sprintf( buf, "`WYou receive %ld experience points.\n\r`w",
                         fch->exp_stack );
            else
                sprintf( buf, "`WYou lost %ld experience points.\n\r`w",
                         fch->exp_stack * -1 );

            if ( !chaos && !gsilentdamage )
                send_to_char( buf, fch );

            fch->exp_stack = 0;
            update_pos( fch );
        }
    }

    return;
}

/*
 * Make a corpse out of a character.
 */
void make_corpse( CHAR_DATA * ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;

    if ( IS_NPC( ch ) )
    {
        name = ch->short_descr;
        corpse = create_object( get_obj_index( OBJ_VNUM_CORPSE_NPC ), 0 );
        corpse->timer = number_range( 3, 6 );
        if ( ch->gold > 0 )
        {
            obj_to_obj( create_money( ch->gold ), corpse );
            ch->gold = 0;
        }
        corpse->cost = 0;
    }
    else
    {
        name = ch->name;
        corpse = create_object( get_obj_index( OBJ_VNUM_CORPSE_PC ), 0 );
        corpse->timer = number_range( 25, 40 );
        REMOVE_BIT( ch->act, PLR_CANLOOT );
        if ( !IS_SET( ch->act, PLR_THIEF ) )
            corpse->owner = str_dup( ch->name );
        else
            corpse->owner = NULL;
        corpse->cost = 0;
    }

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    free_string( &corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( &corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
        obj_next = obj->next_content;
        obj_from_char( obj );
        if ( obj->item_type == ITEM_POTION )
            obj->timer = number_range( 500, 1000 );
        if ( obj->item_type == ITEM_SCROLL )
            obj->timer = number_range( 1000, 2500 );
        if ( IS_SET( obj->extra_flags, ITEM_ROT_DEATH ) )
            obj->timer = number_range( 5, 10 );
        if ( ( !IS_NPC( ch ) )
             && IS_SET( obj->extra_flags, ITEM_ROT_PLAYER_DEATH ) )
            obj->timer = number_range( 5, 10 );
        REMOVE_BIT( obj->extra_flags, ITEM_VIS_DEATH );
        REMOVE_BIT( obj->extra_flags, ITEM_ROT_DEATH );
        if ( !IS_NPC( ch ) )
        {
            REMOVE_BIT( obj->extra_flags, ITEM_ROT_PLAYER_DEATH );
        }
        if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
            extract_obj( obj );
        else
            obj_to_obj( obj, corpse );
    }
#ifdef USE_MORGUE
    if ( IS_NPC( ch ) )
        obj_to_room( corpse, ch->in_room );
    else
        obj_to_room( corpse, get_room_index( ROOM_VNUM_MORGUE ) );
    return;
#else
    obj_to_room( corpse, ch->in_room );
#endif
    return;
}

void make_pk_corpse( CHAR_DATA * ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *corpse;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    char *name;
    int random;

    name = ch->name;
    corpse = create_object( get_obj_index( OBJ_VNUM_CORPSE_PC ), 0 );
    corpse->timer = number_range( 3, 10 );
    REMOVE_BIT( ch->act, PLR_CANLOOT );
    if ( !IS_SET( ch->act, PLR_THIEF ) )
        corpse->owner = str_dup( ch->name );
    else
        corpse->owner = NULL;
    corpse->cost = 0;

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    free_string( &corpse->short_descr );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    free_string( &corpse->description );
    corpse->description = str_dup( buf );

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
        /* These are the values that determin how the player looting is run for PK'ing -Lancelight */
        if ( LOOTING_ALLOWED == 0 )
            random = 0;
        else if ( LOOTING_ALLOWED == 1 )
            random = number_range( 1, LOOTING_CHANCE );
        else if ( LOOTING_ALLOWED == 2 )
            random = 1;
        obj_next = obj->next_content;
        obj_from_char( obj );

        if ( obj->item_type == ITEM_POTION )
            obj->timer = number_range( 500, 1000 );
        if ( obj->item_type == ITEM_SCROLL )
            obj->timer = number_range( 1000, 2500 );
        if ( IS_SET( obj->extra_flags, ITEM_ROT_DEATH ) )
            obj->timer = number_range( 5, 10 );
        if ( ( !IS_NPC( ch ) )
             && IS_SET( obj->extra_flags, ITEM_ROT_PLAYER_DEATH ) )
            obj->timer = number_range( 5, 10 );
        REMOVE_BIT( obj->extra_flags, ITEM_VIS_DEATH );
        REMOVE_BIT( obj->extra_flags, ITEM_ROT_DEATH );
        if ( !IS_NPC( ch ) )
        {
            REMOVE_BIT( obj->extra_flags, ITEM_ROT_PLAYER_DEATH );
        }
        if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
            extract_obj( obj );
        else if ( random == 1 )
            obj_to_room( obj, ch->in_room );
        else
            obj_to_obj( obj, corpse );
    }

    obj_to_room( corpse, ch->in_room );

    return;
}

/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry( CHAR_DATA * ch )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    int door;
    int vnum;

    vnum = 0;
    msg = "`YYou hear $n's death cry.`w";

    switch ( number_bits( 4 ) )
    {
    case 0:
        msg = "$n hits the ground ... DEAD.";
        break;
    case 1:
        if ( ch->material == 0 )
        {
            msg = "`R$n splatters blood on your armor.`w";
            break;
        }
    case 2:
        if ( IS_SET( ch->parts, PART_GUTS ) )
        {
            msg = "`R$n spills $s guts all over the floor.`w";
            vnum = OBJ_VNUM_GUTS;
        }
        break;
    case 3:
        if ( IS_SET( ch->parts, PART_HEAD ) )
        {
            msg = "`R$n's severed head plops on the ground.`w";
            vnum = OBJ_VNUM_SEVERED_HEAD;
        }
        break;
    case 4:
        if ( IS_SET( ch->parts, PART_HEART ) )
        {
            msg = "`R$n's heart is torn from $s chest.`w";
            vnum = OBJ_VNUM_TORN_HEART;
        }
        break;
    case 5:
        if ( IS_SET( ch->parts, PART_ARMS ) )
        {
            msg = "`R$n's arm is sliced from $s dead body.`w";
            vnum = OBJ_VNUM_SLICED_ARM;
        }
        break;
    case 6:
        if ( IS_SET( ch->parts, PART_LEGS ) )
        {
            msg = "`R$n's leg is sliced from $s dead body.`w";
            vnum = OBJ_VNUM_SLICED_LEG;
        }
        break;
    case 7:
        if ( IS_SET( ch->parts, PART_BRAINS ) )
        {
            msg =
                "`R$n's head is shattered, and $s brains splash all over you.`w";
            vnum = OBJ_VNUM_BRAINS;
        }
    }

    act( msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum != 0 )
    {
        OBJ_DATA *obj;
        char *name;

        name = IS_NPC( ch ) ? ch->short_descr : ch->name;
        obj = create_object( get_obj_index( vnum ), 0 );
        obj->timer = number_range( 4, 7 );

        sprintf( buf, obj->short_descr, name );
        free_string( &obj->short_descr );
        obj->short_descr = str_dup( buf );

        sprintf( buf, obj->description, name );
        free_string( &obj->description );
        obj->description = str_dup( buf );

        if ( obj->item_type == ITEM_FOOD )
        {
            if ( IS_SET( ch->form, FORM_POISON ) )
                obj->value[3] = 1;
            else if ( !IS_SET( ch->form, FORM_EDIBLE ) )
                obj->item_type = ITEM_TRASH;
        }

        obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC( ch ) )
        msg = "`RYou hear something's death cry.`w";
    else
        msg = "`RYou hear someone's death cry.`w";

    was_in_room = ch->in_room;
    for ( door = 0; door <= 5; door++ )
    {
        EXIT_DATA *pexit;

        if ( ( pexit = was_in_room->exit[door] ) != NULL
             && pexit->u1.to_room != NULL && pexit->u1.to_room != was_in_room )
        {
            ch->in_room = pexit->u1.to_room;
            act( msg, ch, NULL, NULL, TO_ROOM );
        }
    }
    ch->in_room = was_in_room;

    return;
}

void chaos_kill( CHAR_DATA * victim )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    DESCRIPTOR_DATA *d;

    stop_fighting( victim, TRUE );
    for ( obj = victim->carrying; obj != NULL; obj = obj_next )
    {
        obj_next = obj->next_content;
        obj_from_char( obj );
        obj_to_room( obj, victim->in_room );
    }
    act( "`B$n's corpse is sucked into the ground!!`w", victim, 0, 0, TO_ROOM );
    if ( !IS_NPC( victim ) )
    {
        sprintf( buf, "was slain with %d chaos points.",
                 victim->pcdata->chaos_score );
        chaos_log( victim, buf );
    }
    d = victim->desc;
    extract_char( victim, TRUE );
    if ( d )
    {
#if defined(cbuilder)
        if ( d->character )
            RemoveUser( d->character );
#endif
        close_socket( d );
    }
    return;
}

void raw_kill( CHAR_DATA * victim )
{
    int i;

    make_corpse( victim );
    stop_fighting( victim, TRUE );

    if ( IS_NPC( victim ) )
    {
        victim->pIndexData->killed++;
        kill_table[URANGE( 0, victim->level, MAX_LEVEL - 1 )].killed++;
        extract_char( victim, TRUE );
        return;
    }

    extract_char( victim, FALSE );
    while ( victim->affected )
        affect_remove( victim, victim->affected );
    while ( victim->newaffected )
        newaffect_remove( victim, victim->newaffected );
    victim->affected_by = 0;
    memset( victim->newaff, 0,
            ( MAX_NEWAFF_BIT / 8 ) + ( MAX_NEWAFF_BIT % 8 ? 1 : 0 ) );
    for ( i = 0; i < 4; i++ )
        victim->armor[i] = 100;
    victim->position = POS_RESTING;
    victim->hit = UMAX( 1, victim->hit );
    victim->mana = UMAX( 1, victim->mana );
    victim->move = UMAX( 1, victim->move );
    /* RT added to prevent infinite deaths */
    REMOVE_BIT( victim->act, PLR_THIEF );
    REMOVE_BIT( victim->act, PLR_BOUGHT_PET );
/*  save_char_obj( victim ); */
/* Add back race affects */
    victim->affected_by = victim->affected_by | race_table[victim->race].aff;
    return;
}

void pk_kill( CHAR_DATA * victim )
{
    int i;

    make_pk_corpse( victim );
    stop_fighting( victim, TRUE );

    pk_extract_char( victim, FALSE );
    while ( victim->affected )
        affect_remove( victim, victim->affected );
    while ( victim->newaffected )
        newaffect_remove( victim, victim->newaffected );
    victim->affected_by = 0;
    memset( victim->newaff, 0,
            ( MAX_NEWAFF_BIT / 8 ) + ( MAX_NEWAFF_BIT % 8 ? 1 : 0 ) );
    for ( i = 0; i < 4; i++ )
        victim->armor[i] = 100;
    victim->position = POS_RESTING;
    victim->hit = UMAX( 1, victim->hit );
    victim->mana = UMAX( 1, victim->mana );
    victim->move = UMAX( 1, victim->move );
    /* RT added to prevent infinite deaths */
    REMOVE_BIT( victim->act, PLR_THIEF );
    REMOVE_BIT( victim->act, PLR_BOUGHT_PET );
    /*  save_char_obj( victim ); */
    /* Add back race affects */
    victim->affected_by = victim->affected_by | race_table[victim->race].aff;
    reset_char( victim );       /* players reported weird stats after pkills sometimes. -Kyle */
    return;
}

void group_gain( CHAR_DATA * ch, CHAR_DATA * victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    int xp = 0;
    int members;
    int group_levels;

    /*
     * Monsters don't get kill xp's or alignment changes.
     * P-killing doesn't help either.
     * Dying of mortal wounds or poison doesn't give xp to anyone!
     */
    if ( !IS_NPC( victim ) || victim == ch )
        return;

    /*
     * Nobody gets xp during CHAOS.
     */
    if ( chaos )
        return;

    members = 0;
    group_levels = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( is_same_group( gch, ch ) )
        {
            members++;
            group_levels += gch->level;
        }
    }

    if ( members == 0 )
    {
        bug( "Group_gain: members.", members );
        members = 1;
        group_levels = ch->level;
    }

    lch = ( ch->leader != NULL ) ? ch->leader : ch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        OBJ_DATA *obj;
        OBJ_DATA *obj_next;

        if ( !is_same_group( gch, ch ) || IS_NPC( gch ) )
            continue;

        /* This code looks bad, it gives XP on a per kill basis if your xp stack is
         * empty.  I don't think this code is actually ever called and if it IS
         * called it shouldn't be so I'm removing it.  -Zane */
        /* Changed again to deal with rapid alignment shifts.   With alignment being
         * changed with every hit_xp_compute, players could go from good to satanic
         * with a couple of hits on the mob.  Change is so that we call this at
         * mob death if the player contributed to the death.  Alignment calls are
         * taken out of hit_xp_compute. - Dorzak
         */
        if ( gch->exp_stack > 0 )
            xp = xp_compute( gch, victim, group_levels, members );
        if ( gch->level < LEVEL_HERO && ( gch->exp + xp ) >=
             exp_per_level( gch, gch->pcdata->points ) &&
             gch->exp < exp_per_level( gch, gch->pcdata->points ) )
        {
            strcat( buf, "`WYou're ready to `CLevel`w!\n\r" );
            send_to_char( buf, gch );
        }
        /* This code looks bad, it gives XP on a per kill basis if your xp stack is
         * empty.  I don't think this code is actually ever called and if it IS
         * called it shouldn't be so I'm removing it.  -Zane */
        /* Reenabling this as part of the above mentioned changes about align. -Dorzak */
        if ( gch->exp_stack > 0 )
            gain_exp( gch, xp );

        for ( obj = ch->carrying; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;
            if ( obj->wear_loc == WEAR_NONE )
                continue;

            if ( ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL ) && IS_EVIL( ch ) )
                 || ( IS_OBJ_STAT( obj, ITEM_ANTI_GOOD ) && IS_GOOD( ch ) )
                 || ( IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL )
                      && IS_NEUTRAL( ch ) ) )
            {
                act( "`WYou are zapped by $p.`w", ch, obj, NULL, TO_CHAR );
                act( "`W$n is zapped by $p.`w", ch, obj, NULL, TO_ROOM );
                obj_from_char( obj );
                obj_to_room( obj, ch->in_room );
                oprog_zap_trigger( ch, obj );
            }
        }
    }

    return;
}

/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute( CHAR_DATA * gch, CHAR_DATA * victim, int total_levels,
                int members )
{
    int xp, base_exp = 0;
    int align;
    int change;

    /* compute the base exp */
    switch ( victim->level )
    {
    case 0:
        base_exp = 50;
        break;
    case 1:
        base_exp = 100;
        break;
    case 2:
        base_exp = 200;
        break;
    case 3:
        base_exp = 250;
        break;
    case 4:
        base_exp = 350;
        break;
    case 5:
        base_exp = 550;
        break;
    case 6:
        base_exp = 1000;
        break;
    case 7:
        base_exp = 3000;
        break;
    case 8:
        base_exp = 5000;
        break;
    case 9:
        base_exp = 7500;
        break;
    case 10:
        base_exp = 10000;
        break;
    case 11:
        base_exp = 15000;
        break;
    case 12:
        base_exp = 23000;
        break;
    case 13:
        base_exp = 35000;
        break;
    case 14:
        base_exp = 50000;
        break;
    case 15:
        base_exp = 65000;
        break;
    case 16:
        base_exp = 80000;
        break;
    case 17:
        base_exp = 95000;
        break;
    case 18:
        base_exp = 110000;
        break;
    case 19:
        base_exp = 135000;
        break;
    case 20:
        base_exp = 150000;
        break;
    case 21:
        base_exp = 165000;
        break;
    case 22:
        base_exp = 180000;
        break;
    case 23:
        base_exp = 200000;
        break;
    case 24:
        base_exp = 220000;
        break;
    case 25:
        base_exp = 240000;
        break;
    case 26:
        base_exp = 260000;
        break;
    case 27:
        base_exp = 280000;
        break;
    case 28:
        base_exp = 300000;
        break;
    case 29:
        base_exp = 320000;
        break;
    case 30:
        base_exp = 340000;
        break;
    case 31:
        base_exp = 360000;
        break;
    case 32:
        base_exp = 380000;
        break;
    case 33:
        base_exp = 400000;
        break;
    case 34:
        base_exp = 420000;
        break;
    case 35:
        base_exp = 440000;
        break;
    case 36:
        base_exp = 460000;
        break;
    case 37:
        base_exp = 480000;
        break;
    case 38:
        base_exp = 500000;
        break;
    case 39:
        base_exp = 520000;
        break;
    case 40:
        base_exp = 540000;
        break;
    case 41:
        base_exp = 560000;
        break;
    case 42:
        base_exp = 580000;
        break;
    case 43:
        base_exp = 600000;
        break;
    case 44:
        base_exp = 620000;
        break;
    case 45:
        base_exp = 640000;
        break;
    case 46:
        base_exp = 660000;
        break;
    case 47:
        base_exp = 680000;
        break;
    case 48:
        base_exp = 700000;
        break;
    case 49:
        base_exp = 720000;
        break;
    case 50:
        base_exp = 740000;
        break;
    case 51:
        base_exp = 760000;
        break;
    case 52:
        base_exp = 780000;
        break;
    case 53:
        base_exp = 800000;
        break;
    case 54:
        base_exp = 820000;
        break;
    case 55:
        base_exp = 840000;
        break;
    case 56:
        base_exp = 860000;
        break;
    case 57:
        base_exp = 880000;
        break;
    case 58:
        base_exp = 900000;
        break;
    case 59:
        base_exp = 920000;
        break;
    case 60:
        base_exp = 940000;
        break;
    case 61:
        base_exp = 960000;
        break;
    case 62:
        base_exp = 1000000;
        break;
    case 63:
        base_exp = 1100000;
        break;
    case 64:
        base_exp = 1200000;
        break;
    case 65:
        base_exp = 1300000;
        break;
    case 66:
        base_exp = 1400000;
        break;
    case 67:
        base_exp = 1500000;
        break;
    case 68:
        base_exp = 1600000;
        break;
    case 69:
        base_exp = 1700000;
        break;
    case 70:
        base_exp = 1800000;
        break;
    case 71:
        base_exp = 1900000;
        break;
    case 72:
        base_exp = 2000000;
        break;
    case 73:
        base_exp = 2100000;
        break;
    case 74:
        base_exp = 2200000;
        break;
    case 75:
        base_exp = 2300000;
        break;
    case 76:
        base_exp = 2400000;
        break;
    case 77:
        base_exp = 2500000;
        break;
    case 78:
        base_exp = 2600000;
        break;
    case 79:
        base_exp = 2700000;
        break;
    case 80:
        base_exp = 2800000;
        break;
    case 81:
        base_exp = 2900000;
        break;
    case 82:
        base_exp = 3000000;
        break;
    case 83:
        base_exp = 3100000;
        break;
    case 84:
        base_exp = 3200000;
        break;
    case 85:
        base_exp = 3300000;
        break;
    case 86:
        base_exp = 3400000;
        break;
    case 87:
        base_exp = 3500000;
        break;
    case 88:
        base_exp = 3600000;
        break;
    case 89:
        base_exp = 3700000;
        break;
    case 90:
        base_exp = 3800000;
        break;
    case 91:
        base_exp = 4000000;
        break;
    case 92:
        base_exp = 4500000;
        break;
    case 93:
        base_exp = 5000000;
        break;
    case 94:
        base_exp = 5500000;
        break;
    case 95:
        base_exp = 6000000;
        break;
    case 96:
        base_exp = 6500000;
        break;
    case 97:
        base_exp = 7000000;
        break;
    case 98:
        base_exp = 7500000;
        break;
    case 99:
        base_exp = 8000000;
        break;
    case 100:
        base_exp = 10000000;
        break;
    }

    /* do alignment computations */

    align = victim->alignment - gch->alignment;

    if ( IS_SET( victim->act, ACT_NOALIGN ) )
    {
        /* no change */
    }

    else if ( align > 500 )     /* monster is more good than slayer */
    {
        change =
            ( align - 500 ) * ( gch->level / total_levels +
                                ( 1 / members ) ) / 2;
        change = UMAX( 1, change );
        gch->alignment = UMAX( -1000, gch->alignment - change );
    }

    else if ( align < -500 )    /* monster is more evil than slayer */
    {
        change =
            ( -1 * align - 500 ) * ( gch->level / total_levels +
                                     ( 1 / members ) ) / 2;
        change = UMAX( 1, change );
        gch->alignment = UMIN( 1000, gch->alignment + change );
    }

    else                        /* improve this someday */
    {
        change =
            gch->alignment * ( gch->level / total_levels +
                               ( 1 / members ) ) / 2;
        gch->alignment -= change;
    }

    /* calculate exp multiplier */
    xp = base_exp;

    if ( IS_SET( victim->act, ACT_NOALIGN ) )
    {
        /* no change */
    }
    else if ( gch->alignment > 500 )    /* for goodie two shoes */
    {
        if ( victim->alignment < -750 )
            xp = base_exp * 4 / 3;

        else if ( victim->alignment < -500 )
            xp = base_exp * 5 / 4;

        else if ( victim->alignment > 250 )
            xp = base_exp * 3 / 4;

        else if ( victim->alignment > 750 )
            xp = base_exp / 4;

        else if ( victim->alignment > 500 )
            xp = base_exp / 2;

        else
            xp = base_exp;
    }

    else if ( gch->alignment < -500 )   /* for baddies */
    {
        if ( victim->alignment > 750 )
            xp = base_exp * 5 / 4;

        else if ( victim->alignment > 500 )
            xp = base_exp * 11 / 10;

        else if ( victim->alignment < -750 )
            xp = base_exp * 1 / 2;

        else if ( victim->alignment < -500 )
            xp = base_exp * 3 / 4;

        else if ( victim->alignment < -250 )
            xp = base_exp * 9 / 10;

        else
            xp = base_exp;
    }

    else if ( gch->alignment > 200 )    /* a little good */
    {

        if ( victim->alignment < -500 )
            xp = base_exp * 6 / 5;

        else if ( victim->alignment > 750 )
            xp = base_exp * 1 / 2;

        else if ( victim->alignment > 0 )
            xp = base_exp * 3 / 4;

        else
            xp = base_exp;
    }

    else if ( gch->alignment < -200 )   /* a little bad */
    {
        if ( victim->alignment > 500 )
            xp = base_exp * 6 / 5;

        else if ( victim->alignment < -750 )
            xp = base_exp * 1 / 2;

        else if ( victim->alignment < 0 )
            xp = base_exp * 3 / 4;

        else
            xp = base_exp;
    }

    else                        /* neutral */
    {

        if ( victim->alignment > 500 || victim->alignment < -500 )
            xp = base_exp * 4 / 3;

        else if ( victim->alignment < 200 || victim->alignment > -200 )
            xp = base_exp * 1 / 2;

        else
            xp = base_exp;
    }

    /* randomize the rewards */
    xp = number_range( xp * 9 / 10, xp * 11 / 10 );

    /* adjust for grouping */
    if ( members > 1 )
        xp = ( gch->level * ( xp / total_levels ) );
    if ( xp > ( exp_per_level( gch, gch->pcdata->points ) / 2 ) )
        xp = ( exp_per_level( gch, gch->pcdata->points ) / 2 );
    xp = xp * 1 / 10;           /* This function only runs if you didn't hit the mob
                                   so you don't gain much experience.  But hey, it's
                                   better than nothing. */
    if ( xp > 0 )
    {
        xp = ( int ) ( xp * EXP_MULTIPLIER );
    }

    return xp;
}

int hit_xp_compute( CHAR_DATA * gch, CHAR_DATA * victim, int total_levels,
                    int members, int dam )
{
    int xp, base_exp = 0;
/* Commenting out as part of the align changes */
/*    int align;
    int change; */

    /* compute the base exp */
    switch ( victim->level )
    {
    case 0:
        base_exp = 50;
        break;
    case 1:
        base_exp = 100;
        break;
    case 2:
        base_exp = 200;
        break;
    case 3:
        base_exp = 250;
        break;
    case 4:
        base_exp = 350;
        break;
    case 5:
        base_exp = 550;
        break;
    case 6:
        base_exp = 1000;
        break;
    case 7:
        base_exp = 3000;
        break;
    case 8:
        base_exp = 5000;
        break;
    case 9:
        base_exp = 7500;
        break;
    case 10:
        base_exp = 10000;
        break;
    case 11:
        base_exp = 15000;
        break;
    case 12:
        base_exp = 23000;
        break;
    case 13:
        base_exp = 35000;
        break;
    case 14:
        base_exp = 50000;
        break;
    case 15:
        base_exp = 65000;
        break;
    case 16:
        base_exp = 80000;
        break;
    case 17:
        base_exp = 95000;
        break;
    case 18:
        base_exp = 110000;
        break;
    case 19:
        base_exp = 135000;
        break;
    case 20:
        base_exp = 150000;
        break;
    case 21:
        base_exp = 165000;
        break;
    case 22:
        base_exp = 180000;
        break;
    case 23:
        base_exp = 200000;
        break;
    case 24:
        base_exp = 220000;
        break;
    case 25:
        base_exp = 240000;
        break;
    case 26:
        base_exp = 260000;
        break;
    case 27:
        base_exp = 280000;
        break;
    case 28:
        base_exp = 300000;
        break;
    case 29:
        base_exp = 320000;
        break;
    case 30:
        base_exp = 340000;
        break;
    case 31:
        base_exp = 360000;
        break;
    case 32:
        base_exp = 380000;
        break;
    case 33:
        base_exp = 400000;
        break;
    case 34:
        base_exp = 420000;
        break;
    case 35:
        base_exp = 440000;
        break;
    case 36:
        base_exp = 460000;
        break;
    case 37:
        base_exp = 480000;
        break;
    case 38:
        base_exp = 500000;
        break;
    case 39:
        base_exp = 520000;
        break;
    case 40:
        base_exp = 540000;
        break;
    case 41:
        base_exp = 560000;
        break;
    case 42:
        base_exp = 580000;
        break;
    case 43:
        base_exp = 600000;
        break;
    case 44:
        base_exp = 620000;
        break;
    case 45:
        base_exp = 640000;
        break;
    case 46:
        base_exp = 660000;
        break;
    case 47:
        base_exp = 680000;
        break;
    case 48:
        base_exp = 700000;
        break;
    case 49:
        base_exp = 720000;
        break;
    case 50:
        base_exp = 740000;
        break;
    case 51:
        base_exp = 760000;
        break;
    case 52:
        base_exp = 780000;
        break;
    case 53:
        base_exp = 800000;
        break;
    case 54:
        base_exp = 820000;
        break;
    case 55:
        base_exp = 840000;
        break;
    case 56:
        base_exp = 860000;
        break;
    case 57:
        base_exp = 880000;
        break;
    case 58:
        base_exp = 900000;
        break;
    case 59:
        base_exp = 920000;
        break;
    case 60:
        base_exp = 940000;
        break;
    case 61:
        base_exp = 960000;
        break;
    case 62:
        base_exp = 1000000;
        break;
    case 63:
        base_exp = 1100000;
        break;
    case 64:
        base_exp = 1200000;
        break;
    case 65:
        base_exp = 1300000;
        break;
    case 66:
        base_exp = 1400000;
        break;
    case 67:
        base_exp = 1500000;
        break;
    case 68:
        base_exp = 1600000;
        break;
    case 69:
        base_exp = 1700000;
        break;
    case 70:
        base_exp = 1800000;
        break;
    case 71:
        base_exp = 1900000;
        break;
    case 72:
        base_exp = 2000000;
        break;
    case 73:
        base_exp = 2100000;
        break;
    case 74:
        base_exp = 2200000;
        break;
    case 75:
        base_exp = 2300000;
        break;
    case 76:
        base_exp = 2400000;
        break;
    case 77:
        base_exp = 2500000;
        break;
    case 78:
        base_exp = 2600000;
        break;
    case 79:
        base_exp = 2700000;
        break;
    case 80:
        base_exp = 2800000;
        break;
    case 81:
        base_exp = 2900000;
        break;
    case 82:
        base_exp = 3000000;
        break;
    case 83:
        base_exp = 3100000;
        break;
    case 84:
        base_exp = 3200000;
        break;
    case 85:
        base_exp = 3300000;
        break;
    case 86:
        base_exp = 3400000;
        break;
    case 87:
        base_exp = 3500000;
        break;
    case 88:
        base_exp = 3600000;
        break;
    case 89:
        base_exp = 3700000;
        break;
    case 90:
        base_exp = 3800000;
        break;
    case 91:
        base_exp = 4000000;
        break;
    case 92:
        base_exp = 4500000;
        break;
    case 93:
        base_exp = 5000000;
        break;
    case 94:
        base_exp = 5500000;
        break;
    case 95:
        base_exp = 6000000;
        break;
    case 96:
        base_exp = 6500000;
        break;
    case 97:
        base_exp = 7000000;
        break;
    case 98:
        base_exp = 7500000;
        break;
    case 99:
        base_exp = 8000000;
        break;
    case 100:
        base_exp = 10000000;
        break;
    }

    /* do alignment computations */
    /* No, actually we don't want to here - see rapid align changes notes above */
/*    align = victim->alignment - gch->alignment;

    if ( IS_SET( victim->act, ACT_NOALIGN ) )
    { */
    /* no change */
/*    }

    else if ( align > 500 )   *//* monster is more good than slayer */
/*    {
        change =
            ( align - 500 ) * ( gch->level / total_levels +
                                ( 1 / members ) ) / 2;
        change = UMAX( 1, change );
        gch->alignment = UMAX( -1000, gch->alignment - change );
    }
*/
/*    else if ( align < -500 ) *//* monster is more evil than slayer */
/*    {
        change =
            ( -1 * align - 500 ) * ( gch->level / total_levels +
                                     ( 1 / members ) ) / 2;
        change = UMAX( 1, change );
        gch->alignment = UMIN( 1000, gch->alignment + change );
    }
*/
/*    else  *//* improve this someday */
/*    {
        change =
            gch->alignment * ( gch->level / total_levels +
                               ( 1 / members ) ) / 2;
        gch->alignment -= change;
    }
*/
    /* calculate exp multiplier */
    xp = base_exp;

    if ( IS_SET( victim->act, ACT_NOALIGN ) || victim == gch )
    {
        /* no change */
    }
    else if ( gch->alignment > 500 )    /* for goodie two shoes */
    {
        if ( victim->alignment < -750 )
            xp = base_exp * 4 / 3;

        else if ( victim->alignment < -500 )
            xp = base_exp * 5 / 4;

        else if ( victim->alignment > 250 )
            xp = base_exp * 3 / 4;

        else if ( victim->alignment > 750 )
            xp = base_exp / 4;

        else if ( victim->alignment > 500 )
            xp = base_exp / 2;

        else
            xp = base_exp;
    }

    else if ( gch->alignment < -500 )   /* for baddies */
    {
        if ( victim->alignment > 750 )
            xp = base_exp * 5 / 4;

        else if ( victim->alignment > 500 )
            xp = base_exp * 11 / 10;

        else if ( victim->alignment < -750 )
            xp = base_exp * 1 / 2;

        else if ( victim->alignment < -500 )
            xp = base_exp * 3 / 4;

        else if ( victim->alignment < -250 )
            xp = base_exp * 9 / 10;

        else
            xp = base_exp;
    }

    else if ( gch->alignment > 200 )    /* a little good */
    {

        if ( victim->alignment < -500 )
            xp = base_exp * 6 / 5;

        else if ( victim->alignment > 750 )
            xp = base_exp * 1 / 2;

        else if ( victim->alignment > 0 )
            xp = base_exp * 3 / 4;

        else
            xp = base_exp;
    }

    else if ( gch->alignment < -200 )   /* a little bad */
    {
        if ( victim->alignment > 500 )
            xp = base_exp * 6 / 5;

        else if ( victim->alignment < -750 )
            xp = base_exp * 1 / 2;

        else if ( victim->alignment < 0 )
            xp = base_exp * 3 / 4;

        else
            xp = base_exp;
    }

    else                        /* neutral */
    {

        if ( victim->alignment > 500 || victim->alignment < -500 )
            xp = base_exp * 4 / 3;

        else if ( victim->alignment < 200 || victim->alignment > -200 )
            xp = base_exp * 1 / 2;

        else
            xp = base_exp;
    }

    /* randomize the rewards */
    xp = number_range( xp * 9 / 10, xp * 11 / 10 );

    /* adjust for grouping */
    xp = ( xp * dam / victim->max_hit );
    if ( members > 1 )
        xp = ( int ) ( xp * 1.15 );

    if ( xp > 0 )
    {
        xp = ( int ) ( xp * EXP_MULTIPLIER );
    }

    return xp;
}

int cast_xp_compute( CHAR_DATA * gch, CHAR_DATA * victim, int total_levels,
                     int members, int dam )
{
    int xp, base_exp = 0;

    /* compute the base exp */
    switch ( victim->level )
    {
    case 0:
        base_exp = 50;
        break;
    case 1:
        base_exp = 100;
        break;
    case 2:
        base_exp = 200;
        break;
    case 3:
        base_exp = 250;
        break;
    case 4:
        base_exp = 350;
        break;
    case 5:
        base_exp = 550;
        break;
    case 6:
        base_exp = 1000;
        break;
    case 7:
        base_exp = 3000;
        break;
    case 8:
        base_exp = 5000;
        break;
    case 9:
        base_exp = 7500;
        break;
    case 10:
        base_exp = 10000;
        break;
    case 11:
        base_exp = 15000;
        break;
    case 12:
        base_exp = 23000;
        break;
    case 13:
        base_exp = 35000;
        break;
    case 14:
        base_exp = 50000;
        break;
    case 15:
        base_exp = 65000;
        break;
    case 16:
        base_exp = 80000;
        break;
    case 17:
        base_exp = 95000;
        break;
    case 18:
        base_exp = 110000;
        break;
    case 19:
        base_exp = 135000;
        break;
    case 20:
        base_exp = 150000;
        break;
    case 21:
        base_exp = 165000;
        break;
    case 22:
        base_exp = 180000;
        break;
    case 23:
        base_exp = 200000;
        break;
    case 24:
        base_exp = 220000;
        break;
    case 25:
        base_exp = 240000;
        break;
    case 26:
        base_exp = 260000;
        break;
    case 27:
        base_exp = 280000;
        break;
    case 28:
        base_exp = 300000;
        break;
    case 29:
        base_exp = 320000;
        break;
    case 30:
        base_exp = 340000;
        break;
    case 31:
        base_exp = 360000;
        break;
    case 32:
        base_exp = 380000;
        break;
    case 33:
        base_exp = 400000;
        break;
    case 34:
        base_exp = 420000;
        break;
    case 35:
        base_exp = 440000;
        break;
    case 36:
        base_exp = 460000;
        break;
    case 37:
        base_exp = 480000;
        break;
    case 38:
        base_exp = 500000;
        break;
    case 39:
        base_exp = 520000;
        break;
    case 40:
        base_exp = 540000;
        break;
    case 41:
        base_exp = 560000;
        break;
    case 42:
        base_exp = 580000;
        break;
    case 43:
        base_exp = 600000;
        break;
    case 44:
        base_exp = 620000;
        break;
    case 45:
        base_exp = 640000;
        break;
    case 46:
        base_exp = 660000;
        break;
    case 47:
        base_exp = 680000;
        break;
    case 48:
        base_exp = 700000;
        break;
    case 49:
        base_exp = 720000;
        break;
    case 50:
        base_exp = 740000;
        break;
    case 51:
        base_exp = 760000;
        break;
    case 52:
        base_exp = 780000;
        break;
    case 53:
        base_exp = 800000;
        break;
    case 54:
        base_exp = 820000;
        break;
    case 55:
        base_exp = 840000;
        break;
    case 56:
        base_exp = 860000;
        break;
    case 57:
        base_exp = 880000;
        break;
    case 58:
        base_exp = 900000;
        break;
    case 59:
        base_exp = 920000;
        break;
    case 60:
        base_exp = 940000;
        break;
    case 61:
        base_exp = 960000;
        break;
    case 62:
        base_exp = 1000000;
        break;
    case 63:
        base_exp = 1100000;
        break;
    case 64:
        base_exp = 1200000;
        break;
    case 65:
        base_exp = 1300000;
        break;
    case 66:
        base_exp = 1400000;
        break;
    case 67:
        base_exp = 1500000;
        break;
    case 68:
        base_exp = 1600000;
        break;
    case 69:
        base_exp = 1700000;
        break;
    case 70:
        base_exp = 1800000;
        break;
    case 71:
        base_exp = 1900000;
        break;
    case 72:
        base_exp = 2000000;
        break;
    case 73:
        base_exp = 2100000;
        break;
    case 74:
        base_exp = 2200000;
        break;
    case 75:
        base_exp = 2300000;
        break;
    case 76:
        base_exp = 2400000;
        break;
    case 77:
        base_exp = 2500000;
        break;
    case 78:
        base_exp = 2600000;
        break;
    case 79:
        base_exp = 2700000;
        break;
    case 80:
        base_exp = 2800000;
        break;
    case 81:
        base_exp = 2900000;
        break;
    case 82:
        base_exp = 3000000;
        break;
    case 83:
        base_exp = 3100000;
        break;
    case 84:
        base_exp = 3200000;
        break;
    case 85:
        base_exp = 3300000;
        break;
    case 86:
        base_exp = 3400000;
        break;
    case 87:
        base_exp = 3500000;
        break;
    case 88:
        base_exp = 3600000;
        break;
    case 89:
        base_exp = 3700000;
        break;
    case 90:
        base_exp = 3800000;
        break;
    case 91:
        base_exp = 4000000;
        break;
    case 92:
        base_exp = 4500000;
        break;
    case 93:
        base_exp = 5000000;
        break;
    case 94:
        base_exp = 5500000;
        break;
    case 95:
        base_exp = 6000000;
        break;
    case 96:
        base_exp = 6500000;
        break;
    case 97:
        base_exp = 7000000;
        break;
    case 98:
        base_exp = 7500000;
        break;
    case 99:
        base_exp = 8000000;
        break;
    case 100:
        base_exp = 10000000;
        break;
    }

    /* calculate exp multiplier */

    xp = base_exp;

    /* randomize the rewards */
    xp = number_range( xp * 9 / 10, xp * 11 / 10 );

    /* adjust for grouping */
    xp = ( xp * dam / victim->max_hit );
    if ( members > 1 )
        xp = ( int ) ( xp * 1.15 );

    if ( xp > 0 )
    {
        xp = ( int ) ( xp * EXP_MULTIPLIER );
    }

    return xp;
}

void dam_message( CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
                  bool immune )
{
    char buf1[256], buf2[256], buf3[256];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;
    int damp;

#ifdef EXTRA_DAMAGE_MSGS
    damp = ( dam * 100 / victim->max_hit ); /* Calculate percentage
                                               for punctuation -Lancelight */
    if ( dam == 0 )
    {
        vs = CFG_DAM0;
        vp = CFG_DAM0S;
    }
    else if ( dam <= 2 )
    {
        vs = CFG_DAM2;
        vp = CFG_DAM2S;
    }
    else if ( dam <= 5 )
    {
        vs = CFG_DAM5;
        vp = CFG_DAM5S;
    }
    else if ( dam <= 10 )
    {
        vs = CFG_DAM10;
        vp = CFG_DAM10S;
    }
    else if ( dam <= 15 )
    {
        vs = CFG_DAM15;
        vp = CFG_DAM15S;
    }
    else if ( dam <= 25 )
    {
        vs = CFG_DAM25;
        vp = CFG_DAM25S;
    }
    else if ( dam <= 30 )
    {
        vs = CFG_DAM30;
        vp = CFG_DAM30S;
    }
    else if ( dam <= 35 )
    {
        vs = CFG_DAM35;
        vp = CFG_DAM35S;
    }
    else if ( dam <= 45 )
    {
        vs = CFG_DAM45;
        vp = CFG_DAM45S;
    }
    else if ( dam <= 50 )
    {
        vs = CFG_DAM50;
        vp = CFG_DAM50S;
    }
    else if ( dam <= 55 )
    {
        vs = CFG_DAM55;
        vp = CFG_DAM55S;
    }
    else if ( dam <= 65 )
    {
        vs = CFG_DAM65;
        vp = CFG_DAM65S;
    }
    else if ( dam <= 70 )
    {
        vs = CFG_DAM70;
        vp = CFG_DAM70S;
    }
    else if ( dam <= 75 )
    {
        vs = CFG_DAM75;
        vp = CFG_DAM75S;
    }
    else if ( dam <= 85 )
    {
        vs = CFG_DAM85;
        vp = CFG_DAM85S;
    }
    else if ( dam <= 90 )
    {
        vs = CFG_DAM90;
        vp = CFG_DAM90S;
    }
    else if ( dam <= 95 )
    {
        vs = CFG_DAM95;
        vp = CFG_DAM95S;
    }
    else if ( dam <= 105 )
    {
        vs = CFG_DAM105;
        vp = CFG_DAM105S;
    }
    else if ( dam <= 110 )
    {
        vs = CFG_DAM110;
        vp = CFG_DAM110S;
    }
    else if ( dam <= 115 )
    {
        vs = CFG_DAM115;
        vp = CFG_DAM115S;
    }
    else if ( dam <= 125 )
    {
        vs = CFG_DAM125;
        vp = CFG_DAM125S;
    }
    else if ( dam <= 130 )
    {
        vs = CFG_DAM130;
        vp = CFG_DAM130S;
    }
    else if ( dam <= 135 )
    {
        vs = CFG_DAM135;
        vp = CFG_DAM135S;
    }
    else if ( dam <= 145 )
    {
        vs = CFG_DAM145;
        vp = CFG_DAM145S;
    }
    else if ( dam <= 150 )
    {
        vs = CFG_DAM150;
        vp = CFG_DAM150S;
    }
    else if ( dam <= 155 )
    {
        vs = CFG_DAM155;
        vp = CFG_DAM155S;
    }
    else if ( dam <= 165 )
    {
        vs = CFG_DAM165;
        vp = CFG_DAM165S;
    }
    else if ( dam <= 170 )
    {
        vs = CFG_DAM170;
        vp = CFG_DAM170S;
    }
    else if ( dam <= 175 )
    {
        vs = CFG_DAM175;
        vp = CFG_DAM175S;
    }
    else if ( dam <= 185 )
    {
        vs = CFG_DAM185;
        vp = CFG_DAM185S;
    }
    else if ( dam <= 190 )
    {
        vs = CFG_DAM190;
        vp = CFG_DAM190S;
    }
    else if ( dam <= 195 )
    {
        vs = CFG_DAM195;
        vp = CFG_DAM195S;
    }
    else if ( dam <= 200 )
    {
        vs = CFG_DAM200;
        vp = CFG_DAM200S;
    }
    else if ( dam <= 205 )
    {
        vs = CFG_DAM205;
        vp = CFG_DAM205S;
    }
    else if ( dam <= 215 )
    {
        vs = CFG_DAM215;
        vp = CFG_DAM215S;
    }
    else if ( dam <= 220 )
    {
        vs = CFG_DAM220;
        vp = CFG_DAM220S;
    }
    else if ( dam <= 225 )
    {
        vs = CFG_DAM225;
        vp = CFG_DAM225S;
    }
    else if ( dam <= 230 )
    {
        vs = CFG_DAM230;
        vp = CFG_DAM230S;
    }
    else if ( dam <= 235 )
    {
        vs = CFG_DAM235;
        vp = CFG_DAM235S;
    }
    else if ( dam <= 245 )
    {
        vs = CFG_DAM245;
        vp = CFG_DAM245S;
    }
    else if ( dam <= 250 )
    {
        vs = CFG_DAM250;
        vp = CFG_DAM250S;
    }
    else if ( dam <= 255 )
    {
        vs = CFG_DAM255;
        vp = CFG_DAM255S;
    }
    else if ( dam <= 265 )
    {
        vs = CFG_DAM265;
        vp = CFG_DAM265S;
    }
    else if ( dam <= 270 )
    {
        vs = CFG_DAM265;
        vp = CFG_DAM265S;
    }
    else if ( dam <= 270 )
    {
        vs = CFG_DAM265;
        vp = CFG_DAM265S;
    }
    else if ( dam <= 275 )
    {
        vs = CFG_DAM265;
        vp = CFG_DAM265S;
    }
    else if ( dam <= 285 )
    {
        vs = CFG_DAM265;
        vp = CFG_DAM265S;
    }
    else if ( dam <= 290 )
    {
        vs = CFG_DAM265;
        vp = CFG_DAM265S;
    }
    else if ( dam <= 295 )
    {
        vs = CFG_DAM265;
        vp = CFG_DAM265S;
    }
    else if ( dam <= 300 )
    {
        vs = CFG_DAM265;
        vp = CFG_DAM265S;
    }
    else
    {
        vs = CFG_DAM_HUGE;
        vp = CFG_DAM_HUGES;
    }

    punct = ( damp <= 24 ) ? '.' : '!';

#else
#ifdef DAMAGE_BY_AMOUNT
    damp = dam * 2;
#else
    damp = ( dam * 100 / victim->max_hit );
#endif

    if ( dam == 0 )
    {
        vs = CFG_DAM0;
        vp = CFG_DAM0S;
    }
    else if ( damp <= 2 )
    {
        vs = CFG1_DAM2;
        vp = CFG1_DAM2S;
    }
    else if ( damp <= 4 )
    {
        vs = CFG1_DAM4;
        vp = CFG1_DAM4S;
    }
    else if ( damp <= 6 )
    {
        vs = CFG1_DAM6;
        vp = CFG1_DAM6S;
    }
    else if ( damp <= 8 )
    {
        vs = CFG1_DAM8;
        vp = CFG1_DAM8S;
    }
    else if ( damp <= 10 )
    {
        vs = CFG1_DAM10;
        vp = CFG1_DAM10S;
    }
    else if ( damp <= 12 )
    {
        vs = CFG1_DAM12;
        vp = CFG1_DAM12S;
    }
    else if ( damp <= 14 )
    {
        vs = CFG1_DAM14;
        vp = CFG1_DAM14S;
    }
    else if ( damp <= 16 )
    {
        vs = CFG1_DAM16;
        vp = CFG1_DAM16S;
    }
    else if ( damp <= 18 )
    {
        vs = CFG1_DAM18;
        vp = CFG1_DAM18S;
    }
    else if ( damp <= 20 )
    {
        vs = CFG1_DAM20;
        vp = CFG1_DAM20S;
    }
    else if ( damp <= 22 )
    {
        vs = CFG1_DAM22;
        vp = CFG1_DAM22S;
    }
    else if ( damp <= 24 )
    {
        vs = CFG1_DAM24;
        vp = CFG1_DAM24S;
    }
    else if ( damp <= 26 )
    {
        vs = CFG1_DAM26;
        vp = CFG1_DAM26S;
    }
    else if ( damp <= 28 )
    {
        vs = CFG1_DAM28;
        vp = CFG1_DAM28S;
    }
    else if ( damp <= 30 )
    {
        vs = CFG1_DAM30;
        vp = CFG1_DAM30S;
    }
    else if ( damp <= 37 )
    {
        vs = CFG1_DAM37;
        vp = CFG1_DAM37S;
    }
    else if ( damp <= 50 )
    {
        vs = CFG1_DAM50;
        vp = CFG1_DAM50S;
    }
    else if ( damp <= 63 )
    {
        vs = CFG1_DAM63;
        vp = CFG1_DAM63S;
    }
    else if ( damp <= 75 )
    {
        vs = CFG1_DAM75;
        vp = CFG1_DAM75S;
    }
    else if ( damp <= 83 )
    {
        vs = CFG1_DAM83;
        vp = CFG1_DAM83S;
    }
    else if ( damp <= 93 )
    {
        vs = CFG1_DAM93;
        vp = CFG1_DAM93S;
    }
    else
    {
        vs = CFG1_DAM_HUGE;
        vp = CFG1_DAM_HUGES;
    }

    punct = ( damp <= 24 ) ? '.' : '!';
#endif
#ifdef SHOW_DAMAGE_TO_CHARS
    if ( dt == TYPE_HIT )
    {
        if ( dam > 0 )
            if ( ch == victim )
            {
                sprintf( buf1, "`w$n `R%s`Y $melf%c", vp, punct );
                sprintf( buf2, "`wYou `R%s`Y yourself%c", vs, punct );
            }
            else
            {
                sprintf( buf1, "`G$n `R%s`G $N for `g%d`G points of damage%c",
                         vp, dam, punct );
                sprintf( buf2, "`YYou `R%s`Y $N for `R%d`Y points of damage%c",
                         vs, dam, punct );
                sprintf( buf3, "`C$n `R%s`C you for `c%d`C points of damage%c",
                         vp, dam, punct );
            }
        else if ( ch == victim )
        {
            sprintf( buf1, "`B$n %s $melf%c`w", vp, punct );
            sprintf( buf2, "`BYou %s yourself%c`w", vs, punct );
        }
        else
        {
            sprintf( buf1, "`G$n %s`G $N for `g%d`G points of damage%c`w", vp,
                     dam, punct );
            sprintf( buf2, "`YYou %s`Y $N for `R%d`Y points of damage%c`w", vs,
                     dam, punct );
            sprintf( buf3, "`C$n %s`C you for `c%d`C points of damage%c`w", vp,
                     dam, punct );
        }
    }
    else
    {
        if ( dt >= 0 && dt < MAX_SKILL )
            attack = skill_table[dt].noun_damage;
        else if ( dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE )
            attack = attack_table[dt - TYPE_HIT].name;
        else
        {
            bug( "Dam_message: bad dt %d.", dt );
            logf_string( "BUG: ^^ ch: %s victim: %s ", ch->name, victim->name );
            dt = TYPE_HIT;
            attack = attack_table[0].name;
        }

        if ( immune )
        {
            if ( ch == victim )
            {
                sprintf( buf1, "`B$n is unaffected by $s own %s.`w", attack );
                sprintf( buf2, "`BLuckily, you are immune to that.`w" );
            }
            else
            {
                sprintf( buf1, "`B$N is unaffected by $n's %s!`w", attack );
                sprintf( buf2, "`B$N is unaffected by your %s!`w", attack );
                sprintf( buf3, "`B$n's %s is powerless against you.`w",
                         attack );
            }
        }
        else
        {
            if ( dam > 0 )
                if ( ch == victim )
                {
                    sprintf( buf1, "`w$n's %s `R%s`w $m%c", attack, vp, punct );
                    sprintf( buf2, "`wYour %s `R%s`w you%c", attack, vp,
                             punct );
                }
                else
                {
                    sprintf( buf1,
                             "`G$n's %s`G `R%s`G $N for `g%d`G points of damage%c",
                             attack, vp, dam, punct );
                    sprintf( buf2,
                             "`YYour %s`Y `R%s`Y $N for `R%d`Y points of damage%c",
                             attack, vp, dam, punct );
                    sprintf( buf3,
                             "`C$n's %s`Y `R%s`C you for `c%d`C points of damage%c",
                             attack, vp, dam, punct );
                }
            else if ( ch == victim )
            {
                sprintf( buf1, "`B$n's %s %s $m%c`w", attack, vp, punct );
                sprintf( buf2, "`BYour %s %s you%c`w", attack, vp, punct );
            }
            else
            {
                sprintf( buf1,
                         "`G$n's %s %s`G $N for `g%d`G points of damage%c`w",
                         attack, vp, dam, punct );
                sprintf( buf2,
                         "`YYour %s %s `Y$N for `R%d`Y points of damage%c`w",
                         attack, vp, dam, punct );
                sprintf( buf3,
                         "`C$n's %s %s`C you for `c%d`C points of damage%c`w",
                         attack, vp, dam, punct );
            }
        }
    }
#else
    if ( dt == TYPE_HIT )
    {
        if ( dam > 0 )
            if ( ch == victim )
            {
                sprintf( buf1, "`w$n `R%s`Y $melf%c", vp, punct );
                sprintf( buf2, "`wYou `R%s`Y yourself%c", vs, punct );
            }
            else
            {
                sprintf( buf1, "`G$n `R%s`G $N%c", vp, punct );
                sprintf( buf2, "`YYou `R%s`Y $N%c", vs, punct );
                sprintf( buf3, "`C$n `R%s`C you%c", vp, punct );
            }
        else if ( ch == victim )
        {
            sprintf( buf1, "`B$n %s $melf%c`w", vp, punct );
            sprintf( buf2, "`BYou %s yourself%c`w", vs, punct );
        }
        else
        {
            sprintf( buf1, "`G$n %s `G$N%c`w", vp, punct );
            sprintf( buf2, "`YYou %s `Y$N%c`w", vs, punct );
            sprintf( buf3, "`C$n %s `Cyou%c`w", vp, punct );
        }
    }
    else
    {
        if ( dt >= 0 && dt < MAX_SKILL )
            attack = skill_table[dt].noun_damage;
        else if ( dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE )
            attack = attack_table[dt - TYPE_HIT].name;
        else
        {
            bug( "Dam_message: bad dt %d.", dt );
            dt = TYPE_HIT;
            attack = attack_table[0].name;
        }

        if ( immune )
        {
            if ( ch == victim )
            {
                sprintf( buf1, "`B$n is unaffected by $s own %s.`w", attack );
                sprintf( buf2, "`BLuckily, you are immune to that.`w" );
            }
            else
            {
                sprintf( buf1, "`G$N is unaffected by $n's %s!`w", attack );
                sprintf( buf2, "`Y$N is unaffected by your %s!`w", attack );
                sprintf( buf3, "`C$n's %s is powerless against you.`w",
                         attack );
            }
        }
        else
        {
            if ( dam > 0 )
                if ( ch == victim )
                {
                    sprintf( buf1, "`w$n's %s `R%s`w $m%c", attack, vp, punct );
                    sprintf( buf2, "`wYour %s `R%s`w you%c", attack, vp,
                             punct );
                }
                else
                {
                    sprintf( buf1, "`G$n's %s `R%s`G $N%c", attack, vp, punct );
                    sprintf( buf2, "`YYour %s `R%s`Y $N%c", attack, vp, punct );
                    sprintf( buf3, "`C$n's %s `R%s`C you%c", attack, vp,
                             punct );
                }
            else if ( ch == victim )
            {
                sprintf( buf1, "`B$n's %s %s $m%c`w", attack, vp, punct );
                sprintf( buf2, "`BYour %s %s you%c`w", attack, vp, punct );
            }
            else
            {
                sprintf( buf1, "`G$n's %s %s `G$N%c`w", attack, vp, punct );
                sprintf( buf2, "`YYour %s %s `Y$N%c`w", attack, vp, punct );
                sprintf( buf3, "`C$n's %s %s `Cyou%c`w", attack, vp, punct );
            }
        }
    }
#endif
    if ( ch == victim )
    {
        act( buf1, ch, NULL, NULL, TO_ROOM );
        act( buf2, ch, NULL, NULL, TO_CHAR );
    }
    else
    {
        act( buf1, ch, NULL, victim, TO_NOTVICT );
        act( buf2, ch, NULL, victim, TO_CHAR );
        act( buf3, ch, NULL, victim, TO_VICT );
    }

    return;
}

/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * target_weapon )
{
    if ( IS_OBJ_STAT( target_weapon, ITEM_NOREMOVE ) )
    {
        act( "`W$S weapon won't budge!`w", ch, NULL, victim, TO_CHAR );
        act( "`W$n tries to disarm you, but your weapon won't budge!`w",
             ch, NULL, victim, TO_VICT );
        act( "`W$n tries to disarm $N, but fails.`w", ch, NULL, victim,
             TO_NOTVICT );
        return;
    }
    if ( !IS_NPC( victim ) &&
         victim->level > skill_table[gsn_grip].skill_level[victim->Class] )
    {
        int skill = get_skill( victim, gsn_grip );

        skill +=
            ( get_curr_stat( victim, STAT_STR ) -
              get_curr_stat( ch, STAT_STR ) ) * 5;
        if ( number_percent(  ) < skill )
        {
            act( "$N grips and prevent you to disarm $S!", ch, NULL, victim,
                 TO_CHAR );
            act( "$n tries to disarm you, but you grip and escape!", ch, NULL,
                 victim, TO_VICT );
            act( "$n tries to disarm $N, but fails.", ch, NULL, victim,
                 TO_NOTVICT );
            check_improve( victim, gsn_grip, TRUE, 1 );
            return;
        }
        else
            check_improve( victim, gsn_grip, FALSE, 1 );
    }

    act( "`W$n disarms you and sends your weapon flying!`w",
         ch, NULL, victim, TO_VICT );
    act( "`WYou disarm $N!`w", ch, NULL, victim, TO_CHAR );
    act( "`W$n disarms $N!`w", ch, NULL, victim, TO_NOTVICT );

    obj_from_char( target_weapon );
    if ( IS_OBJ_STAT( target_weapon, ITEM_NODROP )
         || IS_OBJ_STAT( target_weapon, ITEM_INVENTORY ) )
        obj_to_char( target_weapon, victim );
    else
    {
        obj_to_room( target_weapon, victim->in_room );
        if ( IS_NPC( victim ) && victim->wait == 0
             && can_see_obj( victim, target_weapon ) )
            get_obj( victim, target_weapon, NULL );
    }

    return;
}

void do_berserk( CHAR_DATA * ch, char *argument )
{
    int chance, hp_percent;

    if ( ( chance = get_skill( ch, gsn_berserk ) ) == 0
         || ( IS_NPC( ch ) && !IS_SET( ch->off_flags, OFF_BERSERK ) )
         || ( !IS_NPC( ch )
              && ch->level < skill_table[gsn_berserk].skill_level[ch->Class] )
         || !can_use( ch, gsn_berserk ) )
    {
        send_to_char( "You turn red in the face, but nothing happens.\n\r",
                      ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_BERSERK ) || is_affected( ch, gsn_berserk )
         || is_affected( ch, skill_lookup( "frenzy" ) ) )
    {
        send_to_char( "You get a little madder.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CALM ) )
    {
        send_to_char( "You're feeling to mellow to berserk.\n\r", ch );
        return;
    }

    if ( ch->mana < 50 )
    {
        send_to_char( "You can't get up enough energy.\n\r", ch );
        return;
    }

    /* modifiers */

    /* fighting */
    if ( ch->position == POS_FIGHTING )
        chance += 10;

    /* damage -- below 50% of hp helps, above hurts */
    hp_percent = 100 * ch->hit / ch->max_hit;
    chance += 25 - hp_percent / 2;

    if ( number_percent(  ) < chance )
    {
        AFFECT_DATA af;

        WAIT_STATE( ch, PULSE_VIOLENCE );
        ch->mana -= 50;
        ch->move /= 2;

        /* heal a little damage */
        ch->hit += ch->level * 2;
        ch->hit = UMIN( ch->hit, ch->max_hit );

        send_to_char( "`RYour pulse races as you are consumned by rage!\n\r`w",
                      ch );
        act( "`W$n gets a wild look in $s eyes.`w", ch, NULL, NULL, TO_ROOM );
        check_improve( ch, gsn_berserk, TRUE, 2 );

        af.type = gsn_berserk;
        af.level = ch->level;
        af.duration = number_fuzzy( ch->level / 8 );
        af.modifier = UMAX( 1, ch->level / 5 );
        af.bitvector = AFF_BERSERK;

        af.location = APPLY_HITROLL;
        affect_to_char( ch, &af );

        af.location = APPLY_DAMROLL;
        affect_to_char( ch, &af );

        af.modifier = UMAX( 10, 10 * ( ch->level / 5 ) );
        af.location = APPLY_AC;
        affect_to_char( ch, &af );
    }

    else
    {
        WAIT_STATE( ch, 3 * PULSE_VIOLENCE );
        ch->mana -= 25;
        ch->move /= 2;

        send_to_char( "Your pulse speeds up, but nothing happens.\n\r", ch );
        check_improve( ch, gsn_berserk, FALSE, 2 );
    }
}

void do_bash( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument( argument, arg );

    if ( ( chance = get_skill( ch, gsn_bash ) ) == 0
         || ( IS_NPC( ch ) && !IS_SET( ch->off_flags, OFF_BASH ) )
         || ( !IS_NPC( ch )
              && ch->level < skill_table[gsn_bash].skill_level[ch->Class] )
         || !can_use( ch, gsn_bash ) )
    {
        send_to_char( "Bashing? What's that?\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
        victim = ch->fighting;
        if ( victim == NULL )
        {
            send_to_char( "But you aren't fighting anyone!\n\r", ch );
            return;
        }
    }

    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    /* no player killing */
    if ( !IS_NPC( victim ) && !IS_NPC( ch )
         && ( ( !chaos && !IS_SET( victim->act, PLR_KILLER ) )
              || ( !chaos && !IS_SET( ch->act, PLR_KILLER ) ) ) )
    {
        send_to_char( "You can only kill other player killers.\n\r", ch );
        return;
    }

    if ( victim->position < POS_FIGHTING )
    {
        act( "You'll have to let $M get back up first.", ch, NULL, victim,
             TO_CHAR );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You try to bash your brains out, but fail.\n\r", ch );
        return;
    }

    if ( is_safe( ch, victim ) )
        return;

    if ( victim->fighting != NULL && !is_same_group( ch, victim->fighting ) )
    {
        send_to_char( "Kill stealing is not permitted.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
        act( "But $N is your friend!", ch, NULL, victim, TO_CHAR );
        return;
    }

    /* modifiers */

    /* size  and weight */
    chance += ch->carry_weight / 25;
    chance -= victim->carry_weight / 20;

    if ( ch->size < victim->size )
        chance += ( ch->size - victim->size ) * 25;
    else
        chance += ( ch->size - victim->size ) * 10;

    /* stats */
    chance += get_curr_stat( ch, STAT_STR );
    chance -= get_curr_stat( victim, STAT_DEX ) * 4 / 3;

    /* speed */
    if ( IS_SET( ch->off_flags, OFF_FAST ) )
        chance += 10;
    if ( IS_SET( victim->off_flags, OFF_FAST ) )
        chance -= 20;

    /* level */
    chance += ( ch->level - victim->level ) * 2;

    /* now the attack */
    if ( number_percent(  ) < chance )
    {

        act( "`W$n sends you sprawling with a powerful bash!`w",
             ch, NULL, victim, TO_VICT );
        act( "`WYou slam into $N, and send $M flying!`w", ch, NULL, victim,
             TO_CHAR );
        act( "`W$n sends $N sprawling with a powerful bash.`w", ch, NULL,
             victim, TO_NOTVICT );
        check_improve( ch, gsn_bash, TRUE, 1 );

        WAIT_STATE( victim, 3 * PULSE_VIOLENCE );
        WAIT_STATE( ch, skill_table[gsn_bash].beats );
        victim->position = POS_RESTING;
        damage( ch, victim, NULL,
                number_range( 2, 2 + 2 * ch->size + chance / 20 ), gsn_bash,
                DAM_BASH );

    }
    else
    {
        damage( ch, victim, NULL, 0, gsn_bash, DAM_BASH );
        act( "`BYou fall flat on your face!`w", ch, NULL, victim, TO_CHAR );
        act( "`B$n falls flat on $s face.`w", ch, NULL, victim, TO_NOTVICT );
        act( "`BYou evade $n's bash, causing $m to fall flat on $s face.`w",
             ch, NULL, victim, TO_VICT );
        check_improve( ch, gsn_bash, FALSE, 1 );
        ch->position = POS_RESTING;
        WAIT_STATE( ch, skill_table[gsn_bash].beats * 3 / 2 );
    }
}

void do_dirt( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument( argument, arg );

    if ( ( chance = get_skill( ch, gsn_dirt ) ) == 0
         || ( IS_NPC( ch ) && !IS_SET( ch->off_flags, OFF_KICK_DIRT ) )
         || ( !IS_NPC( ch )
              && ch->level < skill_table[gsn_dirt].skill_level[ch->Class] ) )
    {
        send_to_char( "You get your feet dirty.\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
        victim = ch->fighting;
        if ( victim == NULL )
        {
            send_to_char( "But you aren't in combat!\n\r", ch );
            return;
        }
    }

    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( victim, AFF_BLIND ) )
    {
        act( "$e's already been blinded.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "Very funny.\n\r", ch );
        return;
    }

    if ( is_safe( ch, victim ) )
        return;

    /* no player killing */
    if ( !IS_NPC( victim ) && !IS_NPC( ch )
         && ( ( !chaos && !IS_SET( victim->act, PLR_KILLER ) )
              || ( !chaos && !IS_SET( ch->act, PLR_KILLER ) ) ) )
    {
        send_to_char( "You can only kill other player killers.\n\r", ch );
        return;
    }

    if ( victim->fighting != NULL && !is_same_group( ch, victim->fighting ) )
    {
        send_to_char( "Kill stealing is not permitted.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
        act( "But $N is such a good friend!", ch, NULL, victim, TO_CHAR );
        return;
    }

    /* modifiers */

    /* dexterity */
    chance += get_curr_stat( ch, STAT_DEX );
    chance -= 2 * get_curr_stat( victim, STAT_DEX );

    /* speed  */
    if ( IS_SET( ch->off_flags, OFF_FAST ) || IS_AFFECTED( ch, AFF_HASTE ) )
        chance += 10;
    if ( IS_SET( victim->off_flags, OFF_FAST )
         || IS_AFFECTED( victim, AFF_HASTE ) )
        chance -= 25;

    /* level */
    chance += ( ch->level - victim->level ) * 2;

    /* sloppy hack to prevent false zeroes */
    if ( chance % 5 == 0 )
        chance += 1;

    /* terrain */

    switch ( ch->in_room->sector_type )
    {
    case ( SECT_INSIDE ):
        chance -= 20;
        break;
    case ( SECT_CITY ):
        chance -= 10;
        break;
    case ( SECT_FIELD ):
        chance += 5;
        break;
    case ( SECT_FOREST ):
        break;
    case ( SECT_HILLS ):
        break;
    case ( SECT_MOUNTAIN ):
        chance -= 10;
        break;
    case ( SECT_WATER_SWIM ):
        chance = 0;
        break;
    case ( SECT_WATER_NOSWIM ):
        chance = 0;
        break;
    case ( SECT_AIR ):
        chance = 0;
        break;
    case ( SECT_DESERT ):
        chance += 10;
        break;
    }

    if ( chance == 0 )
    {
        send_to_char( "There isn't any dirt to kick.\n\r", ch );
        return;
    }

    /* now the attack */
    if ( number_percent(  ) < chance )
    {
        AFFECT_DATA af;
        act( "`W$n is blinded by the dirt in $s eyes!`w", victim, NULL, NULL,
             TO_ROOM );
        damage( ch, victim, NULL, number_range( 2, 5 ), gsn_dirt, DAM_NONE );
        send_to_char( "`WYou can't see a thing!\n\r`w", victim );
        check_improve( ch, gsn_dirt, TRUE, 2 );
        WAIT_STATE( ch, skill_table[gsn_dirt].beats );

        af.type = gsn_dirt;
        af.level = ch->level;
        af.duration = 0;
        af.location = APPLY_HITROLL;
        af.modifier = -4;
        af.bitvector = AFF_BLIND;

        affect_to_char( victim, &af );
    }
    else
    {
        damage( ch, victim, NULL, 0, gsn_dirt, DAM_NONE );
        check_improve( ch, gsn_dirt, FALSE, 2 );
        WAIT_STATE( ch, skill_table[gsn_dirt].beats );
    }
}

void do_trip( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;

    one_argument( argument, arg );

    if ( ( chance = get_skill( ch, gsn_trip ) ) == 0
         || ( IS_NPC( ch ) && !IS_SET( ch->off_flags, OFF_TRIP ) )
         || ( !IS_NPC( ch )
              && ch->level < skill_table[gsn_trip].skill_level[ch->Class] ) )
    {
        send_to_char( "Tripping?  What's that?\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
        victim = ch->fighting;
        if ( victim == NULL )
        {
            send_to_char( "But you aren't fighting anyone!\n\r", ch );
            return;
        }
    }

    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( is_safe( ch, victim ) )
        return;

    /* no player killing */
    if ( !IS_NPC( victim ) && !IS_NPC( ch )
         && ( ( !chaos && !IS_SET( victim->act, PLR_KILLER ) )
              || ( !chaos && !IS_SET( ch->act, PLR_KILLER ) ) ) )
    {
        send_to_char( "You can only kill other player killers.\n\r", ch );
        return;
    }

    if ( victim->fighting != NULL && !is_same_group( ch, victim->fighting ) )
    {
        send_to_char( "Kill stealing is not permitted.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( victim, AFF_FLYING ) )
    {
        act( "$S feet aren't on the ground.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( victim->position < POS_FIGHTING )
    {
        act( "$N is already down.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "`BYou fall flat on your face!\n\r`w", ch );
        WAIT_STATE( ch, 2 * skill_table[gsn_trip].beats );
        act( "$n trips over $s own feet!", ch, NULL, NULL, TO_ROOM );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
        act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
        return;
    }

    /* modifiers */

    /* size */
    if ( ch->size < victim->size )
        chance += ( ch->size - victim->size ) * 10; /* bigger = harder to trip */

    /* dex */
    chance += get_curr_stat( ch, STAT_DEX );
    chance -= get_curr_stat( victim, STAT_DEX ) * 3 / 2;

    /* speed */
    if ( IS_SET( ch->off_flags, OFF_FAST ) || IS_AFFECTED( ch, AFF_HASTE ) )
        chance += 10;
    if ( IS_SET( victim->off_flags, OFF_FAST )
         || IS_AFFECTED( victim, AFF_HASTE ) )
        chance -= 20;

    /* level */
    chance += ( ch->level - victim->level ) * 2;

    /* now the attack */
    if ( number_percent(  ) < chance )
    {
        act( "`W$n trips you and you go down!`w", ch, NULL, victim, TO_VICT );
        act( "`WYou trip $N and $N goes down!`w", ch, NULL, victim, TO_CHAR );
        act( "`W$n trips $N, sending $M to the ground.`w", ch, NULL, victim,
             TO_NOTVICT );
        check_improve( ch, gsn_trip, TRUE, 1 );

        WAIT_STATE( victim, 2 * PULSE_VIOLENCE );
        WAIT_STATE( ch, skill_table[gsn_trip].beats );
        victim->position = POS_RESTING;
        damage( ch, victim, NULL, number_range( 2, 2 + 2 * victim->size ),
                gsn_trip, DAM_BASH );
    }
    else
    {
        damage( ch, victim, NULL, 0, gsn_trip, DAM_BASH );
        WAIT_STATE( ch, skill_table[gsn_trip].beats * 2 / 3 );
        check_improve( ch, gsn_trip, FALSE, 1 );
    }
}

void do_kill( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Kill whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    /* no player killing */
    if ( !IS_NPC( victim ) && !IS_NPC( ch )
         && ( ( !chaos && !IS_SET( victim->act, PLR_KILLER ) )
              || ( !chaos && !IS_SET( ch->act, PLR_KILLER ) ) ) )
    {
        send_to_char( "You can only kill other player killers.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "You hit yourself.  Ouch!\n\r", ch );
        multi_hit( ch, ch, TYPE_UNDEFINED );
        return;
    }

    if ( is_safe( ch, victim ) )
        return;

    if ( victim->fighting != NULL && !is_same_group( ch, victim->fighting ) )
    {
        send_to_char( "Kill stealing is not permitted.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
        act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "You do the best you can!\n\r", ch );
        return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}

void do_murde( CHAR_DATA * ch, char *argument )
{
    send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
    return;
}

void do_murder( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Murder whom?\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM )
         || ( IS_NPC( ch ) && IS_SET( ch->act, ACT_PET ) ) )
        return;

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "Suicide is a mortal sin.\n\r", ch );
        return;
    }

    if ( is_safe( ch, victim ) )
        return;

    if ( victim->fighting != NULL && !is_same_group( ch, victim->fighting ) )
    {
        send_to_char( "Kill stealing is not permitted.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
        act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "You do the best you can!\n\r", ch );
        return;
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    if ( IS_NPC( ch ) )
        sprintf( buf, "`YHelp! I am being attacked by %s!`w", ch->short_descr );
    else
        sprintf( buf, "`YHelp!  I am being attacked by %s!`w", ch->name );
    do_yell( victim, buf );
    check_killer( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}

void do_backstab( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Backstab whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    if ( !IS_NPC( ch ) && !can_use( ch, gsn_backstab ) )
    {
        send_to_char( "You would probably only slice off your own finger!\n\r",
                      ch );
        return;
    }
    if ( victim == ch )
    {
        send_to_char( "How can you sneak up on yourself?\n\r", ch );
        return;
    }

    /* no player killing */
    if ( !IS_NPC( victim ) && !IS_NPC( ch )
         && ( ( !chaos && !IS_SET( victim->act, PLR_KILLER ) )
              || ( !chaos && !IS_SET( ch->act, PLR_KILLER ) ) ) )
    {
        send_to_char( "You can only kill other player killers.\n\r", ch );
        return;
    }

    if ( victim->fighting != NULL && !is_same_group( ch, victim->fighting ) )
    {
        send_to_char( "Kill stealing is not permitted.\n\r", ch );
        return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
    {
        send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
        return;
    }

#ifdef ONLY_DAGGER_BS
    if ( obj->value[0] != WEAPON_DAGGER )
    {
        send_to_char( "Your weapon is too unwieldy to backstab with.\n\r", ch );
        return;
    }
#endif

    if ( victim->fighting != NULL )
    {
        send_to_char( "You can't backstab a fighting person.\n\r", ch );
        return;
    }

    if ( victim->hit < victim->max_hit && !IS_AFFECTED( ch, AFF_SNEAK ) )
    {
        act( "$N is hurt and suspicious ... you can't sneak up.",
             ch, NULL, victim, TO_CHAR );
        return;
    }

    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[gsn_backstab].beats );
    if ( !IS_AWAKE( victim )
         || IS_NPC( ch )
         || number_percent(  ) < ch->pcdata->learned[gsn_backstab] )
    {
        check_improve( ch, gsn_backstab, TRUE, 1 );
        multi_hit( ch, victim, gsn_backstab );
    }
    else
    {
        check_improve( ch, gsn_backstab, FALSE, 1 );
        damage( ch, victim, NULL, 0, gsn_backstab, DAM_NONE );
    }

    return;
}

void do_circle( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Circle whom?\n\r", ch );
        return;
    }

    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( is_safe( ch, victim ) )
        return;

    if ( IS_NPC( victim ) && victim->fighting != NULL
         && !is_same_group( ch, victim->fighting ) )

    {
        send_to_char( "Kill stealing is not permitted.\n\r", ch );
        return;
    }

    if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
    {
        send_to_char( "You need to wield a weapon to circle.\n\r", ch );
        return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
        send_to_char( "You must be fighting in order to circle.\n\r", ch );
        return;
    }

    check_killer( ch, victim );
    WAIT_STATE( ch, skill_table[gsn_circle].beats );
    if ( number_percent(  ) < get_skill( ch, gsn_circle )
         || ( get_skill( ch, gsn_circle ) >= 2 && !IS_AWAKE( victim ) ) )
    {
        check_improve( ch, gsn_circle, TRUE, 1 );
        multi_hit( ch, victim, gsn_circle );
    }
    else
    {
        check_improve( ch, gsn_circle, FALSE, 1 );
        damage( ch, victim, NULL, 0, gsn_circle, DAM_NONE );
    }

    return;
}

void do_flee( CHAR_DATA * ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    CHAR_DATA *victim;
    int attempt;
    long lost_exp;

    if ( ( victim = ch->fighting ) == NULL )
    {
        if ( ch->position == POS_FIGHTING )
            ch->position = POS_STANDING;
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }

    was_in = ch->in_room;
    for ( attempt = 0; attempt < 6; attempt++ )
    {
        EXIT_DATA *pexit;
        int door;

        door = number_door(  );
        if ( ( pexit = was_in->exit[door] ) == 0
             || pexit->u1.to_room == NULL
             || IS_SET( pexit->exit_info, EX_CLOSED )
             || ( IS_NPC( ch )
                  && IS_SET( pexit->u1.to_room->room_flags, ROOM_NO_MOB ) )
             || IS_SET( pexit->u1.to_room->room_flags, ROOM_NO_FLEE_TO ) )
            continue;

        move_char( ch, door, FALSE );
        if ( ( now_in = ch->in_room ) == was_in )
            continue;

        ch->in_room = was_in;
        act( "$n has fled!", ch, NULL, NULL, TO_ROOM );
        ch->in_room = now_in;

        if ( !IS_NPC( ch ) )
        {
            lost_exp =
                ( long ) ( 0.05 * exp_per_level( ch, ch->pcdata->points ) );
            send_to_char( "You flee from combat!\n\r", ch );
            gain_exp( ch, ( -1 * lost_exp ) );
            ch->exp_stack -= lost_exp;
        }
        stop_hating( ch );
        stop_fighting( ch, TRUE );
        return;
    }

    send_to_char( "`RPANIC! You couldn't escape!\n\r`w", ch );
    return;
}

void do_rescue( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *fch;
    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Rescue whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "What about fleeing instead?\n\r", ch );
        return;
    }

    if ( !is_same_group( ch, victim ) )
    {
        send_to_char( "Kill stealing is not permitted.\n\r", ch );
        return;
    }

    if ( !IS_NPC( ch ) && IS_NPC( victim ) )
    {
        send_to_char( "Doesn't need your help!\n\r", ch );
        return;
    }

    if ( ch->fighting == victim )
    {
        send_to_char( "Too late.\n\r", ch );
        return;
    }

    if ( ( fch = victim->fighting ) == NULL )
    {
        send_to_char( "That person is not fighting right now.\n\r", ch );
        return;
    }

    WAIT_STATE( ch, skill_table[gsn_rescue].beats );
    if ( !IS_NPC( ch ) && number_percent(  ) > ch->pcdata->learned[gsn_rescue] )
    {
        send_to_char( "`BYou fail the rescue.\n\r`w", ch );
        check_improve( ch, gsn_rescue, FALSE, 1 );
        return;
    }
    if ( IS_NPC( ch )
         && number_percent(  ) >
         ( ( ( ( abs( ch->level - RESCUE_PENALTY ) +
                 1 ) / MAX_LEVEL ) * 100 ) ) )
    {
        send_to_char( "`BYou fail the rescue.\n\r`w", ch );
        return;
    }

    act( "`WYou rescue $N!`w", ch, NULL, victim, TO_CHAR );
    act( "`W$n rescues you!`w", ch, NULL, victim, TO_VICT );
    act( "`W$n rescues $N!`w", ch, NULL, victim, TO_NOTVICT );
    check_improve( ch, gsn_rescue, TRUE, 1 );
    stop_fighting( fch, FALSE );
    stop_fighting( victim, FALSE );

    check_killer( ch, fch );
    set_fighting( ch, fch );
    set_fighting( fch, ch );
    return;
}

void do_blackjack( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int chance;
    NEWAFFECT_DATA naf;

    one_argument( argument, arg );
    if ( ( chance = get_skill( ch, gsn_blackjack ) ) == 0 )
    {
        send_to_char( "You cant blackjack.\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' )
    {
        send_to_char( "You must specify a target.\n\r", ch );
        return;
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim->fighting != NULL && !is_same_group( ch, victim->fighting ) )
    {
        send_to_char( "Kill stealing is not permitted.\n\r", ch );
        return;
    }

    /* Don't allow blackjacking a mob you're already fighting. */
    if ( victim->fighting == ch )
    {
        send_to_char
            ( "Kind of hard to hit them on the back of the head when they're already trying to tear you to pieces!\n\r",
              ch );
        return;
    }

    if ( is_safe( ch, victim ) )
        return;
    if ( !IS_NPC( victim ) )
    {
        if ( ( !chaos && !IS_SET( victim->act, PLR_KILLER ) )
             || ( !chaos && !IS_SET( ch->act, PLR_KILLER ) )
             || IS_NEWAFF_SET( victim->newaff, NEWAFF_BLACKJACK ) )
        {
            send_to_char( "You can only kill other player killers.\n\r", ch );
            return;
        }
    }

    if ( IS_AFFECTED( victim, AFF_SLEEP )
         || IS_NEWAFF_SET( victim->newaff, NEWAFF_BLACKJACK ) )
    {
        act( "$E's already been blackjacked.", ch, NULL, victim, TO_CHAR );
        return;
    }
    if ( victim == ch )
    {
        send_to_char( "Very funny.\n\r", ch );
        return;
    }
    if ( is_safe( ch, victim ) )
        return;
    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
    {
        act( "But $N is such a good friend!", ch, NULL, victim, TO_CHAR );
        return;
    }
    /* level */
    chance += ( ch->level - ch->level );
    /* sloppy hack to prevent false zeroes */
    if ( chance % 5 == 0 )
        chance += 1;
    if ( chance == 0 )
    {
        send_to_char( "You failed horrbly!.\n\r", ch );
        return;
    }
/*    if (number_percent() < chance)*/
    chance += ( number_range( 1, 25 ) );
    if ( chance <= 74 )
    {
        act( "`R** WHOOSH ** Uh oo, your Blackjack missed $M!`w", ch, NULL,
             victim, TO_CHAR );
        act( "`RWHOA, $n just tried to take you out with 1 hit, luckily you ducked first!`w", ch, NULL, victim, TO_VICT );
        act( "`R$n just took a HUGE swing at $N and missed! HAHAHAHA`w", ch,
             NULL, victim, TO_NOTVICT );
        multi_hit( ch, victim, gsn_blackjack );
        return;
    }
    else if ( chance <= 110 )
    {
        act( "`ROh yes baby! Direct hit! $N MUST be hurting now Muahaha!`w", ch,
             NULL, victim, TO_CHAR );
        act( "`R*** WHAM! *** $n just nailed you in the face! There are 3 $n's now!`w", ch, NULL, victim, TO_VICT );
        act( "`R$n just nailed $N square in the kisser. Now THAT musta hurt.`w",
             ch, NULL, victim, TO_NOTVICT );

        check_improve( ch, gsn_blackjack, TRUE, 2 );
        SET_NEWAFF( victim->newaff, NEWAFF_BLACKJACK );
        naf.type = gsn_blackjack;
        naf.level = ch->level;
        naf.duration = ch->level / 10;
        naf.location = APPLY_NONE;
        naf.modifier = 0;
        naf.bitvector = NEWAFF_BLACKJACK;
        newaffect_join( victim, &naf );
        victim->position = POS_RESTING;
        multi_hit( ch, victim, gsn_blackjack );

    }
    else if ( chance >= 111 )
    {
        act( "`R *** SMASH!!!!! *** Hehe got'm good that time.`w", ch, NULL,
             victim, TO_CHAR );
        act( "`R*** CRACK *** What in the world was that! It sure did hurt!`w",
             ch, NULL, victim, TO_VICT );
        act( "`RMAN $n MUST be pissed! He just knocked $N out `WCOLD`R!`w", ch,
             NULL, victim, TO_NOTVICT );
        send_to_char( "`WYou are knocked out cold!\n\r", victim );

        check_improve( ch, gsn_blackjack, TRUE, 2 );
        SET_NEWAFF( victim->newaff, NEWAFF_BLACKJACK );
        naf.type = gsn_blackjack;
        naf.level = ch->level;
        naf.duration = ch->level / 10;
        naf.location = APPLY_NONE;
        naf.modifier = 0;
        naf.bitvector = NEWAFF_BLACKJACK;
        newaffect_join( victim, &naf );
        multi_hit( ch, victim, gsn_blackjack );
        stop_fighting( ch, FALSE );
        stop_fighting( victim, FALSE );
        if ( IS_AWAKE( victim ) )
        {
            send_to_char( "Nighty Night.\n\r", victim );
            act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
            victim->position = POS_SLEEPING;
        }

/* This damaged the victim too, but i couldnt get it the numbers right.

damage(ch,victim,number_range(2,5),gsn_blackjack,DAM_NONE,FALSE);*/

    }
    else
    {                           /* Another try damage(ch,victim,0,gsn_blackjack,DAM_NONE,TRUE); */
        check_improve( ch, gsn_blackjack, FALSE, 2 );
    }
}

void do_kick( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA *victim;

    if ( !IS_NPC( ch )
         && ch->level < skill_table[gsn_kick].skill_level[ch->Class] )
    {
        send_to_char( "You better leave the martial arts to fighters.\n\r",
                      ch );
        return;
    }

    if ( IS_NPC( ch ) && !IS_SET( ch->off_flags, OFF_KICK ) )
        return;

    if ( ( victim = ch->fighting ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }

    /* no player killing */
    if ( !IS_NPC( victim ) && !IS_NPC( ch )
         && ( ( !chaos && !IS_SET( victim->act, PLR_KILLER ) )
              || ( !chaos && !IS_SET( ch->act, PLR_KILLER ) ) ) )
    {
        send_to_char( "You can only kill other player killers.\n\r", ch );
        return;
    }

    WAIT_STATE( ch, skill_table[gsn_kick].beats );
    if ( IS_NPC( ch ) || number_percent(  ) < ch->pcdata->learned[gsn_kick] )
    {
        damage( ch, victim, NULL, number_range( 1, ch->level ), gsn_kick,
                DAM_BASH );
        check_improve( ch, gsn_kick, TRUE, 1 );
    }
    else
    {
        damage( ch, victim, NULL, 0, gsn_kick, DAM_BASH );
        check_improve( ch, gsn_kick, FALSE, 1 );
    }

    return;
}

void do_disarm( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA *victim;
    OBJ_DATA *primary_weapon;
    OBJ_DATA *secondary_weapon;

    int chance, hth, ch_weapon, vict_weapon, ch_vict_weapon;

    hth = 0;

    if ( ( chance = get_skill( ch, gsn_disarm ) ) == 0 )
    {
        send_to_char( "You don't know how to disarm opponents.\n\r", ch );
        return;
    }

    if ( get_eq_char( ch, WEAR_WIELD ) == NULL
         && get_eq_char( ch, WEAR_SECOND_WIELD ) == NULL
         && ( ( hth = get_skill( ch, gsn_hand_to_hand ) ) == 0
              || ( IS_NPC( ch ) && !IS_SET( ch->off_flags, OFF_DISARM ) ) ) )
    {
        send_to_char( "You must wield a weapon to disarm.\n\r", ch );
        return;
    }

    if ( ( victim = ch->fighting ) == NULL )
    {
        send_to_char( "You aren't fighting anyone.\n\r", ch );
        return;
    }

    primary_weapon = get_eq_char( victim, WEAR_WIELD );
    secondary_weapon = get_eq_char( victim, WEAR_SECOND_WIELD );

    if ( primary_weapon == NULL && secondary_weapon == NULL )
    {
        send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
        return;
    }

    /* find weapon skills */
    ch_weapon = get_weapon_skill( ch, get_weapon_sn( ch ) );
    vict_weapon = get_weapon_skill( victim, get_weapon_sn( victim ) );
    ch_vict_weapon = get_weapon_skill( ch, get_weapon_sn( victim ) );

    /* modifiers */

    /* skill */
    if ( get_eq_char( ch, WEAR_WIELD ) == NULL
         && get_eq_char( ch, WEAR_SECOND_WIELD ) == NULL )
        chance = chance * hth / 150;
    else
        chance = chance * ch_weapon / 100;

    chance += ( ch_vict_weapon / 2 - vict_weapon ) / 2;

    /* dex vs. strength */
    chance += get_curr_stat( ch, STAT_DEX );
    chance -= 2 * get_curr_stat( victim, STAT_STR );

    /* level */
    chance += ( ch->level - victim->level ) * 2;
    /* and now the attack */
    if ( number_percent(  ) < chance )
    {
        WAIT_STATE( ch, skill_table[gsn_disarm].beats );

        /*
         * If the target is wielding two weapons, flip a coin
         * for the target.
         */
        if ( primary_weapon && secondary_weapon )
            if ( number_percent(  ) <= 50 )
            {
                disarm( ch, victim, primary_weapon );
                if ( secondary_weapon
                     && ( get_eq_char( ch, WEAR_WIELD ) == NULL ) )
                    secondary_weapon->wear_loc = WEAR_WIELD;
            }
            else
                disarm( ch, victim, secondary_weapon );
        else
            disarm( ch, victim,
                    ( primary_weapon ? primary_weapon : secondary_weapon ) );

        check_improve( ch, gsn_disarm, TRUE, 1 );
    }
    else
    {
        WAIT_STATE( ch, skill_table[gsn_disarm].beats );
        act( "`BYou fail to disarm $N.`w", ch, NULL, victim, TO_CHAR );
        act( "`B$n tries to disarm you, but fails.`w", ch, NULL, victim,
             TO_VICT );
        act( "`B$n tries to disarm $N, but fails.`w", ch, NULL, victim,
             TO_NOTVICT );
        check_improve( ch, gsn_disarm, FALSE, 1 );
    }
    return;
}

void do_sla( CHAR_DATA * ch, char *argument )
{
    send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
    return;
}

void do_slay( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    extern bool chaos;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Slay whom?\n\r", ch );
        return;
    }
    if ( !str_cmp( arg, "all" ) && ( ch->level >= MAX_LEVEL ) )
    {
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->connected == CON_PLAYING
                 && d->character != ch
                 && d->character->in_room != NULL
                 && can_see( ch, d->character ) )
            {
                sprintf( buf, "%s", d->character->name );
                do_slay( ch, buf );
            }
        }
        return;
    }
    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ch == victim )
    {
        send_to_char( "Suicide is a mortal sin.\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) && victim->level >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    act( "`RYou slay $M in cold blood!`w", ch, NULL, victim, TO_CHAR );
    act( "`R$n slays you in cold blood!`w", ch, NULL, victim, TO_VICT );
    act( "`R$n slays $N in cold blood!`w", ch, NULL, victim, TO_NOTVICT );

    if ( chaos )
    {
        chaos_kill( victim );
    }
    else if ( IS_NPC( victim ) )
    {
        raw_kill( victim );
    }
    else
    {
        pk_kill( victim );
    }

    return;
}
void do_mortslay( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    extern bool chaos;
    DESCRIPTOR_DATA *d;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Slay whom?\n\r", ch );
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
                sprintf( buf, "%s", d->character->name );
                do_mortslay( ch, buf );
            }
        }
        return;
    }
    if ( ( victim = get_player_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ch == victim )
    {
        send_to_char( "Suicide is a mortal sin.\n\r", ch );
        return;
    }

    if ( !IS_NPC( victim ) && victim->level >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    act( "`RYou slay $M in cold blood!`w", ch, NULL, victim, TO_CHAR );
    act( "`R$n slays you in cold blood!`w", ch, NULL, victim, TO_VICT );
    act( "`R$n slays $N in cold blood!`w", ch, NULL, victim, TO_NOTVICT );

    if ( chaos )
    {
        chaos_kill( victim );
    }
    else if ( IS_NPC( victim ) )
    {
        raw_kill( victim );
    }
    else
    {
        pk_kill( victim );
    }

    return;
}

void chaos_log( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, "%s/%s", sysconfig.area_dir, sysconfig.chaos_file );
    append_file( ch, buf, argument );
    return;
}

bool check_block( CHAR_DATA * ch, CHAR_DATA * victim )
{
    int chancea;
    int chance;
    int dnum;
    if ( !IS_AWAKE( victim ) )
        return FALSE;

    if ( get_eq_char( victim, WEAR_SHIELD ) == NULL )
        return FALSE;

    if ( IS_NPC( victim ) )
    {
        chance = UMIN( 30, victim->level );
    }
    else
    {
        chance = victim->pcdata->learned[gsn_shield_block] / 4;
    }

    /* Must get a successful check before a block can be attempted */
    if ( !IS_NPC( victim ) )
    {
        if ( number_percent(  ) > victim->pcdata->learned[gsn_shield_block] )
            return FALSE;
    }

    chancea = 0;

    if ( !can_see( victim, ch ) )
        chance -= 25;

    if ( !can_see( ch, victim ) )
        chancea -= 25;

    chance += get_curr_stat( victim, STAT_DEX ) / 4;
    chancea +=
        ( get_curr_stat( ch, STAT_DEX ) / 4 ) + ( ( ch->level ) / 2 ) +
        ( get_curr_stat( ch, STAT_WIS ) / 3 );

/* A high chance is good.  A low chance means a failed parry */
    if ( number_percent(  ) >= chance + ( ( victim->level ) / 2 ) - chancea )
        return FALSE;
    dnum = number_range( 1, 5 );

    /*Multiple block Messages. -Lancelight */

    if ( dnum == 1 )
    {
        act( BLK_MSG1, ch, NULL, victim, TO_CHAR );
        act( BLK_MSGS1, ch, NULL, victim, TO_CHAR );
    }
    else if ( dnum == 2 )
    {
        act( BLK_MSG2, ch, NULL, victim, TO_CHAR );
        act( BLK_MSGS2, ch, NULL, victim, TO_VICT );
    }
    else if ( dnum == 3 )
    {
        act( BLK_MSG3, ch, NULL, victim, TO_CHAR );
        act( BLK_MSGS3, ch, NULL, victim, TO_VICT );
    }
    else if ( dnum == 4 )

    {
        act( BLK_MSG4, ch, NULL, victim, TO_CHAR );
        act( BLK_MSGS4, ch, NULL, victim, TO_VICT );
    }
    else if ( dnum == 5 )

    {
        act( BLK_MSG5, ch, NULL, victim, TO_CHAR );
        act( BLK_MSGS5, ch, NULL, victim, TO_VICT );
    }

    if ( ( ( victim->level ) - 5 ) > ( ch->level ) )
        return TRUE;

    check_improve( victim, gsn_shield_block, TRUE, 6 );
    return TRUE;
}

bool check_parry( CHAR_DATA * ch, CHAR_DATA * victim )
{
    int chance;
    int dnum;
    if ( !IS_AWAKE( victim ) )
        return FALSE;

    if ( IS_NPC( victim ) )
    {
        chance = UMIN( 30, victim->level );
    }
    else
    {
        if ( get_eq_char( victim, WEAR_WIELD ) == NULL )
            return FALSE;
        chance = victim->pcdata->learned[gsn_parry] / 2;
    }

    if ( number_percent(  ) >= chance + victim->level - ch->level )
        return FALSE;

    /* Multiple parry messages. -Lancelight */

    dnum = number_range( 1, 5 );
    if ( dnum == 1 )
    {
        act( PRY_MSG1, ch, NULL, victim, TO_CHAR );
        act( PRY_MSGS1, ch, NULL, victim, TO_VICT );
    }
    else if ( dnum == 2 )
    {
        act( PRY_MSG2, ch, NULL, victim, TO_CHAR );
        act( PRY_MSGS2, ch, NULL, victim, TO_VICT );
    }
    else if ( dnum == 3 )
    {
        act( PRY_MSG3, ch, NULL, victim, TO_CHAR );
        act( PRY_MSGS3, ch, NULL, victim, TO_VICT );
    }
    else if ( dnum == 4 )

    {
        act( PRY_MSG4, ch, NULL, victim, TO_CHAR );
        act( PRY_MSGS4, ch, NULL, victim, TO_VICT );
    }
    else if ( dnum == 5 )

    {
        act( PRY_MSG5, ch, NULL, victim, TO_CHAR );
        act( PRY_MSGS5, ch, NULL, victim, TO_VICT );
    }

    check_improve( victim, gsn_parry, TRUE, 6 );
    return TRUE;
}

bool check_dodge( CHAR_DATA * ch, CHAR_DATA * victim )
{
    int chance;
    int dnum;
    if ( !IS_AWAKE( victim ) )
        return FALSE;

    if ( IS_NPC( victim ) )
        chance = UMIN( 30, victim->level );
    else
        chance = victim->pcdata->learned[gsn_dodge] / 2;

    if ( number_percent(  ) >= chance + victim->level - ch->level )
        return FALSE;

    /* Multiple dodge messages. -Lancelight */

    dnum = number_range( 1, 5 );
    if ( dnum == 1 )
    {
        act( DDG_MSG1, ch, NULL, victim, TO_CHAR );
        act( DDG_MSGS1, ch, NULL, victim, TO_VICT );
    }
    else if ( dnum == 2 )
    {
        act( DDG_MSG2, ch, NULL, victim, TO_CHAR );
        act( DDG_MSGS2, ch, NULL, victim, TO_VICT );
    }
    else if ( dnum == 3 )
    {
        act( DDG_MSG3, ch, NULL, victim, TO_CHAR );
        act( DDG_MSGS3, ch, NULL, victim, TO_VICT );
    }
    else if ( dnum == 4 )

    {
        act( DDG_MSG4, ch, NULL, victim, TO_CHAR );
        act( DDG_MSGS4, ch, NULL, victim, TO_VICT );
    }
    else if ( dnum == 5 )

    {
        act( DDG_MSG5, ch, NULL, victim, TO_CHAR );
        act( DDG_MSGS5, ch, NULL, victim, TO_VICT );
    }
    check_improve( victim, gsn_dodge, TRUE, 6 );
    return TRUE;

}
