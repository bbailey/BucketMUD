/**************************************************************************
 * Mudprogram's (Mobprogram, Objprogram and Roomprogram) originaly        *
 * by the SMAUG development team                                          *
 * Ported to EmberMUD by Thanatos and Tyrluk of ToED                      *
 * (Temple of Eternal Death)                                              *
 * Tyrluk   - morn@telmaron.com or dajy@mindspring.com                    *
 * Thanatos - morn@telmaron.com or jonathan_w._rose@ffic.com              *
 **************************************************************************/

/* act_info.c */

#if defined(WIN32)
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"

bool can_practice( CHAR_DATA * ch, long sn );

/* command procedures needed */
DECLARE_DO_FUN( do_exits );
DECLARE_DO_FUN( do_look );
DECLARE_DO_FUN( do_help );
DECLARE_DO_FUN( do_todo );
DECLARE_DO_FUN( do_save );

char buf[MAX_STRING_LENGTH];

char *const where_name[] = {
    WORN_LIGHT,
    WORN_FINGER,
    WORN_FINGER2,
    WORN_NECK,
    WORN_NECK2,
    WORN_BODY,
    WORN_HEAD,
    WORN_LEGS,
    WORN_FEET,
    WORN_HANDS,
    WORN_ARMS,
    WORN_SHIELD,
    WORN_BODY2,
    WORN_WAIST,
    WORN_WRIST,
    WORN_WRIST2,
    WORN_WIELDED,
    WORN_HELD,
    WORN_WIELDED2
};

/*
 * Local functions.
 */
char *format_obj_to_char args( ( OBJ_DATA * obj, CHAR_DATA * ch,
                                 bool fShort ) );
void show_list_to_char args( ( OBJ_DATA * list, CHAR_DATA * ch,
                               bool fShort, bool fShowNothing ) );
void show_char_to_char_0 args( ( CHAR_DATA * victim, CHAR_DATA * ch ) );
void show_char_to_char_1 args( ( CHAR_DATA * victim, CHAR_DATA * ch ) );
void show_char_to_char args( ( CHAR_DATA * list, CHAR_DATA * ch ) );
bool check_blind args( ( CHAR_DATA * ch ) );

/*
 * externs
 */

extern bool can_use( CHAR_DATA * ch, int sn );

char *format_obj_to_char( OBJ_DATA * obj, CHAR_DATA * ch, bool fShort )
{
    buf[0] = '\0';

    if ( IS_OBJ_STAT( obj, ITEM_INVIS ) )
        strcat( buf, "`K(`bI`Bnvi`bs`K)`w " );
    if ( IS_AFFECTED( ch, AFF_DETECT_EVIL ) && IS_OBJ_STAT( obj, ITEM_EVIL ) )
        strcat( buf, "`r(`RRe`Kd Au`Rra`r)`w " );
    if ( IS_AFFECTED( ch, AFF_DETECT_MAGIC ) && IS_OBJ_STAT( obj, ITEM_MAGIC ) )
        strcat( buf, "`W(`MMa`mgic`Mal`W)`w " );
    if ( IS_OBJ_STAT( obj, ITEM_GLOW ) )
        strcat( buf, "`Y(G`Wl`Yo`Ww`Yi`Wn`Yg)`w " );
    if ( IS_OBJ_STAT( obj, ITEM_HUM ) )
        strcat( buf, "`W(`KH`Wu`Km`Wm`Ki`Wn`Kg`W)`w " );
    if ( fShort )
    {
        if ( obj->short_descr != NULL )
            strcat( buf, obj->short_descr );
    }
    else
    {
        if ( obj->description != NULL )
            strcat( buf, obj->description );
    }

    return buf;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA * list, CHAR_DATA * ch, bool fShort,
                        bool fShowNothing )
{
    char buf[MAX_STRING_LENGTH];
    char **prgpstrShow;
    int *prgnShow;
    char *pstrShow;
    OBJ_DATA *obj;
    int nShow;
    int iShow;
    int count;
    bool fCombine;

    if ( ch->desc == NULL )
        return;

    /*
     * Alloc space for output lines.
     */
    count = 0;
    for ( obj = list; obj != NULL; obj = obj->next_content )
        count++;

    /*
     * If there were no objects in the list, return and do nothing.
     */
    if ( count <= 0 )
        return;

    prgpstrShow = alloc_mem( count * sizeof( char * ) );
    prgnShow = alloc_mem( count * sizeof( int ) );
    nShow = 0;

    /*
     * Format the list of objects.
     */
    for ( obj = list; obj != NULL; obj = obj->next_content )
    {
        if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
        {
            pstrShow = format_obj_to_char( obj, ch, fShort );
            fCombine = FALSE;

            if ( IS_NPC( ch ) || IS_SET( ch->comm, COMM_COMBINE ) )
            {
                /*
                 * Look for duplicates, case sensitive.
                 * Matches tend to be near end so run loop backwords.
                 */
                for ( iShow = nShow - 1; iShow >= 0; iShow-- )
                {
                    if ( !strcmp( prgpstrShow[iShow], pstrShow ) )
                    {
                        prgnShow[iShow]++;
                        fCombine = TRUE;
                        break;
                    }
                }
            }

            /*
             * Couldn't combine, or didn't want to.
             */
            if ( !fCombine )
            {
                prgpstrShow[nShow] = str_dup( pstrShow );
                prgnShow[nShow] = 1;
                nShow++;
            }
        }
    }

    /*
     * Output the formatted list.
     */
    for ( iShow = 0; iShow < nShow; iShow++ )
    {
        if ( IS_NPC( ch ) || IS_SET( ch->comm, COMM_COMBINE ) )
        {
            if ( prgnShow[iShow] != 1 )
            {
                sprintf( buf, "`g(%2d) ", prgnShow[iShow] );
                send_to_char( buf, ch );
            }
            else
            {
                send_to_char( "`g     ", ch );
            }
        }
        send_to_char( prgpstrShow[iShow], ch );
        send_to_char( "\n\r`w", ch );
        free_string( &prgpstrShow[iShow] );
    }

    if ( fShowNothing && nShow == 0 )
    {
        if ( IS_NPC( ch ) || IS_SET( ch->comm, COMM_COMBINE ) )
            send_to_char( "`g     ", ch );
        send_to_char( "Nothing.\n\r`w", ch );
    }

    /*
     * Clean up.
     */
    free_mem( &prgpstrShow );
    free_mem( &prgnShow );

    return;
}

void show_char_to_char_0( CHAR_DATA * victim, CHAR_DATA * ch )
{
    char buf[MAX_STRING_LENGTH];
    char message[MAX_STRING_LENGTH];
    buf[0] = '\0';

    if ( IS_AFFECTED( victim, AFF_INVISIBLE ) )
        strcat( buf, "`K(`bI`Bnvi`bs`K) `C" );
    if ( IS_SET( victim->act, PLR_BUILDING ) && !IS_NPC( victim ) )
        strcat( buf, "`W[`BBu`bildi`Bng`W] `C" );
    if ( IS_AFFECTED( victim, AFF_HIDE ) )
        strcat( buf, "`K(H`wi`Kdd`we`Kn) `C" );
    if ( IS_AFFECTED( victim, AFF_CHARM ) )
        strcat( buf, "`W(`GCh`garm`Ged`W) `C" );
    if ( IS_AFFECTED( victim, AFF_PASS_DOOR ) )
        strcat( buf, "`C(Tr`Wan`wslu`Wce`Cnt) `C" );
    if ( IS_AFFECTED( victim, AFF_FAERIE_FIRE ) )
        strcat( buf, "`W(`MP`mi`Mn`mk `MA`mu`Mr`ma`W) `C" );
    if ( IS_EVIL( victim ) && IS_AFFECTED( ch, AFF_DETECT_EVIL ) )
        strcat( buf, "`r(`RRe`Kd Au`Rra`r) `C" );
    if ( IS_AFFECTED( victim, AFF_SANCTUARY ) )
        strcat( buf, "`W(`wWhi`Wte Au`wra`W) `C" );
    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_AFK ) )
        strcat( buf, "`W(`CA`cF`CK`W) `C" );
    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_KILLER ) )
        strcat( buf, "`W(`RPK`W) `C" );
    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_THIEF ) )
        strcat( buf, "`W(`YT`yHIE`YF`W) `C" );
    if ( victim->position == victim->start_pos
         && victim->long_descr[0] != '\0' )
    {
        strcat( buf, "`c" );
        strcat( buf, victim->long_descr );
    }
    if ( IS_AFFECTED( victim, AFF_WEB ) && IS_NPC( victim )
         && victim->position == victim->start_pos )
    {
        strcat( buf, "`g" );
        strcat( buf, PERS( victim, ch ) );
        strcat( buf, " is covered in sticky webs.\n`w" );
    }
    if ( victim->position == victim->start_pos
         && victim->long_descr[0] != '\0' )
    {
        if ( !IS_NPC( victim ) )
        {
            send_to_char( buf, ch );
            send_to_char( "`w", ch );
            return;

        }
        if ( IS_NPC( victim )
             && ( victim->pIndexData->vnum != MOB_VNUM_SUPERMOB ) )
        {
            send_to_char( buf, ch );
            send_to_char( "`w", ch );
            return;
        }
    }

    strcat( buf, PERS( victim, ch ) );
    if ( !IS_NPC( victim ) && !IS_SET( ch->comm, COMM_BRIEF )
         && victim->position == POS_STANDING && ch->on == NULL )
        strcat( buf, victim->pcdata->title );

    switch ( victim->position )
    {
    case POS_DEAD:
        strcat( buf, " `Cis DEAD!!" );
        break;
    case POS_MORTAL:
        strcat( buf, " `Cis mortally wounded." );
        break;
    case POS_INCAP:
        strcat( buf, " `Cis incapacitated." );
        break;
    case POS_STUNNED:
        strcat( buf, " `Cis lying here stunned." );
        break;
/*      case POS_SLEEPING: strcat( buf, " `Cis sleeping here." );      break;
      case POS_RESTING:  strcat( buf, " `Cis resting here." );       break;
      case POS_SITTING:  strcat( buf, " `Cis sitting here." );       break;
      case POS_STANDING: strcat( buf, " `Cis here." );               break;*/

    case POS_SLEEPING:
        if ( victim->on != NULL )
        {
            if ( IS_SET( victim->on->value[2], SLEEP_AT ) )
            {
                sprintf( message, " is sleeping at %s.",
                         victim->on->short_descr );
                strcat( buf, message );
            }
            else if ( IS_SET( victim->on->value[2], SLEEP_ON ) )
            {
                sprintf( message, " is sleeping on %s.",
                         victim->on->short_descr );
                strcat( buf, message );
            }
            else
            {
                sprintf( message, " is sleeping in %s.",
                         victim->on->short_descr );
                strcat( buf, message );
            }
        }
        else
            strcat( buf, " is sleeping here." );
        break;
    case POS_RESTING:
        if ( victim->on != NULL )
        {
            if ( IS_SET( victim->on->value[2], REST_AT ) )
            {
                sprintf( message, " is resting at %s.",
                         victim->on->short_descr );
                strcat( buf, message );
            }
            else if ( IS_SET( victim->on->value[2], REST_ON ) )
            {
                sprintf( message, " is resting on %s.",
                         victim->on->short_descr );
                strcat( buf, message );
            }
            else
            {
                sprintf( message, " is resting in %s.",
                         victim->on->short_descr );
                strcat( buf, message );
            }
        }
        else
            strcat( buf, " is resting here." );
        break;
    case POS_SITTING:
        if ( victim->on != NULL )
        {
            if ( IS_SET( victim->on->value[2], SIT_AT ) )
            {
                sprintf( message, " is sitting at %s.",
                         victim->on->short_descr );
                strcat( buf, message );
            }
            else if ( IS_SET( victim->on->value[2], SIT_ON ) )
            {
                sprintf( message, " is sitting on %s.",
                         victim->on->short_descr );
                strcat( buf, message );
            }
            else
            {
                sprintf( message, " is sitting in %s.",
                         victim->on->short_descr );
                strcat( buf, message );
            }
        }
        else
            strcat( buf, " is sitting here." );
        break;
    case POS_STANDING:
        if ( victim->on != NULL )
        {
            if ( IS_SET( victim->on->value[2], STAND_AT ) )
            {
                sprintf( message, " is standing at %s.",
                         victim->on->short_descr );
                strcat( buf, message );
            }
            else if ( IS_SET( victim->on->value[2], STAND_ON ) )
            {
                sprintf( message, " is standing on %s.",
                         victim->on->short_descr );
                strcat( buf, message );
            }
            else
            {
                sprintf( message, " is standing in %s.",
                         victim->on->short_descr );
                strcat( buf, message );
            }
        }
        else
            strcat( buf, " is here." );
        break;
    case POS_FIGHTING:
        strcat( buf, " is here, fighting " );
        if ( victim->fighting == NULL )
            strcat( buf, "thin air??" );
        else if ( victim->fighting == ch )
            strcat( buf, "YOU!" );
        else if ( victim->in_room == victim->fighting->in_room )
        {
            strcat( buf, PERS( victim->fighting, ch ) );
            strcat( buf, "." );
        }
        else
            strcat( buf, "somone who left??" );
        break;
    }
    if ( IS_AFFECTED( victim, AFF_WEB ) )
    {
        strcat( buf, "\n\r`g" );
        strcat( buf, PERS( victim, ch ) );
        strcat( buf, " is covered in sticky webs.`w" );
    }
    strcat( buf, "\n\r`w" );
    buf[0] = UPPER( buf[0] );

    if ( !IS_NPC( victim ) )
    {
        send_to_char( "`C", ch );
        send_to_char( buf, ch );
        return;
    }
    if ( IS_NPC( victim ) && ( victim->pIndexData->vnum != MOB_VNUM_SUPERMOB ) )
    {
        send_to_char( "`C", ch );
        send_to_char( buf, ch );
        return;
    }
    return;
}

void show_char_to_char_1( CHAR_DATA * victim, CHAR_DATA * ch )
{

    char buf[MAX_STRING_LENGTH];

    OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;

    if ( can_see( victim, ch ) )
    {
        if ( ch == victim )
            act( "$n looks at $mself.", ch, NULL, NULL, TO_ROOM );
        else
        {
            act( "$n looks at you.", ch, NULL, victim, TO_VICT );
            act( "$n looks at $N.", ch, NULL, victim, TO_NOTVICT );
        }
    }

    if ( victim->description[0] != '\0' )
    {
        send_to_char( victim->description, ch );
        send_to_char( "`w", ch );
    }
    else
    {
        act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }

    if ( victim->max_hit > 0 )
        percent = ( 100 * victim->hit ) / victim->max_hit;
    else
        percent = -1;

    strcpy( buf, PERS( victim, ch ) );

    if ( percent >= 100 )
        strcat( buf, " is in excellent condition.\n\r" );
    else if ( percent >= 90 )
        strcat( buf, " has a few scratches.\n\r" );
    else if ( percent >= 75 )
        strcat( buf, " has some small wounds and bruises.\n\r" );
    else if ( percent >= 50 )
        strcat( buf, " has quite a few wounds.\n\r" );
    else if ( percent >= 30 )
        strcat( buf, " has some big nasty wounds and scratches.\n\r" );
    else if ( percent >= 15 )
        strcat( buf, " looks pretty hurt.\n\r" );
    else if ( percent >= 0 )
        strcat( buf, " is in awful condition.\n\r" );
    else
        strcat( buf, " is `Rbleeding to death.`w\n\r" );

    buf[0] = UPPER( buf[0] );
    send_to_char( buf, ch );

    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
        if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
             && can_see_obj( ch, obj ) )
        {
            if ( !found )
            {
                send_to_char( "\n\r", ch );
                act( "$N is using:", ch, NULL, victim, TO_CHAR );
                found = TRUE;
            }

            printf_to_char( ch, "  %s`0", where_name[iWear] );

            /* TODO: There's gotta be a better way to send a given
               number of spaces to a character... */
            {
                int x = 25 - str_len( where_name[iWear] );
                int y;

                for ( y = 1; y <= x; y++ )
                {
                    send_to_char( " ", ch );
                }
            }

            send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
            send_to_char( "\n\r", ch );
        }
    }

    if ( victim != ch
         && !IS_NPC( ch )
         && number_percent(  ) < ch->pcdata->learned[gsn_peek] )
    {
        send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
        check_improve( ch, gsn_peek, TRUE, 4 );
        show_list_to_char( victim->carrying, ch, TRUE, TRUE );
    }

    return;
}

