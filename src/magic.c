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
#include "magic.h"

bool gsilentdamage;

/* command procedures needed */
DECLARE_DO_FUN( do_look );

/*
 * Local functions.
 */
void say_spell args( ( CHAR_DATA * ch, int sn ) );

/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
    int sn;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
        if ( skill_table[sn].name == NULL )
            break;
        if ( LOWER( name[0] ) == LOWER( skill_table[sn].name[0] )
             && !str_prefix( name, skill_table[sn].name ) )
            return sn;
    }

    return -1;
}

/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup( int slot )
{
    extern bool fBootDb;
    int sn;

    if ( slot <= 0 )
        return -1;

    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
        if ( slot == skill_table[sn].slot )
            return sn;
    }

    if ( fBootDb )
    {
        bug( "Slot_lookup: bad slot %d.", slot );
        abort(  );
    }

    return -1;
}

/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA * ch, int sn )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *rch;
    char *pName;
    int iSyl;
    int length;

    struct syl_type {
        char *old;
        char *new;
    };

    static const struct syl_type syl_table[] = {
        {" ", " "},
        {"ar", "abra"},
        {"au", "kada"},
        {"bless", "fido"},
        {"blind", "nose"},
        {"bur", "mosa"},
        {"cu", "judi"},
        {"de", "oculo"},
        {"en", "unso"},
        {"light", "dies"},
        {"lo", "hi"},
        {"mor", "zak"},
        {"move", "sido"},
        {"ness", "lacri"},
        {"ning", "illa"},
        {"per", "duda"},
        {"ra", "gru"},
        {"fresh", "ima"},
        {"re", "candus"},
        {"son", "sabru"},
        {"tect", "infra"},
        {"tri", "cula"},
        {"ven", "nofo"},
        {"a", "a"}, {"b", "b"}, {"c", "q"}, {"d", "e"},
        {"e", "z"}, {"f", "y"}, {"g", "o"}, {"h", "p"},
        {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"},
        {"m", "w"}, {"n", "i"}, {"o", "a"}, {"p", "s"},
        {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"},
        {"u", "j"}, {"v", "z"}, {"w", "x"}, {"x", "n"},
        {"y", "l"}, {"z", "k"},
        {"", ""}
    };

    buf[0] = '\0';
    for ( pName = skill_table[sn].name; *pName != '\0'; pName += length )
    {
        for ( iSyl = 0; ( length = strlen( syl_table[iSyl].old ) ) != 0;
              iSyl++ )
        {
            if ( !str_prefix( syl_table[iSyl].old, pName ) )
            {
                strcat( buf, syl_table[iSyl].new );
                break;
            }
        }

        if ( length == 0 )
            length = 1;
    }

    sprintf( buf2, "$n utters the words, '%s'.", buf );
    sprintf( buf, "$n utters the words, '%s'.", skill_table[sn].name );

    for ( rch = ch->in_room->people; rch; rch = rch->next_in_room )
    {
        if ( rch != ch )
            act( ch->Class == rch->Class ? buf : buf2, ch, NULL, rch, TO_VICT );
    }

    return;
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA * victim )
{
    int save;

    save = 50 + ( victim->level - level - victim->saving_throw ) * 5;
    if ( IS_AFFECTED( victim, AFF_BERSERK ) )
        save += victim->level / 2;
    save = URANGE( 5, save, 95 );
    return number_percent(  ) < save;
}

/* RT save for dispels */

bool saves_dispel( int dis_level, int spell_level, int duration )
{
    int save;

    if ( duration == -1 )
        spell_level += 5;
    /* very hard to dispel permanent effects */

    save = 50 + ( spell_level - dis_level ) * 5;
    save = URANGE( 5, save, 95 );
    return number_percent(  ) < save;
}

/* co-routine for dispel magic and cancellation */

bool check_dispel( int dis_level, CHAR_DATA * victim, int sn )
{
    AFFECT_DATA *af;
    NEWAFFECT_DATA *naf;

    if ( is_affected( victim, sn ) )
    {
        for ( af = victim->affected; af != NULL; af = af->next )
        {
            if ( af->type == sn )
            {
                if ( !saves_dispel( dis_level, af->level, af->duration ) )
                {
                    affect_strip( victim, sn );
                    if ( skill_table[sn].msg_off )
                    {
                        send_to_char( skill_table[sn].msg_off, victim );
                        send_to_char( "\n\r", victim );
                    }
                    return TRUE;
                }
                else
                    af->level--;
            }
        }
    }

    if ( is_newaffected( victim, sn ) )
    {
        for ( naf = victim->newaffected; naf != NULL; naf = naf->next )
        {
            if ( naf->type == sn )
            {
                if ( !saves_dispel( dis_level, naf->level, naf->duration ) )
                {
                    newaffect_strip( victim, sn );
                    if ( skill_table[sn].msg_off )
                    {
                        send_to_char( skill_table[sn].msg_off, victim );
                        send_to_char( "\n\r", victim );
                    }
                    return TRUE;
                }
                else
                    naf->level--;
            }
        }
    }

    return FALSE;
}

/* for finding mana costs -- temporary version */
int mana_cost( CHAR_DATA * ch, int min_mana, int level )
{
    if ( ch->level + 2 == level )
        return 1000;
    return UMAX( min_mana, ( 100 / ( 2 + ch->level - level ) ) );
}

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;

void do_cast( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int mana;
    int sn;

    /*
     * Charmed NPC's can't cast spells, but others can.
     */
    if ( IS_NPC( ch ) && IS_AFFECTED( ch, AFF_CHARM ) )
        return;

    target_name = one_argument( argument, arg );
    one_argument( target_name, arg2 );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Cast which what where?\n\r", ch );
        return;
    }
    if ( IS_SET( ch->act, PLR_JAILED ) )
    {
        send_to_char( "Your magic fizzles and does nothing.", ch );
        return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_NOMAGIC ) )
    {
        send_to_char
            ( "You are unable to summon the means to cast the spell.\n\r", ch );
        return;
    }

    if ( ( sn = skill_lookup( arg ) ) < 0
         || skill_table[sn].spell_fun == spell_null
         || ( !IS_NPC( ch )
              && ch->level < skill_table[sn].skill_level[ch->Class] ) )
    {
        send_to_char( "You don't know any spells of that name.\n\r", ch );
        return;
    }

    if ( ch->position < skill_table[sn].minimum_position )
    {
        send_to_char( "You can't concentrate enough.\n\r", ch );
        return;
    }

    if ( ch->level + 2 == skill_table[sn].skill_level[ch->Class] )
        mana = 50;
    else
        mana = UMAX( skill_table[sn].min_mana,
                     100 / ( 2 + ch->level -
                             skill_table[sn].skill_level[ch->Class] ) );

    /*
     * Locate targets.
     */
    victim = NULL;
    obj = NULL;
    vo = NULL;

    switch ( skill_table[sn].target )
    {
    default:
        bug( "Do_cast: bad target for sn %d.", sn );
        return;

    case TAR_IGNORE:
        break;

    case TAR_CHAR_OFFENSIVE:
        if ( arg2[0] == '\0' )
        {
            if ( ( victim = ch->fighting ) == NULL )
            {
                send_to_char( "Cast the spell on whom?\n\r", ch );
                return;
            }
        }
        else
        {
            if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
            {
                send_to_char( "They aren't here.\n\r", ch );
                return;
            }
        }
/*
        if ( ch == victim )
        {
            send_to_char( "You can't do that to yourself.\n\r", ch );
            return;
        }
*/

        if ( !IS_NPC( ch ) )
        {

            if ( is_safe_spell( ch, victim, FALSE ) && victim != ch )
            {
                send_to_char( "Not on that target.\n\r", ch );
                return;
            }
        }

        if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
        {
            act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
            return;
        }

        vo = ( void * ) victim;
        break;

    case TAR_CHAR_DEFENSIVE:
        if ( arg2[0] == '\0' )
        {
            victim = ch;
        }
        else
        {
            if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
            {
                send_to_char( "They aren't here.\n\r", ch );
                return;
            }
        }

        vo = ( void * ) victim;
        break;

    case TAR_CHAR_SELF:
        if ( arg2[0] != '\0' && !is_name( arg2, ch->name ) )
        {
            if ( IS_NPC( ch ) )
            {
                if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
                {
                    send_to_char( "They aren't here.\n\r", ch );
                    return;
                }
            }
            else
            {
                send_to_char( "You cannot cast this spell on another.\n\r",
                              ch );
                return;
            }

            vo = ( void * ) victim;
        }
        else
            vo = ( void * ) ch;

        break;

    case TAR_OBJ_INV:
        if ( arg2[0] == '\0' )
        {
            send_to_char( "What should the spell be cast upon?\n\r", ch );
            return;
        }

        if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
        {
            send_to_char( "You are not carrying that.\n\r", ch );
            return;
        }

        vo = ( void * ) obj;
        break;
    }

    if ( !IS_NPC( ch ) && ch->mana < mana )
    {
        send_to_char( "You don't have enough mana.\n\r", ch );
        return;
    }

    if ( str_cmp( skill_table[sn].name, "ventriloquate" ) )
        say_spell( ch, sn );

    WAIT_STATE( ch, skill_table[sn].beats );

    if ( !IS_NPC( ch ) && number_percent(  ) > ch->pcdata->learned[sn] )
    {
        send_to_char( "You lost your concentration.\n\r", ch );
        check_improve( ch, sn, FALSE, 1 );
        ch->mana -= mana / 2;
    }
    else
    {
        ch->mana -= mana;
        ( *skill_table[sn].spell_fun ) ( sn, ch->level, ch, vo );
        check_improve( ch, sn, TRUE, 1 );
    }

    if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
         && victim != ch && victim->master != ch )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        for ( vch = ch->in_room->people; vch; vch = vch_next )
        {
            vch_next = vch->next_in_room;
            if ( victim == vch && victim->fighting == NULL )
            {
                multi_hit( victim, ch, TYPE_UNDEFINED );
                break;
            }
        }
    }

    return;
}

void do_mpsilentcast( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    void *vo;
    int mana;
    int sn;

    /* NPCs only */
    if ( !IS_NPC( ch ) )
        return;

    target_name = one_argument( argument, arg );
    one_argument( target_name, arg2 );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Cast which what where?\n\r", ch );
        return;
    }

    if ( ( sn = skill_lookup( arg ) ) < 0
         || ( !IS_NPC( ch )
              && ch->level < skill_table[sn].skill_level[ch->Class] ) )
    {
        send_to_char( "You don't know any spells of that name.\n\r", ch );
        return;
    }

    if ( ch->position < skill_table[sn].minimum_position )
    {
        send_to_char( "You can't concentrate enough.\n\r", ch );
        return;
    }

    if ( ch->level + 2 == skill_table[sn].skill_level[ch->Class] )
        mana = 50;
    else
        mana = UMAX( skill_table[sn].min_mana,
                     100 / ( 2 + ch->level -
                             skill_table[sn].skill_level[ch->Class] ) );

    /*
     * Locate targets.
     */
    victim = NULL;
    obj = NULL;
    vo = NULL;

    switch ( skill_table[sn].target )
    {
    default:
        bug( "Do_cast: bad target for sn %d.", sn );
        return;

    case TAR_IGNORE:
        break;

    case TAR_CHAR_OFFENSIVE:
        if ( arg2[0] == '\0' )
        {
            if ( ( victim = ch->fighting ) == NULL )
            {
                send_to_char( "Cast the spell on whom?\n\r", ch );
                return;
            }
        }
        else
        {
            if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
            {
                send_to_char( "They aren't here.\n\r", ch );
                return;
            }
        }
/*
        if ( ch == victim )
        {
            send_to_char( "You can't do that to yourself.\n\r", ch );
            return;
        }
*/

        if ( !IS_NPC( ch ) )
        {

            if ( is_safe_spell( ch, victim, FALSE ) && victim != ch )
            {
                send_to_char( "Not on that target.\n\r", ch );
                return;
            }
        }

        if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim )
        {
            send_to_char( "You can't do that on your own follower.\n\r", ch );
            return;
        }

        vo = ( void * ) victim;
        break;

    case TAR_CHAR_DEFENSIVE:
        if ( arg2[0] == '\0' )
        {
            victim = ch;
        }
        else
        {
            if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
            {
                send_to_char( "They aren't here.\n\r", ch );
                return;
            }
        }

        vo = ( void * ) victim;
        break;

    case TAR_CHAR_SELF:
        if ( arg2[0] != '\0' && !is_name( arg2, ch->name ) )
        {
/*	    send_to_char( "You cannot cast this spell on another.\n\r", ch );
	    return;*/
            if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
            {
                send_to_char( "They aren't here.\n\r", ch );
                return;
            }
            vo = ( void * ) victim;
        }
        else
            vo = ( void * ) ch;

        break;

    case TAR_OBJ_INV:
        if ( arg2[0] == '\0' )
        {
            send_to_char( "What should the spell be cast upon?\n\r", ch );
            return;
        }

        if ( ( obj = get_obj_carry( ch, arg2 ) ) == NULL )
        {
            send_to_char( "You are not carrying that.\n\r", ch );
            return;
        }

        vo = ( void * ) obj;
        break;
    }

    if ( !IS_NPC( ch ) && ch->mana < mana )
    {
        send_to_char( "You don't have enough mana.\n\r", ch );
        return;
    }

    WAIT_STATE( ch, skill_table[sn].beats );

    if ( !IS_NPC( ch ) && number_percent(  ) > ch->pcdata->learned[sn] )
    {
        send_to_char( "You lost your concentration.\n\r", ch );
        check_improve( ch, sn, FALSE, 1 );
        ch->mana -= mana / 2;
    }
    else
    {
        ch->mana -= mana;
        gsilentdamage = TRUE;
        ( *skill_table[sn].spell_fun ) ( sn, ch->level, ch, vo );
        check_improve( ch, sn, TRUE, 1 );

        /* If supermob is doing the casting, don't let a fight start */
        if ( IS_NPC( ch ) && ch->pIndexData->vnum == MOB_VNUM_SUPERMOB )
            stop_fighting( ch, TRUE );

        gsilentdamage = FALSE;
    }

    if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
         && victim != ch && victim->master != ch )
    {
        if ( IS_NPC( ch ) && ch->pIndexData->vnum == MOB_VNUM_SUPERMOB )
        {
            return;
        }
        else
        {
            CHAR_DATA *vch;
            CHAR_DATA *vch_next;

            for ( vch = ch->in_room->people; vch; vch = vch_next )
            {
                vch_next = vch->next_in_room;
                if ( victim == vch && victim->fighting == NULL )
                {
                    multi_hit( victim, ch, TYPE_UNDEFINED );
                    break;
                }
            }
        }
    }

    return;
}

/*
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim,
                     OBJ_DATA * obj )
{
    void *vo;

    if ( sn <= 0 )
        return;

    if ( sn >= MAX_SKILL || skill_table[sn].spell_fun == 0 )
    {
        bug( "Obj_cast_spell: bad sn %d.", sn );
        return;
    }

    switch ( skill_table[sn].target )
    {
    default:
        bug( "Obj_cast_spell: bad target for sn %d.", sn );
        return;

    case TAR_IGNORE:
        vo = NULL;
        break;

    case TAR_CHAR_OFFENSIVE:
        if ( victim == NULL )
            victim = ch->fighting;
        if ( victim == NULL )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }
        if ( is_safe_spell( ch, victim, FALSE ) && ch != victim )
        {
            send_to_char( "Something isn't right...\n\r", ch );
            return;
        }
        vo = ( void * ) victim;
        break;

    case TAR_CHAR_DEFENSIVE:
        if ( victim == NULL )
            victim = ch;
        vo = ( void * ) victim;
        break;

    case TAR_CHAR_SELF:
        vo = ( void * ) ch;
        break;

    case TAR_OBJ_INV:
        if ( obj == NULL )
        {
            send_to_char( "You can't do that.\n\r", ch );
            return;
        }
        vo = ( void * ) obj;
        break;
    }

    target_name = "";
    ( *skill_table[sn].spell_fun ) ( sn, level, ch, vo );

    if ( skill_table[sn].target == TAR_CHAR_OFFENSIVE
         && victim != ch && victim->master != ch )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        for ( vch = ch->in_room->people; vch; vch = vch_next )
        {
            vch_next = vch->next_in_room;
            if ( victim == vch && victim->fighting == NULL )
            {
                multi_hit( victim, ch, TYPE_UNDEFINED );
                break;
            }
        }
    }

    return;
}

/* Immortal Spell-up command
 * Donated to EmberMUD by Ian
 * of Shadowforge MUD
 */