void show_char_to_char( CHAR_DATA * list, CHAR_DATA * ch )
{
    CHAR_DATA *rch;

    for ( rch = list; rch != NULL; rch = rch->next_in_room )
    {
        if ( rch == ch )
            continue;

        if ( !IS_NPC( rch )
             && IS_SET( rch->act, PLR_WIZINVIS )
             && get_trust( ch ) < rch->invis_level )
            continue;

        if ( can_see( ch, rch ) )
        {
            show_char_to_char_0( rch, ch );
        }
        else if ( room_is_dark( ch->in_room )
                  && IS_AFFECTED( rch, AFF_INFRARED ) )
        {
            send_to_char( "`RYou see glowing red eyes watching you!\n\r`w",
                          ch );
        }
    }

    return;
}

bool check_blind( CHAR_DATA * ch )
{

    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_HOLYLIGHT ) )
        return TRUE;

    if ( IS_AFFECTED( ch, AFF_BLIND ) )
    {
        send_to_char( "You can't see a thing!\n\r", ch );
        return FALSE;
    }

    return TRUE;
}

/* changes your scroll */
void do_scroll( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    int lines;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        if ( ch->lines == 0 )
            send_to_char( "You do not page long messages.\n\r", ch );
        else
        {
            sprintf( buf, "You currently display %d lines per page.\n\r",
                     ch->lines + 2 );
            send_to_char( buf, ch );
        }
        return;
    }

    if ( !is_number( arg ) )
    {
        send_to_char( "You must provide a number.\n\r", ch );
        return;
    }

    lines = atoi( arg );

    if ( lines == 0 )
    {
        send_to_char( "Paging disabled.\n\r", ch );
        ch->lines = 0;
        return;
    }

    if ( lines < 10 || lines > 100 )
    {
        send_to_char( "You must provide a reasonable number.\n\r", ch );
        return;
    }

    sprintf( buf, "Scroll set to %d lines.\n\r", lines );
    send_to_char( buf, ch );
    ch->lines = lines - 2;
}

/* RT does socials */

/* RT Commands to replace news, motd, imotd, etc from ROM */
void do_news( CHAR_DATA * ch, char *argument )
{
    do_help( ch, "news" );
}

void do_motd( CHAR_DATA * ch, char *argument )
{
    do_help( ch, "motd" );
}

void do_imotd( CHAR_DATA * ch, char *argument )
{
    do_help( ch, "imotd" );
}

void do_rules( CHAR_DATA * ch, char *argument )
{
    do_help( ch, "rules" );
}

void do_story( CHAR_DATA * ch, char *argument )
{
    do_help( ch, "story" );
}

void do_changes( CHAR_DATA * ch, char *argument )
{
    do_help( ch, "changes" );
}

void do_wizlist( CHAR_DATA * ch, char *argument )
{
    do_help( ch, "wizlist" );
}

void do_autolist( CHAR_DATA * ch, char *argument )
{
    /* lists most player flags */
    if ( IS_NPC( ch ) )
        return;

    send_to_char( "   action     status\n\r", ch );
    send_to_char( "---------------------\n\r", ch );

    send_to_char( "autoassist     ", ch );
    if ( IS_SET( ch->act, PLR_AUTOASSIST ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    send_to_char( "autoexit       ", ch );
    if ( IS_SET( ch->act, PLR_AUTOEXIT ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    send_to_char( "autogold       ", ch );
    if ( IS_SET( ch->act, PLR_AUTOGOLD ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    send_to_char( "autoloot       ", ch );
    if ( IS_SET( ch->act, PLR_AUTOLOOT ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    send_to_char( "autosac        ", ch );
    if ( IS_SET( ch->act, PLR_AUTOSAC ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    send_to_char( "autosplit      ", ch );
    if ( IS_SET( ch->act, PLR_AUTOSPLIT ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    send_to_char( "prompt         ", ch );
    if ( IS_SET( ch->comm, COMM_PROMPT ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    send_to_char( "combine items  ", ch );
    if ( IS_SET( ch->comm, COMM_COMBINE ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    if ( !IS_SET( ch->act, PLR_CANLOOT ) )
        send_to_char( "Your corpse is safe from thieves.\n\r", ch );
    else
        send_to_char( "Your corpse may be looted.\n\r", ch );

    if ( IS_SET( ch->act, PLR_NOSUMMON ) )
        send_to_char
            ( "You cannot be summoned. There may be exceptions to this!\n\r",
              ch );
    else
        send_to_char( "You can be summoned.\n\r", ch );

    if ( IS_SET( ch->act, PLR_NOFOLLOW ) )
        send_to_char( "You do not welcome followers.\n\r", ch );
    else
        send_to_char( "You accept followers.\n\r", ch );

    if ( IS_SET( ch->act, PLR_COLOR ) )
        send_to_char( "You have ansi `Yc`Ro`Bl`Co`Gr`w turned `Yon`w.\n\r",
                      ch );
    else
        send_to_char( "You have ansi color turned off.\n\r", ch );
}

void do_autoassist( CHAR_DATA * ch, char *argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( IS_SET( ch->act, PLR_AUTOASSIST ) )
    {
        send_to_char( "Autoassist removed.\n\r", ch );
        REMOVE_BIT( ch->act, PLR_AUTOASSIST );
    }
    else
    {
        send_to_char( "You will now assist when needed.\n\r", ch );
        SET_BIT( ch->act, PLR_AUTOASSIST );
    }
}

void do_autoexit( CHAR_DATA * ch, char *argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( IS_SET( ch->act, PLR_AUTOEXIT ) )
    {
        send_to_char( "Exits will no longer be displayed.\n\r", ch );
        REMOVE_BIT( ch->act, PLR_AUTOEXIT );
    }
    else
    {
        send_to_char( "Exits will now be displayed.\n\r", ch );
        SET_BIT( ch->act, PLR_AUTOEXIT );
    }
}

void do_autogold( CHAR_DATA * ch, char *argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( IS_SET( ch->act, PLR_AUTOGOLD ) )
    {
        send_to_char( "Autogold removed.\n\r", ch );
        REMOVE_BIT( ch->act, PLR_AUTOGOLD );
    }
    else
    {
        send_to_char( "Automatic gold looting set.\n\r", ch );
        SET_BIT( ch->act, PLR_AUTOGOLD );
    }
}

void do_autoloot( CHAR_DATA * ch, char *argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( IS_SET( ch->act, PLR_AUTOLOOT ) )
    {
        send_to_char( "Autolooting removed.\n\r", ch );
        REMOVE_BIT( ch->act, PLR_AUTOLOOT );
    }
    else
    {
        send_to_char( "Automatic corpse looting set.\n\r", ch );
        SET_BIT( ch->act, PLR_AUTOLOOT );
    }
}

void do_autosac( CHAR_DATA * ch, char *argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( IS_SET( ch->act, PLR_AUTOSAC ) )
    {
        send_to_char( "Autosacrificing removed.\n\r", ch );
        REMOVE_BIT( ch->act, PLR_AUTOSAC );
    }
    else
    {
        send_to_char( "Automatic corpse sacrificing set.\n\r", ch );
        SET_BIT( ch->act, PLR_AUTOSAC );
    }
}

void do_autosplit( CHAR_DATA * ch, char *argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( IS_SET( ch->act, PLR_AUTOSPLIT ) )
    {
        send_to_char( "Autosplitting removed.\n\r", ch );
        REMOVE_BIT( ch->act, PLR_AUTOSPLIT );
    }
    else
    {
        send_to_char( "Automatic gold splitting set.\n\r", ch );
        SET_BIT( ch->act, PLR_AUTOSPLIT );
    }
}

void do_brief( CHAR_DATA * ch, char *argument )
{
    if ( IS_SET( ch->comm, COMM_BRIEF ) )
    {
        send_to_char( "Full descriptions activated.\n\r", ch );
        REMOVE_BIT( ch->comm, COMM_BRIEF );
    }
    else
    {
        send_to_char( "Short descriptions activated.\n\r", ch );
        SET_BIT( ch->comm, COMM_BRIEF );
    }
}

void do_compact( CHAR_DATA * ch, char *argument )
{
    if ( IS_SET( ch->comm, COMM_COMPACT ) )
    {
        send_to_char( "Compact mode removed.\n\r", ch );
        REMOVE_BIT( ch->comm, COMM_COMPACT );
    }
    else
    {
        send_to_char( "Compact mode set.\n\r", ch );
        SET_BIT( ch->comm, COMM_COMPACT );
    }
}

void do_prompt( CHAR_DATA * ch, char *argument )
{
    if ( !IS_NPC( ch ) )
    {
        if ( !strcmp( argument, "default" ) )
        {
            free_string( &ch->pcdata->prompt );
            ch->pcdata->prompt =
                str_dup( "%i`K/`W%H`w HP %n`K/`W%M`w MP %w`K/`W%V`w MV `K> " );
        }

        else if ( !strcmp( argument, "combat" ) )
        {
            free_string( &ch->pcdata->prompt );
            ch->pcdata->prompt =
                str_dup
                ( "`gTank: %l  `rEnemy: %e%r%i`K/`W%H `wHP %n`K/`W%M `wMP %w`K/`W%V `wMV `K>" );
        }

        else if ( !strcmp( argument, "ghioti" ) )
        {
            free_string( &ch->pcdata->prompt );
            ch->pcdata->prompt =
                str_dup
                ( "`cMe: %s  `gTank: %l  `rEnemy: %e`w%r<%i/%Hhp %n/%Mm %w/%Vmv %Xtnl>" );
        }

        else if ( IS_IMMORTAL( ch ) && !strcmp( argument, "imm" ) )
        {
            free_string( &ch->pcdata->prompt );
            ch->pcdata->prompt =
                str_dup
                ( " %B `cRoom: `C%# `gTime: `G%T `wActs: `W%A%r`Y:>`W `w" );
        }

        else
        {
            free_string( &ch->pcdata->prompt );
            smash_tilde( argument );
            ch->pcdata->prompt = str_dup( argument );
        }
        send_to_char( "Prompt set.\n\r", ch );
        return;
    }
    else
        send_to_char( "Mobiles may not change thier prompts.\n\r", ch );
}

void do_combine( CHAR_DATA * ch, char *argument )
{
    if ( IS_SET( ch->comm, COMM_COMBINE ) )
    {
        send_to_char( "Long inventory selected.\n\r", ch );
        REMOVE_BIT( ch->comm, COMM_COMBINE );
    }
    else
    {
        send_to_char( "Combined inventory selected.\n\r", ch );
        SET_BIT( ch->comm, COMM_COMBINE );
    }
}

void do_noloot( CHAR_DATA * ch, char *argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( IS_SET( ch->act, PLR_CANLOOT ) )
    {
        send_to_char( "Your corpse is now safe from thieves.\n\r", ch );
        REMOVE_BIT( ch->act, PLR_CANLOOT );
    }
    else
    {
        send_to_char( "Your corpse may now be looted.\n\r", ch );
        SET_BIT( ch->act, PLR_CANLOOT );
    }
}

void do_nofollow( CHAR_DATA * ch, char *argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( IS_SET( ch->act, PLR_NOFOLLOW ) )
    {
        send_to_char( "You now accept followers.\n\r", ch );
        REMOVE_BIT( ch->act, PLR_NOFOLLOW );
    }
    else
    {
        send_to_char( "You no longer accept followers.\n\r", ch );
        SET_BIT( ch->act, PLR_NOFOLLOW );
        if ( !IS_AFFECTED( ch, AFF_CHARM ) )    /* bugfix... if a pc was charmed you could type nofollow to become uncharmed. */
            die_follower( ch );
    }
}

void do_nosummon( CHAR_DATA * ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
        if ( IS_SET( ch->imm_flags, IMM_SUMMON ) )
        {
            send_to_char( "You are no longer immune to summon.\n\r", ch );
            REMOVE_BIT( ch->imm_flags, IMM_SUMMON );
        }
        else
        {
            send_to_char( "You are now immune to summoning.\n\r", ch );
            SET_BIT( ch->imm_flags, IMM_SUMMON );
        }
    }
    else
    {
        if ( IS_SET( ch->act, PLR_NOSUMMON ) )
        {
            send_to_char( "You are no longer immune to summon.\n\r", ch );
            REMOVE_BIT( ch->act, PLR_NOSUMMON );
        }
        else
        {
            send_to_char
                ( "You are now immune to summoning. There may  be exceptions to this!\n\r",
                  ch );
            SET_BIT( ch->act, PLR_NOSUMMON );
        }
    }
}

void do_nocolor( CHAR_DATA * ch, char *argument )
{
    if ( IS_SET( ch->act, PLR_COLOR ) )
    {
        send_to_char( "You no longer see in color.\n\r", ch );
        REMOVE_BIT( ch->act, PLR_COLOR );
    }
    else
    {
        send_to_char( "You can see in `Yc`Ro`Bl`Co`Gr`w.\n\r", ch );
        SET_BIT( ch->act, PLR_COLOR );
    }
}

void do_afk( CHAR_DATA * ch, char *argument )
{
    char buf2[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
        return;

    if ( IS_SET( ch->act, PLR_AFK ) )
    {
        send_to_char( "You are no longer set AFK.\n\r", ch );
        REMOVE_BIT( ch->act, PLR_AFK );
        if ( ch->pcdata->message != NULL )
        {
            sprintf( buf2,
                     "You have `W%d message%s`w waiting, type `Wmessages`w to read them.\n\r",
                     ch->pcdata->messages,
                     ( ch->pcdata->messages > 1 ) ? "s" : "" );
            send_to_char( buf2, ch );
        }
        act( "`R$n returns to $s keyboard.`w", ch, NULL, NULL, TO_ROOM );
        sprintf( buf2, "%s is no-longer AFK.", ch->name );
        if ( !IS_SET( ch->act, PLR_WIZINVIS ) )
            do_sendinfo( ch, buf2 );
    }
    else
    {
        send_to_char( "You are now set AFK.  Messages are being recorded.\n\r",
                      ch );
        SET_BIT( ch->act, PLR_AFK );
        act( "`W$n is away from $s keyboard for awhile.`w", ch, NULL, NULL,
             TO_ROOM );
        sprintf( buf2, "%s has gone AFK.", ch->name );
        if ( !IS_SET( ch->act, PLR_WIZINVIS ) )
            do_sendinfo( ch, buf2 );
    }
}

void do_anonymous( CHAR_DATA * ch, char *argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( ch->anonymous == TRUE )
    {
        send_to_char( "You are no longer `KA`wN`WO`wN`KY`wM`WO`wU`KS`W.\n\r",
                      ch );
        ch->anonymous = FALSE;
    }
    else
    {
        send_to_char
            ( "You are now set `KA`wN`WO`wN`KY`wM`WO`wU`KS`w.  Your Level, Race, and Class will no longer be displayed under \"who\".\n\r",
              ch );
        ch->anonymous = TRUE;
    }
}

void do_pk( CHAR_DATA * ch, char *argument )
{
    char buf2[MAX_STRING_LENGTH];
    CLAN_DATA *clan;

    if ( IS_NPC( ch ) )
        return;

    if ( ch->pcdata->confirm_pk )
    {
        if ( argument[0] != '\0' )
        {
            send_to_char( "PK readiness status removed.\n\r", ch );
            ch->pcdata->confirm_pk = FALSE;
            return;
        }
        else
        {
            if ( IS_SET( ch->act, PLR_KILLER ) )
                return;
            SET_BIT( ch->act, PLR_KILLER );
            act( "`R$n glows briefly with a red aura, you get the feeling you should keep your distance.`w", ch, NULL, NULL, TO_ROOM );
            send_to_char
                ( "`RYou are now a Player Killer, good luck, you'll need it.\n\r`w",
                  ch );
            sprintf( buf2, "%s has become a player killer!", ch->name );
            do_sendinfo( ch, buf2 );
            return;
        }
    }

    if ( argument[0] != '\0' )
    {
        send_to_char( "Just type pk. No argument.\n\r", ch );
        return;
    }
    if ( ch->pcdata->clan != 0 )
    {
        clan = get_clan( ch->pcdata->clan );
        if ( IS_SET( clan->join_flags, CLAN_NO_PK ) )
        {
            send_to_char( "Your clan does not allow PK!\n\r", ch );
            return;
        }
    }

    send_to_char( "Type pk again to confirm this command.\n\r", ch );
    send_to_char( "WARNING: this command is virtually irreversible.\n\r", ch );
    send_to_char
        ( "If you don't know what pk is for read help pk, DON'T type this command again.\n\r",
          ch );
    send_to_char
        ( "Typing pk with an argument will undo pk readiness status.\n\r", ch );
    ch->pcdata->confirm_pk = TRUE;
}

void eval_dir( char *dir, int mov_dir, int num, CHAR_DATA * ch, int *see,
               ROOM_INDEX_DATA * first_room )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob_in_room;

    if ( room_is_dark( first_room ) )
    {
        if ( *see < 1 )
        {
            sprintf( buf, "  `W%s `wfrom you is hidden in darkness.\n\r", dir );
            send_to_char( buf, ch );
            return;
        }
        else
        {
            send_to_char
                ( "     It's too dark to see any further in this direction.\n\r",
                  ch );
            return;
        }
    }

    for ( mob_in_room = first_room->people; mob_in_room != NULL;
          mob_in_room = mob_in_room->next_in_room )
    {

        if ( can_see( ch, mob_in_room ) )
        {

            if ( *see == 0 )
            {
                sprintf( buf, "  `W%s `wfrom you, you see :\n\r", dir );
                send_to_char( buf, ch );
                *see += 1;
            }
            if ( !IS_NPC( mob_in_room ) )
            {
                sprintf( buf, "     %s%s - %d %s\n\r", mob_in_room->name,
                         mob_in_room->pcdata->title, num, dir );
                send_to_char( buf, ch );
            }
            else
            {
                sprintf( buf, "     %s - %d %s\n\r",
                         mob_in_room->short_descr, num, dir );
                send_to_char( buf, ch );
            }
        }
    }
}

void show_dir_mobs( char *dir, int move_dir, CHAR_DATA * ch, int depth )
{
    ROOM_INDEX_DATA *cur_room = ch->in_room;
    EXIT_DATA *pexit;
    int see = 0;
    int i;

    for ( i = 1; ( i <= depth && ( pexit = cur_room->exit[move_dir] )
                   && pexit->u1.to_room
                   && pexit->u1.to_room != cur_room )
          && !IS_SET( pexit->exit_info, EX_CLOSED ); i++ )
    {
        cur_room = pexit->u1.to_room;
        eval_dir( dir, move_dir, i, ch, &see, cur_room );
    }
}

char *dir_text[] = { "North", "East", "South", "West", "Up", "Down" };

void do_scan( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *mob_in_room;
    int door;

    if ( room_is_dark( ch->in_room ) )
    {
        send_to_char( "It's too dark in here, you can't see anything!\n\r",
                      ch );
        return;
    }

    for ( mob_in_room = ch->in_room->people; mob_in_room != NULL;
          mob_in_room = mob_in_room->next_in_room )
    {
        if ( can_see( ch, mob_in_room ) )
        {

            if ( !IS_NPC( mob_in_room ) )
            {
                sprintf( buf, "     %s%s - right here.\n\r",
                         mob_in_room->name, mob_in_room->pcdata->title );
                send_to_char( buf, ch );
            }
            else
            {
                sprintf( buf, "     %s - right here.\n\r",
                         mob_in_room->short_descr );
                send_to_char( buf, ch );
            }

        }

    }
    for ( door = 0; door <= 5; door++ )
        show_dir_mobs( dir_text[door], door, ch, 3 );
    act( "$n scans $s surroundings.", ch, NULL, NULL, TO_ROOM );

}

void do_effects( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    AFFECT_DATA *paf;
    NEWAFFECT_DATA *npaf;

    switch ( ch->position )
    {
    case POS_DEAD:
        send_to_char( "You are DEAD!!\n\r", ch );
        break;
    case POS_MORTAL:
        send_to_char( "You are mortally wounded.\n\r", ch );
        break;
    case POS_INCAP:
        send_to_char( "You are incapacitated.\n\r", ch );
        break;
    case POS_STUNNED:
        send_to_char( "You are stunned.\n\r", ch );
        break;
    case POS_SLEEPING:
        send_to_char( "You are sleeping.\n\r", ch );
        break;
    case POS_RESTING:
        send_to_char( "You are resting.\n\r", ch );
        break;
    case POS_STANDING:
        send_to_char( "You are standing.\n\r", ch );
        break;
    case POS_FIGHTING:
        send_to_char( "You are fighting.\n\r", ch );
        break;
    }
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
        send_to_char( "You are drunk.\n\r", ch );
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_THIRST] == 0 )
        send_to_char( "You are thirsty.\n\r", ch );
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] == 0 )
        send_to_char( "You are hungry.\n\r", ch );
    if ( ch->affected != NULL )
    {
        send_to_char( "You are affected by:\n\r", ch );
        for ( paf = ch->affected; paf != NULL; paf = paf->next )
        {
            sprintf( buf, "Spell: '%s'", skill_table[paf->type].name );
            send_to_char( buf, ch );

            if ( ch->level >= 20 )
            {
                sprintf( buf, " modifies %s by %d for %d hours",
                         affect_loc_name( paf->location ),
                         paf->modifier, paf->duration );
                send_to_char( buf, ch );
            }

            send_to_char( ".\n\r", ch );
        }
    }
    if ( ch->newaffected != NULL )
    {
        for ( npaf = ch->newaffected; npaf != NULL; npaf = npaf->next )
        {
            sprintf( buf, "ZaksBugSpell: '%s'", skill_table[npaf->type].name );
            send_to_char( buf, ch );

            if ( ch->level >= 20 )
            {
                sprintf( buf, " modifies %s by %d for %d hours",
                         affect_loc_name( npaf->location ),
                         npaf->modifier, npaf->duration );
                send_to_char( buf, ch );
            }

            send_to_char( ".\n\r", ch );
        }
    }
}

void do_levels( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    int x;
    int orig_level;

    if ( !IS_NPC( ch ) )
    {
        orig_level = ch->level;
        if ( ch->level < 4 )
            switch ( ch->level )
            {
            case 1:
                break;
            case 2:
            case 3:
                ch->level = 1;
                break;
            }
        else
            ch->level -= 3;
        for ( x = ch->level; x < orig_level + 5; x++ )
        {
            if ( x == orig_level )
                sprintf( buf, "`BLevel %d: %ld <--- You're here.\n\r`w",
                         ch->level, exp_per_level( ch, ch->pcdata->points ) );
            else
                sprintf( buf, "Level %d: %ld\n\r", ch->level,
                         exp_per_level( ch, ch->pcdata->points ) );
            ch->level++;
            send_to_char( buf, ch );
        }
        ch->level = orig_level;
    }
    else
        printf( "Mobs don't level.\n\r" );
}

void do_tick( CHAR_DATA * ch, char *argument )
{
    if ( IS_NPC( ch ) )
        return;

    if ( ch->pcdata->tick == 1 )
    {
        ch->pcdata->tick = 0;
        send_to_char( "The MUD will no longer alert you of ticks.\n\r", ch );
    }
    else
    {
        ch->pcdata->tick = 1;
        send_to_char( "The MUD will now alert you of ticks.\n\r", ch );
    }
}

void do_look( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    char *pdesc;
    int door;
    int number, count;
    ROOM_INDEX_DATA *original;
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *pRoom;

    if ( ch->desc == NULL )
        return;

    if ( ch->position < POS_SLEEPING )
    {
        send_to_char( "You can't see anything but stars!\n\r", ch );
        return;
    }

    if ( ch->position == POS_SLEEPING )
    {
        send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
        return;
    }

    if ( !check_blind( ch ) )
        return;

    if ( !IS_NPC( ch )
         && !IS_SET( ch->act, PLR_HOLYLIGHT ) && room_is_dark( ch->in_room ) )
    {
        send_to_char( "It is pitch black ... \n\r", ch );
        show_char_to_char( ch->in_room->people, ch );
        return;
    }

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    number = number_argument( arg, arg3 );
    count = 0;

    if ( arg[0] == '\0' || !str_cmp( arg, "auto" ) )
    {
        /* 'look' or 'look auto' */
        send_to_char( "`B", ch );
        send_to_char( ch->in_room->name, ch );
        send_to_char( "`w\n\r", ch );

        if ( arg[0] == '\0'
             || ( !IS_NPC( ch ) && !IS_SET( ch->comm, COMM_BRIEF ) ) )
        {
            send_to_char( "  ", ch );
            send_to_char( ch->in_room->description, ch );
        }

        if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AUTOEXIT ) )
        {
            send_to_char( "\n\r`W", ch );
            do_exits( ch, "auto" );
            send_to_char( "`w", ch );
        }

        show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
        show_char_to_char( ch->in_room->people, ch );
        return;
    }

    if ( !str_cmp( arg, "i" ) || !str_cmp( arg, "in" ) )
    {
        /* 'look in' */
        if ( arg2[0] == '\0' )
        {
            send_to_char( "Look in what?\n\r", ch );
            return;
        }

        if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL )
        {
            send_to_char( "You do not see that here.\n\r", ch );
            return;
        }

        switch ( obj->item_type )
        {
        default:
            send_to_char( "That is not a container.\n\r", ch );
            break;

        case ITEM_DRINK_CON:
            if ( obj->value[1] <= 0 )
            {
                send_to_char( "It is empty.\n\r", ch );
                break;
            }

            sprintf( buf, "It's %s full of a %s liquid.\n\r",
                     obj->value[1] < obj->value[0] / 4
                     ? "less than" :
                     obj->value[1] < 3 * obj->value[0] / 4
                     ? "about" : "more than",
                     liq_table[obj->value[2]].liq_color );

            send_to_char( buf, ch );
            break;

        case ITEM_PORTAL:
            /* okay, so I stole it from do_at, but it was easier that way */
            if ( ( pRoom = get_room_index( obj->value[0] ) ) == NULL
                 || obj->value[0] == 0 )
            {
                send_to_char( "You can't see anything but a hazy mist.", ch );
                return;
            }
            location = get_room_index( obj->value[0] );
            original = ch->in_room;
            char_from_room( ch );
            char_to_room( ch, location );
            do_look( ch, "auto" );
            act( "$n peeps $s head out of a portal and quickly ducks it back in.", ch, NULL, NULL, TO_ROOM );
            char_from_room( ch );
            char_to_room( ch, original );
            break;

        case ITEM_CONTAINER:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
            if ( IS_SET( obj->value[1], CONT_CLOSED ) )
            {
                send_to_char( "It is closed.\n\r", ch );
                break;
            }

            act( "$p contains:", ch, obj, NULL, TO_CHAR );
            show_list_to_char( obj->contains, ch, TRUE, TRUE );
            break;
        }
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) != NULL )
    {
        show_char_to_char_1( victim, ch );
        return;
    }

    for ( obj = ch->carrying; obj != NULL; obj = obj->next_content )
    {
        if ( can_see_obj( ch, obj ) )
        {
            pdesc = get_extra_descr( arg3, obj->extra_descr );
            if ( pdesc )
            {
                if ( ++count == number )
                {
                    send_to_char( pdesc, ch );
                    return;
                }
                else
                    continue;
            }

            pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
            if ( pdesc )
            {
                if ( ++count == number )
                {
                    send_to_char( pdesc, ch );
                    return;
                }
                else
                    continue;
            }

            if ( is_name( arg3, obj->name ) )
                if ( ++count == number )
                {
                    send_to_char( obj->description, ch );
                    send_to_char( "\n\r", ch );
                    return;
                }
        }
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content )
    {
        if ( can_see_obj( ch, obj ) )
        {
            pdesc = get_extra_descr( arg3, obj->extra_descr );
            if ( pdesc != NULL )
                if ( ++count == number )
                {
                    send_to_char( pdesc, ch );
                    return;
                }

            pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
            if ( pdesc != NULL )
                if ( ++count == number )
                {
                    send_to_char( pdesc, ch );
                    return;
                }
        }

        if ( is_name( arg3, obj->name ) )
            if ( ++count == number )
            {
                send_to_char( obj->description, ch );
                send_to_char( "\n\r", ch );
                return;
            }
    }

    if ( count > 0 && count != number )
    {
        if ( count == 1 )
            sprintf( buf, "You only see one %s here.\n\r", arg3 );
        else
            sprintf( buf, "You only see %d %s's here.\n\r", count, arg3 );

        send_to_char( buf, ch );
        return;
    }

    pdesc = get_extra_descr( arg, ch->in_room->extra_descr );
    if ( pdesc != NULL )
    {
        send_to_char( pdesc, ch );
        return;
    }

    if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) )
        door = 0;
    else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east" ) )
        door = 1;
    else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) )
        door = 2;
    else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west" ) )
        door = 3;
    else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up" ) )
        door = 4;
    else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down" ) )
        door = 5;
    else
    {
        send_to_char( "You do not see that here.\n\r", ch );
        return;
    }

    /* 'look direction' */
    if ( ( pexit = ch->in_room->exit[door] ) == NULL )
    {
        send_to_char( "Nothing special there.\n\r", ch );
        return;
    }

    if ( pexit->description != NULL && pexit->description[0] != '\0' )
        send_to_char( pexit->description, ch );
    else
        send_to_char( "Nothing special there.\n\r", ch );

    if ( pexit->keyword != NULL
         && pexit->keyword[0] != '\0' && pexit->keyword[0] != ' ' )
    {
        if ( IS_SET( pexit->exit_info, EX_CLOSED )
             && !IS_SET( pexit->exit_info, EX_HIDDEN ) )
        {
            act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
        }
        else if ( ( IS_SET( pexit->exit_info, EX_ISDOOR )
                    && !IS_SET( pexit->exit_info, EX_HIDDEN ) )
                  || ( IS_SET( pexit->exit_info, EX_ISDOOR )
                       && IS_SET( pexit->exit_info, EX_HIDDEN )
                       && !IS_SET( pexit->exit_info, EX_CLOSED ) ) )
        {
            act( "The $d is open.", ch, NULL, pexit->keyword, TO_CHAR );
        }
    }

    return;
}

/* RT added back for the hell of it */
void do_read( CHAR_DATA * ch, char *argument )
{
    do_look( ch, argument );
}

void do_examine( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg2 );

    if ( arg2[0] == '\0' )
    {
        send_to_char( "Examine what?\n\r", ch );
        return;
    }

    do_look( ch, arg2 );

    if ( ( obj = get_obj_here( ch, arg2 ) ) != NULL )
    {
        switch ( obj->item_type )
        {
        default:
            break;

        case ITEM_DRINK_CON:
        case ITEM_CONTAINER:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
            send_to_char( "When you look inside, you see:\n\r", ch );
            sprintf( buf, "in %s", arg2 );
            do_look( ch, buf );
        }
    }

    if ( obj )
        oprog_examine_trigger( ch, obj );
    return;
}

/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    extern char *const dir_name[];
    EXIT_DATA *pexit;
    bool found;
    bool fAuto;
    int door;

    buf[0] = '\0';
    fAuto = !str_cmp( argument, "auto" );

    if ( !check_blind( ch ) )
        return;

    strcpy( buf, fAuto ? "[Exits:" : "Obvious exits:\n\r" );

    found = FALSE;
    for ( door = 0; door <= 5; door++ )
    {
        if ( ( pexit = ch->in_room->exit[door] ) != NULL
             && pexit->u1.to_room != NULL
             && can_see_room( ch, pexit->u1.to_room )
             && !IS_SET( pexit->exit_info, EX_CLOSED )
             && !IS_SET( pexit->exit_info, EX_HIDDEN ) )
        {
            found = TRUE;
            if ( fAuto )
            {
                strcat( buf, " " );
                strcat( buf, dir_name[door] );
            }
            else
            {
                sprintf( buf + strlen( buf ), "%-5s - %s\n\r",
                         capitalize( dir_name[door] ),
                         room_is_dark( pexit->u1.to_room )
                         ? "Too dark to tell" : pexit->u1.to_room->name );
            }
        }
    }

    if ( !found )
        strcat( buf, fAuto ? " none" : "None.\n\r" );

    if ( fAuto )
        strcat( buf, "]\n\r" );

    send_to_char( buf, ch );
    return;
}

void do_worth( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
    {
        sprintf( buf, "You have %ld gold.\n\r", ch->gold );
        send_to_char( buf, ch );
        return;
    }

    sprintf( buf,
             "You have %ld gold, %ld gold in the bank, and %ld experience (%ld exp to level).\n\r",
             ch->gold, ch->pcdata->gold_bank, ch->exp,
             ( exp_per_level( ch, ch->pcdata->points ) - ch->exp ) );
    send_to_char( buf, ch );

    return;
}

char *statdiff( int normal, int modified )
{
    static char tempstr[10];

    strcpy( tempstr, "\0" );
    if ( normal < modified )
        sprintf( tempstr, "+%d", modified - normal );
    else if ( normal > modified )
        sprintf( tempstr, "-%d", normal - modified );
    else if ( normal == modified )
        sprintf( tempstr, "  " );
    return ( tempstr );
}

void do_score( CHAR_DATA * ch, char *argument )
{

    char buf[MAX_STRING_LENGTH];

    char tempbuf[200];
    AFFECT_DATA *paf;
    NEWAFFECT_DATA *npaf;
    unsigned int i, x;

    if ( !IS_NPC( ch ) )
    {
        sprintf( buf,
                 "\n\r      `y/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\\n\r" );
        send_to_char( buf, ch );
        sprintf( buf, "     |   `W%s%s", ch->name, ch->pcdata->title );
        send_to_char( buf, ch );
        strcpy( buf, "\0" );
        for ( x = 0; x < 47 - strlen( ch->name ) - str_len( ch->pcdata->title );
              x++ )
            strcat( buf, " " );
        send_to_char( buf, ch );
        sprintf( buf, "%3d years old  `y|____|\n\r", get_age( ch ) );
        send_to_char( buf, ch );
        sprintf( buf,
                 "     |+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+|\n\r" );
        send_to_char( buf, ch );
        sprintf( buf, "     | `YSTR:     `G%2d `W%s `y| `YRace: `G%s",
                 ch->perm_stat[STAT_STR], statdiff( ch->perm_stat[STAT_STR],
                                                    get_curr_stat( ch,
                                                                   STAT_STR ) ),
                 race_table[ch->race].name );
        send_to_char( buf, ch );
        strcpy( buf, "\0" );
        for ( x = 0; x < 41 - strlen( race_table[ch->race].name ); x++ )
            strcat( buf, " " );
        send_to_char( buf, ch );
        sprintf( buf, "`y|\n\r     | `YINT:     `G%2d `W%s `y| `YClass: `G%s",
                 ch->perm_stat[STAT_INT], statdiff( ch->perm_stat[STAT_INT],
                                                    get_curr_stat( ch,
                                                                   STAT_INT ) ),
                 class_table[ch->Class].name );
        send_to_char( buf, ch );
        strcpy( buf, "\0" );
        for ( x = 0; x < 40 - strlen( class_table[ch->Class].name ); x++ )
            strcat( buf, " " );
        send_to_char( buf, ch );
        sprintf( buf,
                 "`y|\n\r     | `YWIS:     `G%2d `W%s `y| `YLevel: `G%2d                                      `y|\n\r",
                 ch->perm_stat[STAT_WIS], statdiff( ch->perm_stat[STAT_WIS],
                                                    get_curr_stat( ch,
                                                                   STAT_WIS ) ),
                 ch->level );
        send_to_char( buf, ch );
        sprintf( buf, "     | `YDEX:     `G%2d `W%s `y| `YAlignment: `G%5d `W[",
                 ch->perm_stat[STAT_DEX], statdiff( ch->perm_stat[STAT_DEX],
                                                    get_curr_stat( ch,
                                                                   STAT_DEX ) ),
                 ch->alignment );
        send_to_char( buf, ch );
        if ( ch->alignment > 900 )
            sprintf( buf, "Angelic]                     `y|\n\r" );
        else if ( ch->alignment > 700 )
            sprintf( buf, "Saintly]                     `y|\n\r" );
        else if ( ch->alignment > 350 )
            sprintf( buf, "Good]                        `y|\n\r" );
        else if ( ch->alignment > 100 )
            sprintf( buf, "Kind]                        `y|\n\r" );
        else if ( ch->alignment > -100 )
            sprintf( buf, "Neutral]                     `y|\n\r" );
        else if ( ch->alignment > -350 )
            sprintf( buf, "Mean]                        `y|\n\r" );
        else if ( ch->alignment > -700 )
            sprintf( buf, "Evil]                        `y|\n\r" );
        else if ( ch->alignment > -900 )
            sprintf( buf, "Demonic]                     `y|\n\r" );
        else
            sprintf( buf, "Satanic]                     `y|\n\r" );
        send_to_char( buf, ch );
        sprintf( buf,
                 "     | `YCON:     `G%2d `W%s `y| `YSex: `G%s          `YCreation Points : `G%2d%s     `y|\n\r",
                 ch->perm_stat[STAT_CON], statdiff( ch->perm_stat[STAT_CON],
                                                    get_curr_stat( ch,
                                                                   STAT_CON ) ),
                 ch->sex == 0 ? "Sexless" : ch->sex == 1 ? "Male  " : "female",
                 ch->pcdata->points, ch->pcdata->points >= 100 ? "" : " " );
        send_to_char( buf, ch );
        sprintf( buf,
                 "     |+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+|\n\r" );
        send_to_char( buf, ch );
        sprintf( buf, "     | `YItems Carried   " );
        send_to_char( buf, ch );
        sprintf( tempbuf, "`G%d`y/`G%d", ch->carry_number, can_carry_n( ch ) );
        send_to_char( tempbuf, ch );
        strcpy( buf, "\0" );
        for ( x = 0; x < 19 - strlen( tempbuf ) + 6; x++ )
            strcat( buf, " " );
        send_to_char( buf, ch );
        strcpy( buf, "\0" );
        sprintf( buf, "`YArmor vs magic  : `G%4d       `y|\n\r",
                 GET_AC( ch, AC_EXOTIC ) );
        send_to_char( buf, ch );
        sprintf( buf, "     | `YWeight Carried  " );
        send_to_char( buf, ch );
        sprintf( tempbuf, "`G%d`y/`G%d", ch->carry_weight, can_carry_w( ch ) );
        send_to_char( tempbuf, ch );
        strcpy( buf, "\0" );
        for ( x = 0; x < 19 - strlen( tempbuf ) + 6; x++ )
            strcat( buf, " " );
        send_to_char( buf, ch );
        strcpy( buf, "\0" );
        sprintf( buf, "`YArmor vs bash   : `G%4d       `y|\n\r",
                 GET_AC( ch, AC_BASH ) );
        send_to_char( buf, ch );
        sprintf( buf, "     | `YGold            " );
        send_to_char( buf, ch );
        sprintf( tempbuf, "`G%ld", ch->gold );
        send_to_char( tempbuf, ch );
        strcpy( buf, "\0" );
        for ( x = 0; x < 19 - strlen( tempbuf ) + 2; x++ )
            strcat( buf, " " );
        send_to_char( buf, ch );
        strcpy( buf, "\0" );
        sprintf( buf, "`YArmor vs pierce : `G%4d       `y|\n\r",
                 GET_AC( ch, AC_PIERCE ) );
        send_to_char( buf, ch );
        sprintf( buf,
                 "     |                                    `YArmor vs slash  : `G%4d       `y|\n\r",
                 GET_AC( ch, AC_SLASH ) );
        send_to_char( buf, ch );
        sprintf( buf, "     | `YCurrent XP       " );
        send_to_char( buf, ch );
        sprintf( tempbuf, "`G%ld", ch->exp );
        send_to_char( tempbuf, ch );
        strcpy( buf, "\0" );
        for ( x = 0; x < 47 - strlen( tempbuf ) + 2; x++ )
            strcat( buf, " " );
        send_to_char( buf, ch );
        strcpy( buf, "\0" );
        sprintf( buf, "`y|\n\r     | `YXP to level      " );
        send_to_char( buf, ch );
        sprintf( tempbuf, "`G%ld (%d%% of normal for your level)",
                 exp_per_level( ch, ch->pcdata->points ) - ch->exp,
                 figure_difference( ch->pcdata->points ) );
        send_to_char( tempbuf, ch );
        strcpy( buf, "\0" );
        for ( x = 0; x < 47 - strlen( tempbuf ) + 2; x++ )
            strcat( buf, " " );
        send_to_char( buf, ch );
        strcpy( buf, "\0" );
        sprintf( buf,
                 "`y|\n\r     |                                     `YHitP: `G%5d `y/ `G%5d         `y|\n\r",
                 ch->hit, ch->max_hit );
        send_to_char( buf, ch );
        sprintf( buf, "     | `YBonus to Hit: `W+" );
        send_to_char( buf, ch );
        sprintf( tempbuf, "%d", GET_HITROLL( ch ) );
        send_to_char( tempbuf, ch );
        strcpy( buf, "\0" );
        for ( x = 0; x < 21 - strlen( tempbuf ); x++ )
            strcat( buf, " " );
        send_to_char( buf, ch );
        strcpy( buf, "\0" );
        sprintf( buf, "`YMana: `G%5d `y/ `G%5d         `y|\n\r", ch->mana,
                 ch->max_mana );
        send_to_char( buf, ch );
        sprintf( buf, "     | `YBonus to Dam: `W+" );
        send_to_char( buf, ch );
        sprintf( tempbuf, "%d", GET_DAMROLL( ch ) );
        send_to_char( tempbuf, ch );
        strcpy( buf, "\0" );
        for ( x = 0; x < 21 - strlen( tempbuf ); x++ )
            strcat( buf, " " );
        send_to_char( buf, ch );
        sprintf( buf, "`YMove: `G%5d `y/ `G%5d         `y|\n\r", ch->move,
                 ch->max_move );
        send_to_char( buf, ch );
        sprintf( buf,
                 "  /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |\n\r" );
        send_to_char( buf, ch );
        sprintf( buf,
                 "  \\________________________________________________________________\\__/`w\n\r" );
        send_to_char( buf, ch );
        if ( ch->pcdata->clan > 0 )
        {
            CLAN_DATA *clan;
            clan = get_clan( ch->pcdata->clan );
            if ( clan != NULL )
            {
                sprintf( buf, "Your rank is %s in clan %s.\n\r",
                         clan->rank[ch->pcdata->clan_rank - 1], clan->name );
                send_to_char( buf, ch );
            }
        }

    }
    else
    {
        sprintf( buf,
                 "You are %s%s, level %d, %d years old (%d hours).\n\r",
                 ch->name,
                 IS_NPC( ch ) ? "" : ch->pcdata->title,
                 ch->level, get_age( ch ),
                 ( ch->played + ( int ) ( current_time - ch->logon ) ) / 3600 );
        send_to_char( buf, ch );

        if ( get_trust( ch ) != ch->level )
        {
            sprintf( buf, "You are trusted at level %d.\n\r", get_trust( ch ) );
            send_to_char( buf, ch );
        }

        sprintf( buf, "Race: %s  Sex: %s  Class:  %s\n\r",
                 race_table[ch->race].name,
                 ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
                 IS_NPC( ch ) ? "mobile" : class_table[ch->Class].name );
        send_to_char( buf, ch );

        sprintf( buf,
                 "You have %d/%d hit, %d/%d mana, %d/%d movement.\n\r",
                 ch->hit, ch->max_hit,
                 ch->mana, ch->max_mana, ch->move, ch->max_move );
        send_to_char( buf, ch );

        sprintf( buf,
                 "You have %d practices and %d training sessions.\n\r",
                 ch->practice, ch->train );
        send_to_char( buf, ch );

        sprintf( buf,
                 "You are carrying %d/%d items with weight %d/%d pounds.\n\r",
                 ch->carry_number, can_carry_n( ch ),
                 ch->carry_weight, can_carry_w( ch ) );
        send_to_char( buf, ch );

        sprintf( buf,
                 "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
                 ch->perm_stat[STAT_STR],
                 get_curr_stat( ch, STAT_STR ),
                 ch->perm_stat[STAT_INT],
                 get_curr_stat( ch, STAT_INT ),
                 ch->perm_stat[STAT_WIS],
                 get_curr_stat( ch, STAT_WIS ),
                 ch->perm_stat[STAT_DEX],
                 get_curr_stat( ch, STAT_DEX ),
                 ch->perm_stat[STAT_CON], get_curr_stat( ch, STAT_CON ) );
        send_to_char( buf, ch );

        sprintf( buf,
                 "You have scored %ld exp, and have %ld gold coins.\n\r",
                 ch->exp, ch->gold );
        send_to_char( buf, ch );

        /* RT shows exp to level */
        if ( !IS_NPC( ch ) && ch->level < LEVEL_HERO )
        {
            sprintf( buf,
                     "You need %ld exp to level.\n\r",
                     ( ( ch->level + 1 ) * exp_per_level( ch,
                                                          ch->pcdata->points ) -
                       ch->exp ) );
            send_to_char( buf, ch );
        }

        sprintf( buf, "Wimpy set to %d hit points.\n\r", ch->wimpy );
        send_to_char( buf, ch );

        if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
            send_to_char( "You are drunk.\n\r", ch );
        if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_THIRST] == 0 )
            send_to_char( "You are thirsty.\n\r", ch );
        if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_FULL] == 0 )
            send_to_char( "You are hungry.\n\r", ch );

        switch ( ch->position )
        {
        case POS_DEAD:
            send_to_char( "You are DEAD!!\n\r", ch );
            break;
        case POS_MORTAL:
            send_to_char( "You are mortally wounded.\n\r", ch );
            break;
        case POS_INCAP:
            send_to_char( "You are incapacitated.\n\r", ch );
            break;
        case POS_STUNNED:
            send_to_char( "You are stunned.\n\r", ch );
            break;
        case POS_SLEEPING:
            send_to_char( "You are sleeping.\n\r", ch );
            break;
        case POS_RESTING:
            send_to_char( "You are resting.\n\r", ch );
            break;
        case POS_STANDING:
            send_to_char( "You are standing.\n\r", ch );
            break;
        case POS_FIGHTING:
            send_to_char( "You are fighting.\n\r", ch );
            break;
        }

        /* print AC values */
        if ( ch->level >= 25 )
        {
            sprintf( buf,
                     "Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
                     GET_AC( ch, AC_PIERCE ), GET_AC( ch, AC_BASH ), GET_AC( ch,
                                                                             AC_SLASH ),
                     GET_AC( ch, AC_EXOTIC ) );
            send_to_char( buf, ch );
        }

        for ( i = 0; i < 4; i++ )
        {
            char *temp;

            switch ( i )
            {
            case ( AC_PIERCE ):
                temp = "piercing";
                break;
            case ( AC_BASH ):
                temp = "bashing";
                break;
            case ( AC_SLASH ):
                temp = "slashing";
                break;
            case ( AC_EXOTIC ):
                temp = "magic";
                break;
            default:
                temp = "error";
                break;
            }

            send_to_char( "You are ", ch );

            if ( GET_AC( ch, i ) >= 101 )
                sprintf( buf, "hopelessly vulnerable to %s.\n\r", temp );
            else if ( GET_AC( ch, i ) >= 80 )
                sprintf( buf, "defenseless against %s.\n\r", temp );
            else if ( GET_AC( ch, i ) >= 60 )
                sprintf( buf, "barely protected from %s.\n\r", temp );
            else if ( GET_AC( ch, i ) >= 40 )
                sprintf( buf, "slighty armored against %s.\n\r", temp );
            else if ( GET_AC( ch, i ) >= 20 )
                sprintf( buf, "somewhat armored against %s.\n\r", temp );
            else if ( GET_AC( ch, i ) >= 0 )
                sprintf( buf, "armored against %s.\n\r", temp );
            else if ( GET_AC( ch, i ) >= -20 )
                sprintf( buf, "well-armored against %s.\n\r", temp );
            else if ( GET_AC( ch, i ) >= -40 )
                sprintf( buf, "very well-armored against %s.\n\r", temp );
            else if ( GET_AC( ch, i ) >= -60 )
                sprintf( buf, "heavily armored against %s.\n\r", temp );
            else if ( GET_AC( ch, i ) >= -80 )
                sprintf( buf, "superbly armored against %s.\n\r", temp );
            else if ( GET_AC( ch, i ) >= -100 )
                sprintf( buf, "almost invulnerable to %s.\n\r", temp );
            else
                sprintf( buf, "divinely armored against %s.\n\r", temp );

        }

        /* RT wizinvis and holy light */
        if ( IS_IMMORTAL( ch ) )
        {
            send_to_char( "Holy Light: ", ch );
            if ( IS_SET( ch->act, PLR_HOLYLIGHT ) )
                send_to_char( "on", ch );
            else
                send_to_char( "off", ch );

            if ( IS_SET( ch->act, PLR_WIZINVIS ) )
            {
                sprintf( buf, "  Invisible: level %d", ch->invis_level );
                send_to_char( buf, ch );
            }
            send_to_char( "\n\r", ch );
        }

        if ( ch->level >= 15 )
        {
            sprintf( buf, "Hitroll: %d  Damroll: %d.\n\r",
                     GET_HITROLL( ch ), GET_DAMROLL( ch ) );
            send_to_char( buf, ch );
        }

        if ( ch->level >= 10 )
        {
            sprintf( buf, "Alignment: %d.  ", ch->alignment );
            send_to_char( buf, ch );
        }

        send_to_char( "You are ", ch );
        if ( ch->alignment > 900 )
            send_to_char( "angelic.\n\r", ch );
        else if ( ch->alignment > 700 )
            send_to_char( "saintly.\n\r", ch );
        else if ( ch->alignment > 350 )
            send_to_char( "good.\n\r", ch );
        else if ( ch->alignment > 100 )
            send_to_char( "kind.\n\r", ch );
        else if ( ch->alignment > -100 )
            send_to_char( "neutral.\n\r", ch );
        else if ( ch->alignment > -350 )
            send_to_char( "mean.\n\r", ch );
        else if ( ch->alignment > -700 )
            send_to_char( "evil.\n\r", ch );
        else if ( ch->alignment > -900 )
            send_to_char( "demonic.\n\r", ch );
        else
            send_to_char( "satanic.\n\r", ch );

        if ( ch->affected != NULL )
        {
            send_to_char( "You are affected by:\n\r", ch );
            for ( paf = ch->affected; paf != NULL; paf = paf->next )
            {
                sprintf( buf, "Spell: '%s'", skill_table[paf->type].name );
                send_to_char( buf, ch );

                if ( ch->level >= 20 )
                {
                    sprintf( buf, " modifies %s by %d for %d hours",
                             affect_loc_name( paf->location ),
                             paf->modifier, paf->duration );
                    send_to_char( buf, ch );
                }

                send_to_char( ".\n\r", ch );
            }
            for ( npaf = ch->newaffected; npaf != NULL; npaf = npaf->next )
            {
                sprintf( buf, "Spell: '%s'", skill_table[npaf->type].name );
                send_to_char( buf, ch );

                if ( ch->level >= 20 )
                {
                    sprintf( buf, " modifies %s by %d for %d hours",
                             affect_loc_name( npaf->location ),
                             npaf->modifier, npaf->duration );
                    send_to_char( buf, ch );
                }

                send_to_char( ".\n\r", ch );
            }
        }
    }
    return;
}