void do_spellup( CHAR_DATA * ch, char *argument )
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int level, sn;
    SPELL_FUN *spell;

    argument = one_argument( argument, arg );

    level = MAX_LEVEL;

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: spellup <char>\n\r"
                      "{w        spellup all\n\r", ch );
        return;
    }

    if ( get_trust( ch ) >= MAX_LEVEL - 1 && !str_cmp( arg, "all" ) )
    {
        /* cure all */

        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            victim = d->character;

            if ( victim == NULL || IS_NPC( victim ) || victim == ch )
                continue;

            spell = spell_cancellation;
            sn = skill_lookup( "cancellation" );
            spell( sn, level, ch, victim );

            spell = spell_giant_strength;
            sn = skill_lookup( "giant strength" );
            spell( sn, level, ch, victim );

            spell = spell_fly;
            sn = skill_lookup( "fly" );
            spell( sn, level, ch, victim );

            spell = spell_frenzy;
            sn = skill_lookup( "frenzy" );
            spell( sn, level, ch, victim );

            spell = spell_sanctuary;
            sn = skill_lookup( "sanctuary" );
            spell( sn, level, ch, victim );

            spell = spell_armor;
            sn = skill_lookup( "armor" );
            spell( sn, level, ch, victim );

            spell = spell_invis;
            sn = skill_lookup( "invis" );
            spell( sn, level, ch, victim );

            spell = spell_bless;
            sn = skill_lookup( "bless" );
            spell( sn, level, ch, victim );

            spell = spell_protection;
            sn = skill_lookup( "protection" );
            spell( sn, level, ch, victim );

            spell = spell_pass_door;
            sn = skill_lookup( "pass door" );
            spell( sn, level, ch, victim );

            spell = spell_shield;
            sn = skill_lookup( "shield" );
            spell( sn, level, ch, victim );

            spell = spell_haste;
            sn = skill_lookup( "haste" );
            spell( sn, level, ch, victim );

            act( "$n has given you an immortal spellup.", ch, NULL, victim,
                 TO_VICT );

/*      sprintf(buf, "\n\r\n\rYou have been given an Immortal spellup from %s\n\r",  ch->name 
);
      send_to_char(buf, victim);*/
        }
        sprintf( buf, "\n\r\n\rAll active players spelled up.\n\r" );
        send_to_char( buf, ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "No such player on right now.\n\r", ch );
        return;
    }
    else

        spell = spell_cancellation;
    sn = skill_lookup( "cancellation" );
    spell( sn, level, ch, victim );

    spell = spell_giant_strength;
    sn = skill_lookup( "giant strength" );
    spell( sn, level, ch, victim );

    spell = spell_fly;
    sn = skill_lookup( "fly" );
    spell( sn, level, ch, victim );

    spell = spell_frenzy;
    sn = skill_lookup( "frenzy" );
    spell( sn, level, ch, victim );

    spell = spell_sanctuary;
    sn = skill_lookup( "sanctuary" );
    spell( sn, level, ch, victim );

    spell = spell_armor;
    sn = skill_lookup( "armor" );
    spell( sn, level, ch, victim );

/*      spell = spell_stone_skin;
      sn = skill_lookup("stone skin");
      spell(sn,level,ch,victim); */

    spell = spell_invis;
    sn = skill_lookup( "invis" );
    spell( sn, level, ch, victim );

    spell = spell_bless;
    sn = skill_lookup( "bless" );
    spell( sn, level, ch, victim );

    spell = spell_protection;
    sn = skill_lookup( "protection" );
    spell( sn, level, ch, victim );

    spell = spell_pass_door;
    sn = skill_lookup( "pass door" );
    spell( sn, level, ch, victim );

    spell = spell_shield;
    sn = skill_lookup( "shield" );
    spell( sn, level, ch, victim );

    spell = spell_haste;
    sn = skill_lookup( "haste" );
    spell( sn, level, ch, victim );

    act( "$n has given you an immortal spellup.", ch, NULL, victim, TO_VICT );

/* sprintf(buf, "\n\r\n\rYou have been given an Immortal spellup from %s\n\r",  ch->name );
send_to_char(buf, victim); */
    sprintf( buf, "\n\r\n\rYou have given %s an Immortal spellup.\n\r",
             victim->name );
    send_to_char( buf, ch );

}

/*
 * Spell functions.
 */
void spell_acid_blast( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int dam;

    dam = dice( level, 10 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    act( "$n grins as acid erupts from $s hand.", ch, NULL, NULL, TO_ROOM );
    damage( ch, victim, NULL, dam, sn, DAM_ACID );
    return;
}

void spell_armor( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if ( victim == ch )
            send_to_char( "You are already protected.\n\r", ch );
        else
            act( "$N is already armored.", ch, NULL, victim, TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = 24;
    af.modifier = -20;
    af.location = APPLY_AC;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel someone protecting you.\n\r", victim );
    if ( ch != victim )
        act( "$N is protected by your magic.", ch, NULL, victim, TO_CHAR );
    return;
}

void spell_bless( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( victim->position == POS_FIGHTING || is_affected( victim, sn ) )
    {
        if ( victim == ch )
            send_to_char( "You are already blessed.\n\r", ch );
        else
            act( "$N already has divine favor.", ch, NULL, victim, TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = 6 + level;
    af.location = APPLY_HITROLL;
    af.modifier = level / 8;
    af.bitvector = 0;
    affect_to_char( victim, &af );

    af.location = APPLY_SAVING_SPELL;
    af.modifier = 0 - level / 8;
    affect_to_char( victim, &af );
    send_to_char( "You feel righteous.\n\r", victim );
    if ( ch != victim )
        act( "You grant $N the favor of your god.", ch, NULL, victim, TO_CHAR );
    return;
}

void spell_blindness( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_BLIND ) || saves_spell( level, victim ) )
        return;

    af.type = sn;
    af.level = level;
    af.location = APPLY_HITROLL;
    af.modifier = -4;
    af.duration = 1 + level;
    af.bitvector = AFF_BLIND;
    affect_to_char( victim, &af );
    send_to_char( "You are blinded!\n\r", victim );
    act( "$n appears to be blinded.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_burning_hands( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    static const sh_int dam_each[] = {
        0,
        0, 0, 0, 0, 14, 17, 20, 23, 26, 29,
        29, 29, 30, 30, 31, 31, 32, 32, 33, 33,
        34, 34, 35, 35, 36, 36, 37, 37, 38, 38,
        39, 39, 40, 40, 41, 41, 42, 42, 43, 43,
        44, 44, 45, 45, 46, 46, 47, 47, 48, 48
    };
    int dam;

    level = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level = UMAX( 0, level );
    dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    act( "Jets of flame erupt from $n's hands.", ch, NULL, NULL, TO_ROOM );
    damage( ch, victim, NULL, dam, sn, DAM_FIRE );
    return;
}

void spell_call_lightning( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    if ( !IS_OUTSIDE( ch ) )
    {
        send_to_char( "You must be out of doors.\n\r", ch );
        return;
    }

    if ( weather_info.sky < SKY_RAINING )
    {
        send_to_char( "You need bad weather.\n\r", ch );
        return;
    }

    dam = dice( level / 2, 8 );

    send_to_char( "The gods' lightning strikes your foes!\n\r", ch );
    act( "$n calls the gods' lightning to strike $s foes!",
         ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next;
        if ( vch->in_room == NULL )
            continue;
        if ( vch->in_room == ch->in_room )
        {
            if ( vch != ch
                 && ( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) ) )
                damage( ch, vch, NULL,
                        saves_spell( level, vch ) ? dam / 2 : dam, sn,
                        DAM_LIGHTNING );
            continue;
        }

        if ( vch->in_room->area == ch->in_room->area
             && IS_OUTSIDE( vch ) && IS_AWAKE( vch ) )
            send_to_char( "Lightning flashes in the sky.\n\r", vch );
    }

    return;
}

/* RT calm spell stops all fighting in the room */

void spell_calm( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *vch;
    int mlevel = 0;
    int count = 0;
    int high_level = 0;
    int chance;
    AFFECT_DATA af;

    /* get sum of all mobile levels in the room */
    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
        if ( vch->position == POS_FIGHTING )
        {
            count++;
            if ( IS_NPC( vch ) )
                mlevel += vch->level;
            else
                mlevel += vch->level / 2;
            high_level = UMAX( high_level, vch->level );
        }
    }

    /* compute chance of stopping combat */
    chance = 4 * level - high_level + 2 * count;

    if ( IS_IMMORTAL( ch ) )    /* always works */
        mlevel = 0;

    if ( number_range( 0, chance ) >= mlevel )  /* hard to stop large fights */
    {
        for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
        {
            if ( IS_NPC( vch ) && ( IS_SET( vch->imm_flags, IMM_MAGIC ) ||
                                    IS_SET( vch->act, ACT_UNDEAD ) ) )
                return;

            if ( IS_AFFECTED( vch, AFF_CALM ) || IS_AFFECTED( vch, AFF_BERSERK )
                 || is_affected( vch, skill_lookup( "frenzy" ) ) )
                return;

            send_to_char( "A wave of calm passes over you.\n\r", vch );

            if ( vch->fighting || vch->position == POS_FIGHTING )
                stop_fighting( vch, FALSE );

            af.type = sn;
            af.level = level;
            af.duration = level / 4;
            af.location = APPLY_HITROLL;
            if ( !IS_NPC( vch ) )
                af.modifier = -5;
            else
                af.modifier = -2;
            af.bitvector = AFF_CALM;
            affect_to_char( vch, &af );

            af.location = APPLY_DAMROLL;
            affect_to_char( vch, &af );
        }
    }
}

void spell_cancellation( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    bool found = FALSE;

    level += 2;

/* Commenting out part so that npc's may cancel players
 * this is so the healer can cast cancellation.
 */
    if ( ( !IS_NPC( ch ) && IS_NPC( victim ) && !( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim ) ) /* ||
                                                                                                               ( IS_NPC( ch ) && !IS_NPC( victim ) ) */  )
    {
        send_to_char( "You failed, try dispel magic.\n\r", ch );
        return;
    }

    /* unlike dispel magic, the victim gets NO save */

    /* begin running through the spells */

    if ( check_dispel( level, victim, skill_lookup( "armor" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "bless" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "blindness" ) ) )
    {
        found = TRUE;
        act( "$n is no longer blinded.", victim, NULL, NULL, TO_ROOM );
    }

    if ( check_dispel( level, victim, skill_lookup( "calm" ) ) )
    {
        found = TRUE;
        act( "$n no longer looks so peaceful...", victim, NULL, NULL, TO_ROOM );
    }

    if ( check_dispel( level, victim, skill_lookup( "change sex" ) ) )
    {
        found = TRUE;
        act( "$n looks more like $mself again.", victim, NULL, NULL, TO_ROOM );
        send_to_char( "You feel more like yourself again.\n\r", victim );
    }

    if ( check_dispel( level, victim, skill_lookup( "charm person" ) ) )
    {
        found = TRUE;
        act( "$n regains $s free will.", victim, NULL, NULL, TO_ROOM );
    }

    if ( check_dispel( level, victim, skill_lookup( "chill touch" ) ) )
    {
        found = TRUE;
        act( "$n looks warmer.", victim, NULL, NULL, TO_ROOM );
    }

    if ( check_dispel( level, victim, skill_lookup( "curse" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "detect evil" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "detect hidden" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "detect invis" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "detect hidden" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "detect magic" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "faerie fire" ) ) )
    {
        act( "$n's outline fades.", victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "fly" ) ) )
    {
        act( "$n falls to the ground!", victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "frenzy" ) ) )
    {
        act( "$n no longer looks so wild.", victim, NULL, NULL, TO_ROOM );;
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "giant strength" ) ) )
    {
        act( "$n no longer looks so mighty.", victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "haste" ) ) )
    {
        act( "$n is no longer moving so quickly.", victim, NULL, NULL,
             TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "infravision" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "invis" ) ) )
    {
        act( "$n fades into existance.", victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "mass invis" ) ) )
    {
        act( "$n fades into existance.", victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "pass door" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "protection" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "sanctuary" ) ) )
    {
        act( "The white aura around $n's body vanishes.",
             victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "shield" ) ) )
    {
        act( "The shield protecting $n vanishes.", victim, NULL, NULL,
             TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "sleep" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "stone skin" ) ) )
    {
        act( "$n's skin regains its normal texture.", victim, NULL, NULL,
             TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "weaken" ) ) )
    {
        act( "$n looks stronger.", victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }
    if ( check_dispel( level, victim, skill_lookup( "web" ) ) )
    {
        act( "The webs around $n disolve.", victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( found )
        send_to_char( "Ok.\n\r", ch );
    else
        send_to_char( "Spell failed.\n\r", ch );
}

void spell_cause_light( int sn, int level, CHAR_DATA * ch, void *vo )
{
    damage( ch, ( CHAR_DATA * ) vo, NULL, dice( 1, 8 ) + level / 3, sn,
            DAM_HARM );
    return;
}

void spell_cause_critical( int sn, int level, CHAR_DATA * ch, void *vo )
{
    damage( ch, ( CHAR_DATA * ) vo, NULL, dice( 3, 8 ) + level - 6, sn,
            DAM_HARM );
    return;
}

void spell_cause_serious( int sn, int level, CHAR_DATA * ch, void *vo )
{
    damage( ch, ( CHAR_DATA * ) vo, NULL, dice( 2, 8 ) + level / 2, sn,
            DAM_HARM );
    return;
}

void spell_chain_lightning( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    CHAR_DATA *tmp_vict, *last_vict, *next_vict;
    bool found;
    int dam;

    /* first strike */

    act( "A lightning bolt leaps from $n's hand and arcs to $N.",
         ch, NULL, victim, TO_ROOM );
    act( "A lightning bolt leaps from your hand and arcs to $N.",
         ch, NULL, victim, TO_CHAR );
    act( "A lightning bolt leaps from $n's hand and hits you!",
         ch, NULL, victim, TO_VICT );

    dam = dice( level, 6 );
    if ( saves_spell( level, victim ) )
        dam /= 3;
    damage( ch, victim, NULL, dam, sn, DAM_LIGHTNING );
    last_vict = victim;
    level -= 4;                 /* decrement damage */

    /* new targets */
    while ( level > 0 )
    {
        found = FALSE;
        for ( tmp_vict = ch->in_room->people;
              tmp_vict != NULL; tmp_vict = next_vict )
        {
            next_vict = tmp_vict->next_in_room;
            if ( !is_safe_spell( ch, tmp_vict, TRUE ) && tmp_vict != last_vict )
            {
                found = TRUE;
                last_vict = tmp_vict;
                act( "The bolt arcs to $n!", tmp_vict, NULL, NULL, TO_ROOM );
                act( "The bolt hits you!", tmp_vict, NULL, NULL, TO_CHAR );
                dam = dice( level, 6 );
                if ( saves_spell( level, tmp_vict ) )
                    dam /= 3;
                damage( ch, tmp_vict, NULL, dam, sn, DAM_LIGHTNING );
                level -= 4;     /* decrement damage */
            }
        }                       /* end target searching loop */

        if ( !found )           /* no target found, hit the caster */
        {
            if ( ch == NULL )
                return;

            if ( last_vict == ch )  /* no double hits */
            {
                act( "The bolt seems to have fizzled out.", ch, NULL, NULL,
                     TO_ROOM );
                act( "The bolt grounds out through your body.", ch, NULL, NULL,
                     TO_CHAR );
                return;
            }

            last_vict = ch;
            act( "The bolt arcs to $n...whoops!", ch, NULL, NULL, TO_ROOM );
            send_to_char( "You are struck by your own lightning!\n\r", ch );
            dam = dice( level, 6 );
            if ( saves_spell( level, ch ) )
                dam /= 3;
            damage( ch, ch, NULL, dam, sn, DAM_LIGHTNING );
            level -= 4;         /* decrement damage */
            if ( ch == NULL )
                return;
        }
        /* now go back and find more targets */
    }
}

void spell_change_sex( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if ( victim == ch )
            send_to_char( "You've already been changed.\n\r", ch );
        else
            act( "$N has already had $s(?) sex changed.", ch, NULL, victim,
                 TO_CHAR );
        return;
    }
    if ( saves_spell( level, victim ) )
        return;
    af.type = sn;
    af.level = level;
    af.duration = 2 * level;
    af.location = APPLY_SEX;
    do
    {
        af.modifier = number_range( 0, 2 ) - victim->sex;
    }
    while ( af.modifier == 0 );
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "You feel different.\n\r", victim );
    act( "$n doesn't look like $mself anymore...", victim, NULL, NULL,
         TO_ROOM );
    return;
}

void spell_charm_person( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( victim == ch )
    {
        send_to_char( "You like yourself even better!\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( victim, AFF_CHARM )
         || IS_AFFECTED( ch, AFF_CHARM )
         || level < victim->level
         || IS_SET( victim->imm_flags, IMM_CHARM )
         || saves_spell( level, victim ) )
    {
        return;
    }

    if ( IS_SET( victim->in_room->room_flags, ROOM_LAW ) )
    {
        send_to_char
            ( "The mayor does not allow charming in the city limits.\n\r", ch );
        return;
    }

    if ( victim->master )
    {
        stop_follower( victim );
    }

    add_follower( victim, ch );
    victim->leader = ch;
    af.type = sn;
    af.level = level;
    af.duration = number_fuzzy( level / 4 );
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );
    act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );

    if ( ch != victim )
    {
        act( "$N looks at you with adoring eyes.", ch, NULL, victim, TO_CHAR );
    }

    return;
}

void spell_chill_touch( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    static const sh_int dam_each[] = {
        0,
        0, 0, 6, 7, 8, 9, 12, 13, 13, 13,
        14, 14, 14, 15, 15, 15, 16, 16, 16, 17,
        17, 17, 18, 18, 18, 19, 19, 19, 20, 20,
        20, 21, 21, 21, 22, 22, 22, 23, 23, 23,
        24, 24, 24, 25, 25, 25, 26, 26, 26, 27
    };
    AFFECT_DATA af;
    int dam;

    level = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level = UMAX( 0, level );
    dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( !saves_spell( level, victim ) )
    {
        act( "$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM );
        af.type = sn;
        af.level = level;
        af.duration = 6;
        af.location = APPLY_STR;
        af.modifier = -1;
        af.bitvector = 0;
        affect_join( victim, &af );
    }
    else
    {
        dam /= 2;
    }

    damage( ch, victim, NULL, dam, sn, DAM_COLD );
    return;
}

void spell_colour_spray( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    static const sh_int dam_each[] = {
        0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        30, 35, 40, 45, 50, 55, 55, 55, 56, 57,
        58, 58, 59, 60, 61, 61, 62, 63, 64, 64,
        65, 66, 67, 67, 68, 69, 70, 70, 71, 72,
        73, 73, 74, 75, 76, 76, 77, 78, 79, 79
    };
    int dam;

    level = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level = UMAX( 0, level );
    dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    else
        spell_blindness( skill_lookup( "blindness" ), level / 2, ch,
                         ( void * ) victim );

    act( "A rainbow shoots from $n's hand.", ch, NULL, NULL, TO_ROOM );
    damage( ch, victim, NULL, dam, sn, DAM_LIGHT );
    return;
}

void spell_continual_light( int sn, int level, CHAR_DATA * ch, void *vo )
{
    OBJ_DATA *light;

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    obj_to_room( light, ch->in_room );
    act( "$n twiddles $s thumbs and $p appears.", ch, light, NULL, TO_ROOM );
    act( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
    return;
}

void spell_control_weather( int sn, int level, CHAR_DATA * ch, void *vo )
{
    if ( !str_cmp( target_name, "better" ) )
        weather_info.change += dice( level / 3, 4 );
    else if ( !str_cmp( target_name, "worse" ) )
        weather_info.change -= dice( level / 3, 4 );
    else
        send_to_char( "Do you want it to get better or worse?\n\r", ch );

    send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_create_food( int sn, int level, CHAR_DATA * ch, void *vo )
{
    OBJ_DATA *mushroom;

    mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
    mushroom->value[0] = 5 + level;
    obj_to_room( mushroom, ch->in_room );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
    act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
    return;
}

void spell_create_spring( int sn, int level, CHAR_DATA * ch, void *vo )
{
    OBJ_DATA *spring;

    spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
    spring->timer = level;
    obj_to_room( spring, ch->in_room );
    act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
    act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );
    return;
}

void spell_create_water( int sn, int level, CHAR_DATA * ch, void *vo )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj = ( OBJ_DATA * ) vo;
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
        send_to_char( "It is unable to hold water.\n\r", ch );
        return;
    }

    if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 )
    {
        send_to_char( "It contains some other liquid.\n\r", ch );
        return;
    }

    water = UMIN( level * ( weather_info.sky >= SKY_RAINING ? 4 : 2 ),
                  obj->value[0] - obj->value[1] );

    if ( water > 0 )
    {
        obj->value[2] = LIQ_WATER;
        obj->value[1] += water;
        if ( !is_name( "water", obj->name ) )
        {
            sprintf( buf, "%s water", obj->name );
            free_string( &obj->name );
            obj->name = str_dup( buf );
        }
        act( "$p is filled.", ch, obj, NULL, TO_CHAR );
    }

    return;
}

void spell_cure_blindness( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;

    if ( !is_affected( victim, gsn_blindness ) )
    {
        if ( victim == ch )
            send_to_char( "You aren't blind.\n\r", ch );
        else
            act( "$N doesn't appear to be blinded.", ch, NULL, victim,
                 TO_CHAR );
        return;
    }

    if ( check_dispel( level, victim, gsn_blindness ) )
    {
        send_to_char( "Your vision returns!\n\r", victim );
        act( "$n is no longer blinded.", victim, NULL, NULL, TO_ROOM );
        if ( victim != ch )
            gain_exp( ch,
                      cast_xp_compute( ch, ch, ch->level, 1, ch->level ) / 2 );
    }
    else
        send_to_char( "Spell failed.\n\r", ch );
}

void spell_cure_critical( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int heal;

    if ( ( !IS_IMMORTAL( ch ) && !IS_NPC( ch ) && IS_NPC( victim ) &&
           !( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim ) ) )
    {
        send_to_char( "Why would you want to do that?\n\r", ch );
        return;
    }

    else

        heal = dice( 3, 8 ) + level - 6;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
    {
        send_to_char( "Ok.\n\r", ch );
        gain_exp( ch,
                  ( int ) ( 1.25 *
                            cast_xp_compute( ch, ch, ch->level, 1,
                                             UMIN( heal,
                                                   victim->max_hit -
                                                   victim->hit ) ) ) );
    }
    return;
}

/* RT added to cure plague */
void spell_cure_disease( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;

    if ( !is_affected( victim, gsn_plague ) )
    {
        if ( victim == ch )
            send_to_char( "You aren't ill.\n\r", ch );
        else
            act( "$N doesn't appear to be diseased.", ch, NULL, victim,
                 TO_CHAR );
        return;
    }

    if ( check_dispel( level, victim, gsn_plague ) )
    {
        send_to_char( "Your sores vanish.\n\r", victim );
        act( "$n looks relieves as $s sores vanish.", victim, NULL, NULL,
             TO_ROOM );
        if ( ch != victim )
            gain_exp( ch,
                      cast_xp_compute( ch, ch, ch->level, 1, ch->level ) / 2 );
    }
    else
        send_to_char( "Spell failed.\n\r", ch );
}

void spell_cure_light( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int heal;

    if ( ( !IS_IMMORTAL( ch ) && !IS_NPC( ch ) && IS_NPC( victim ) &&
           !( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim ) ) )
    {
        send_to_char( "Why would you want to do that?\n\r", ch );
        return;
    }

    else

        heal = dice( 1, 8 ) + level / 3;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
    {
        send_to_char( "Ok.\n\r", ch );
        gain_exp( ch,
                  ( int ) ( 2.25 *
                            cast_xp_compute( ch, ch, ch->level, 1,
                                             UMIN( heal,
                                                   victim->max_hit -
                                                   victim->hit ) ) ) );
    }
    return;
}

void spell_cure_poison( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;

    if ( !is_affected( victim, gsn_poison ) )
    {
        if ( victim == ch )
            send_to_char( "You aren't poisoned.\n\r", ch );
        else
            act( "$N doesn't appear to be poisoned.", ch, NULL, victim,
                 TO_CHAR );
        return;
    }

    if ( check_dispel( level, victim, gsn_poison ) )
    {
        send_to_char( "A warm feeling runs through your body.\n\r", victim );
        act( "$n looks much better.", victim, NULL, NULL, TO_ROOM );
        if ( ch != victim )
            gain_exp( ch,
                      cast_xp_compute( ch, ch, ch->level, 1, ch->level ) / 2 );
    }
    else
        send_to_char( "Spell failed.\n\r", ch );
}

void spell_cure_serious( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int heal;

    if ( ( !IS_IMMORTAL( ch ) && !IS_NPC( ch ) && IS_NPC( victim ) &&
           !( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim ) ) )
    {
        send_to_char( "Why would you want to do that?\n\r", ch );
        return;
    }

    else

        heal = dice( 2, 8 ) + level / 2;
    victim->hit = UMIN( victim->hit + heal, victim->max_hit );
    update_pos( victim );
    send_to_char( "You feel better!\n\r", victim );
    if ( ch != victim )
    {
        send_to_char( "Ok.\n\r", ch );
        gain_exp( ch,
                  ( int ) ( 1.75 *
                            cast_xp_compute( ch, ch, ch->level, 1,
                                             UMIN( heal,
                                                   victim->max_hit -
                                                   victim->hit ) ) ) );
    }
    return;
}

void spell_curse( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_CURSE ) || saves_spell( level, victim ) )
        return;
    af.type = sn;
    af.level = level;
    af.duration = 2 * level;
    af.location = APPLY_HITROLL;
    af.modifier = -1 * ( level / 8 );
    af.bitvector = AFF_CURSE;
    affect_to_char( victim, &af );

    af.location = APPLY_SAVING_SPELL;
    af.modifier = level / 8;
    affect_to_char( victim, &af );

    send_to_char( "You feel unclean.\n\r", victim );
    if ( ch != victim )
        act( "$N looks very uncomfortable.", ch, NULL, victim, TO_CHAR );
    return;
}

/* RT replacement demonfire spell */

void spell_demonfire( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int dam;

    if ( !IS_NPC( ch ) && !IS_EVIL( ch ) )
    {
        victim = ch;
        send_to_char( "The demons turn upon you!\n\r", ch );
    }

    ch->alignment = UMAX( -1000, ch->alignment - 50 );

    if ( victim != ch )
    {
        act( "$n calls forth the demons of Hell upon $N!",
             ch, NULL, victim, TO_ROOM );
        act( "$n has assailed you with the demons of Hell!",
             ch, NULL, victim, TO_VICT );
        send_to_char( "You conjure forth the demons of hell!\n\r", ch );
    }
    dam = dice( level, 10 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    damage( ch, victim, NULL, dam, sn, DAM_NEGATIVE );
}

void spell_detect_evil( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_DETECT_EVIL ) )
    {
        if ( victim == ch )
            send_to_char( "You can already sense evil.\n\r", ch );
        else
            act( "$N can already detect evil.", ch, NULL, victim, TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_detect_hidden( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_DETECT_HIDDEN ) )
    {
        if ( victim == ch )
            send_to_char( "You are already as alert as you can be. \n\r", ch );
        else
            act( "$N can already sense hidden lifeforms.", ch, NULL, victim,
                 TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_DETECT_HIDDEN;
    affect_to_char( victim, &af );
    send_to_char( "Your awareness improves.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_detect_invis( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_DETECT_INVIS ) )
    {
        if ( victim == ch )
            send_to_char( "You can already see invisible.\n\r", ch );
        else
            act( "$N can already see invisible things.", ch, NULL, victim,
                 TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVIS;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_detect_magic( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_DETECT_MAGIC ) )
    {
        if ( victim == ch )
            send_to_char( "You can already sense magical auras.\n\r", ch );
        else
            act( "$N can already detect magic.", ch, NULL, victim, TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes tingle.\n\r", victim );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_detect_poison( int sn, int level, CHAR_DATA * ch, void *vo )
{
    OBJ_DATA *obj = ( OBJ_DATA * ) vo;

    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
        if ( obj->value[3] != 0 )
            send_to_char( "You smell poisonous fumes.\n\r", ch );
        else
            send_to_char( "It looks delicious.\n\r", ch );
    }
    else
    {
        send_to_char( "It doesn't look poisoned.\n\r", ch );
    }

    return;
}

void spell_dispel_evil( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int dam;

    if ( !IS_NPC( ch ) && IS_EVIL( ch ) )
        victim = ch;

    if ( IS_GOOD( victim ) )
    {
        act( "The gods protect $N.", ch, NULL, victim, TO_ROOM );
        return;
    }

    if ( IS_NEUTRAL( victim ) )
    {
        act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( victim->hit > ( ch->level * 4 ) )
        dam = dice( level, 4 );
    else
        dam = UMAX( victim->hit, dice( level, 4 ) );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    damage( ch, victim, NULL, dam, sn, DAM_HOLY );
    return;
}

/* modified for enhanced use */

void spell_dispel_magic( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    bool found = FALSE;

    if ( saves_spell( level, victim ) )
    {
        send_to_char( "You feel a brief tingling sensation.\n\r", victim );
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    /* begin running through the spells */

    if ( check_dispel( level, victim, skill_lookup( "armor" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "bless" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "blindness" ) ) )
    {
        found = TRUE;
        act( "$n is no longer blinded.", victim, NULL, NULL, TO_ROOM );
    }

    if ( check_dispel( level, victim, skill_lookup( "calm" ) ) )
    {
        found = TRUE;
        act( "$n no longer looks so peaceful...", victim, NULL, NULL, TO_ROOM );
    }

    if ( check_dispel( level, victim, skill_lookup( "change sex" ) ) )
    {
        found = TRUE;
        act( "$n looks more like $mself again.", victim, NULL, NULL, TO_ROOM );
    }

    if ( check_dispel( level, victim, skill_lookup( "charm person" ) ) )
    {
        found = TRUE;
        act( "$n regains $s free will.", victim, NULL, NULL, TO_ROOM );
    }

    if ( check_dispel( level, victim, skill_lookup( "chill touch" ) ) )
    {
        found = TRUE;
        act( "$n looks warmer.", victim, NULL, NULL, TO_ROOM );
    }

    if ( check_dispel( level, victim, skill_lookup( "curse" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "detect evil" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "detect hidden" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "detect invis" ) ) )
        found = TRUE;

    found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "detect hidden" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "detect magic" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "faerie fire" ) ) )
    {
        act( "$n's outline fades.", victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "fly" ) ) )
    {
        act( "$n falls to the ground!", victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "frenzy" ) ) )
    {
        act( "$n no longer looks so wild.", victim, NULL, NULL, TO_ROOM );;
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "giant strength" ) ) )
    {
        act( "$n no longer looks so mighty.", victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "haste" ) ) )
    {
        act( "$n is no longer moving so quickly.", victim, NULL, NULL,
             TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "infravision" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "invis" ) ) )
    {
        act( "$n fades into existance.", victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "mass invis" ) ) )
    {
        act( "$n fades into existance.", victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "pass door" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "protection" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "sanctuary" ) ) )
    {
        act( "The white aura around $n's body vanishes.",
             victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( IS_AFFECTED( victim, AFF_SANCTUARY )
         && !saves_dispel( level, victim->level, -1 )
         && !is_affected( victim, skill_lookup( "sanctuary" ) ) )
    {
        REMOVE_BIT( victim->affected_by, AFF_SANCTUARY );
        act( "The white aura around $n's body vanishes.",
             victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "shield" ) ) )
    {
        act( "The shield protecting $n vanishes.", victim, NULL, NULL,
             TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "sleep" ) ) )
        found = TRUE;

    if ( check_dispel( level, victim, skill_lookup( "stone skin" ) ) )
    {
        act( "$n's skin regains its normal texture.", victim, NULL, NULL,
             TO_ROOM );
        found = TRUE;
    }

    if ( check_dispel( level, victim, skill_lookup( "weaken" ) ) )
    {
        act( "$n looks stronger.", victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }
    if ( check_dispel( level, victim, skill_lookup( "web" ) ) )
    {
        act( "The webs around $n disolve.", victim, NULL, NULL, TO_ROOM );
        found = TRUE;
    }

    if ( found )
        send_to_char( "Ok.\n\r", ch );
    else
        send_to_char( "Spell failed.\n\r", ch );
}