char *const day_name[] = {
    "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
    "the Great Gods", "the Sun"
};

char *const month_name[] = {
    "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
    "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
    "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
    "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time( CHAR_DATA * ch, char *argument )
{

    char buf[MAX_STRING_LENGTH];

    extern char str_boot_time[];
    char *suf;
    int day;

    day = time_info.day + 1;

    if ( day > 4 && day < 20 )
        suf = "th";
    else if ( day % 10 == 1 )
        suf = "st";
    else if ( day % 10 == 2 )
        suf = "nd";
    else if ( day % 10 == 3 )
        suf = "rd";
    else
        suf = "th";

    sprintf( buf,
             "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\rStarted up (or hotbooted) at %s\n\rThe system time is %s\r",
             ( time_info.hour % 12 == 0 ) ? 12 : time_info.hour % 12,
             time_info.hour >= 12 ? "pm" : "am",
             day_name[day % 7],
             day, suf,
             month_name[time_info.month], str_boot_time, get_curtime(  ) );

    send_to_char( buf, ch );
    return;
}

void do_weather( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    static char *const sky_look[4] = {
        "cloudless",
        "cloudy",
        "rainy",
        "lit by flashes of lightning"
    };

    if ( !IS_OUTSIDE( ch ) )
    {
        send_to_char( "You can't see the weather indoors.\n\r", ch );
        return;
    }

    sprintf( buf, "The sky is %s and %s.\n\r",
             sky_look[weather_info.sky],
             weather_info.change >= 0
             ? "a warm southerly breeze blows" : "a cold northern gust blows" );
    send_to_char( buf, ch );
    return;
}

void do_version( CHAR_DATA * ch )
{

    printf_to_char( ch, "%s\n\r", EMBER_MUD_VERSION );
    return;
}

void do_help( CHAR_DATA * ch, char *argument )
{
    HELP_DATA *pHelp;
    char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];
    char nohelp[MAX_STRING_LENGTH];

    strcpy( nohelp, argument );

    if ( argument[0] == '\0' )
        argument = "summary";

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while ( argument[0] != '\0' )
    {
        argument = one_argument( argument, argone );
        if ( argall[0] != '\0' )
            strcat( argall, " " );
        strcat( argall, argone );
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
    {
        if ( pHelp->level > get_trust( ch ) )
            continue;

        if ( is_name( argall, pHelp->keyword ) )
        {
            if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) )
            {
                send_to_char( pHelp->keyword, ch );
                send_to_char( "\n\r", ch );
            }

            /*
             * Strip leading '.' to allow initial blanks.
             */
            if ( pHelp->text[0] == '.' )
                page_to_char( pHelp->text + 1, ch );
            else
                page_to_char( pHelp->text, ch );
            return;
        }
    }
    {
        send_to_char( "No help on that word.\n\r", ch );
        append_file( ch, sysconfig.help_log_file, nohelp );
    }
    return;
}

void do_todo( CHAR_DATA * ch, char *argument )
{
    bool found = FALSE;

    TODO_DATA *pTodo;
    char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];

    if ( argument[0] == '\0' )
    {
        printf_to_char( ch,
                        "`KSyntax: `Wtodo all [show]\n\r`KSyntax: `Wtodo <keyword>`w" );
        return;
    }

    /* this parts handles todo a b so that it returns todo 'a b' */
    argall[0] = '\0';
    while ( argument[0] != '\0' )
    {
        argument = one_argument( argument, argone );
        if ( argall[0] != '\0' )
            strcat( argall, " " );
        strcat( argall, argone );
    }
    if ( !strcmp( argall, "all show" ) || !strcmp( argall, "all" ) )
    {
        found = TRUE;
        for ( pTodo = todo_first; pTodo != NULL; pTodo = pTodo->next )
        {
            if ( pTodo->level > get_trust( ch ) )
                continue;
            if ( !strcmp( argall, "all show" ) )
            {

                printf_to_char( ch, "`R" );
                send_to_char( pTodo->keyword, ch );
                send_to_char( "`Y\n", ch );

                /*
                 * Strip leading '.' to allow initial blanks.
                 */
                if ( pTodo->text[0] == '.' )
                {
                    page_to_char( pTodo->text + 1, ch );
                    printf_to_char( ch,
                                    "`C  ---------------------------------------------------------------------------\n" );
                }
                else
                {
                    page_to_char( pTodo->text, ch );
                    printf_to_char( ch,
                                    "`C  ---------------------------------------------------------------------------\n" );

                }

            }

            if ( !strcmp( argall, "all" ) )
            {

                printf_to_char( ch, "`R" );
                send_to_char( pTodo->keyword, ch );
                send_to_char( "`Y\n\r", ch );

            }

        }
        printf_to_char( ch, "End of Task list.\n\r" );
    }

    /* Begin */
    for ( pTodo = todo_first; pTodo != NULL; pTodo = pTodo->next )
    {

        if ( is_name( argall, pTodo->keyword ) )
        {
            found = TRUE;
            printf_to_char( ch, "`R" );
            send_to_char( pTodo->keyword, ch );
            send_to_char( "\n\r`Y", ch );

            /*
             * Strip leading '.' to allow initial blanks.
             */
            if ( pTodo->text[0] == '.' )
                page_to_char( pTodo->text + 1, ch );
            else
                page_to_char( pTodo->text, ch );
            return;
        }

    }
    if ( found == FALSE )
    {
        printf_to_char( ch, "There is no task with that name" );
    }
}

/* Stuff for clans in the who listing.  */

char *who_clan( CHAR_DATA * ch, CHAR_DATA * looker, char *empty )
{
    CLAN_DATA *clan;

    clan = get_clan( ch->pcdata->clan );
    if ( clan == NULL )
    {
        bug( "NULL nonzero clan %d in who_clan.", ch->pcdata->clan );
        return empty;
    }

    if ( IS_NPC( looker ) )
    {
        if ( !IS_SET( clan->clan_flags, CLAN_PRIVATE )
             && !IS_SET( clan->clan_flags, CLAN_SECRET ) )
        {
            return clan->whoname;
        }
        return empty;
    }

    if ( !IS_SET( clan->clan_flags, CLAN_PRIVATE )
         && !IS_SET( clan->clan_flags, CLAN_SECRET ) )
    {
        return clan->whoname;
    }

    if ( IS_SET( clan->clan_flags, CLAN_PRIVATE )
         && ch->pcdata->clan == looker->pcdata->clan )
    {
        return clan->whoname;
    }
/* in secret clans, only rank 1 and 2 members can see other clan members.
   the rest have to use clantalk to learn wh their clanmates are.  */
    if ( IS_SET( clan->clan_flags, CLAN_SECRET ) &&
         ( ch->pcdata->clan_rank == 1 || ch->pcdata->clan_rank == 2
           || !str_cmp( ch->name, clan->god ) )
         && ch->pcdata->clan == looker->pcdata->clan )
    {
        return clan->whoname;
    }

    if ( looker->level >= LEVEL_ADMIN )
    {
        return clan->whoname;
    }

    return empty;

}
char *pre_clan( CHAR_DATA * ch, CHAR_DATA * looker, char *empty, char *private,
                char *secret )
{
    CLAN_DATA *clan;

    clan = get_clan( ch->pcdata->clan );
    if ( clan == NULL )
    {
        bug( "NULL nonzero clan %d in who_clan.", ch->pcdata->clan );
        return empty;
    }

    if ( looker->level < LEVEL_ADMIN )
        return empty;

    if ( IS_SET( clan->clan_flags, CLAN_PRIVATE ) )
        return private;

    if ( IS_SET( clan->clan_flags, CLAN_SECRET ) )
        return secret;

    return empty;
}

/* old whois command */
void do_whoname( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char empty[] = "";
    char private[] = "`m(P)`w";
    char secret[] = "`r(S)`w";
    char output[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool found = FALSE;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "You must provide a name.\n\r", ch );
        return;
    }

    output[0] = '\0';

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *wch;
        char const *Class;

        if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
            continue;

        wch = ( d->original != NULL ) ? d->original : d->character;

        if ( !can_see( ch, wch ) )
            continue;

        if ( !str_prefix( arg, wch->name ) )
        {
            found = TRUE;

            /* work out the printing */
            Class = class_table[wch->Class].who_name;
            switch ( wch->level )
            {
            case MAX_LEVEL + 2:
                Class = "IMP";
                break;
            case MAX_LEVEL + 1:
                Class = "IMP";
                break;
            case MAX_LEVEL - 0:
                Class = "IMP";
                break;
            case MAX_LEVEL - 1:
                Class = "CRE";
                break;
            case MAX_LEVEL - 2:
                Class = "SUP";
                break;
            case MAX_LEVEL - 3:
                Class = "DEI";
                break;
            case MAX_LEVEL - 4:
                Class = "GOD";
                break;
            case MAX_LEVEL - 5:
                Class = "IMM";
                break;
            case MAX_LEVEL - 6:
                Class = "DEM";
                break;
            case MAX_LEVEL - 7:
                Class = "ANG";
                break;
            case MAX_LEVEL - 8:
                Class = "AVA";
                break;
            }

            /* a little formatting */
/* Yes, I know this new version is messy.  Really messy in fact.
   Clean it up if you like.  -Kyle */
            sprintf( buf, "`K[`W%2d `Y%s `G%s`K] %s%s%s%s%s%s%s`w%s%s%s\n\r", ( wch->level > MAX_LEVEL ? MAX_LEVEL : wch->level ), wch->pcdata != NULL && wch->pcdata->who_race ? wch->pcdata->who_race : wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name : "          ", Class, IS_NPC( wch ) ? "" : ( wch->pcdata->clan == 0 ) ? "" : pre_clan( wch, ch, empty, private, secret ), IS_NPC( wch ) ? "" : ( wch->pcdata->clan == 0 ) ? "" : who_clan( wch, ch, empty ), "", /* <---- if you need to add something, remove this */
                     IS_SET( wch->act, PLR_WIZINVIS ) ? "W" : "",
                     IS_SET( wch->act, PLR_AFK ) ? "A" : "-",
                     IS_SET( wch->act, PLR_KILLER ) ? "`RP`W" : "-",
                     IS_SET( wch->act, PLR_THIEF ) ? "T" : "-",
                     wch->pcdata != NULL &&
                     wch->pcdata->who_prefix ?
                     wch->pcdata->who_prefix : "",
                     IS_NPC( wch ) ?
                     wch->short_descr :
                     wch->name, IS_NPC( wch ) ? "" : wch->pcdata->title );
            strcat( output, buf );
        }
    }

    if ( !found )
    {
        send_to_char( "No one of that name is playing.\n\r", ch );
        return;
    }

    page_to_char( output, ch );
}

void insert_sort( CHAR_DATA * who_list[300], CHAR_DATA * ch, int length )
{
    while ( ( length ) && who_list[length - 1]->level < ch->level )
    {
        who_list[length] = who_list[length - 1];
        length--;
    }
    who_list[length] = ch;
}

void chaos_sort( CHAR_DATA * who_list[300], CHAR_DATA * ch, int length )
{
    while ( ( length )
            && who_list[length - 1]->pcdata->chaos_score <
            ch->pcdata->chaos_score )
    {
        who_list[length] = who_list[length - 1];
        length--;
    }
    who_list[length] = ch;
}