void spell_earthquake( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    send_to_char( "The earth trembles beneath your feet!\n\r", ch );
    act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next;
        if ( vch->in_room == NULL )
            continue;
        if ( vch->in_room == ch->in_room )
        {
            if ( vch != ch && !is_safe_spell( ch, vch, TRUE ) )
            {
                if ( IS_AFFECTED( vch, AFF_FLYING ) )
                    damage( ch, vch, NULL, 0, sn, DAM_BASH );
                else
                    damage( ch, vch, NULL, level + dice( 2, 8 ), sn, DAM_BASH );
            }
            continue;
        }

        if ( vch->in_room->area == ch->in_room->area )
            send_to_char( "The earth trembles and shivers.\n\r", vch );
    }

    return;
}

void spell_enchant_armor( int sn, int level, CHAR_DATA * ch, void *vo )
{
    OBJ_DATA *obj = ( OBJ_DATA * ) vo;
    AFFECT_DATA *paf;
    int result, fail;
    int ac_bonus, added;
    bool ac_found = FALSE;

    if ( obj->item_type != ITEM_ARMOR )
    {
        send_to_char( "That isn't an armor.\n\r", ch );
        return;
    }

    if ( obj->wear_loc != -1 )
    {
        send_to_char( "The item must be carried to be enchanted.\n\r", ch );
        return;
    }

    /* this means they have no bonus */
    ac_bonus = 0;
    fail = 25;                  /* base 25% chance of failure */

    /* find the bonuses */

    if ( !obj->enchanted )
        for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
        {
            if ( paf->location == APPLY_AC )
            {
                ac_bonus = paf->modifier;
                ac_found = TRUE;
                ac_bonus = ( ac_bonus * -1 );
                ac_bonus = ( ac_bonus / 5 );
                if ( ac_bonus <= 1 )
                {
                    ac_bonus = 1;
                }
                fail += 5 * ( ac_bonus * ac_bonus );
            }

            else                /* things get a little harder */
                fail += 20;
        }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
        if ( paf->location == APPLY_AC )
        {
            ac_bonus = paf->modifier;
            ac_found = TRUE;
            fail += 5 * ( ac_bonus * ac_bonus );
        }

        else                    /* things get a little harder */
            fail += 20;
    }

    /* apply other modifiers */
    fail -= level;

    if ( IS_OBJ_STAT( obj, ITEM_BLESS ) )
        fail -= 15;
    if ( IS_OBJ_STAT( obj, ITEM_GLOW ) )
        fail -= 5;

    fail = URANGE( 5, fail, 95 );

    result = number_percent(  );

    /* the moment of truth */
    if ( result < ( fail / 5 ) )    /* item destroyed */
    {
        act( "$p flares blindingly... and evaporates!", ch, obj, NULL,
             TO_CHAR );
        act( "$p flares blindingly... and evaporates!", ch, obj, NULL,
             TO_ROOM );
        extract_obj( obj );
        return;
    }

    if ( result < ( fail / 2 ) )    /* item disenchanted */
    {
        AFFECT_DATA *paf_next;

        act( "$p glows brightly, then fades...oops.", ch, obj, NULL, TO_CHAR );
        act( "$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM );
        obj->enchanted = TRUE;

        /* remove all affects */
        for ( paf = obj->affected; paf != NULL; paf = paf_next )
        {
            paf_next = paf->next;
            paf->next = affect_free;
            affect_free = paf;
        }
        obj->affected = NULL;

        /* clear all flags */
        obj->extra_flags = 0;
        return;
    }

    if ( result <= fail )       /* failed, no bad result */
    {
        send_to_char( "Nothing seemed to happen.\n\r", ch );
        return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if ( !obj->enchanted )
    {
        AFFECT_DATA *af_new;
        obj->enchanted = TRUE;

        for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
        {
            if ( affect_free == NULL )
                af_new = alloc_perm( sizeof( *af_new ) );
            else
            {
                af_new = affect_free;
                affect_free = affect_free->next;
            }

            af_new->next = obj->affected;
            obj->affected = af_new;

            af_new->type = UMAX( 0, paf->type );
            af_new->level = paf->level;
            af_new->duration = paf->duration;
            af_new->location = paf->location;
            af_new->modifier = paf->modifier;
            af_new->bitvector = paf->bitvector;
        }
    }

    if ( result <= ( 100 - level / 5 ) )    /* success! */
    {
        act( "$p shimmers with a gold aura.", ch, obj, NULL, TO_CHAR );
        act( "$p shimmers with a gold aura.", ch, obj, NULL, TO_ROOM );
        SET_BIT( obj->extra_flags, ITEM_MAGIC );
        added = -5;
    }

    else                        /* exceptional enchant */
    {
        act( "$p glows a brillant gold!", ch, obj, NULL, TO_CHAR );
        act( "$p glows a brillant gold!", ch, obj, NULL, TO_ROOM );
        SET_BIT( obj->extra_flags, ITEM_MAGIC );
        SET_BIT( obj->extra_flags, ITEM_GLOW );
        added = -10;
    }

    /* now add the enchantments */

    if ( obj->level < LEVEL_HERO )
        obj->level = UMIN( LEVEL_HERO - 1, obj->level + 1 );

    if ( ac_found )
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next )
        {
            if ( paf->location == APPLY_AC )
            {
                paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX( paf->level, level );
            }
        }
    }
    else                        /* add a new affect */
    {
        if ( affect_free == NULL )
            paf = alloc_perm( sizeof( *paf ) );
        else
        {
            paf = affect_free;
            affect_free = affect_free->next;
        }

        paf->type = sn;
        paf->level = level;
        paf->duration = -1;
        paf->location = APPLY_AC;
        paf->modifier = added;
        paf->bitvector = 0;
        paf->next = obj->affected;
        obj->affected = paf;
    }

}

void spell_enchant_weapon( int sn, int level, CHAR_DATA * ch, void *vo )
{
    OBJ_DATA *obj = ( OBJ_DATA * ) vo;
    AFFECT_DATA *paf;
    int result, fail;
    int hit_bonus, dam_bonus, added;
    bool hit_found = FALSE, dam_found = FALSE;

    if ( obj->item_type != ITEM_WEAPON )
    {
        send_to_char( "That isn't a weapon.\n\r", ch );
        return;
    }

    if ( obj->wear_loc != -1 )
    {
        send_to_char( "The item must be carried to be enchanted.\n\r", ch );
        return;
    }

    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;                  /* base 25% chance of failure */

    /* find the bonuses */

    if ( !obj->enchanted )
        for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
        {
            if ( paf->location == APPLY_HITROLL )
            {
                hit_bonus = paf->modifier;
                hit_found = TRUE;
                fail += 2 * ( hit_bonus * hit_bonus );
            }

            else if ( paf->location == APPLY_DAMROLL )
            {
                dam_bonus = paf->modifier;
                dam_found = TRUE;
                fail += 2 * ( dam_bonus * dam_bonus );
            }

            else                /* things get a little harder */
                fail += 25;
        }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
        if ( paf->location == APPLY_HITROLL )
        {
            hit_bonus = paf->modifier;
            hit_found = TRUE;
            fail += 2 * ( hit_bonus * hit_bonus );
        }

        else if ( paf->location == APPLY_DAMROLL )
        {
            dam_bonus = paf->modifier;
            dam_found = TRUE;
            fail += 2 * ( dam_bonus * dam_bonus );
        }

        else                    /* things get a little harder */
            fail += 25;
    }

    /* apply other modifiers */
    fail -= 3 * level / 2;

    if ( IS_OBJ_STAT( obj, ITEM_BLESS ) )
        fail -= 15;
    if ( IS_OBJ_STAT( obj, ITEM_GLOW ) )
        fail -= 5;

    fail = URANGE( 5, fail, 95 );

    result = number_percent(  );

    /* the moment of truth */
    if ( result < ( fail / 5 ) )    /* item destroyed */
    {
        act( "$p shivers violently and explodes!", ch, obj, NULL, TO_CHAR );
        act( "$p shivers violently and explodeds!", ch, obj, NULL, TO_ROOM );
        extract_obj( obj );
        return;
    }

    if ( result < ( fail / 2 ) )    /* item disenchanted */
    {
        AFFECT_DATA *paf_next;

        act( "$p glows brightly, then fades...oops.", ch, obj, NULL, TO_CHAR );
        act( "$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM );
        obj->enchanted = TRUE;

        /* remove all affects */
        for ( paf = obj->affected; paf != NULL; paf = paf_next )
        {
            paf_next = paf->next;
            paf->next = affect_free;
            affect_free = paf;
        }
        obj->affected = NULL;

        /* clear all flags */
        obj->extra_flags = 0;
        return;
    }

    if ( result <= fail )       /* failed, no bad result */
    {
        send_to_char( "Nothing seemed to happen.\n\r", ch );
        return;
    }

    /* okay, move all the old flags into new vectors if we have to */
    if ( !obj->enchanted )
    {
        AFFECT_DATA *af_new;
        obj->enchanted = TRUE;

        for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
        {
            if ( affect_free == NULL )
                af_new = alloc_perm( sizeof( *af_new ) );
            else
            {
                af_new = affect_free;
                affect_free = affect_free->next;
            }

            af_new->next = obj->affected;
            obj->affected = af_new;

            af_new->type = UMAX( 0, paf->type );
            af_new->level = paf->level;
            af_new->duration = paf->duration;
            af_new->location = paf->location;
            af_new->modifier = paf->modifier;
            af_new->bitvector = paf->bitvector;
        }
    }

    if ( result <= ( 100 - level / 5 ) )    /* success! */
    {
        act( "$p glows blue.", ch, obj, NULL, TO_CHAR );
        act( "$p glows blue.", ch, obj, NULL, TO_ROOM );
        SET_BIT( obj->extra_flags, ITEM_MAGIC );
        added = 1;
    }

    else                        /* exceptional enchant */
    {
        act( "$p glows a brillant blue!", ch, obj, NULL, TO_CHAR );
        act( "$p glows a brillant blue!", ch, obj, NULL, TO_ROOM );
        SET_BIT( obj->extra_flags, ITEM_MAGIC );
        SET_BIT( obj->extra_flags, ITEM_GLOW );
        added = 2;
    }

    /* now add the enchantments */

    if ( obj->level < LEVEL_HERO - 1 )
        obj->level = UMIN( LEVEL_HERO - 1, obj->level + 1 );

    if ( dam_found )
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next )
        {
            if ( paf->location == APPLY_DAMROLL )
            {
                paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX( paf->level, level );
                if ( paf->modifier > 4 )
                    SET_BIT( obj->extra_flags, ITEM_HUM );
            }
        }
    }
    else                        /* add a new affect */
    {
        if ( affect_free == NULL )
            paf = alloc_perm( sizeof( *paf ) );
        else
        {
            paf = affect_free;
            affect_free = affect_free->next;
        }

        paf->type = sn;
        paf->level = level;
        paf->duration = -1;
        paf->location = APPLY_DAMROLL;
        paf->modifier = added;
        paf->bitvector = 0;
        paf->next = obj->affected;
        obj->affected = paf;
    }

    if ( hit_found )
    {
        for ( paf = obj->affected; paf != NULL; paf = paf->next )
        {
            if ( paf->location == APPLY_HITROLL )
            {
                paf->type = sn;
                paf->modifier += added;
                paf->level = UMAX( paf->level, level );
                if ( paf->modifier > 4 )
                    SET_BIT( obj->extra_flags, ITEM_HUM );
            }
        }
    }
    else                        /* add a new affect */
    {
        if ( affect_free == NULL )
            paf = alloc_perm( sizeof( *paf ) );
        else
        {
            paf = affect_free;
            affect_free = affect_free->next;
        }

        paf->type = sn;
        paf->level = level;
        paf->duration = -1;
        paf->location = APPLY_HITROLL;
        paf->modifier = added;
        paf->bitvector = 0;
        paf->next = obj->affected;
        obj->affected = paf;
    }

}

/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_energy_drain( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int dam;

    if ( saves_spell( level, victim ) )
    {
        send_to_char( "You feel a momentary chill.\n\r", victim );
        return;
    }

    ch->alignment = UMAX( -1000, ch->alignment - 50 );
    if ( victim->level <= 2 )
    {
        dam = ch->hit + 1;
    }
    else
    {
        gain_exp( victim, 0 - 5 * number_range( level / 2, 3 * level / 2 ) );
        victim->mana /= 2;
        victim->move /= 2;
        dam = dice( 1, level );
        ch->hit += dam;
    }

    send_to_char( "You feel your life slipping away!\n\r", victim );
    send_to_char( "Wow....what a rush!\n\r", ch );
    damage( ch, victim, NULL, dam, sn, DAM_NEGATIVE );

    return;
}

void spell_fireball( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    static const sh_int dam_each[] = {
        0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 30, 35, 40, 45, 50, 55,
        60, 65, 70, 75, 80, 82, 84, 86, 88, 90,
        92, 94, 96, 98, 100, 102, 104, 106, 108, 110,
        112, 114, 116, 118, 120, 122, 124, 126, 128, 130
    };
    int dam;

    level = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level = UMAX( 0, level );
    dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    act( "$n smirks and throws a fireball at $N.", ch, NULL, victim,
         TO_NOTVICT );
    act( "You throw a fireball at $N.", ch, NULL, victim, TO_CHAR );
    act( "$n smirks, and throws a fireball at you.", ch, NULL, victim,
         TO_VICT );
    damage( ch, victim, NULL, dam, sn, DAM_FIRE );
    return;
}

void spell_flamestrike( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int dam;

    dam = dice( 6, 8 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    act( "$n calls down a column of fire from the sky.", ch, NULL, NULL,
         TO_ROOM );
    damage( ch, victim, NULL, dam, sn, DAM_FIRE );
    return;
}

void spell_faerie_fire( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_FAERIE_FIRE ) )
        return;
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_AC;
    af.modifier = 2 * level;
    af.bitvector = AFF_FAERIE_FIRE;
    affect_to_char( victim, &af );
    send_to_char( "You are surrounded by a pink outline.\n\r", victim );
    act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_faerie_fog( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *ich;

    act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You conjure a cloud of purple smoke.\n\r", ch );

    for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room )
    {
        if ( !IS_NPC( ich ) && IS_SET( ich->act, PLR_WIZINVIS ) )
            continue;

        if ( ich == ch || saves_spell( level, ich ) )
            continue;

        affect_strip( ich, gsn_invis );
        affect_strip( ich, gsn_mass_invis );
        affect_strip( ich, gsn_sneak );
        REMOVE_BIT( ich->affected_by, AFF_HIDE );
        REMOVE_BIT( ich->affected_by, AFF_INVISIBLE );
        REMOVE_BIT( ich->affected_by, AFF_SNEAK );
        act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
        send_to_char( "You are revealed!\n\r", ich );
    }

    return;
}