/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */
void do_who( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    char buf2[MAX_STRING_LENGTH];
    char output[4 * MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *who_list[300];
    int iClass;
    int iRace;
    int iLevelLower;
    int iLevelUpper;
    int nNumber;
    int nMatch;
    int length;
    int maxlength;
    int count;
    bool rgfClass[MAX_CLASS];
    bool rgfRace[MAX_PC_RACE];
    bool fClassRestrict;
    bool fRaceRestrict;
    bool fImmortalOnly;
    bool doneimmort = FALSE;
    bool donemort = FALSE;
    char empty[] = "";
    char private[] = "`m(P)`w";
    char secret[] = "`r(S)`w";

    /*
     * Set default arguments.
     */
    iLevelLower = 0;
    iLevelUpper = MAX_LEVEL + 2;
    fClassRestrict = FALSE;
    fRaceRestrict = FALSE;
    fImmortalOnly = FALSE;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
        rgfClass[iClass] = FALSE;
    for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
        rgfRace[iRace] = FALSE;

    /*
     * Parse arguments.
     */
    nNumber = 0;
    for ( ;; )
    {
        argument = one_argument( argument, arg );
        if ( arg[0] == '\0' )
            break;

        if ( is_number( arg ) )
        {
            switch ( ++nNumber )
            {
            case 1:
                iLevelLower = atoi( arg );
                break;
            case 2:
                iLevelUpper = atoi( arg );
                break;
            default:
                send_to_char( "Only two level numbers allowed.\n\r", ch );
                return;
            }
        }
        else
        {

            /*
             * Look for classes to turn on.
             */
            if ( !str_cmp( argument, "imm" ) )
            {
                fImmortalOnly = TRUE;
            }
            else
            {
                iClass = class_lookup( arg );
                if ( iClass == -1 )
                {
                    iRace = race_lookup( arg );

                    if ( iRace == 0 || iRace >= MAX_PC_RACE )
                    {
                        do_whoname( ch, arg );
                        return;
                    }
                    else
                    {
                        fRaceRestrict = TRUE;
                        rgfRace[iRace] = TRUE;
                    }
                }
                else
                {
                    fClassRestrict = TRUE;
                    rgfClass[iClass] = TRUE;
                }
            }
        }
    }

    length = 0;
    for ( d = descriptor_list; d; d = d->next )
    {
        if ( ( d->connected == CON_PLAYING ) || ( d->connected == CON_NOTE_TO )
             || ( d->connected == CON_NOTE_SUBJECT )
             || ( d->connected == CON_NOTE_EXPIRE )
             || ( d->connected == CON_NOTE_TEXT )
             || ( d->connected == CON_NOTE_FINISH ) )
        {
            /* If descriptor belongs to a switched imm... */
            if ( d->original != NULL )
            {
                /* Put the imm in the who_list, not the mob */
                insert_sort( who_list, d->original, length );
            }
            else
            {
                insert_sort( who_list, d->character, length );
            }

            length++;
        }
    }

    maxlength = length;

    /*
     * Now show matching chars.
     */
    nMatch = 0;
    buf[0] = '\0';
    output[0] = '\0';
    for ( length = 0; length < maxlength; length++ )
    {
        char const *Class;

        /*
         * Check for match against restrictions.
         * Don't use trust as that exposes trusted mortals.
         */
        if ( who_list[length]->level > MAX_LEVEL - 10
             && can_see( ch, who_list[length] ) && doneimmort == FALSE )
        {
            sprintf( buf, "`K[`RVisible Immortals`K]\n\r\n\r" );
            doneimmort = TRUE;
            strcat( output, buf );
        }
        else if ( ( who_list[length]->level <= MAX_LEVEL - 10 )
                  && donemort == FALSE )
        {
            if ( doneimmort == TRUE )
            {
                sprintf( buf, "\n\r" );
                strcat( output, buf );
            }
            sprintf( buf, "`K[`RVisible Mortals`K]\n\r\n\r" );
            donemort = TRUE;
            strcat( output, buf );
        }
        if (                    /*who_list[length]->desc->connected != CON_PLAYING || */
                !can_see( ch, who_list[length] ) )
            continue;

        if ( who_list[length]->level < iLevelLower
             || who_list[length]->level > iLevelUpper
             || ( fImmortalOnly && who_list[length]->level < LEVEL_HERO )
             || ( fClassRestrict && !rgfClass[who_list[length]->Class] )
             || ( fRaceRestrict && !rgfRace[who_list[length]->race] ) )
            continue;

        nMatch++;

        /*
         * Figure out what to print for class.
         */

        Class = class_table[who_list[length]->Class].who_name;
        switch ( who_list[length]->level )
        {
        default:
            break;
            {
        case MAX_LEVEL + 2:
                Class = "IMP";
                break;
        case MAX_LEVEL + 1:
                Class = "IMP";
                break;
        case MAX_LEVEL - 0:
                Class = "IMP";
                break;
        case MAX_LEVEL - 1:
                Class = "SUP";
                break;
        case MAX_LEVEL - 2:
                Class = "GOD";
                break;
        case MAX_LEVEL - 3:
                Class = "DIV";
                break;
        case MAX_LEVEL - 4:
                Class = "DEI";
                break;
        case MAX_LEVEL - 5:
                Class = "ARC";
                break;
        case MAX_LEVEL - 6:
                Class = "ANG";
                break;
        case MAX_LEVEL - 7:
                Class = "CRE";
                break;
        case MAX_LEVEL - 8:
                Class = "SKR";
                break;
            }
        }

        /*
         * Format it up.
         */

        /* Yes, I know this new version is messy.  Really messy in fact.
           Clean it up if you like.  -Kyle */
        if ( who_list[length]->anonymous )
        {
            sprintf( buf, "`K[    `KA`wN`WO`wN`KY`wM`WO`wU`KS`K     ] %s%s%s `R[`W%s%s%s%s%s`R] `w%s%s%s\n\r", IS_NPC( who_list[length] ) ? "" : ( who_list[length]->pcdata->clan == 0 ) ? "" : pre_clan( who_list[length], ch, empty, private, secret ), IS_NPC( who_list[length] ) ? "" : ( who_list[length]->pcdata->clan == 0 ) ? "" : who_clan( who_list[length], ch, empty ), "", /* <---- if you need to add something, remove this */
                     IS_NPC( who_list[length] ) ? "-" :
                     !is_name( who_list[length]->pcdata->spouse,
                               "(none)" ) ? "M" : "-",
                     IS_SET( who_list[length]->act, PLR_WIZINVIS ) ? "W" : "",
                     IS_SET( who_list[length]->act, PLR_AFK ) ? "A" : "-",
                     IS_SET( who_list[length]->act,
                             PLR_KILLER ) ? "`RP`W" : "-",
                     IS_SET( who_list[length]->act, PLR_THIEF ) ? "`KT" : "-",
                     who_list[length]->pcdata != NULL
                     && who_list[length]->pcdata->
                     who_prefix ? who_list[length]->pcdata->who_prefix : "",
                     IS_NPC( who_list[length] ) ? who_list[length]->
                     short_descr : who_list[length]->name,
                     IS_NPC( who_list[length] ) ? "" : who_list[length]->
                     pcdata->title );
            strcat( output, buf );
        }
        else
        {
            sprintf( buf, "`K[`W%3d `Y%s `G%s`K] %s%s%s`R[`W%s%s%s%s%s`R] `w%s%s%s\n\r", ( who_list[length]->level > MAX_LEVEL ? MAX_LEVEL : who_list[length]->level ), who_list[length]->pcdata != NULL && who_list[length]->pcdata->who_race ? who_list[length]->pcdata->who_race : who_list[length]->race < MAX_PC_RACE ? pc_race_table[who_list[length]->race].who_name : "          ", Class, IS_NPC( who_list[length] ) ? "" : ( who_list[length]->pcdata->clan == 0 ) ? "" : pre_clan( who_list[length], ch, empty, private, secret ), IS_NPC( who_list[length] ) ? "" : ( who_list[length]->pcdata->clan == 0 ) ? "" : who_clan( who_list[length], ch, empty ), "", /* <---- if you need to add something, remove this */
                     IS_NPC( who_list[length] ) ? "-" :
                     !is_name( who_list[length]->pcdata->spouse,
                               "(none)" ) ? "M" : "-",
                     IS_SET( who_list[length]->act, PLR_WIZINVIS ) ? "W" : "",
                     IS_SET( who_list[length]->act, PLR_AFK ) ? "A" : "-",
                     IS_SET( who_list[length]->act,
                             PLR_KILLER ) ? "`RP`W" : "-",
                     IS_SET( who_list[length]->act, PLR_THIEF ) ? "`KT`W" : "-",
                     who_list[length]->pcdata != NULL
                     && who_list[length]->pcdata->
                     who_prefix ? who_list[length]->pcdata->who_prefix : "",
                     IS_NPC( who_list[length] ) ? who_list[length]->
                     short_descr : who_list[length]->name,
                     IS_NPC( who_list[length] ) ? "" : who_list[length]->
                     pcdata->title );
            strcat( output, buf );
        }

    }
    sprintf( buf2, "\n\r`wVisible Players Shown: `W%d\n\r", nMatch );
    strcat( output, buf2 );
    count = 0;
    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->connected == CON_PLAYING
             && !( IS_SET( d->character->act, PLR_WIZINVIS ) ) )
        {
            count++;
        }
        else if ( ( ( d->connected == CON_PLAYING )
                    || ( d->connected == CON_NOTE_TO )
                    || ( d->connected == CON_NOTE_SUBJECT )
                    || ( d->connected == CON_NOTE_EXPIRE )
                    || ( d->connected == CON_NOTE_TEXT )
                    || ( d->connected == CON_NOTE_FINISH ) )
                  && !( d->character->invis_level > ch->level ) )

        {
            count++;
        }

    }

    sprintf( buf2,
             "`wTotal Players Online: `W%d\n\r`wTo see the legend for the [---] format, type help wlegend\n\r",
             count );
    strcat( output, buf2 );
    page_to_char( output, ch );
    return;
}

void do_inventory( CHAR_DATA * ch, char *argument )
{
    send_to_char( "You are carrying:\n\r", ch );
    show_list_to_char( ch->carrying, ch, TRUE, TRUE );
    return;
}

void do_equipment( CHAR_DATA * ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;

    send_to_char( "You are using:\n\r", ch );
    found = FALSE;
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
        if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
            continue;

        printf_to_char( ch, "  %s`0", where_name[iWear] );

        /* TODO: There's gotta be a better way to send a given
           number of spaces to a character... */
        {
            int x = 25 - str_len( where_name[iWear] );
            int y;

            for ( y = 1; y <= x; y++ )
            {
                send_to_char( " ", ch );
            }
        }

        if ( can_see_obj( ch, obj ) )
        {
            send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
            send_to_char( "\n\r`w", ch );
        }
        else
        {
            send_to_char( "Something.\n\r", ch );
        }

        found = TRUE;
    }

    if ( !found )
        send_to_char( "Nothing.\n\r", ch );

    return;
}

void do_compare( CHAR_DATA * ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj1;
    OBJ_DATA *obj2;
    int value1;
    int value2;
    char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
        send_to_char( "Compare what to what?\n\r", ch );
        return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        for ( obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content )
        {
            if ( obj2->wear_loc != WEAR_NONE
                 && can_see_obj( ch, obj2 )
                 && obj1->item_type == obj2->item_type
                 && ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE ) != 0 )
                break;
        }

        if ( obj2 == NULL )
        {
            send_to_char( "You aren't wearing anything comparable.\n\r", ch );
            return;
        }
    }

    else if ( ( obj2 = get_obj_carry( ch, arg2 ) ) == NULL )
    {
        send_to_char( "You do not have that item.\n\r", ch );
        return;
    }

    msg = NULL;
    value1 = 0;
    value2 = 0;

    if ( obj1 == obj2 )
    {
        msg = "You compare $p to itself.  It looks about the same.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
        msg = "You can't compare $p and $P.";
    }
    else
    {
        switch ( obj1->item_type )
        {
        default:
            msg = "You can't compare $p and $P.";
            break;

        case ITEM_ARMOR:
            value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
            value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
            break;

        case ITEM_WEAPON:
            value1 = ( 1 + obj1->value[2] ) * obj1->value[1];
            value2 = ( 1 + obj2->value[2] ) * obj2->value[1];
            break;
        }
    }

    if ( msg == NULL )
    {
        if ( value1 == value2 )
            msg = "$p and $P look about the same.";
        else if ( value1 > value2 )
            msg = "$p looks better than $P.";
        else
            msg = "$p looks worse than $P.";
    }

    act( msg, ch, obj1, obj2, TO_CHAR );
    return;
}

void do_credits( CHAR_DATA * ch, char *argument )
{
    do_help( ch, "diku" );
    return;
}

void do_where( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    bool found;
    extern bool chaos;

    one_argument( argument, arg );

    if ( ( chaos ) && ( ch->level < HERO ) )
    {
        send_to_char( "Where? Your killer is right behind you!\n\r", ch );
        return;
    }

    if ( arg[0] == '\0' && ( ch->level > HERO ) )
    {
        send_to_char( "Current players:\n\r", ch );
        found = FALSE;
        for ( d = descriptor_list; d; d = d->next )
        {
            if ( d->connected == CON_PLAYING
                 && ( victim = d->character ) != NULL
                 && !IS_NPC( victim )
                 && victim->in_room != NULL && can_see( ch, victim ) )
            {
                found = TRUE;
                sprintf( buf, "%-28s [%5d] %s`w\n\r",
                         victim->name, victim->in_room->vnum,
                         victim->in_room->name );
                send_to_char( buf, ch );
            }
        }
        if ( !found )
            send_to_char( "None\n\r", ch );
    }
    else if ( ch->level > HERO )
    {
        found = FALSE;
        for ( victim = char_list; victim != NULL; victim = victim->next )
        {
            if ( victim->in_room != NULL
                 && can_see( ch, victim ) && is_name( arg, victim->name ) )
            {
                found = TRUE;
                sprintf( buf, "%-28s %s`w\n\r",
                         PERS( victim, ch ), victim->in_room->name );
                send_to_char( buf, ch );
                break;
            }
        }
        if ( !found )
            act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }
    else if ( arg[0] == '\0' && ( ch->level <= HERO ) )
    {
        found = FALSE;
        send_to_char( "Players near you:\n\r", ch );
        for ( d = descriptor_list; d; d = d->next )

        {
            if ( d->connected == CON_PLAYING
                 && ( victim = d->character ) != NULL
                 && !IS_NPC( victim )
                 && victim->in_room != NULL
                 && victim->in_room->area == ch->in_room->area
                 && can_see( ch, victim ) )
            {
                found = TRUE;
                sprintf( buf, "%-28s %s`w\n\r",
                         victim->name, victim->in_room->name );
                send_to_char( buf, ch );
            }
        }
        if ( !found )
            send_to_char( "None\n\r", ch );
    }
    else
    {
        found = FALSE;
        for ( victim = char_list; victim != NULL; victim = victim->next )
        {
            if ( victim->in_room != NULL
                 && victim->in_room->area == ch->in_room->area
                 && !IS_AFFECTED( victim, AFF_HIDE )
                 && !IS_AFFECTED( victim, AFF_SNEAK )
                 && can_see( ch, victim ) && is_name( arg, victim->name ) )
            {
                found = TRUE;
                sprintf( buf, "%-28s %s`w\n\r",
                         PERS( victim, ch ), victim->in_room->name );
                send_to_char( buf, ch );
                break;
            }
        }
        if ( !found )
            act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
    }

    return;
}

void do_consider( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    CHAR_DATA *victim;
    char *msg;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Consider killing whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They're not here.\n\r", ch );
        return;
    }

    if ( is_safe( ch, victim ) )
    {
        send_to_char( "Don't even think about it.\n\r", ch );
        return;
    }

    diff = ( victim->max_hit / ch->max_hit ) * 100;

    if ( diff <= 50 )
        msg = "Comparing HP: $N is sickly and ill next to you.";
    else if ( diff <= 65 )
        msg = "Comparing HP: $N almost makes you want to laugh.";
    else if ( diff <= 85 )
        msg = "Comparing HP: $N isn't quite up to your level.";
    else if ( diff <= 115 )
        msg = "Comparing HP: You're about equal.";
    else if ( diff <= 125 )
        msg = "Comparing HP: $N's just a bit tougher than you.";
    else if ( diff <= 140 )
        msg =
            "Comparing HP: Maybe you should consider attacking something else.";
    else
        msg = "Comparing HP: $N puts you to shame.";
    act( msg, ch, NULL, victim, TO_CHAR );

    diff = victim->level - ch->level;

    if ( diff <= -20 )
        msg = CON_MSG1;
    else if ( diff <= -10 )
        msg = CON_MSG2;
    else if ( diff <= -5 )
        msg = CON_MSG3;
    else if ( diff <= 1 )
        msg = CON_MSG4;
    else if ( diff <= 5 )
        msg = CON_MSG5;
    else if ( diff <= 10 )
        msg = CON_MSG6;
    else
        msg = CON_MSG7;
    act( msg, ch, NULL, victim, TO_CHAR );

    consider_factions( ch, victim, TRUE );

    return;
}

void set_title( CHAR_DATA * ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
    {
        bug( "Set_title: NPC.", 0 );
        return;
    }

    if ( title[0] != '.' && title[0] != ',' && title[0] != '!'
         && title[0] != '?' )
    {
        buf[0] = ' ';
        strcpy( buf + 1, title );
    }
    else
    {
        strcpy( buf, title );
    }

    strcat( buf, "`w" );

    free_string( &ch->pcdata->title );
    ch->pcdata->title = str_dup( buf );
    return;
}

/* new do_title... allows longer title for shorter named characters AND doesn't count
color codes anymore.  -Kyle */
void do_title( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
        return;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Change your title to what?\n\r", ch );
        return;
    }

    if ( str_len( argument ) + str_len( ch->name ) > 45 )
    {
        send_to_char( "Title too long, redo.\n\r", ch );
        return;
    }

    smash_tilde( argument );
    set_title( ch, argument );
    sprintf( buf, "Ok, you are now %s%s\n\r", ch->name, ch->pcdata->title );
    send_to_char( buf, ch );
}