void spell_fly( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_FLYING ) )
    {
        if ( victim == ch )
            send_to_char( "You are already airborne.\n\r", ch );
        else
            act( "$N doesn't need your help to fly.", ch, NULL, victim,
                 TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = level + 3;
    af.location = 0;
    af.modifier = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char( victim, &af );
    send_to_char( "Your feet rise off the ground.\n\r", victim );
    act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
    return;
}

/* RT clerical berserking spell */

void spell_frenzy( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || IS_AFFECTED( victim, AFF_BERSERK ) )
    {
        if ( victim == ch )
            send_to_char( "You are already in a frenzy.\n\r", ch );
        else
            act( "$N is already in a frenzy.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( is_affected( victim, skill_lookup( "calm" ) ) )
    {
        if ( victim == ch )
            send_to_char( "Why don't you just relax for a while?\n\r", ch );
        else
            act( "$N doesn't look like $e wants to fight anymore.",
                 ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( ( IS_GOOD( ch ) && !IS_GOOD( victim ) ) ||
         ( IS_NEUTRAL( ch ) && !IS_NEUTRAL( victim ) ) ||
         ( IS_EVIL( ch ) && !IS_EVIL( victim ) ) )
    {
        act( "Your god doesn't seem to like $N", ch, NULL, victim, TO_CHAR );
        return;
    }

    af.type = sn;
    af.level = level;
    af.duration = level / 3;
    af.modifier = level / 6;
    af.bitvector = 0;

    af.location = APPLY_HITROLL;
    affect_to_char( victim, &af );

    af.location = APPLY_DAMROLL;
    affect_to_char( victim, &af );

    af.modifier = 10 * ( level / 6 );
    af.location = APPLY_AC;
    affect_to_char( victim, &af );

    send_to_char( "You are filled with holy wrath!\n\r", victim );
    act( "$n gets a wild look in $s eyes!", victim, NULL, NULL, TO_ROOM );
}

/* RT ROM-style gate */

void spell_gate( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim;
    extern bool chaos;
    bool gate_pet;

    if ( ( ( victim = get_char_world( ch, target_name ) ) == NULL ) )
    {
        send_to_char
            ( "A disturbance in the planar fabric prevents your transportation.\n\r",
              ch );
        return;
    }
    if ( NOSUMMONFULL == 1 )
    {
        if ( ( victim = get_char_world( ch, target_name ) ) == NULL || victim == ch || victim->in_room == NULL || !can_see_room( ch, victim->in_room ) || IS_SET( victim->in_room->room_flags, ROOM_SAFE ) || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) || victim->level >= level + 3 || ( chaos ) || ( !IS_NPC( victim ) && victim->level >= LEVEL_HERO )  /* NOT trust */
             || ( IS_NPC( victim ) && IS_SET( victim->imm_flags, IMM_SUMMON ) )
             || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
             || IS_SET( victim->in_room->room_flags, ROOM_NOMAGIC )
             || ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_NOSUMMON ) )
             || ( IS_NPC( victim ) && saves_spell( level, victim ) )
             || ( !IS_NPC( victim )
                  && IS_SET( victim->in_room->room_flags, ROOM_LAW ) ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your transponsportation",
                  ch );
            return;
        }
    }
    if ( IS_NPC( victim ) )
    {

        if ( victim == ch
             || victim->in_room == NULL
             || !can_see_room( ch, victim->in_room )
             || IS_SET( victim->in_room->room_flags, ROOM_SAFE )
             || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE )
             || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY )
             || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
             || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
             || IS_SET( victim->in_room->room_flags, ROOM_NOMAGIC )
             || IS_SET( ch->in_room->room_flags, ROOM_NOMAGIC )
             || victim->level >= ch->level + NOSUMMONLEVELMOB
             || ( chaos )
             || IS_SET( victim->imm_flags, IMM_SUMMON )
             || saves_spell( level, victim ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your transportation.\n\r",
                  ch );
            return;
        }
    }

    if ( !IS_NPC( victim ) && !IS_SET( victim->act, PLR_KILLER ) )
    {

        if ( victim == ch || victim->in_room == NULL || !can_see_room( ch, victim->in_room ) || IS_SET( victim->in_room->room_flags, ROOM_SAFE ) || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) || victim->level >= ch->level + NOSUMMONLEVELNONPK || ( chaos ) || ( !IS_NPC( victim ) && victim->level >= LEVEL_HERO )   /* NOT trust */
             || ( IS_NPC( victim ) && IS_SET( victim->imm_flags, IMM_SUMMON ) )
             || ( IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) )
             || ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_NOSUMMON ) )
             || ( IS_NPC( victim ) && saves_spell( level, victim ) ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your transportation.\n\r",
                  ch );
            return;
        }
    }

    if ( NOSUMMONPKSAME == 1 && IS_SET( victim->act, PLR_KILLER ) )
    {
        if ( victim == ch || victim->in_room == NULL || !can_see_room( ch, victim->in_room ) || IS_SET( victim->in_room->room_flags, ROOM_SAFE ) || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) || victim->level > ch->level + NOSUMMONLEVELPK || ( chaos ) || ( !IS_NPC( victim ) && victim->level >= LEVEL_HERO )   /* NOT trust */
             || ( IS_NPC( victim ) && IS_SET( victim->imm_flags, IMM_SUMMON ) )
             || ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_NOSUMMON )
                  && victim->level > ch->level + NOSUMMONLEVELPK )
             || ( IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) )
             || ( IS_NPC( victim ) && saves_spell( level, victim ) ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your transportation.\n\r",
                  ch );
            return;
        }
    }
    if ( IS_SET( victim->act, PLR_KILLER ) && NOSUMMONPKSAME == 0 )
    {

        if ( victim == ch || victim->in_room == NULL || !can_see_room( ch, victim->in_room ) || IS_SET( victim->in_room->room_flags, ROOM_SAFE ) || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) || ( chaos ) || ( ch->level > ( victim->level + NOSUMMONLEVELPK ) || victim->level > ( ch->level + NOSUMMONLEVELPK ) ) || ( !IS_NPC( victim ) && victim->level >= LEVEL_HERO )    /* NOT trust */
             || ( IS_NPC( victim ) && IS_SET( victim->imm_flags, IMM_SUMMON ) )
             || ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_NOSUMMON )
                  && ( ch->level > ( victim->level + NOSUMMONLEVELPK )
                       || victim->level > ( ch->level + NOSUMMONLEVELPK ) ) )
             || ( IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) )
             || ( IS_NPC( victim ) && saves_spell( level, victim ) ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your transportation.\n\r",
                  ch );
            return;
        }

    }

    if ( ch->pet != NULL && ch->in_room == ch->pet->in_room )
        gate_pet = TRUE;
    else
        gate_pet = FALSE;

    act( "$n steps through a gate and vanishes.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You step through a gate and vanish.\n\r", ch );
    char_from_room( ch );
    char_to_room( ch, victim->in_room );

    act( "$n has arrived through a gate.", ch, NULL, NULL, TO_ROOM );
    do_look( ch, "auto" );

    if ( gate_pet )
    {
        act( "$n steps through a gate and vanishes.", ch->pet, NULL, NULL,
             TO_ROOM );
        send_to_char( "You step through a gate and vanish.\n\r", ch->pet );
        char_from_room( ch->pet );
        char_to_room( ch->pet, victim->in_room );
        act( "$n has arrived through a gate.", ch->pet, NULL, NULL, TO_ROOM );
        do_look( ch->pet, "auto" );
    }
}

void spawn_portal( int vnum1, int vnum2 )
{
    OBJ_DATA *obj;
    int decay;

    decay = number_range( 3, 6 );

    obj = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );
    obj->value[0] = vnum1;
    obj->timer = decay;
    obj_to_room( obj, get_room_index( vnum2 ) );

    obj = create_object( get_obj_index( OBJ_VNUM_PORTAL ), 0 );
    obj->value[0] = vnum2;
    obj->timer = decay;
    obj_to_room( obj, get_room_index( vnum1 ) );

    return;
}

void spell_nexus( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim;
    extern bool chaos;

    if ( ( ( victim = get_char_world( ch, target_name ) ) == NULL ) )
    {
        send_to_char
            ( "A disturbance in the planar fabric prevents your portal from opening.\n\r",
              ch );
        return;
    }
    if ( NOSUMMONFULL == 1 )
    {
        if ( ( victim = get_char_world( ch, target_name ) ) == NULL || victim == ch || victim->in_room == NULL || !can_see_room( ch, victim->in_room ) || IS_SET( victim->in_room->room_flags, ROOM_SAFE ) || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) || victim->level >= level + 3 || ( chaos ) || ( !IS_NPC( victim ) && victim->level >= LEVEL_HERO )  /* NOT trust */
             || ( IS_NPC( victim ) && IS_SET( victim->imm_flags, IMM_SUMMON ) )
             || ( IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) )
             || ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_NOSUMMON ) )
             || ( IS_NPC( victim ) && saves_spell( level, victim ) ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your  portal from opening.",
                  ch );
            return;
        }
    }
    if ( IS_NPC( victim ) )
    {

        if ( victim == ch
             || victim->in_room == NULL
             || !can_see_room( ch, victim->in_room )
             || IS_SET( victim->in_room->room_flags, ROOM_SAFE )
             || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE )
             || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY )
             || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
             || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
             || ( IS_SET( victim->in_room->room_flags, ROOM_NOMAGIC ) )
             || ( IS_SET( ch->in_room->room_flags, ROOM_NOMAGIC ) )
             || victim->level >= ch->level + NOSUMMONLEVELMOB
             || ( chaos )
             || IS_SET( victim->imm_flags, IMM_SUMMON )
             || saves_spell( level, victim ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your portal from opening.\n\r",
                  ch );
            return;
        }
    }

    if ( !IS_NPC( victim ) && !IS_SET( victim->act, PLR_KILLER ) )
    {

        if ( victim == ch || victim->in_room == NULL || !can_see_room( ch, victim->in_room ) || IS_SET( victim->in_room->room_flags, ROOM_SAFE ) || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) || victim->level >= ch->level + NOSUMMONLEVELNONPK || ( chaos ) || ( !IS_NPC( victim ) && victim->level >= LEVEL_HERO )   /* NOT trust */
             || ( IS_NPC( victim ) && IS_SET( victim->imm_flags, IMM_SUMMON ) )
             || ( IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) )
             || ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_NOSUMMON ) )
             || ( IS_NPC( victim ) && saves_spell( level, victim ) ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your portal from opening.\n\r",
                  ch );
            return;
        }
    }

    if ( NOSUMMONPKSAME == 1 && IS_SET( victim->act, PLR_KILLER ) )
    {
        if ( victim == ch || victim->in_room == NULL || !can_see_room( ch, victim->in_room ) || IS_SET( victim->in_room->room_flags, ROOM_SAFE ) || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) || victim->level > ch->level + NOSUMMONLEVELPK || ( chaos ) || ( !IS_NPC( victim ) && victim->level >= LEVEL_HERO )   /* NOT trust */
             || ( IS_NPC( victim ) && IS_SET( victim->imm_flags, IMM_SUMMON ) )
             || ( IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) )
             || ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_NOSUMMON )
                  && victim->level > ch->level + NOSUMMONLEVELPK )
             || ( IS_NPC( victim ) && saves_spell( level, victim ) ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your portal from opening.\n\r",
                  ch );
            return;
        }
    }
    if ( IS_SET( victim->act, PLR_KILLER ) && NOSUMMONPKSAME == 0 )
    {

        if ( victim == ch || victim->in_room == NULL || !can_see_room( ch, victim->in_room ) || IS_SET( victim->in_room->room_flags, ROOM_SAFE ) || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) || ( chaos ) || ( ch->level > ( victim->level + NOSUMMONLEVELPK ) || victim->level > ( ch->level + NOSUMMONLEVELPK ) ) || ( !IS_NPC( victim ) && victim->level >= LEVEL_HERO )    /* NOT trust */
             || ( IS_NPC( victim ) && IS_SET( victim->imm_flags, IMM_SUMMON ) )
             || ( IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) )
             || ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_NOSUMMON )
                  && ( ch->level > ( victim->level + NOSUMMONLEVELPK )
                       || victim->level > ( ch->level + NOSUMMONLEVELPK ) ) )
             || ( IS_NPC( victim ) && saves_spell( level, victim ) ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your portal from opening.\n\r",
                  ch );
            return;
        }

    }

    spawn_portal( ch->in_room->vnum, victim->in_room->vnum );
    act( "$n waves his hands and a portal appears out of thin air.", ch, NULL,
         NULL, TO_ROOM );
    send_to_char
        ( "As you wave your hands, a portal appears in front of you.\n\r", ch );

}

void spell_giant_strength( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if ( victim == ch )
            send_to_char( "You are already as strong as you can get!\n\r", ch );
        else
            act( "$N can't get any stronger.", ch, NULL, victim, TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_STR;
    af.modifier = 1 + ( level >= 18 ) + ( level >= 25 ) + ( level >= 32 );
    af.bitvector = 0;
    affect_to_char( victim, &af );
    send_to_char( "Your muscles surge with heightened power!\n\r", victim );
    act( "$n's muscles surge with heightened power.", victim, NULL, NULL,
         TO_ROOM );
    return;
}

void spell_harm( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int dam;

    dam = UMAX( 20, victim->hit - dice( 1, 4 ) );
    if ( saves_spell( level, victim ) )
        dam = UMIN( 50, dam / 2 );
    dam = UMIN( 100, dam );
    damage( ch, victim, NULL, dam, sn, DAM_HARM );
    return;
}

/* RT haste spell */

void spell_haste( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || IS_AFFECTED( victim, AFF_HASTE )
         || IS_SET( victim->off_flags, OFF_FAST ) )
    {
        if ( victim == ch )
            send_to_char( "You can't move any faster!\n\r", ch );
        else
            act( "$N is already moving as fast as $e can.",
                 ch, NULL, victim, TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    if ( victim == ch )
        af.duration = level / 2;
    else
        af.duration = level / 4;
    af.location = APPLY_DEX;
    af.modifier = 1 + ( level >= 18 ) + ( level >= 25 ) + ( level >= 32 );
    af.bitvector = AFF_HASTE;
    affect_to_char( victim, &af );
    send_to_char( "You feel yourself moving more quickly.\n\r", victim );
    act( "$n is moving more quickly.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
        send_to_char( "Ok.\n\r", ch );
    return;
}

void spell_heal( int sn, int level, CHAR_DATA * ch, void *vo )
{

    CHAR_DATA *victim = ( CHAR_DATA * ) vo;

    if ( ( !IS_IMMORTAL( ch ) && !IS_NPC( ch ) && IS_NPC( victim ) &&
           !( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == victim ) ) )
    {
        send_to_char( "Why would you want to do that?\n\r", ch );
        return;
    }

    else

        victim->hit = UMIN( victim->hit + 100, victim->max_hit );
    update_pos( victim );
    send_to_char( "A warm feeling fills your body.\n\r", victim );
    if ( ch != victim )
    {
        send_to_char( "A healing power flows through you.\n\r", ch );
        gain_exp( ch,
                  cast_xp_compute( ch, ch, ch->level, 1,
                                   UMIN( 100,
                                         victim->max_hit - victim->hit ) ) );
    }
    return;
}

/* RT really nasty high-level attack spell */
void spell_holy_word( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int bless_num, curse_num, frenzy_num;

    bless_num = skill_lookup( "bless" );
    curse_num = skill_lookup( "curse" );
    frenzy_num = skill_lookup( "frenzy" );

    act( "$n utters a word of divine power!", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You utter a word of divine power.\n\r", ch );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;

        if ( ( IS_GOOD( ch ) && IS_GOOD( vch ) ) ||
             ( IS_EVIL( ch ) && IS_EVIL( vch ) ) ||
             ( IS_NEUTRAL( ch ) && IS_NEUTRAL( vch ) ) )
        {
            send_to_char( "You feel full more powerful.\n\r", vch );
            spell_frenzy( frenzy_num, level, ch, ( void * ) vch );
            spell_bless( bless_num, level, ch, ( void * ) vch );
        }

        else if ( ( IS_GOOD( ch ) && IS_EVIL( vch ) ) ||
                  ( IS_EVIL( ch ) && IS_GOOD( vch ) ) )
        {
            if ( !is_safe_spell( ch, vch, TRUE ) )
            {
                spell_curse( curse_num, level, ch, ( void * ) vch );
                act( "$n screams in agony as a holy word is burned into $s body", vch, NULL, NULL, TO_NOTVICT );
                send_to_char( "You are struck down!\n\r", vch );
                dam = dice( level, 6 );
                damage( ch, vch, NULL, dam, sn, DAM_HOLY );
            }
        }

        else if ( IS_NEUTRAL( ch ) )
        {
            if ( !is_safe_spell( ch, vch, TRUE ) )
            {
                spell_curse( curse_num, level / 2, ch, ( void * ) vch );
                act( "$n screams in agony as a holy word is burned into $s body", vch, NULL, NULL, TO_NOTVICT );
                send_to_char( "You are struck down!\n\r", vch );
                dam = dice( level, 4 );
                damage( ch, vch, NULL, dam, sn, DAM_HOLY );
            }
        }
    }

    send_to_char( "You feel drained.\n\r", ch );
    gain_exp( ch, -1 * number_range( 1, 10 ) * 10 );
    ch->move = 0;
    ch->hit /= 2;
}

void spell_identify( int sn, int level, CHAR_DATA * ch, void *vo )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj = ( OBJ_DATA * ) vo;
    AFFECT_DATA *paf;

    sprintf( buf,
             "Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\r",
             obj->name,
             item_type_name( obj ),
             extra_bit_name( obj->extra_flags ),
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

void spell_infravision( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_INFRARED ) )
    {
        if ( victim == ch )
            send_to_char( "You can already see in the dark.\n\r", ch );
        else
            act( "$N already has infravision.\n\r", ch, NULL, victim, TO_CHAR );
        return;
    }
    act( "$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM );
    af.type = sn;
    af.level = level;
    af.duration = 2 * level;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_INFRARED;
    affect_to_char( victim, &af );
    send_to_char( "Your eyes glow red.\n\r", victim );
    return;
}

void spell_invis( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_INVISIBLE ) )
        return;

    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );
    af.type = sn;
    af.level = level;
    af.duration = 24;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char( victim, &af );
    send_to_char( "You fade out of existence.\n\r", victim );
    return;
}

void spell_know_alignment( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    char *msg;
    int ap;

    ap = victim->alignment;

    if ( ap > 700 )
        msg = "$N has a pure and good aura.";
    else if ( ap > 350 )
        msg = "$N is of excellent moral character.";
    else if ( ap > 100 )
        msg = "$N is often kind and thoughtful.";
    else if ( ap > -100 )
        msg = "$N doesn't have a firm moral commitment.";
    else if ( ap > -350 )
        msg = "$N lies to $S friends.";
    else if ( ap > -700 )
        msg = "$N is a black-hearted murderer.";
    else
        msg = "$N is the embodiment of pure evil!.";

    act( msg, ch, NULL, victim, TO_CHAR );
    return;
}

void spell_lightning_bolt( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    static const sh_int dam_each[] = {
        0,
        0, 0, 0, 0, 0, 0, 0, 0, 25, 28,
        31, 34, 37, 40, 40, 41, 42, 42, 43, 44,
        44, 45, 46, 46, 47, 48, 48, 49, 50, 50,
        51, 52, 52, 53, 54, 54, 55, 56, 56, 57,
        58, 58, 59, 60, 60, 61, 62, 62, 63, 64
    };
    int dam;

    level = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level = UMAX( 0, level );
    dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    act( "$n directs a bolt of lightning at $N.", ch, NULL, victim, TO_ROOM );
    damage( ch, victim, NULL, dam, sn, DAM_LIGHTNING );
    return;
}

void spell_locate_object( int sn, int level, CHAR_DATA * ch, void *vo )
{
    char buf[MAX_STRING_LENGTH];
    char buffer[4 * MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    buffer[0] = '\0';
    max_found = IS_IMMORTAL( ch ) ? 200 : 2 * level;

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || !is_name( target_name, obj->name )
             || ( !IS_IMMORTAL( ch ) && number_percent(  ) > 2 * level )
             || ch->level < obj->level )
            continue;

        found = TRUE;
        number++;

        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
            ;

        if ( in_obj->carried_by != NULL && can_see )
        {
            sprintf( buf, "%s carried by %s\n\r",
                     obj->short_descr, PERS( in_obj->carried_by, ch ) );
        }
        else
        {
            if ( IS_IMMORTAL( ch ) && in_obj->in_room != NULL )
                sprintf( buf, "%s in %s [Room %d]\n\r",
                         obj->short_descr,
                         in_obj->in_room->name, in_obj->in_room->vnum );
            else
                sprintf( buf, "%s in %s\n\r",
                         obj->short_descr, in_obj->in_room == NULL
                         ? "somewhere" : in_obj->in_room->name );
        }

        buf[0] = UPPER( buf[0] );
        strcat( buffer, buf );

        if ( number >= max_found )
            break;
    }

    if ( !found )
        send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    else if ( ch->lines )
        page_to_char( buffer, ch );
    else
        send_to_char( buffer, ch );

    return;
}

void spell_magic_missile( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int dam;

/*  static const sh_int dam_each[] = 
    {
	 0,
	 3,  3,  4,  4,  5,	 6,  6,  6,  6,  6,
	 7,  7,  7,  7,  7,	 8,  8,  8,  8,  8,
	 9,  9,  9,  9,  9,	10, 10, 10, 10, 10,
	11, 11, 11, 11, 11,	12, 12, 12, 12, 12,
	13, 13, 13, 13, 13,	14, 14, 14, 14, 14
    };

    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 ); */
    if ( level > 25 )
        dam = dice( 3, 6 );
    /*else */
    dam = ( dice( level / 5, 3 ) + ( level / 10 ) );

    if ( saves_spell( level, victim ) )
        dam /= 2;
    act( "A green glowing arrow fires from $n's outstretched hand.", ch, NULL,
         NULL, TO_ROOM );
    damage( ch, victim, NULL, dam, sn, DAM_ENERGY );
    return;
}

void spell_mass_healing( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *gch;
    int heal_num, refresh_num;

    heal_num = skill_lookup( "heal" );
    refresh_num = skill_lookup( "refresh" );

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( ( IS_NPC( ch ) && IS_NPC( gch ) ) ||
             ( !IS_NPC( ch ) && !IS_NPC( gch ) ) )
        {
            spell_heal( heal_num, level, ch, ( void * ) gch );
            spell_refresh( refresh_num, level, ch, ( void * ) gch );
        }
    }
}

void spell_mass_invis( int sn, int level, CHAR_DATA * ch, void *vo )
{
    AFFECT_DATA af;
    CHAR_DATA *gch;

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( !is_same_group( gch, ch ) || IS_AFFECTED( gch, AFF_INVISIBLE ) )
            continue;
        act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly fade out of existence.\n\r", gch );
        af.type = sn;
        af.level = level / 2;
        af.duration = 24;
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = AFF_INVISIBLE;
        affect_to_char( gch, &af );
    }
    send_to_char( "Ok.\n\r", ch );

    return;
}

void spell_null( int sn, int level, CHAR_DATA * ch, void *vo )
{
    send_to_char( "That's not a spell!\n\r", ch );
    return;
}

void spell_pass_door( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_PASS_DOOR ) )
    {
        if ( victim == ch )
            send_to_char( "You are already out of phase.\n\r", ch );
        else
            act( "$N is already shifted out of phase.", ch, NULL, victim,
                 TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = number_fuzzy( level / 4 );
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );
    act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You turn translucent.\n\r", victim );
    return;
}

/* RT plague spell, very nasty */

void spell_plague( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( saves_spell( level, victim ) ||
         ( IS_NPC( victim ) && IS_SET( victim->act, ACT_UNDEAD ) ) )
    {
        if ( ch == victim )
            send_to_char( "You feel momentarily ill, but it passes.\n\r", ch );
        else
            act( "$N seems to be unaffected.", ch, NULL, victim, TO_CHAR );
        return;
    }

    af.type = sn;
    af.level = level * 3 / 4;
    af.duration = level;
    af.location = APPLY_STR;
    af.modifier = -5;
    af.bitvector = AFF_PLAGUE;
    affect_join( victim, &af );

    send_to_char
        ( "You scream in agony as plague sores erupt from your skin.\n\r",
          victim );
    act( "$n screams in agony as plague sores erupt from $s skin.", victim,
         NULL, NULL, TO_ROOM );
}

void spell_poison( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( saves_spell( level, victim ) )
    {
        act( "$n turns slightly green, but it passes.", victim, NULL, NULL,
             TO_ROOM );
        send_to_char( "You feel momentarily ill, but it passes.\n\r", victim );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_STR;
    af.modifier = -2;
    af.bitvector = AFF_POISON;
    affect_join( victim, &af );
    send_to_char( "You feel very sick.\n\r", victim );
    act( "$n looks very ill.", victim, NULL, NULL, TO_ROOM );
    return;
}

void spell_protection( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_PROTECT ) )
    {
        if ( victim == ch )
            send_to_char( "You are already protected.\n\r", ch );
        else
            act( "$N is already protected.", ch, NULL, victim, TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = 24;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_PROTECT;
    affect_to_char( victim, &af );
    send_to_char( "You feel protected.\n\r", victim );
    if ( ch != victim )
        act( "$N is protected from harm.", ch, NULL, victim, TO_CHAR );
    return;
}

void spell_refresh( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    victim->move = UMIN( victim->move + level, victim->max_move );
    if ( victim->max_move == victim->move )
        send_to_char( "You feel fully refreshed!\n\r", victim );
    else
        send_to_char( "You feel less tired.\n\r", victim );
    if ( ch != victim )
    {
        send_to_char( "Ok.\n\r", ch );
        gain_exp( ch, cast_xp_compute( ch, ch, ch->level, 1, ch->level ) / 2 );
    }
    return;
}

void spell_remove_curse( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    bool found = FALSE;
    OBJ_DATA *obj;
    int iWear;

    if ( check_dispel( level, victim, gsn_curse ) )
    {
        send_to_char( "You feel better.\n\r", victim );
        act( "$n looks more relaxed.", victim, NULL, NULL, TO_ROOM );
    }

    for ( iWear = 0; ( iWear < MAX_WEAR && !found ); iWear++ )
    {
        if ( ( obj = get_eq_char( victim, iWear ) ) == NULL )
            continue;

        if ( IS_OBJ_STAT( obj, ITEM_NODROP )
             || IS_OBJ_STAT( obj, ITEM_NOREMOVE ) )
        {                       /* attempt to remove curse */
            if ( !saves_dispel( level, obj->level, 0 ) )
            {
                found = TRUE;
                REMOVE_BIT( obj->extra_flags, ITEM_NODROP );
                REMOVE_BIT( obj->extra_flags, ITEM_NOREMOVE );
                act( "$p glows blue.", victim, obj, NULL, TO_CHAR );
                act( "$p glows blue.", victim, obj, NULL, TO_ROOM );
                if ( ch != victim )
                    gain_exp( ch,
                              cast_xp_compute( ch, ch, ch->level, 1,
                                               ch->level ) / 2 );
            }
        }
    }

    for ( obj = victim->carrying; ( obj != NULL && !found );
          obj = obj->next_content )
    {
        if ( IS_OBJ_STAT( obj, ITEM_NODROP )
             || IS_OBJ_STAT( obj, ITEM_NOREMOVE ) )
        {                       /* attempt to remove curse */
            if ( !saves_dispel( level, obj->level, 0 ) )
            {
                found = TRUE;
                REMOVE_BIT( obj->extra_flags, ITEM_NODROP );
                REMOVE_BIT( obj->extra_flags, ITEM_NOREMOVE );
                act( "Your $p glows blue.", victim, obj, NULL, TO_CHAR );
                act( "$n's $p glows blue.", victim, obj, NULL, TO_ROOM );
                if ( ch != victim )
                    gain_exp( ch,
                              cast_xp_compute( ch, ch, ch->level, 1,
                                               ch->level ) / 2 );
            }
        }
    }
}

void spell_sanctuary( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_SANCTUARY ) )
    {
        if ( victim == ch )
            send_to_char( "You are already in sanctuary.\n\r", ch );
        else
            act( "$N is already in sanctuary.", ch, NULL, victim, TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = number_fuzzy( level / 6 );
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_SANCTUARY;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a white aura.\n\r", victim );
    return;
}

void spell_shield( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if ( victim == ch )
            send_to_char( "You are already shielded from harm.\n\r", ch );
        else
            act( "$N is already protected by a shield.", ch, NULL, victim,
                 TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = 8 + level;
    af.location = APPLY_AC;
    af.modifier = -20;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are surrounded by a force shield.\n\r", victim );
    return;
}

void spell_shocking_grasp( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    static const int dam_each[] = {
        0,
        0, 0, 0, 0, 0, 0, 20, 25, 29, 33,
        36, 39, 39, 39, 40, 40, 41, 41, 42, 42,
        43, 43, 44, 44, 45, 45, 46, 46, 47, 47,
        48, 48, 49, 49, 50, 50, 51, 51, 52, 52,
        53, 53, 54, 54, 55, 55, 56, 56, 57, 57
    };
    int dam;

    level = UMIN( level, sizeof( dam_each ) / sizeof( dam_each[0] ) - 1 );
    level = UMAX( 0, level );
    dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim ) )
        dam /= 2;

    act( "$n grabs hold of you and shocks you.", ch, NULL, victim, TO_VICT );
    act( "$n grabs hold of $N and shocks $M.", ch, NULL, victim, TO_ROOM );
    damage( ch, victim, NULL, dam, sn, DAM_LIGHTNING );
    return;
}

void spell_sleep( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_SLEEP )
         || ( IS_NPC( victim ) && IS_SET( victim->act, ACT_UNDEAD ) )
         || level < victim->level || saves_spell( level, victim ) )
        return;

    af.type = sn;
    af.level = level;
    af.duration = 4 + level;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.bitvector = AFF_SLEEP;
    affect_join( victim, &af );

    if ( IS_AWAKE( victim ) )
    {
        send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
        act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
        victim->position = POS_SLEEPING;
    }

    return;
}

void spell_stone_skin( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( is_affected( ch, sn ) )
    {
        if ( victim == ch )
            send_to_char( "Your skin is already as hard as a rock.\n\r", ch );
        else
            act( "$N is already as hard as can be.", ch, NULL, victim,
                 TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = level;
    af.location = APPLY_AC;
    af.modifier = -40;
    af.bitvector = 0;
    affect_to_char( victim, &af );
    act( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "Your skin turns to stone.\n\r", victim );
    return;
}

void spell_summon( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim;
    extern bool chaos;

    if ( ( ( victim = get_char_world( ch, target_name ) ) == NULL ) )
    {
        send_to_char
            ( "A disturbance in the planar fabric prevents your summons.\n\r",
              ch );
        return;
    }
    if ( NOSUMMONFULL == 1 )
    {
        if ( ( victim = get_char_world( ch, target_name ) ) == NULL || victim == ch || victim->in_room == NULL || !can_see_room( ch, victim->in_room ) || IS_SET( victim->in_room->room_flags, ROOM_SAFE ) || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) || victim->level >= level + 3 || ( chaos ) || ( !IS_NPC( victim ) && victim->level >= LEVEL_HERO )  /* NOT trust */
             || ( IS_NPC( victim ) && IS_SET( victim->imm_flags, IMM_SUMMON ) )
             || ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_NOSUMMON ) )
             || ( IS_NPC( victim ) && saves_spell( level, victim ) ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your summons.\n\r",
                  ch );
            return;
        }
    }
    if ( IS_NPC( victim ) )
    {

        if ( victim == ch
             || victim->in_room == NULL
             || !can_see_room( ch, victim->in_room )
             || IS_SET( victim->in_room->room_flags, ROOM_SAFE )
             || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE )
             || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY )
             || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
             || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL )
             || victim->level >= ch->level + NOSUMMONLEVELMOB
             || ( chaos )
             || IS_SET( victim->imm_flags, IMM_SUMMON )
             || saves_spell( level, victim ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your summons.\n\r",
                  ch );
            return;
        }
    }

    if ( !IS_NPC( victim ) && !IS_SET( victim->act, PLR_KILLER ) )
    {

        if ( victim == ch || victim->in_room == NULL || !can_see_room( ch, victim->in_room ) || IS_SET( victim->in_room->room_flags, ROOM_SAFE ) || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) || victim->level >= ch->level + NOSUMMONLEVELNONPK || ( chaos ) || ( !IS_NPC( victim ) && victim->level >= LEVEL_HERO )   /* NOT trust */
             || ( IS_NPC( victim ) && IS_SET( victim->imm_flags, IMM_SUMMON ) )
             || ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_NOSUMMON ) )
             || ( IS_NPC( victim ) && saves_spell( level, victim ) ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your summons.\n\r",
                  ch );
            return;
        }
    }

    if ( NOSUMMONPKSAME == 1 && IS_SET( victim->act, PLR_KILLER ) )
    {
        if ( victim == ch || victim->in_room == NULL || !can_see_room( ch, victim->in_room ) || IS_SET( victim->in_room->room_flags, ROOM_SAFE ) || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) || victim->level > ch->level + NOSUMMONLEVELPK || ( chaos ) || ( !IS_NPC( victim ) && victim->level >= LEVEL_HERO )   /* NOT trust */
             || ( IS_NPC( victim ) && IS_SET( victim->imm_flags, IMM_SUMMON ) )
             || ( IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) )
             || ( IS_SET( victim->in_room->room_flags, ROOM_NOMAGIC ) )
             || ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_NOSUMMON )
                  && victim->level > ch->level + NOSUMMONLEVELPK )
             || ( IS_NPC( victim ) && saves_spell( level, victim ) ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your summons.\n\r",
                  ch );
            return;
        }
    }
    if ( IS_SET( victim->act, PLR_KILLER ) && NOSUMMONPKSAME == 0 )
    {

        if ( victim == ch || victim->in_room == NULL || !can_see_room( ch, victim->in_room ) || IS_SET( victim->in_room->room_flags, ROOM_SAFE ) || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) || IS_SET( ch->in_room->room_flags, ROOM_NO_RECALL ) || ( chaos ) || ( ch->level > ( victim->level + NOSUMMONLEVELPK ) || victim->level > ( ch->level + NOSUMMONLEVELPK ) ) || ( !IS_NPC( victim ) && victim->level >= LEVEL_HERO )    /* NOT trust */
             || ( IS_NPC( victim ) && IS_SET( victim->imm_flags, IMM_SUMMON ) )
             || ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_NOSUMMON )
                  && ( ch->level > ( victim->level + NOSUMMONLEVELPK )
                       || victim->level > ( ch->level + NOSUMMONLEVELPK ) ) )
             || ( IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL ) )
             || ( IS_SET( victim->in_room->room_flags, ROOM_NOMAGIC ) )
             || ( IS_NPC( victim ) && saves_spell( level, victim ) ) )
        {
            send_to_char
                ( "A disturbance in the planar fabric prevents your summons.\n\r",
                  ch );
            return;
        }

    }

    act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
    act( "$n has summoned you!", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    return;
}