void do_comment( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
        return;

    if ( argument[0] == '\0' )
    {
        free_string( &ch->pcdata->comment );
        ch->pcdata->comment = str_dup( "(none)" );
        send_to_char( "Whois comment deleted.\n\r", ch );
        return;
    }

    if ( str_len( argument ) > 60 )
    {
        send_to_char( "Your comment cannot be longer than 60 characters.\n\r",
                      ch );
        return;
    }

    smash_tilde( argument );
    free_string( &ch->pcdata->comment );
    ch->pcdata->comment = str_dup( argument );
    sprintf( buf, "Ok, your comment is  now -\n\r\n\r     %s\n\r",
             ch->pcdata->comment );
    send_to_char( buf, ch );
}

void do_email( CHAR_DATA * ch, char *argument )
{
    char last_char;
    int i, length, dots;
    bool at;
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC( ch ) )
        return;

    if ( argument[0] == '\0' )
    {
        free_string( &ch->pcdata->email );
        ch->pcdata->email = str_dup( "(none)" );
        send_to_char( "Email address deleted.\n\r", ch );
        return;
    }

    length = str_len( argument );

    if ( length > 45 )
    {
        send_to_char( "Email address can't be longer than 45 characters.\n\r",
                      ch );
        return;
    }

    smash_tilde( argument );

    if ( argument[0] == '@' || argument[0] == '.' || argument[length - 1] == '@'
         || argument[length - 1] == '.' )
    {
        send_to_char
            ( "Invalid email address, your e-mail address must be in the\n\r"
              "name@host.domain format.\n\r", ch );
        return;
    }

    at = FALSE;
    dots = 0;
    last_char = '\0';

    for ( i = 0; i < length; i++ )
    {
        switch ( argument[i] )
        {
        case '@':
            if ( !at )
                at = TRUE;
            else
            {
                send_to_char
                    ( "Invalid email address, your e-mail address must be in the\n\r"
                      "name@host.domain format.\n\r", ch );
                return;
            }
            break;
        case '.':
            if ( last_char == '@' || last_char == '.' || !at )
            {
                send_to_char
                    ( "Invalid email address, your e-mail address must be in the\n\r"
                      "name@host.domain format.\n\r", ch );
                return;
            }
            dots++;
            break;
        case ' ':
            send_to_char
                ( "Invalid email address, your e-mail address must be in the\n\r"
                  "name@host.domain format.\n\r", ch );
            return;
            break;
        }
        last_char = argument[i];
    }

    if ( dots < 1 )
    {
        send_to_char
            ( "Invalid email address, your e-mail address must be in the\n\r"
              "name@host.domain format.\n\r", ch );
        return;
    }

    free_string( &ch->pcdata->email );
    ch->pcdata->email = str_dup( argument );
    sprintf( buf, "Ok, your email address is now: %s\n\r", ch->pcdata->email );
    send_to_char( buf, ch );
}

void do_description( CHAR_DATA * ch, char *argument )
{
    if ( ch->desc != NULL )     /* only if ch has a descriptor 'cause string_append will barf */
    {
        string_append( ch, &ch->description );
        return;
    }

    return;
}

void do_report( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    sprintf( buf,
             "`BYou say 'I have %d/%d hp %d/%d mana %d/%d mv %ld xp.'\n\r`w",
             ch->hit, ch->max_hit,
             ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp );

    send_to_char( buf, ch );

    sprintf( buf, "`B$n says 'I have %d/%d hp %d/%d mana %d/%d mv %ld xp.'`w",
             ch->hit, ch->max_hit,
             ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp );

    act( buf, ch, NULL, NULL, TO_ROOM );

    return;
}

void do_practice( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    int sn;

    if ( IS_NPC( ch ) )
        return;

    if ( argument[0] == '\0' )
    {
        int col;

        col = 0;
        for ( sn = 0; sn < MAX_SKILL; sn++ )
        {
            if ( skill_table[sn].name == NULL )
                break;
            if ( ch->level < skill_table[sn].skill_level[ch->Class]
                 || ch->pcdata->learned[sn] < 1 /* skill is not known */  )
                continue;

            sprintf( buf, "%-18s %3d%%  ",
                     skill_table[sn].name, ch->pcdata->learned[sn] );
            send_to_char( buf, ch );
            if ( ++col % 3 == 0 )
                send_to_char( "\n\r", ch );
        }

        if ( col % 3 != 0 )
            send_to_char( "\n\r", ch );

        sprintf( buf, "You have %d practice sessions left.\n\r", ch->practice );
        send_to_char( buf, ch );
    }
    else
    {
        CHAR_DATA *mob;
        int adept, cp;

        if ( !IS_AWAKE( ch ) )
        {
            send_to_char( "In your dreams, or what?\n\r", ch );
            return;
        }

        for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room )
        {
            if ( IS_NPC( mob ) && IS_SET( mob->act, ACT_PRACTICE ) )
                break;
        }

        if ( mob == NULL )
        {
            send_to_char( "You can't do that here.\n\r", ch );
            return;
        }

        if ( ch->practice <= 0 )
        {
            send_to_char( "You have no practice sessions left.\n\r", ch );
            return;
        }

        sn = skill_lookup( argument );

        if ( !can_practice( ch, sn ) )
        {
            send_to_char( "You can't practice that.\n\r", ch );
            return;
        }

        cp = skill_table[sn].rating[ch->Class];

        if ( cp == 0 )
            cp = MAX_CREATION_POINTS;

        adept = IS_NPC( ch ) ? 100 : class_table[ch->Class].skill_adept;

        if ( ch->pcdata->learned[sn] >= adept )
        {
            sprintf( buf, "You are already learned at %s.\n\r",
                     skill_table[sn].name );
            send_to_char( buf, ch );
        }
        else
        {
            ch->practice--;
            ch->pcdata->learned[sn] +=
                int_app[get_curr_stat( ch, STAT_INT )].learn / cp;
            if ( ch->pcdata->learned[sn] < adept )
            {
                act( "You practice $T.",
                     ch, NULL, skill_table[sn].name, TO_CHAR );
                act( "$n practices $T.",
                     ch, NULL, skill_table[sn].name, TO_ROOM );
            }
            else
            {
                ch->pcdata->learned[sn] = adept;
                act( "You are now learned at $T.",
                     ch, NULL, skill_table[sn].name, TO_CHAR );
                act( "$n is now learned at $T.",
                     ch, NULL, skill_table[sn].name, TO_ROOM );
            }
        }
    }
    return;
}

/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    int wimpy;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
        wimpy = ch->max_hit / 5;
    else
        wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
        send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
        return;
    }

    if ( wimpy > ch->max_hit / 2 )
    {
        send_to_char( "Such cowardice ill becomes you.\n\r", ch );
        return;
    }

    ch->wimpy = wimpy;
    sprintf( buf, "Wimpy set to %d hit points.\n\r", wimpy );
    send_to_char( buf, ch );
    return;
}

void do_password( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( IS_NPC( ch ) )
        return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg;
    while ( isspace( *argument ) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( isspace( *argument ) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: password <old> <new>.\n\r", ch );
        return;
    }

    if ( strcmp( crypt( arg, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
        WAIT_STATE( ch, 40 );
        send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
        return;
    }

    if ( strlen( arg2 ) < 5 )
    {
        send_to_char( "New password must be at least five characters long.\n\r",
                      ch );
        return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt( arg2, ch->name );
    for ( p = pwdnew; *p != '\0'; p++ )
    {
        if ( *p == '~' )
        {
            send_to_char( "New password not acceptable, try again.\n\r", ch );
            return;
        }
    }

    free_string( &ch->pcdata->pwd );
    ch->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( ch );
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_search( CHAR_DATA * ch )
{
    char buf[MAX_STRING_LENGTH];

    extern char *const dir_name[];
    EXIT_DATA *pexit;
    int door;
    bool found;

    if ( !check_blind( ch ) )
        return;

    send_to_char( "You start searching for secret doors.\n\r", ch );
    found = FALSE;

    for ( door = 0; door <= 5; door++ )
    {
        if ( ( pexit = ch->in_room->exit[door] ) != NULL
             && pexit->u1.to_room != NULL
             && IS_SET( pexit->exit_info, EX_CLOSED )
             && IS_SET( pexit->exit_info, EX_HIDDEN ) )
        {
            found = TRUE;
            sprintf( buf, "You found a secret exit %s.\n\r", dir_name[door] );
            send_to_char( buf, ch );
        }
    }
    if ( !found )
    {
        send_to_char( "You found no secret exits.\n\r", ch );
        return;
    }
}

void do_cwho( CHAR_DATA * ch )
{
    char buf[MAX_STRING_LENGTH];

    char output[4 * MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *who_list[300];
    int length;
    int maxlength;
    extern bool chaos;

    if ( !chaos )
    {
        send_to_char( "There is no `rC`RH`YA`RO`rS`w active.\n\r", ch );
        return;
    }

    length = 0;
    for ( d = descriptor_list; d; d = d->next )
    {
        if ( d->connected == CON_PLAYING )
        {
            chaos_sort( who_list, d->character, length );
            length++;
        }
    }

    buf[0] = '\0';
    output[0] = '\0';
    maxlength = length;
    for ( length = 0; length < maxlength; length++ )
    {
        sprintf( buf, "`K[`W%4d`K] `w%s\n\r",
                 who_list[length]->pcdata->chaos_score,
                 who_list[length]->name );
        strcat( output, buf );
    }
    send_to_char( output, ch );
    return;
}

/* I changed this a bit.... got rid of the pet bug and added clan recognition. 
	-Kyle */
void do_finger( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    static char *const he_she[] = { "It", "He", "She" };
    CHAR_DATA *victim;
    FILE *fp;
    char pfile[MAX_STRING_LENGTH], *title;
    char *word, *class, *race, *comment, *email, *spouse, *nemesis;
    long played = 0, logon = 0;
    char buf2[MAX_STRING_LENGTH], ltime[MAX_STRING_LENGTH];
    unsigned int x;
    sh_int sex = 0, level, pk_kills, pk_deaths;
    sh_int nclan = 0;
    sh_int incarnations = 0;
    CLAN_DATA *clan;

    argument = one_argument( argument, arg );
    pfile[0] = '\0';
    word = NULL;
    ltime[0] = '\0';
    class = NULL;
    race = NULL;
    title = NULL;
    /* clan=0; */
    level = 0;
    pk_kills = 0;
    pk_deaths = 0;
    incarnations = 0;
    comment = NULL;
    email = NULL;
    spouse = NULL;
    nemesis = NULL;
    word = NULL;

    if ( arg[0] == '\0' || arg[0] == '.' || arg[0] == '/' )
    {
        send_to_char( "You want information about whom?\n\r", ch );
        return;
    }

    victim = get_player_world( ch, arg );

    if ( victim != NULL )
    {
        if ( victim->anonymous )
        {
            printf_to_char( ch, "%s is `KA`wN`WO`wN`KY`wM`WO`wU`KS`K",
                            victim->name );
            return;
        }

        sprintf( buf,
                 "\n\r      `y/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\\n\r" );
        send_to_char( buf, ch );
        sprintf( buf, "     |   `W%s%s", victim->name, victim->pcdata->title );
        send_to_char( buf, ch );
        strcpy( buf, "\0" );
        for ( x = 0;
              x <
              62 - strlen( victim->name ) - str_len( victim->pcdata->title );
              x++ )
            strcat( buf, " " );
        send_to_char( buf, ch );
        sprintf( buf, "`y|____|\n\r" );
        send_to_char( buf, ch );
        sprintf( buf,
                 "     |+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+|\n\r" );
        send_to_char( buf, ch );
        if ( victim->pcdata->clan > 0 )
        {
            clan = get_clan( victim->pcdata->clan );
            if ( clan == NULL )
                return;
            if ( ( !IS_NPC( ch )
                   &&
                   ( ( !IS_SET( clan->clan_flags, CLAN_PRIVATE )
                       && !IS_SET( clan->clan_flags, CLAN_SECRET ) )
                     || ( ch->pcdata->clan == victim->pcdata->clan
                          && ( IS_SET( clan->clan_flags, CLAN_PRIVATE )
                               || ( IS_SET( clan->clan_flags, CLAN_SECRET )
                                    && !str_cmp( victim->name,
                                                 clan->leader ) ) ) ) ) )
                 || IS_IMMORTAL( ch ) )
            {
                sprintf( buf2, "%s of %s",
                         !str_cmp( victim->name,
                                   clan->
                                   leader ) ? "the leader" : !str_cmp( victim->
                                                                       name,
                                                                       clan->
                                                                       god ) ?
                         "the sponsor" : "a member", clan->whoname );
            }
            else
                sprintf( buf2, "not a member of any clan." );
        }
        else
            sprintf( buf2, "not a member of any clan." );

        sprintf( buf, "     | `GSex  : `Y%-7s   `W%s is %s`y",
                 victim->sex == 0 ? "Sexless" : victim->sex ==
                 1 ? "Male" : "Female", he_she[URANGE( 0, victim->sex, 2 )],
                 buf2 );
        send_to_char( buf, ch );
        x = str_len( buf );
        strcpy( buf, "\0" );
        for ( ; x < 71; x++ )
            strcat( buf, " " );
        strcat( buf, "|\n\r" );
        send_to_char( buf, ch );
        sprintf( buf,
                 "     | `GLevel: `Y%2d        `WLast login: %s`y",
                 UMIN( MAX_LEVEL, victim->level ),
                 victim->desc ? "Currently playing" : "Currently link-dead" );
        send_to_char( buf, ch );
        x = str_len( buf );
        strcpy( buf, "\0" );
        for ( ; x < 71; x++ )
            strcat( buf, " " );
        strcat( buf, "|\n\r" );
        send_to_char( buf, ch );
        sprintf( buf, "     | `GAge  :`Y%3d        `W%s is a %s %s.`y",
                 get_age( victim ), he_she[URANGE( 0, victim->sex, 2 )],
                 pc_race_table[victim->race].name,
                 class_table[victim->Class].name );
        send_to_char( buf, ch );
        x = str_len( buf );
        strcpy( buf, "\0" );
        for ( ; x < 71; x++ )
            strcat( buf, " " );
        strcat( buf, "|\n\r" );
        send_to_char( buf, ch );
        sprintf( buf, "     | `GPK kills : `Y%-5d `WLast killed by: %s`y",
                 victim->pcdata->pk_kills, victim->pcdata->nemesis );
        send_to_char( buf, ch );
        x = str_len( buf );
        strcpy( buf, "\0" );
        for ( ; x < 71; x++ )
            strcat( buf, " " );
        strcat( buf, "|\n\r" );
        send_to_char( buf, ch );
        sprintf( buf, "     | `GPK deaths: `Y%-5d `WEmail: %s`y",
                 victim->pcdata->pk_deaths, victim->pcdata->email );
        send_to_char( buf, ch );
        x = str_len( buf );
        strcpy( buf, "\0" );
        for ( ; x < 71; x++ )
            strcat( buf, " " );
        strcat( buf, "|\n\r" );
        send_to_char( buf, ch );
        sprintf( buf, "     | `GIncarnations:`Y %d`y", victim->incarnations );
        send_to_char( buf, ch );
        x = str_len( buf );
        strcpy( buf, "\0" );
        for ( ; x < 71; x++ )
            strcat( buf, " " );
        strcat( buf, "|\n\r" );
        send_to_char( buf, ch );
        sprintf( buf, "     | `GSpouse:`Y %s`y", victim->pcdata->spouse );
        send_to_char( buf, ch );
        x = str_len( buf );
        strcpy( buf, "\0" );
        for ( ; x < 71; x++ )
            strcat( buf, " " );
        strcat( buf, "|\n\r" );
        send_to_char( buf, ch );
        sprintf( buf, "     | `GComment: `Y%s`y", victim->pcdata->comment );
        send_to_char( buf, ch );
        x = str_len( buf );
        strcpy( buf, "\0" );
        for ( ; x < 71; x++ )
            strcat( buf, " " );
        strcat( buf, "|\n\r" );
        send_to_char( buf, ch );
        send_to_char
            ( "`y  /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |\n\r",
              ch );
        send_to_char
            ( "`y  \\________________________________________________________________\\__/`w\n\r",
              ch );
        return;
    }
    else
    {
#if defined(unix)
        /* decompress if .gz file exists */
        sprintf( pfile, "%s/%s%s", sysconfig.player_dir, capitalize( arg ),
                 ".gz" );
        if ( ( fp = fopen( pfile, "r" ) ) != NULL )
        {
            char buf[MAX_INPUT_LENGTH];
            fclose( fp );
            sprintf( buf, "gzip -dfq %s", pfile );
            system( buf );
        }
#endif
        sprintf( pfile, "%s/%s", sysconfig.player_dir, capitalize( arg ) );
        if ( ( fp = fopen( pfile, "r" ) ) != NULL )
        {
            for ( ;; )
            {
                word = get_word( fp );
                if ( !str_cmp( word, "Anon" ) )
                {
                    if ( fread_number( fp ) == 1 )
                    {
                        printf_to_char( ch,
                                        "%s is `KA`wN`WO`wN`KY`wM`WO`wU`KS`W",
                                        arg );
                        fclose( fp );
                        return;
                    }
                }

                if ( !str_cmp( word, "End" ) )
                {
                    break;
                }
                if ( !str_cmp( word, "#END" ) )
                {
                    break;
                }
                if ( !str_cmp( word, "#PET" ) )
                {
                    break;
                }
                if ( !str_cmp( word, "Spou" ) )
                {
                    spouse = fread_string( fp );
                }
                if ( !str_cmp( word, "Neme" ) )
                {
                    nemesis = fread_string( fp );
                }
                if ( !str_cmp( word, "PKdi" ) )
                {
                    pk_deaths = fread_number( fp );
                }
                if ( !str_cmp( word, "PKki" ) )
                {
                    pk_kills = fread_number( fp );
                }
                if ( !str_cmp( word, "Cmnt" ) )
                {
                    comment = fread_string( fp );
                }
                if ( !str_cmp( word, "Eml" ) )
                {
                    email = fread_string( fp );
                }
                if ( !str_cmp( word, "Logn" ) )
                {
                    logon = fread_number( fp );
                    sprintf( ltime, "%s at %s", get_date( logon ),
                             get_time( logon ) );
                }
                if ( !str_cmp( word, "Plyd" ) )
                {
                    played = fread_number( fp );
                }
                if ( !str_cmp( word, "Levl" ) )
                {
                    level = fread_number( fp );
                }
                if ( !str_cmp( word, "Race" ) )
                {
                    race = fread_string( fp );
                }
                if ( !str_cmp( word, "Sex" ) )
                {
                    sex = fread_number( fp );
                }
                if ( !str_cmp( word, "Cla" ) )
                {
                    class = ( class_table[fread_number( fp )].name );
                }
                if ( !str_cmp( word, "Titl" ) )
                {
                    title = fread_string( fp );
                }
                if ( !str_cmp( word, "Clan" ) )
                {
                    nclan = fread_number( fp );
                }
                fread_to_eol( fp );
                if ( word )
                    free( word );
            }
            fclose( fp );
            sprintf( buf,
                     "\n\r      `y/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\\n\r" );
            send_to_char( buf, ch );
            sprintf( buf, "     |   `W%s %s", capitalize( arg ), title );
            send_to_char( buf, ch );
            strcpy( buf, "\0" );
            for ( x = 0; x < 61 - strlen( arg ) - str_len( title ); x++ )
                strcat( buf, " " );
            send_to_char( buf, ch );
            sprintf( buf, "`y|____|\n\r" );
            send_to_char( buf, ch );
            sprintf( buf,
                     "     |+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+|\n\r" );
            send_to_char( buf, ch );
            if ( nclan > 0 )
            {
                clan = get_clan( nclan );
                if ( clan == NULL )
                    return;
                if ( ( !IS_NPC( ch ) && IS_SET( clan->clan_flags, CLAN_SECRET )
                       && ch->pcdata->clan == nclan ) ||
                     ( !IS_SET( clan->clan_flags, CLAN_SECRET ) )
                     || get_trust( ch ) >= MAX_LEVEL )
                {
                    sprintf( buf2, "%s of %s",
                             !str_cmp( capitalize( arg ),
                                       clan->
                                       leader ) ? "the leader" :
                             !str_cmp( capitalize( arg ),
                                       clan->god ) ? "the sponsor" : "a member",
                             clan->whoname );
                }
                else
                    sprintf( buf2, "not a member of any clan." );
            }
            else
                sprintf( buf2, "not a member of any clan." );
            /*          sprintf(buf2,"not a member of any clan." ); */
            sprintf( buf, "     | `GSex  : `Y%-7s   `W%s is %s`y",
                     sex == 0 ? "Sexless" : sex == 1 ? "Male" : "Female",
                     he_she[URANGE( 0, sex, 2 )], buf2 );
            send_to_char( buf, ch );
            x = str_len( buf );
            strcpy( buf, "\0" );
            for ( ; x < 71; x++ )
                strcat( buf, " " );
            strcat( buf, "|\n\r" );
            send_to_char( buf, ch );
            sprintf( buf, "     | `GLevel: `Y%3d       `WLast login: %s`y",
                     UMIN( MAX_LEVEL, level ),
                     ( logon == 0 ) ? "Unknown" : ltime );
            send_to_char( buf, ch );
            x = str_len( buf );
            strcpy( buf, "\0" );
            for ( ; x < 71; x++ )
                strcat( buf, " " );
            strcat( buf, "|\n\r" );
            send_to_char( buf, ch );
            sprintf( buf, "     | `GAge  :`Y%3ld        `W%s is a %s %s.`y",
                     ( 17 + ( played / 72000 ) ),
                     he_she[URANGE( 0, sex, 2 )], race, class );
            send_to_char( buf, ch );
            x = str_len( buf );
            strcpy( buf, "\0" );
            for ( ; x < 71; x++ )
                strcat( buf, " " );
            strcat( buf, "|\n\r" );
            send_to_char( buf, ch );
            sprintf( buf, "     | `GPK kills : `Y%-5d `WLast killed by: %s`y",
                     pk_kills, nemesis );
            send_to_char( buf, ch );
            x = str_len( buf );
            strcpy( buf, "\0" );
            for ( ; x < 71; x++ )
                strcat( buf, " " );
            strcat( buf, "|\n\r" );
            send_to_char( buf, ch );
            sprintf( buf, "     | `GPK deaths: `Y%-5d `WEmail: %s`y",
                     pk_deaths, email );
            send_to_char( buf, ch );
            x = str_len( buf );
            strcpy( buf, "\0" );
            for ( ; x < 71; x++ )
                strcat( buf, " " );
            strcat( buf, "|\n\r" );
            send_to_char( buf, ch );
            sprintf( buf, "     | `GSpouse:`Y %s`y", spouse );
            send_to_char( buf, ch );
            x = str_len( buf );
            strcpy( buf, "\0" );
            for ( ; x < 71; x++ )
                strcat( buf, " " );
            strcat( buf, "|\n\r" );
            send_to_char( buf, ch );
            sprintf( buf, "     | `GComment: `Y%s`y", comment );
            send_to_char( buf, ch );
            x = str_len( buf );
            strcpy( buf, "\0" );
            for ( ; x < 71; x++ )
                strcat( buf, " " );
            strcat( buf, "|\n\r" );
            send_to_char( buf, ch );
            send_to_char
                ( "`y  /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |\n\r",
                  ch );
            send_to_char
                ( "`y  \\________________________________________________________________\\__/`w\n\r",
                  ch );
/*           if (class) free_string(&class);
 *           if (race) free_string(&race);
 *           if (comment) free_string(&comment);
 *           if (email) free_string(&email);
 *           if (nemesis) free_string(&nemesis); */
            return;
        }
        send_to_char( "That character does not exist on this mud.\n\r", ch );
        return;
    }
}

void do_levelgain( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->level < LEVEL_HERO && ch->exp >=
         exp_per_level( ch, ch->pcdata->points ) )
    {
        sprintf( buf, "%s has made it to level %d!", ch->name, ch->level + 1 );
        log_string( buf );
        do_sendinfo( ch, buf );
        send_to_char( "You raise a level!!  ", ch );
        advance_level( ch );
        ch->level += 1;
        save_char_obj( ch );
    }
    else
        send_to_char( "\n\rYou're not quite ready yet.\n\r", ch );
}
void do_rebirt( CHAR_DATA * ch, char *argument )
{
    send_to_char( "If you want to REBIRTH, spell it out!\n\r", ch );
    return;
}

void do_rebirth( CHAR_DATA * ch, char *argument )
{
    OBJ_DATA *obj, *obj_next;
    char buf[MAX_STRING_LENGTH];
    int iWear;

    if ( str_cmp( ch->desc->incomm, "rebirth yes" ) )
    {
        send_to_char( "If you want to be reborn, type 'rebirth yes'\n\r", ch );
        send_to_char( "CAUTION: You will lose eq, trains, and more.\n\r", ch );
        return;
    }
    if ( ch->level < LEVEL_HERO )
    {
        sprintf( buf, "You must be level %d to be reborn.\n\r", LEVEL_HERO );
        send_to_char( buf, ch );
        return;
    }

    if ( ch->level > 52 )
    {
        send_to_char( "Why the hell would you want to do that?\n\r", ch );
        return;
    }

    if ( ch->incarnations >= 4 )
    {
        send_to_char( "You have been reborn max times.\n\r", ch );
        return;
    }

    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {

        if ( !( obj = get_eq_char( ch, iWear ) ) )
            continue;

        unequip_char( ch, obj );

        extract_obj( obj );
    }

    for ( obj = ch->carrying; obj; obj = obj->next_content )
    {
        obj_next = obj->next_content;
        extract_obj( obj );
    }
    if ( !IS_SET( ch->act, PLR_REMORT ) )
        SET_BIT( ch->act, PLR_REMORT );
    ch->incarnations = ++ch->incarnations;
    /* reset misc */
    ch->pcdata->condition[COND_THIRST] = 0;
    ch->pcdata->condition[COND_FULL] = 0;
    ch->pcdata->condition[COND_DRUNK] = 0;
    ch->saving_throw = 0;

    /* level one stats */
    ch->max_hit = 20 + ( 20 * ch->incarnations );
    ch->max_mana = 100 + ( 50 * ch->incarnations );
    ch->max_move = 100 + ( 50 * ch->incarnations );
    ch->pcdata->perm_hit = ch->max_hit;
    ch->pcdata->perm_mana = ch->max_mana;
    ch->pcdata->perm_move = ch->max_move;
    ch->level = 1;
    ch->hitroll = 0;
    ch->damroll = 0;
    ch->exp = 0;
    ch->train = 0;
/*    ch->raisepts = 0; *//*  We don't have raise points */

    /* restore */
    ch->hit = ch->max_hit;
    ch->mana = ch->max_mana;
    ch->move = ch->max_move;
    /* save the character */
    do_save( ch, "" );

    /* completed message */
    send_to_char( "You have been reborn annew.\n\r", ch );
}

/* This was alias.c and that's why this huge header is here.  */

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
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

/* does aliasing and other fun stuff */
void substitute_alias( DESCRIPTOR_DATA * d, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    char name[MAX_INPUT_LENGTH];
    char *point;
    int alias;

    ch = d->original ? d->original : d->character;

    if ( IS_NPC( ch ) || ch->pcdata->alias[0] == NULL
         || !str_prefix( argument, "alias" )
         || !str_prefix( argument, "unalias" ) )
    {
        interpret( d->character, argument );
        return;
    }

    strcpy( buf, argument );

    for ( alias = 0; alias < MAX_ALIAS; alias++ )   /* go through the aliases */
    {
        if ( ch->pcdata->alias[alias] == NULL )
            break;

        if ( !str_prefix( argument, ch->pcdata->alias[alias] ) )
        {
            point = one_argument( argument, name );
            if ( !strcmp( ch->pcdata->alias[alias], name ) )
            {
/*		buf[0] = '\0';
		strcat(buf,ch->pcdata->alias_sub[alias]);
		strcat(buf," ");
		strcat(buf,point);*/

/*The above code causes 'alias x goto 3001' to not work. The code below
fixes this small problem. -Lancelight */

                buf[0] = '\0';
                strcat( buf, ch->pcdata->alias_sub[alias] );
                if ( point[0] )
                {
                    strcat( buf, " " );
                    strcat( buf, point );
                }
/* end alias bug fix*/
                break;
            }
            if ( strlen( buf ) > MAX_INPUT_LENGTH )
            {
                send_to_char( "Alias substitution too long. Truncated.\r\n",
                              ch );
                buf[MAX_INPUT_LENGTH - 1] = '\0';
            }
        }
    }
    interpret( d->character, buf );
}

void do_alia( CHAR_DATA * ch, char *argument )
{
    send_to_char( "I'm sorry, alias must be entered in full.\n\r", ch );
    return;
}

void do_alias( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int pos;

    if ( ch->desc == NULL )
        rch = ch;
    else
        rch = ch->desc->original ? ch->desc->original : ch;

    if ( IS_NPC( rch ) )
        return;

    argument = one_argument( argument, arg );

    smash_tilde( arg );
    smash_tilde( argument );

    if ( arg[0] == '\0' )
    {

        if ( rch->pcdata->alias[0] == NULL )
        {
            send_to_char( "You have no aliases defined.\n\r", ch );
            return;
        }
        send_to_char( "Your current aliases are:\n\r", ch );

        for ( pos = 0; pos < MAX_ALIAS; pos++ )
        {
            if ( rch->pcdata->alias[pos] == NULL
                 || rch->pcdata->alias_sub[pos] == NULL )
                break;

            sprintf( buf, "    %s:  %s\n\r", rch->pcdata->alias[pos],
                     rch->pcdata->alias_sub[pos] );
            send_to_char( buf, ch );
        }
        return;
    }

    if ( !str_prefix( arg, "unalias" ) || !str_cmp( arg, "alias" ) )
    {
        send_to_char( "Sorry, that word is reserved.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        for ( pos = 0; pos < MAX_ALIAS; pos++ )
        {
            if ( rch->pcdata->alias[pos] == NULL
                 || rch->pcdata->alias_sub[pos] == NULL )
                break;

            if ( !str_cmp( arg, rch->pcdata->alias[pos] ) )
            {
                sprintf( buf, "%s aliases to '%s'.\n\r",
                         rch->pcdata->alias[pos], rch->pcdata->alias_sub[pos] );
                send_to_char( buf, ch );
                return;
            }
        }

        send_to_char( "That alias is not defined.\n\r", ch );
        return;
    }

    if ( !str_prefix( argument, "delete" ) )
    {
        send_to_char( "That shall not be done!\n\r", ch );
        return;
    }

    for ( pos = 0; pos < MAX_ALIAS; pos++ )
    {
        if ( rch->pcdata->alias[pos] == NULL )
            break;

        if ( !str_cmp( arg, rch->pcdata->alias[pos] ) ) /* redefine an alias */
        {
            free_string( &rch->pcdata->alias_sub[pos] );
            rch->pcdata->alias_sub[pos] = str_dup( argument );
            sprintf( buf, "%s is now realiased to '%s'.\n\r", arg, argument );
            send_to_char( buf, ch );
            return;
        }
    }

    if ( pos >= MAX_ALIAS )
    {
        send_to_char( "Sorry, you have reached the alias limit.\n\r", ch );
        return;
    }

    /* make a new alias */
    rch->pcdata->alias[pos] = str_dup( arg );
    rch->pcdata->alias_sub[pos] = str_dup( argument );
    sprintf( buf, "%s is now aliased to '%s'.\n\r", arg, argument );
    send_to_char( buf, ch );
}

void do_unalias( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *rch;
    int pos;
    bool found = FALSE;

    if ( ch->desc == NULL )
        rch = ch;
    else
        rch = ch->desc->original ? ch->desc->original : ch;

    if ( IS_NPC( rch ) )
        return;

    argument = one_argument( argument, arg );

    if ( arg == '\0' )
    {
        send_to_char( "Unalias what?\n\r", ch );
        return;
    }

    for ( pos = 0; pos < MAX_ALIAS; pos++ )
    {
        if ( rch->pcdata->alias[pos] == NULL )
            break;

        if ( found )
        {
            rch->pcdata->alias[pos - 1] = rch->pcdata->alias[pos];
            rch->pcdata->alias_sub[pos - 1] = rch->pcdata->alias_sub[pos];
            rch->pcdata->alias[pos] = NULL;
            rch->pcdata->alias_sub[pos] = NULL;
            continue;
        }

        if ( !strcmp( arg, rch->pcdata->alias[pos] ) )
        {
            send_to_char( "Alias removed.\n\r", ch );
            free_string( &rch->pcdata->alias[pos] );
            free_string( &rch->pcdata->alias_sub[pos] );
            found = TRUE;
        }
    }

    if ( !found )
        send_to_char( "No alias of that name to remove.\n\r", ch );
}