void spell_teleport( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    ROOM_INDEX_DATA *pRoomIndex;
    extern bool chaos;

    if ( victim->in_room == NULL
         || IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
         || ( !IS_NPC( ch ) && victim->fighting != NULL )
         || ( chaos )
         || ( victim != ch
              && ( saves_spell( level, victim )
                   || saves_spell( level, victim ) ) ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    for ( ;; )
    {
        pRoomIndex = get_room_index( number_range( 0, 65535 ) );
        if ( pRoomIndex != NULL )
            if ( can_see_room( ch, pRoomIndex )
                 && !IS_SET( pRoomIndex->room_flags, ROOM_PRIVATE )
                 && !IS_SET( pRoomIndex->room_flags, ROOM_SOLITARY )
                 && !IS_SET( pRoomIndex->room_flags, ROOM_NOTELEPORT ) )
                break;
    }

    if ( victim != ch )
        send_to_char( "You have been teleported!\n\r", victim );

    act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
    return;
}

void spell_ventriloquate( int sn, int level, CHAR_DATA * ch, void *vo )
{
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    CHAR_DATA *vch;

    target_name = one_argument( target_name, speaker );

    sprintf( buf1, "%s says '%s'.\n\r", speaker, target_name );
    sprintf( buf2, "Someone makes %s say '%s'.\n\r", speaker, target_name );
    buf1[0] = UPPER( buf1[0] );

    for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room )
    {
        if ( !is_name( speaker, vch->name ) )
            send_to_char( saves_spell( level, vch ) ? buf2 : buf1, vch );
    }

    return;
}

void spell_weaken( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) || saves_spell( level, victim ) )
        return;
    af.type = sn;
    af.level = level;
    af.duration = level / 2;
    af.location = APPLY_STR;
    af.modifier = -1 * ( level / 5 );
    af.bitvector = AFF_WEAKEN;
    affect_to_char( victim, &af );
    send_to_char( "You feel weaker.\n\r", victim );
    act( "$n looks tired and weak.", victim, NULL, NULL, TO_ROOM );
    return;
}

/* RT recall spell is back */

void spell_word_of_recall( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    ROOM_INDEX_DATA *location;

    if ( IS_NPC( victim ) )
        return;

    if ( ( location = get_room_index( ROOM_VNUM_TEMPLE ) ) == NULL )
    {
        send_to_char( "You are completely lost.\n\r", victim );
        return;
    }

    if ( IS_SET( victim->in_room->room_flags, ROOM_NO_RECALL )
         || IS_SET( victim->in_room->room_flags, ROOM_NOMAGIC )
         || IS_AFFECTED( victim, AFF_CURSE ) )
    {
        send_to_char( "Spell failed.\n\r", victim );
        return;
    }

    if ( victim->fighting != NULL )
        stop_fighting( victim, TRUE );

    ch->move /= 2;
    act( "$n disappears.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n appears in the room.", victim, NULL, NULL, TO_ROOM );
    do_look( victim, "auto" );
}

/*
 * NPC spells.
 */
void spell_acid_breath( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    OBJ_DATA *obj_lose;
    OBJ_DATA *obj_next;
    OBJ_DATA *t_obj, *n_obj;
    int dam;
    int hpch;
    int i;

    if ( number_percent(  ) < 2 * level && !saves_spell( level, victim ) )
    {
        for ( obj_lose = ch->carrying; obj_lose != NULL; obj_lose = obj_next )
        {
            int iWear;

            obj_next = obj_lose->next_content;

            if ( number_bits( 2 ) != 0 )
                continue;

            switch ( obj_lose->item_type )
            {
            case ITEM_ARMOR:
                if ( obj_lose->value[0] > 0 )
                {
                    act( "$p is pitted and etched!",
                         victim, obj_lose, NULL, TO_CHAR );
                    if ( ( iWear = obj_lose->wear_loc ) != WEAR_NONE )
                        for ( i = 0; i < 4; i++ )
                            victim->armor[i] -= apply_ac( obj_lose, iWear, i );
                    for ( i = 0; i < 4; i++ )
                        obj_lose->value[i] -= 1;
                    obj_lose->cost = 0;
                    if ( iWear != WEAR_NONE )
                        for ( i = 0; i < 4; i++ )
                            victim->armor[i] += apply_ac( obj_lose, iWear, i );
                }
                break;

            case ITEM_CONTAINER:
                act( "$p fumes and dissolves, destroying some of the contents.",
                     victim, obj_lose, NULL, TO_CHAR );
                /* save some of  the contents */

                for ( t_obj = obj_lose->contains; t_obj != NULL; t_obj = n_obj )
                {
                    n_obj = t_obj->next_content;
                    obj_from_obj( t_obj );

                    if ( number_bits( 2 ) == 0 || victim->in_room == NULL )
                        extract_obj( t_obj );
                    else
                        obj_to_room( t_obj, victim->in_room );
                }

                extract_obj( obj_lose );
                break;

            }
        }
    }

    hpch = UMAX( 10, ch->hit );
    dam = number_range( hpch / 16 + 1, hpch / 8 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    damage( ch, victim, NULL, dam, sn, DAM_ACID );
    return;
}

void spell_fire_breath( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    OBJ_DATA *obj_lose;
    OBJ_DATA *obj_next;
    OBJ_DATA *t_obj, *n_obj;
    int dam;
    int hpch;

    if ( number_percent(  ) < 2 * level && !saves_spell( level, victim ) )
    {
        for ( obj_lose = victim->carrying; obj_lose != NULL;
              obj_lose = obj_next )
        {
            char *msg;

            obj_next = obj_lose->next_content;
            if ( number_bits( 2 ) != 0 )
                continue;

            switch ( obj_lose->item_type )
            {
            default:
                continue;
            case ITEM_CONTAINER:
                msg = "$p ignites and burns!";
                break;
            case ITEM_POTION:
                msg = "$p bubbles and boils!";
                break;
            case ITEM_SCROLL:
                msg = "$p crackles and burns!";
                break;
            case ITEM_STAFF:
                msg = "$p smokes and chars!";
                break;
            case ITEM_WAND:
                msg = "$p sparks and sputters!";
                break;
            case ITEM_FOOD:
                msg = "$p blackens and crisps!";
                break;
            case ITEM_PILL:
                msg = "$p melts and drips!";
                break;
            }

            act( msg, victim, obj_lose, NULL, TO_CHAR );
            if ( obj_lose->item_type == ITEM_CONTAINER )
            {
                /* save some of  the contents */

                for ( t_obj = obj_lose->contains; t_obj != NULL; t_obj = n_obj )
                {
                    n_obj = t_obj->next_content;
                    obj_from_obj( t_obj );

                    if ( number_bits( 2 ) == 0 || ch->in_room == NULL )
                        extract_obj( t_obj );
                    else
                        obj_to_room( t_obj, ch->in_room );
                }
            }

            extract_obj( obj_lose );
        }
    }

    hpch = UMAX( 10, ch->hit );
    dam = number_range( hpch / 16 + 1, hpch / 8 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    damage( ch, victim, NULL, dam, sn, DAM_FIRE );
    return;
}

void spell_frost_breath( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    OBJ_DATA *obj_lose;
    OBJ_DATA *obj_next;
    int dam;
    int hpch;

    if ( number_percent(  ) < 2 * level && !saves_spell( level, victim ) )
    {
        for ( obj_lose = victim->carrying; obj_lose != NULL;
              obj_lose = obj_next )
        {
            char *msg;

            obj_next = obj_lose->next_content;
            if ( number_bits( 2 ) != 0 )
                continue;

            switch ( obj_lose->item_type )
            {
            default:
                continue;
            case ITEM_DRINK_CON:
            case ITEM_POTION:
                msg = "$p freezes and shatters!";
                break;
            }

            act( msg, victim, obj_lose, NULL, TO_CHAR );
            extract_obj( obj_lose );
        }
    }

    hpch = UMAX( 10, ch->hit );
    dam = number_range( hpch / 16 + 1, hpch / 8 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    damage( ch, victim, NULL, dam, sn, DAM_COLD );
    return;
}

void spell_gas_breath( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int hpch;

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        if ( !is_safe_spell( ch, vch, TRUE ) )
        {
            hpch = UMAX( 10, ch->hit );
            dam = number_range( hpch / 16 + 1, hpch / 8 );
            if ( saves_spell( level, vch ) )
                dam /= 2;
            damage( ch, vch, NULL, dam, sn, DAM_POISON );
        }
    }
    return;
}

void spell_lightning_breath( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int dam;
    int hpch;

    hpch = UMAX( 10, ch->hit );
    dam = number_range( hpch / 16 + 1, hpch / 8 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    damage( ch, victim, NULL, dam, sn, DAM_LIGHTNING );
    return;
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int dam;

    dam = number_range( 25, 100 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    damage( ch, victim, NULL, dam, sn, DAM_PIERCE );
    return;
}

void spell_high_explosive( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int dam;

    dam = number_range( 30, 120 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    damage( ch, victim, NULL, dam, sn, DAM_PIERCE );
    return;
}

/*  Spells added by Thexder Firehawk */

void spell_firewind( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int dam;

    dam = dice( level, 17 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    act( "$n dissappears in a gust of burning winds.", victim, NULL, NULL,
         TO_ROOM );
    damage( ch, victim, NULL, dam, sn, DAM_FIRE );
    return;
}

void spell_meteor_swarm( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;

    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        if ( !is_safe_spell( ch, vch, TRUE ) )
        {
            dam = dice( 10, 10 );
            if ( saves_spell( level, vch ) )
                dam /= 2;
            act( "$n is hit full force with a fireball.", vch, NULL, NULL,
                 TO_ROOM );
            damage( ch, vch, NULL, dam, sn, DAM_FIRE );
        }
    }
    return;
}

void spell_multi_missile( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    CHAR_DATA *tmp_vict, *last_vict, *next_vict;
    bool found;
    int dam;
    int num_missile;

    /* first strike */
    num_missile = level / 5;

    act( "A magic missile erupts from $n's hand and hits $N in the chest.",
         ch, NULL, victim, TO_NOTVICT );
    act( "A magic missile flies from your hand and hits $N in the chest.",
         ch, NULL, victim, TO_CHAR );
    act( "A magic missile flies from $n's hand and hits you in the chest!",
         ch, NULL, victim, TO_VICT );

    dam = dice( level / 2, 3 );
    if ( saves_spell( level, victim ) )
        dam /= 3;
    damage( ch, victim, NULL, dam, sn, DAM_ENERGY );
    last_vict = victim;
    num_missile -= 1;           /* decrement number of missiles */

    /* new targets */
    while ( num_missile > 0 )
    {
        found = FALSE;
        for ( tmp_vict = ch->in_room->people;
              tmp_vict != NULL; tmp_vict = next_vict )
        {
            next_vict = tmp_vict->next_in_room;
            if ( !is_safe_spell( ch, tmp_vict, TRUE ) && tmp_vict != last_vict
                 && num_missile > 0 )
            {
                found = TRUE;
                last_vict = tmp_vict;
                act( "A magic missile erupts from $n's hand and hits $N in the chest.", ch, NULL, tmp_vict, TO_NOTVICT );
                act( "A magic missile flies from your hand and hits $N in the chest.", ch, NULL, tmp_vict, TO_CHAR );
                act( "A magic missile flies from $n's hand and hits you in the chest!", ch, NULL, tmp_vict, TO_VICT );
                dam = dice( level / 2, 3 );
                if ( saves_spell( level, tmp_vict ) )
                    dam /= 3;
                damage( ch, tmp_vict, NULL, dam, sn, DAM_ENERGY );
                num_missile -= 1;   /* decrement number of missiles */
            }
        }                       /* end target searching loop */

        if ( !found )           /* no target found, hit the caster */
        {
            if ( ch == NULL )
                return;

            if ( last_vict == ch )  /* no double hits */
            {
                return;
            }

            last_vict = ch;
            num_missile -= 1;   /* decrement damage */
            if ( ch == NULL )
                return;
        }
        /* now go back and find more targets */
    }
}
void spell_disintegrate( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int dam;

    dam = dice( level, 14 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    act( "$n's blast disintegrates a piece of $N's body!", ch, NULL, victim,
         TO_NOTVICT );
    act( "Your blast disintegrates a piece of $N's body!", ch, NULL, victim,
         TO_CHAR );
    act( "$n's blast disintegrates a piece of your body!", ch, NULL, victim,
         TO_VICT );
    damage( ch, victim, NULL, dam, sn, DAM_ENERGY );
    return;
}

void spell_ice_ray( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;
    int dam;

    dam = dice( level, 12 );

    act( "A blue ray from $n's hand strikes $N.", ch, NULL, victim,
         TO_NOTVICT );
    act( "A blue ray shoots from your finger and strikes $N.", ch, NULL, victim,
         TO_CHAR );
    act( "$n points at you sending a chilling ray into your chest.", ch, NULL,
         victim, TO_VICT );

    if ( !saves_spell( level, victim ) )
    {
        act( "$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM );
        af.type = sn;
        af.level = level;
        af.duration = 12;
        af.location = APPLY_STR;
        af.modifier = -2;
        af.bitvector = 0;
        affect_join( victim, &af );
    }
    else
    {
        dam /= 2;
    }

    damage( ch, victim, NULL, dam, sn, DAM_COLD );
    return;
}

void spell_holyfire( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    CHAR_DATA *tmp_vict, *last_vict, *next_vict;
    bool found;
    int dam;
    int num_blast;

    /* first strike */
    num_blast = level / 7;

    act( "A jet of flame erupts from $n's hand and engulfs $N.",
         ch, NULL, victim, TO_NOTVICT );
    act( "A jet of flames fly from your hand and engulf $N.",
         ch, NULL, victim, TO_CHAR );
    act( "A jet of flames flies from $n's hand and engulfs you!",
         ch, NULL, victim, TO_VICT );

    dam = dice( level / 2, 5 );
    if ( saves_spell( level, victim ) )
        dam /= 2;
    damage( ch, victim, NULL, dam, sn, DAM_FIRE );
    last_vict = victim;
    num_blast -= 1;             /* decrement number of blasts */

    /* new targets */
    while ( num_blast > 0 )
    {
        found = FALSE;
        for ( tmp_vict = ch->in_room->people;
              tmp_vict != NULL; tmp_vict = next_vict )
        {
            next_vict = tmp_vict->next_in_room;
            if ( !is_safe_spell( ch, tmp_vict, TRUE ) && tmp_vict != last_vict
                 && num_blast > 0 )
            {
                found = TRUE;
                last_vict = tmp_vict;

                act( "A jet of flame erupts from $n's hand and engulfs $N.",
                     ch, NULL, tmp_vict, TO_NOTVICT );
                act( "A jet of flames fly from your hand and engulf $N.",
                     ch, NULL, tmp_vict, TO_CHAR );
                act( "A jet of flames flies from $n's hand and engulfs you!",
                     ch, NULL, tmp_vict, TO_VICT );

                dam = dice( level / 2, 5 );
                if ( saves_spell( level, tmp_vict ) )
                    dam /= 2;
                damage( ch, tmp_vict, NULL, dam, sn, DAM_FIRE );
                num_blast -= 1; /* decrement number of blasts */
            }
        }                       /* end target searching loop */

        if ( !found )           /* no target found, hit the caster */
        {
            if ( ch == NULL )
                return;

            if ( last_vict == ch )  /* no double hits */
            {
                return;
            }

            last_vict = ch;
            num_blast -= 1;     /* decrement number of blasts */
            if ( ch == NULL )
                return;
        }
        /* now go back and find more targets */
    }
}

void spell_ice_storm( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    CHAR_DATA *tmp_vict, *last_vict, *next_vict;
    bool found;
    AFFECT_DATA af;
    int dam;
    int num_blast;

    /* first strike */
    num_blast = level / 10;
    act( "A blue ray from $n's hand strikes $N.", ch, NULL, victim,
         TO_NOTVICT );
    act( "A blue ray shoots from your finger and strikes $N.", ch, NULL, victim,
         TO_CHAR );
    act( "$n points at you sending a chilling ray into your chest.", ch, NULL,
         victim, TO_VICT );

    dam = dice( level / 2, 10 );
    if ( !saves_spell( level, victim ) )
    {
        act( "$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM );
        af.type = sn;
        af.level = level;
        af.duration = 8;
        af.location = APPLY_STR;
        af.modifier = -1;
        af.bitvector = 0;
        affect_join( victim, &af );
    }
    else
    {
        dam /= 2;
    }

    damage( ch, victim, NULL, dam, sn, DAM_COLD );
    last_vict = victim;
    num_blast -= 1;             /* decrement number of blasts */

    /* new targets */
    while ( num_blast > 0 )
    {
        found = FALSE;
        for ( tmp_vict = ch->in_room->people;
              tmp_vict != NULL; tmp_vict = next_vict )
        {
            next_vict = tmp_vict->next_in_room;
            if ( !is_safe_spell( ch, tmp_vict, TRUE ) && tmp_vict != last_vict
                 && num_blast > 0 )
            {
                found = TRUE;
                last_vict = tmp_vict;
                act( "A blue ray from $n's hand strikes $N.", ch, NULL,
                     tmp_vict, TO_NOTVICT );
                act( "A blue ray shoots from your finger and strikes $N.", ch,
                     NULL, tmp_vict, TO_CHAR );
                act( "$n points at you sending a chilling ray into your chest.",
                     ch, NULL, tmp_vict, TO_VICT );

                dam = dice( level / 2, 10 );
                if ( !saves_spell( level, tmp_vict ) )
                {
                    act( "$n turns blue and shivers.", tmp_vict, NULL, NULL,
                         TO_ROOM );
                    af.type = sn;
                    af.level = level;
                    af.duration = 8;
                    af.location = APPLY_STR;
                    af.modifier = -1;
                    af.bitvector = 0;
                    affect_join( tmp_vict, &af );
                }
                else
                {
                    dam /= 2;
                }

                damage( ch, tmp_vict, NULL, dam, sn, DAM_COLD );
                num_blast -= 1; /* decrement number of blasts */
            }
        }                       /* end target searching loop */

        if ( !found )           /* no target found, hit the caster */
        {
            if ( ch == NULL )
                return;

            if ( last_vict == ch )  /* no double hits */
            {
                return;
            }

            last_vict = ch;
            num_blast -= 1;     /* decrement number of blasts */
            if ( ch == NULL )
                return;
        }
        /* now go back and find more targets */
    }
}

/* Vision spell similiar to 'at person look' */

void spell_vision( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *original;

    if ( ( victim = get_char_world( ch, target_name ) ) == NULL || victim == ch || victim->in_room == NULL || !can_see_room( ch, victim->in_room ) || IS_SET( victim->in_room->room_flags, ROOM_PRIVATE ) || IS_SET( victim->in_room->room_flags, ROOM_SOLITARY ) || victim->level >= level + 3 || ( !IS_NPC( victim ) && victim->level >= LEVEL_HERO ) /* NOT trust */
         || ( IS_NPC( victim ) && IS_SET( victim->imm_flags, IMM_MENTAL ) )
         || ( IS_NPC( victim ) && saves_spell( level, victim ) ) )
    {
        send_to_char( "There is a disturbance in the ether.\n\r", ch );
        return;
    }

    act( "$n falls into a trance.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "Through a cloudy fog, you have a vision.\n\r", ch );
    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, victim->in_room );

    send_to_char( "You feel a shiver down your spine.\n\r", victim );
    do_look( ch, "auto" );
    char_from_room( ch );
    char_to_room( ch, original );
}
void spell_restoration( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    victim->hit = UMIN( victim->hit + 175, victim->max_hit );
    update_pos( victim );
    act( "$n glows with a blinding light.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "A warm feeling fills your body.\n\r", victim );
    if ( ch != victim )
        send_to_char( "You surge with healing power.\n\r", ch );
    return;
}

void spell_regeneration( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( IS_AFFECTED( victim, AFF_REGENERATION ) )
    {
        if ( victim == ch )
            send_to_char( "Your body is already in a regenerative state.\n\r",
                          ch );
        else
            act( "$N's body is already in a regerative state.", ch, NULL,
                 victim, TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = level / 12;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_REGENERATION;
    affect_to_char( victim, &af );
    send_to_char( "Your body tingles.\n\r", victim );
    if ( ch != victim )
        act( "You imbue $N's body with regenerative powers.", ch, NULL, victim,
             TO_CHAR );
    return;
}

void spell_test_area( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    int dam;
    int num_blast;

    num_blast = 5;
    for ( vch = ch->in_room->people; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next_in_room;
        if ( !is_safe_spell( ch, vch, TRUE ) && num_blast > 0 )
        {
            dam = dice( 10, 10 );

            act( "A jet of flame erupts from $n's hand and engulfs $N.",
                 ch, NULL, vch, TO_NOTVICT );
            act( "A jet of flames fly from your hand and engulf $N.",
                 ch, NULL, vch, TO_CHAR );
            act( "A jet of flames flies from $n's hand and engulfs you!",
                 ch, NULL, vch, TO_VICT );

            if ( saves_spell( level, vch ) )
                dam /= 2;
            damage( ch, vch, NULL, dam, sn, DAM_ENERGY );
            num_blast -= 1;
        }
    }
    return;
}

void spell_web( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    AFFECT_DATA af;

    if ( is_affected( victim, sn ) )
    {
        if ( victim == ch )
            send_to_char( "You are already webbed.\n\r", ch );
        else
            act( "$N is already webbed.", ch, NULL, victim, TO_CHAR );
        return;
    }
    af.type = sn;
    af.level = level;
    af.duration = level / 3;
    af.location = APPLY_AC;
    af.modifier = 80;
    af.bitvector = AFF_WEB;
    affect_to_char( victim, &af );
    send_to_char( "Your are covered in sticky webs!\n\r", victim );
    act( "$n is enmeshed in sticky webs!", victim, NULL, NULL, TO_ROOM );
    return;
}

/* 2 new spells donated to EmberMUD by Slaythar (slaythar@earthlink.net) */
/* Energize and Life Drain */
void spell_energize( int sn, int level, CHAR_DATA * ch, void *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int heal;

    if ( ch == victim )
    {
        send_to_char
            ( "ZAP!! You short circuit, maybe you should target someone else.\n\r",
              ch );
        ch->hit -= 5;           /* modify the damage as you see fit */
        return;
    }

    heal = dice( level, 3 );
    victim->mana = UMIN( victim->mana + heal, victim->max_mana );
    send_to_char( "You feel energized!\n\r", victim );
    send_to_char( "Ok.\n\r", ch );

    return;
}

/* Donated by Slaythar (modified energy drain to make life drain)*/

void spell_life_drain( int sn, int level, CHAR_DATA * ch, void
                       *vo )
{
    CHAR_DATA *victim = ( CHAR_DATA * ) vo;
    int dam;

    if ( IS_SET( victim->imm_flags, IMM_NEGATIVE ) )
    {
        send_to_char( "Your life drain was ineffective!\n\r", ch );
        return;
    }

    if ( saves_spell( level, victim ) )
    {
        send_to_char
            ( "You feel a momentary chill, as your heart lurches in your chest.\n\r",
              victim );
        return;
    }

    ch->alignment = UMAX( -1000, ch->alignment - 50 );
    if ( victim->level <= 2 )
    {
        dam = ch->hit + 1;
    }
    else
    {
        dam = dice( 3, level );
        ch->hit += dam;
        send_to_char( "You feel your life slipping away!\n\r", victim );
        send_to_char( "Wow....what a rush!!!\n\r", ch );
        damage( ch, victim, NULL, dam, sn, DAM_NEGATIVE );
    }

    return;
}

/* First attempt at a remove align spell */
void spell_remove_align( int sn, int level, CHAR_DATA * ch, void *vo )
{
    OBJ_DATA *obj = ( OBJ_DATA * ) vo;
    int result, fail;

    if ( obj->wear_loc != -1 )
    {
        send_to_char( "The item must be carried to be cast on.\n\r", ch );
        return;
    }

    fail = 25;

    /* apply other modifiers */
    fail -= 3 * level / 2;

    if ( IS_OBJ_STAT( obj, ITEM_BLESS ) )
        fail -= 15;
    if ( IS_OBJ_STAT( obj, ITEM_GLOW ) )
        fail -= 5;

    fail = URANGE( 5, fail, 95 );

    result = number_percent(  );

    /* the moment of truth */
    if ( result < ( fail / 5 ) )    /* item destroyed */
    {
        act( "$p shivers violently and explodes!", ch, obj, NULL, TO_CHAR );
        act( "$p shivers violently and explodeds!", ch, obj, NULL, TO_ROOM );
        extract_obj( obj );
        return;
    }

    if ( result <= fail )       /* failed, no bad result */
    {
        send_to_char( "Nothing seemed to happen.\n\r", ch );
        return;
    }
    if ( IS_OBJ_STAT( obj, ITEM_ANTI_EVIL )
         || IS_OBJ_STAT( obj, ITEM_ANTI_NEUTRAL )
         || IS_OBJ_STAT( obj, ITEM_ANTI_GOOD ) )
    {                           /* attempt to remove curse */
        if ( !saves_dispel( level, obj->level, 0 ) )
        {
            REMOVE_BIT( obj->extra_flags, ITEM_ANTI_EVIL );
            REMOVE_BIT( obj->extra_flags, ITEM_ANTI_NEUTRAL );
            REMOVE_BIT( obj->extra_flags, ITEM_ANTI_GOOD );
            act( "$p glows brightly.", ch, obj, NULL, TO_CHAR );
            act( "$p glows brightly.", ch, obj, NULL, TO_ROOM );
            gain_exp( ch,
                      cast_xp_compute( ch, ch, ch->level, 1, ch->level ) / 2 );
            return;
        }
    }
    send_to_char( "Nothing seemed to happen.\n\r", ch );
    return;

}

/* Attempt at adding knock */
void spell_knock( int sn, int level, CHAR_DATA * ch, void *vo )
{

    char arg[MAX_INPUT_LENGTH];
    int chance = 0;
    int door = 0;

    const sh_int rev_dir[] = {
        2, 3, 0, 1, 5, 4, 9, 8, 7, 6
    };

    target_name = one_argument( target_name, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Knock which door or direction.\n\r", ch );
        return;
    }

    if ( ch->fighting )
    {
        send_to_char( "Wait until the fight finishes.\n\r", ch );
        return;
    }

    if ( ( door = find_door( ch, arg ) ) >= 0 )

    {
        ROOM_INDEX_DATA *to_room;
        EXIT_DATA *pexit;
        EXIT_DATA *pexit_rev;

        pexit = ch->in_room->exit[door];
        if ( !IS_SET( pexit->exit_info, EX_CLOSED ) )
        {
            send_to_char( "It's already open.\n\r", ch );
            return;
        }
        if ( !IS_SET( pexit->exit_info, EX_LOCKED ) )
        {
            send_to_char( "Just try to open it.\n\r", ch );
            return;
        }
        if ( IS_SET( pexit->exit_info, EX_PASSPROOF ) )
        {
            send_to_char( "A mystical shield protects the exit.\n\r", ch );
            return;
        }

        chance =
            ch->level / 5 + get_curr_stat( ch, STAT_INT ) + get_skill( ch,
                                                                       sn ) / 5;

        act( "You cast knock on the $d, and try to open the $d!",
             ch, NULL, pexit->keyword, TO_CHAR );

        if ( room_is_dark( ch->in_room ) )
            chance /= 2;

        /* now the attack */
        if ( number_percent(  ) < chance )
        {
            REMOVE_BIT( pexit->exit_info, EX_LOCKED );
            REMOVE_BIT( pexit->exit_info, EX_CLOSED );
            act( "$n knocks on the $d and opens the lock.", ch, NULL,
                 pexit->keyword, TO_ROOM );
            send_to_char( "You successfully open the door.\n\r", ch );

            /* open the other side */
            if ( ( to_room = pexit->u1.to_room ) != NULL
                 && ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
                 && pexit_rev->u1.to_room == ch->in_room )
            {
                CHAR_DATA *rch;
                REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
                REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
                for ( rch = to_room->people; rch != NULL;
                      rch = rch->next_in_room )
                    act( "The $d opens.", rch, NULL, pexit_rev->keyword,
                         TO_CHAR );
            }
        }
        else
        {
            act( "You couldn't magically open the $d!",
                 ch, NULL, pexit->keyword, TO_CHAR );
            act( "$n failed to magically open the $d.",
                 ch, NULL, pexit->keyword, TO_ROOM );
        }
        return;
    }

    send_to_char( "You can't see that here.\n\r", ch );
    return;

}

/*
brew.c by Jason Huang (huangjac@netcom.com) 
*/

void spell_imprint( int sn, int level, CHAR_DATA * ch, void *vo )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj = ( OBJ_DATA * ) vo;
    int sp_slot, i, mana, lev;
    if ( skill_table[sn].spell_fun == spell_null )
    {
        send_to_char( "That is not a spell.\n\r", ch );
        return;
    }

    /* counting the number of spells contained within */

    for ( sp_slot = i = 1; i < 4; i++ )
        if ( obj->value[i] != -1 )
            sp_slot++;

    if ( sp_slot > 3 )
    {
        act( "$p cannot contain any more spells.", ch, obj, NULL, TO_CHAR );
        return;
    }

    /* scribe/brew costs 4 times the normal mana required to cast the spell */

    lev = skill_table[sn].skill_level[ch->Class];
    mana =
        4 * ( UMAX( skill_table[sn].min_mana, 100 / ( 2 + ch->level - lev ) ) );
    /*   mana = 4 * mana_cost(ch, sn); */

    if ( !IS_NPC( ch ) && ch->mana < mana )
    {
        send_to_char( "You don't have enough mana.\n\r", ch );
        return;
    }

    if ( number_percent(  ) > ch->pcdata->learned[sn] )
    {
        send_to_char( "You lost your concentration.\n\r", ch );
        ch->mana -= mana / 2;
        return;
    }

    /* executing the imprinting process */
    ch->mana -= mana;
    obj->value[sp_slot] = sn;

    /* Making it successively harder to pack more spells into potions or 
       scrolls - JH */

    switch ( sp_slot )
    {

    default:
        bug( "sp_slot has more than %d spells.", sp_slot );
        return;

    case 1:
        if ( number_percent(  ) > 80 )
        {
            sprintf( buf,
                     "The magic enchantment has failed --- the %s vanishes.\n\r",
                     item_type_name( obj ) );
            send_to_char( buf, ch );
            extract_obj( obj );
            return;
        }
        break;
    case 2:
        if ( number_percent(  ) > 25 )
        {
            sprintf( buf,
                     "The magic enchantment has failed --- the %s vanishes.\n\r",
                     item_type_name( obj ) );
            send_to_char( buf, ch );
            extract_obj( obj );
            return;
        }
        break;

    case 3:
        if ( number_percent(  ) > 10 )
        {
            sprintf( buf,
                     "The magic enchantment has failed --- the %s vanishes.\n\r",
                     item_type_name( obj ) );
            send_to_char( buf, ch );
            extract_obj( obj );
            return;
        }
        break;
    }

    /* labeling the item */

    free_string( &obj->short_descr );
    sprintf( buf, "a %s of ", item_type_name( obj ) );
    for ( i = 1; i <= sp_slot; i++ )
        if ( obj->value[i] != -1 )
        {
            strcat( buf, skill_table[obj->value[i]].name );
            ( i != sp_slot ) ? strcat( buf, ", " ) : strcat( buf, "" );
        }
    obj->short_descr = str_dup( buf );
    sprintf( buf, "%s %s", item_type_name( obj ), skill_table[sn].name );
    free_string( &obj->name );
    obj->name = str_dup( buf );

    sprintf( buf, "You have imbued a new spell to the %s.\n\r",
             item_type_name( obj ) );
    send_to_char( buf, ch );

    return;
}
