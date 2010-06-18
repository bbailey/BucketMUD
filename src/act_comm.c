/**************************************************************************
 * Mudprogram's (Mobprogram, Objprogram and Roomprogram) originaly        *
 * by the SMAUG development team                                          *
 * Ported to EmberMUD by Thanatos and Tyrluk of ToED                      *
 * (Temple of Eternal Death)                                              *
 * Tyrluk   - morn@telmaron.com or dajy@mindspring.com                    *
 * Thanatos - morn@telmaron.com or jonathan_w._rose@ffic.com              *
 **************************************************************************/

/* act_comm.c */

#if defined(WIN32)
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"

/* command procedures needed */
DECLARE_DO_FUN( do_quit );

extern void ChannelMessage( char *mesg, CHAR_DATA * ch );
extern bool can_do_immcmd( CHAR_DATA * ch, char *cmd );
extern bool check_social
args( ( CHAR_DATA * ch, char *command, char *argument ) );

/*
 * Local functions.
 */

/* RT code to delete yourself */
void do_delet( CHAR_DATA * ch, char *argument )
{
    send_to_char( "You must type the full command to delete yourself.\n\r",
                  ch );
}

void do_delete( CHAR_DATA * ch, char *argument )
{
    char strsave[MAX_INPUT_LENGTH];

    if ( IS_NPC( ch ) )
        return;

    /* This is more of a personal prefference. You dont need this. -Lancelight
     */

    if ( IS_SET( ch->act, PLR_JAILED ) )
    {
        send_to_char( "You wont get out of it that easily.", ch );
        return;
    }

    /* Lets make sure they dont get any bright ideas. -Lancelight */

    if ( auction_info.high_bidder == ch || auction_info.owner == ch )
    {
        send_to_char( "You still have a stake in the auction!\n\r", ch );
        return;
    }

    if ( ch->pcdata->confirm_delete )
    {
        if ( argument[0] != '\0' )
        {
            send_to_char( "Delete status removed.\n\r", ch );
            ch->pcdata->confirm_delete = FALSE;
            return;
        }
        else
        {
            sprintf( strsave, "%s/%s", sysconfig.player_dir,
                     capitalize( ch->name ) );
            do_quit( ch, "" );
            unlink( strsave );
            return;
        }
    }

    if ( argument[0] != '\0' )
    {
        send_to_char( "Just type delete. No argument.\n\r", ch );
        return;
    }

    send_to_char( "Type delete again to confirm this command.\n\r", ch );
    send_to_char
        ( "WARNING: this command is irreversible and your character will be permanently gone.\n\r",
          ch );
    send_to_char( "Typing delete with an argument will undo delete status.\n\r",
                  ch );
    ch->pcdata->confirm_delete = TRUE;
}

void do_channels( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    /* lists all channels and their status */
    send_to_char( "`W   channel   status`w\n\r", ch );
    send_to_char( "`K---------------------`w\n\r", ch );

    printf_to_char( ch, "`w%-15s", CFG_GOS_NAME );
    if ( !IS_SET( ch->comm, COMM_NOGOSSIP ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

#ifdef USE_GOCIAL

    printf_to_char( ch, "`w%-15s", CFG_GOC_NAME );
    if ( !IS_SET( ch->comm, COMM_NOGOC ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

#endif

    send_to_char( "`Yau`ycti`Yon`w        ", ch );
    if ( !IS_SET( ch->comm, COMM_NOAUCTION ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    printf_to_char( ch, "`w%-15s", CFG_OOC_NAME );
    if ( !IS_SET( ch->comm, COMM_NO_OOC ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    send_to_char( "`RQ`r/`RA            ", ch );
    if ( !IS_SET( ch->comm, COMM_NOQUESTION ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    if ( IS_HERO( ch ) && ( can_do_immcmd( ch, "immtalk" ) ) )
    {
        send_to_char( "`Ki`Wm`Km `Kc`Wh`Ka`Wn`Kn`We`Kl`w    ", ch );
        if ( !IS_SET( ch->comm, COMM_NOWIZ ) )
            send_to_char( "`GON`w\n\r", ch );
        else
            send_to_char( "`ROFF`w\n\r", ch );
    }

#ifdef USE_ADMINTALK

    if ( IS_HERO( ch ) && ( can_do_immcmd( ch, "admintalk" ) ) )
    {
        send_to_char( "`Ka`Wd`Km`Ki`Wn`Kt`Wa`Kl`Wk`K `w     ", ch );
        if ( !IS_SET( ch->comm, COMM_NOADMIN ) )
            send_to_char( "`GON`w\n\r", ch );
        else
            send_to_char( "`ROFF`w\n\r", ch );
    }

#endif
#ifdef USE_HEROTALK

    if ( ( IS_HERO( ch ) || IS_SET( ch->act, PLR_REMORT ) )
         && ( can_do_immcmd( ch, "herotalk" ) ) )
    {
        send_to_char( "`Kh`We`Kr`Ko`Wt`Ka`Wl`Kk`W `K `w     ", ch );
        if ( !IS_SET( ch->comm, COMM_NOHERO ) )
            send_to_char( "`GON`w\n\r", ch );
        else
            send_to_char( "`ROFF`w\n\r", ch );
    }

#endif

    send_to_char( "`Csh`cou`Cts`w         ", ch );
    if ( !IS_SET( ch->comm, COMM_DEAF ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    send_to_char( "`Rinfo`w           ", ch );
    if ( !IS_SET( ch->comm, COMM_NOINFO ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    send_to_char( "`Bmusic`w          ", ch );
    if ( !IS_SET( ch->comm, COMM_NOMUSIC ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    send_to_char( "`Kquiet mode`w     ", ch );
    if ( IS_SET( ch->comm, COMM_QUIET ) )
        send_to_char( "`GON`w\n\r", ch );
    else
        send_to_char( "`ROFF`w\n\r", ch );

    if ( !IS_NPC( ch ) )
    {
        if ( ch->pcdata->spousec != 0 )
        {
            send_to_char( "`wsp`Wou`wse         ", ch );

            if ( !IS_SET( ch->comm, COMM_NOSPOUSETALK ) )
                send_to_char( "`GON`w\n\r", ch );
            else
                send_to_char( "`ROFF`w\n\r", ch );
        }
    }

    if ( !IS_NPC( ch ) )
    {
        send_to_char( "show ticks     ", ch );
        if ( ch->pcdata->tick == 1 )
            send_to_char( "`GON`w\n\r", ch );
        else
            send_to_char( "`ROFF`w\n\r", ch );
    }

    if ( ch->lines != PAGELEN )
    {
        if ( ch->lines )
        {
            sprintf( buf, "You display %d lines of scroll.\n\r",
                     ch->lines + 2 );
            send_to_char( buf, ch );
        }
        else
            send_to_char( "Scroll buffering is off.\n\r", ch );
    }

    if ( IS_SET( ch->comm, COMM_NOSHOUT ) )
        send_to_char( "You cannot shout.\n\r", ch );

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
        send_to_char( "You cannot use tell.\n\r", ch );

    if ( IS_SET( ch->comm, COMM_NOCHANNELS ) )
        send_to_char( "You cannot use channels.\n\r", ch );

    if ( IS_SET( ch->comm, COMM_NOEMOTE ) )
        send_to_char( "You cannot emote.\n\r", ch );

}

/* RT deaf blocks out all shouts */
void do_deaf( CHAR_DATA * ch, char *argument )
{
    if ( IS_SET( ch->comm, COMM_NOSHOUT ) )
    {
        send_to_char( "The gods have taken away your ability to shout.\n\r",
                      ch );
        return;
    }

    if ( IS_SET( ch->comm, COMM_DEAF ) )
    {
        send_to_char( "You can now hear shouts again.\n\r", ch );
        REMOVE_BIT( ch->comm, COMM_DEAF );
    }
    else
    {
        send_to_char( "From now on, you won't hear shouts.\n\r", ch );
        SET_BIT( ch->comm, COMM_DEAF );
    }
}

/* DNC - telloff for imms */
void do_telloff( CHAR_DATA * ch, char *argument )
{
    if ( IS_SET( ch->comm, COMM_TELLOFF ) )
    {
        send_to_char( "You can now hear tells again.\n\r", ch );
        REMOVE_BIT( ch->comm, COMM_TELLOFF );
    }
    else
    {
        send_to_char( "From now on, you won't hear tells.\n\r", ch );
        SET_BIT( ch->comm, COMM_TELLOFF );
    }
}

/* RT quiet blocks out all communication */
void do_quiet( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
        send_to_char( "Quiet mode removed.\n\r", ch );
        REMOVE_BIT( ch->comm, COMM_QUIET );
        if ( ch->pcdata->message != NULL )
        {
            sprintf( buf,
                     "You have `W%d message%s`w waiting, type `Wmessages`w to read them.\n\r",
                     ch->pcdata->messages,
                     ( ch->pcdata->messages > 1 ) ? "s" : "" );
            send_to_char( buf, ch );
        }
    }
    else
    {
        send_to_char( "From now on, you will only hear says and emotes.\n\r",
                      ch );
        send_to_char( "Messages are being recorded.\n\r", ch );
        SET_BIT( ch->comm, COMM_QUIET );
    }
}

void do_gossip( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    /* No argument - toggle on/off */
    if ( argument[0] == '\0' )
    {
        if ( IS_SET( ch->comm, COMM_NOGOSSIP ) )
        {
            REMOVE_BIT( ch->comm, COMM_NOGOSSIP );
            send_to_char( CFG_GOS_NAME "`0 channel is now `GON`0.\n\r", ch );
        }
        else
        {
            SET_BIT( ch->comm, COMM_NOGOSSIP );
            send_to_char( CFG_GOS_NAME "`0 channel is now `ROFF`0.\n\r", ch );
        }

        return;
    }

    /* Check for channel being OFF */
    if ( IS_SET( ch->comm, COMM_NOGOSSIP ) )
    {
        send_to_char( "`0Cannot send to the " CFG_GOS_NAME
                      "`0 channel when you have it turned off!\n\r", ch );
        return;
    }

    /* Check for quiet mode */
    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
        send_to_char( "`0Cannot send to the " CFG_OOC_NAME
                      "`0 channel without turning off quiet mode!\n\r", ch );
        return;
    }

    /* Check for nochannel */
    if ( IS_SET( ch->comm, COMM_NOCHANNELS ) )
    {
        send_to_char
            ( "The gods have revoked all of your channel priviliges!\n\r", ch );
        return;
    }

#ifdef GOS_DRUNK
    /* Make the words drunk if needed */
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
        argument = makedrunk( argument, ch );
#endif

    /* Send to everyone that should hear it */
    /* Loop through entire descriptor_list to catch anyone that's switched */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->character;

        if ( victim )
        {
            if ( victim != ch &&
                 d->connected == CON_PLAYING &&
                 !IS_SET( victim->comm, COMM_NOGOSSIP ) &&
                 !IS_SET( victim->comm, COMM_DEAF ) &&
                 !IS_SET( victim->comm, COMM_QUIET ) )
            {
                act_new( CFG_GOS, ch, argument, victim, TO_VICT, MIN_POS_GOS );
            }
        }
    }

    /* Send to the speaker and add to the last queue */
    act_new( CFG_GOS, ch, argument, ch, TO_CHAR, MIN_POS_GOS );
    add2last( CFG_GOS, argument, ch );

    return;
}

#ifdef USE_GOCIAL

/*  This is an attempt to add gocials to EmberMUD.  They will be
 *  fully configurable and documented in config.h when I am done
 *
 */
void do_gocial( CHAR_DATA * ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
/*    int cmd; */
    bool found;
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];
    int counter;
    int count;
    char buf2[MAX_STRING_LENGTH];
    SOCIALLIST_DATA *cmd;

    argument = one_argument( argument, command );

    /* No argument - toggle on/off */
    if ( argument[0] == '\0' && command[0] == '\0' )
    {
        if ( IS_SET( ch->comm, COMM_NOGOC ) )
        {
            REMOVE_BIT( ch->comm, COMM_NOGOC );
            send_to_char( CFG_GOC_NAME "`0 channel is now `GON`0.\n\r", ch );
        }
        else
        {
            SET_BIT( ch->comm, COMM_NOGOC );
            send_to_char( CFG_GOC_NAME "`0 channel is now `ROFF`0.\n\r", ch );
        }

        return;
    }

    found = FALSE;
    for ( cmd = social_first; cmd != NULL; cmd = cmd->next )
    {
        if ( ( is_name( command, cmd->name ) ) )
        {
            found = TRUE;
            break;
        }
    }

    if ( !found )
    {
        send_to_char( "What kind of social is that?!?!\n\r", ch );
        return;
    }

    if ( !IS_NPC( ch ) && IS_SET( ch->comm, COMM_QUIET ) )
    {
        send_to_char( "You must turn off quiet mode first.\n\r", ch );
        return;
    }

    if ( !IS_NPC( ch ) && IS_SET( ch->comm, COMM_NOCHANNELS ) )
    {
        send_to_char( "The gods have revoked your channel priviliges.\n\r",
                      ch );
        return;
    }

    switch ( ch->position )
    {

    case POS_DEAD:
        send_to_char( "Lie still; you are DEAD!\n\r", ch );
        return;
    case POS_INCAP:
    case POS_MORTAL:
        send_to_char( "You are hurt far too bad for that.\n\r", ch );
        return;
    case POS_STUNNED:
        send_to_char( "You are too stunned for that.\n\r", ch );
        return;
    }

    one_argument( argument, arg );
    victim = NULL;
    if ( !*arg )
    {
        sprintf( buf, "`K[`WGOCIAL`K]`M %s", cmd->char_no_arg );
        act_new( buf, ch, NULL, NULL, TO_CHAR, POS_DEAD );
        sprintf( buf, "`K[`WGOCIAL`K]`M %s", cmd->others_no_arg );
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            CHAR_DATA *vch;
            vch = d->original ? d->original : d->character;
            if ( d->connected == CON_PLAYING &&
                 d->character != ch &&
                 !IS_SET( vch->comm, COMM_NOGOC ) &&
                 !IS_SET( vch->comm, COMM_QUIET ) )
            {
                act_new( buf, ch, NULL, vch, TO_VICT, POS_DEAD );
            }
        }
    }
    else if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    else if ( victim == ch )
    {
        sprintf( buf, "`K[`WGOCIAL`K]`M %s", cmd->char_auto );
        act_new( buf, ch, NULL, NULL, TO_CHAR, POS_DEAD );
        sprintf( buf, "`K[`WGOCIAL`K]`M %s", cmd->others_auto );
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            CHAR_DATA *vch;
            vch = d->original ? d->original : d->character;
            if ( d->connected == CON_PLAYING &&
                 d->character != ch &&
                 !IS_SET( vch->comm, COMM_NOGOC ) &&
                 !IS_SET( vch->comm, COMM_QUIET ) )
            {
                act_new( buf, ch, NULL, vch, TO_VICT, POS_DEAD );
            }
        }
    }
    else
    {
        sprintf( buf, "`K[`WGOCIAL`K]`M %s", cmd->char_found );
        act_new( buf, ch, NULL, victim, TO_CHAR, POS_DEAD );
        sprintf( buf, "`K[`WGOCIAL`K]`M %s", cmd->vict_found );
        act_new( buf, ch, NULL, victim, TO_VICT, POS_DEAD );

        sprintf( buf, "`K[`WGOCIAL`K]`M %s", cmd->others_found );
        for ( counter = 0; buf[counter + 1] != '\0'; counter++ )
        {
            if ( buf[counter] == '$' && buf[counter + 1] == 'N' )
            {
                strcpy( buf2, buf );
                buf2[counter] = '\0';
                strcat( buf2, victim->name );
                for ( count = 0; buf[count] != '\0'; count++ )
                {
                    buf[count] = buf[count + counter + 2];
                }
                strcat( buf2, buf );
                strcpy( buf, buf2 );

            }
            else if ( buf[counter] == '$' && buf[counter + 1] == 'E' )
            {
                switch ( victim->sex )
                {
                default:
                    strcpy( buf2, buf );
                    buf2[counter] = '\0';
                    strcat( buf2, "it" );
                    for ( count = 0; buf[count] != '\0'; count++ )
                    {
                        buf[count] = buf[count + counter + 2];
                    }
                    strcat( buf2, buf );
                    strcpy( buf, buf2 );
                    break;
                case 1:
                    strcpy( buf2, buf );
                    buf2[counter] = '\0';
                    strcat( buf2, "it" );
                    for ( count = 0; buf[count] != '\0'; count++ )
                    {
                        buf[count] = buf[count + counter + 2];
                    }
                    strcat( buf2, buf );
                    strcpy( buf, buf2 );
                    break;
                case 2:
                    strcpy( buf2, buf );
                    buf2[counter] = '\0';
                    strcat( buf2, "it" );
                    for ( count = 0; buf[count] != '\0'; count++ )
                    {
                        buf[count] = buf[count + counter + 2];
                    }
                    strcat( buf2, buf );
                    strcpy( buf, buf2 );
                    break;
                }
            }
            else if ( buf[counter] == '$' && buf[counter + 1] == 'M' )
            {
                buf[counter] = '%';
                buf[counter + 1] = 's';
                switch ( victim->sex )
                {
                default:
                    strcpy( buf2, buf );
                    buf2[counter] = '\0';
                    strcat( buf2, "it" );
                    for ( count = 0; buf[count] != '\0'; count++ )
                    {
                        buf[count] = buf[count + counter + 2];
                    }
                    strcat( buf2, buf );
                    strcpy( buf, buf2 );
                    break;
                case 1:
                    strcpy( buf2, buf );
                    buf2[counter] = '\0';
                    strcat( buf2, "him" );
                    for ( count = 0; buf[count] != '\0'; count++ )
                    {
                        buf[count] = buf[count + counter + 2];
                    }
                    strcat( buf2, buf );
                    strcpy( buf, buf2 );
                    break;
                case 2:
                    strcpy( buf2, buf );
                    buf2[counter] = '\0';
                    strcat( buf2, "her" );
                    for ( count = 0; buf[count] != '\0'; count++ );
                    {
                        buf[count] = buf[count + counter + 2];
                    }
                    strcat( buf2, buf );
                    strcpy( buf, buf2 );
                    break;
                }
            }
            else if ( buf[counter] == '$' && buf[counter + 1] == 'S' )
            {
                switch ( victim->sex )
                {
                default:
                    strcpy( buf2, buf );
                    buf2[counter] = '\0';
                    strcat( buf2, "its" );
                    for ( count = 0; buf[count] != '\0'; count++ )
                    {
                        buf[count] = buf[count + counter + 2];
                    }
                    strcat( buf2, buf );
                    strcpy( buf, buf2 );
                    break;
                case 1:
                    strcpy( buf2, buf );
                    buf2[counter] = '\0';
                    strcat( buf2, "his" );
                    for ( count = 0; buf[count] != '\0'; count++ )
                    {
                        buf[count] = buf[count + counter + 2];
                    }
                    strcat( buf2, buf );
                    strcpy( buf, buf2 );
                    break;
                case 2:
                    strcpy( buf2, buf );
                    buf2[counter] = '\0';
                    strcat( buf2, "hers" );
                    for ( count = 0; buf[count] != '\0'; count++ )
                    {
                        buf[count] = buf[count + counter + 2];
                    }
                    strcat( buf2, buf );
                    strcpy( buf, buf2 );
                    break;
                }
            }

        }
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            CHAR_DATA *vch;
            vch = d->original ? d->original : d->character;
            if ( d->connected == CON_PLAYING &&
                 d->character != ch &&
                 d->character != victim &&
                 !IS_SET( vch->comm, COMM_NOGOC ) &&
                 !IS_SET( vch->comm, COMM_QUIET ) )
            {
                act_new( buf, ch, NULL, vch, TO_VICT, POS_DEAD );
            }
        }
    }
    return;
}
#endif

#ifdef USE_MUSIC
void do_music( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    /* No argument - toggle on/off */
    if ( argument[0] == '\0' )
    {
        if ( IS_SET( ch->comm, COMM_NOMUSIC ) )
        {
            REMOVE_BIT( ch->comm, COMM_NOMUSIC );
            send_to_char( CFG_MUS_NAME "`0 channel is now `GON`0.\n\r", ch );
        }
        else
        {
            SET_BIT( ch->comm, COMM_NOMUSIC );
            send_to_char( CFG_MUS_NAME "`0 channel is now `ROFF`0.\n\r", ch );
        }

        return;
    }

    /* Check for channel being OFF */
    if ( IS_SET( ch->comm, COMM_NOMUSIC ) )
    {
        send_to_char( "`0Cannot send to the " CFG_MUS_NAME
                      "`0 channel when you have it turned off!\n\r", ch );
        return;
    }

    /* Check for quiet mode */
    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
        send_to_char( "`0Cannot send to the " CFG_MUS_NAME
                      "`0 channel without turning off quiet mode!\n\r", ch );
        return;
    }

    /* Check for nochannel */
    if ( IS_SET( ch->comm, COMM_NOCHANNELS ) )
    {
        send_to_char
            ( "The gods have revoked all of your channel priviliges!\n\r", ch );
        return;
    }
    /* Make the words drunk if needed */
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
        argument = makedrunk( argument, ch );

    /* Send to everyone that should hear it */
    /* Loop through entire descriptor_list to catch anyone that's switched */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->character;

        if ( victim )
        {
            if ( victim != ch &&
                 d->connected == CON_PLAYING &&
                 !IS_SET( victim->comm, COMM_NOMUSIC ) &&
                 !IS_SET( victim->comm, COMM_DEAF ) &&
                 !IS_SET( victim->comm, COMM_QUIET ) )
            {
                act_new( CFG_MUS, ch, argument, victim, TO_VICT, MIN_POS_MUS );
            }
        }
    }

    /* Send to the speaker and add to the last queue */
    act_new( CFG_MUS, ch, argument, ch, TO_CHAR, MIN_POS_GOS );
    add2last( CFG_MUS, argument, ch );

    return;
}
#endif

void do_question( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
        if ( IS_SET( ch->comm, COMM_NOQUESTION ) )
        {
            if ( IS_SET( ch->comm, COMM_NOQUESTION ) )
            {
                REMOVE_BIT( ch->comm, COMM_NOQUESTION );
                send_to_char( "`0Q/A channel is now `GON`0.\n\r", ch );
            }
            else
            {
                SET_BIT( ch->comm, COMM_NOQUESTION );
                send_to_char( "`0Q/A channel is now `ROFF`0.\n\r", ch );
            }

            return;
        }
    }

    /* Check for channel being OFF */
    if ( IS_SET( ch->comm, COMM_NOQUESTION ) )
    {
        send_to_char
            ( "`0Cannot send to the Q/A channel when you have it turned off!\n\r",
              ch );
        return;
    }

    /* Check for quiet mode */
    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
        send_to_char
            ( "`0Cannot send to the Q/A channel without turning off quiet mode!\n\r",
              ch );
        return;
    }

    /* Check for nochannel */
    if ( IS_SET( ch->comm, COMM_NOCHANNELS ) )
    {
        send_to_char( "The gods have revoked all of your channel priviliges!",
                      ch );
        return;
    }

#ifdef QA_DRUNK
    /* Make the words drunk if needed */
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
        argument = makedrunk( argument, ch );
#endif

    /* Send to everyone that should hear it */
    /* Loop through entire descriptor_list to catch anyone that's switched */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->character;

        if ( victim )
        {
            if ( victim != ch &&
                 d->connected == CON_PLAYING &&
                 !IS_SET( victim->comm, COMM_NOQUESTION ) &&
                 !IS_SET( victim->comm, COMM_DEAF ) &&
                 !IS_SET( victim->comm, COMM_QUIET ) )
            {
                act_new( CFG_QUESTION, ch, argument, victim, TO_VICT,
                         MIN_POS_QA );
            }
        }
    }

    /* Send to the speaker and add to the last queue */
    act_new( CFG_QUESTION, ch, argument, ch, TO_CHAR, MIN_POS_QA );
    add2last( CFG_QUESTION, argument, ch );
}

/* RT answer channel - uses same line as questions */
void do_answer( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
        if ( IS_SET( ch->comm, COMM_NOQUESTION ) )
        {
            REMOVE_BIT( ch->comm, COMM_NOQUESTION );
            send_to_char( "`0Q/A channel is now `GON`0.\n\r", ch );
        }
        else
        {
            SET_BIT( ch->comm, COMM_NOQUESTION );
            send_to_char( "`0Q/A channel is now `ROFF`0.\n\r", ch );
        }

        return;
    }

    /* Check for channel being OFF */
    if ( IS_SET( ch->comm, COMM_NOQUESTION ) )
    {
        send_to_char
            ( "`0Cannot send to the Q/A channel when you have it turned off!\n\r",
              ch );
        return;
    }

    /* Check for quiet mode */
    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
        send_to_char
            ( "`0Cannot send to the Q/A channel without turning off quiet mode!\n\r",
              ch );
        return;
    }

#ifdef QA_DRUNK
    /* Make the words drunk if needed */
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
        argument = makedrunk( argument, ch );
#endif

    /* Check for nochannel */
    if ( IS_SET( ch->comm, COMM_NOCHANNELS ) )
    {
        send_to_char( "The gods have revoked all of your channel priviliges!",
                      ch );
        return;
    }

    /* Send to everyone that should hear it */
    /* Loop through entire descriptor_list to catch anyone that's switched */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->character;

        if ( victim )
        {
            if ( victim != ch &&
                 d->connected == CON_PLAYING &&
                 !IS_SET( victim->comm, COMM_NOQUESTION ) &&
                 !IS_SET( victim->comm, COMM_DEAF ) &&
                 !IS_SET( victim->comm, COMM_QUIET ) )
            {
                act_new( CFG_ANSWER, ch, argument, victim, TO_VICT,
                         MIN_POS_QA );
            }
        }
    }

    /* Send to the speaker and add to the last queue */
    act_new( CFG_ANSWER, ch, argument, ch, TO_CHAR, MIN_POS_QA );
    add2last( CFG_ANSWER, argument, ch );
}

void do_ooc( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    /* No argument - toggle on/off */
    if ( argument[0] == '\0' )
    {
        if ( IS_SET( ch->comm, COMM_NO_OOC ) )
        {
            REMOVE_BIT( ch->comm, COMM_NO_OOC );
            send_to_char( CFG_OOC_NAME "`0 channel is now `GON`0.\n\r", ch );
        }
        else
        {
            SET_BIT( ch->comm, COMM_NO_OOC );
            send_to_char( CFG_OOC_NAME "`0 channel is now `ROFF`0.\n\r", ch );
        }

        return;
    }

    /* Check for channel being OFF */
    if ( IS_SET( ch->comm, COMM_NO_OOC ) )
    {
        send_to_char( "`0Cannot send to the " CFG_OOC_NAME
                      "`0 channel when you have it turned off!\n\r", ch );
        return;
    }

    /* Check for quiet mode */
    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
        send_to_char( "`0Cannot send to the " CFG_OOC_NAME
                      "`0 channel without turning off quiet mode!\n\r", ch );
        return;
    }

    /* Check for nochannel */
    if ( IS_SET( ch->comm, COMM_NOCHANNELS ) )
    {
        send_to_char
            ( "The gods have revoked all of your channel priviliges!\n\r", ch );
        return;
    }

    /* Turn the channel on if we're using it. */
    REMOVE_BIT( ch->comm, COMM_NO_OOC );

#ifdef OOC_DRUNK
    /* Make the words drunk if needed */
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
        argument = makedrunk( argument, ch );
#endif

    /* Send to everyone that should hear it */
    /* Loop through entire descriptor_list to catch anyone that's switched */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->character;

        if ( victim )
        {
            if ( victim != ch &&
                 d->connected == CON_PLAYING &&
                 !IS_SET( victim->comm, COMM_NO_OOC ) &&
                 !IS_SET( victim->comm, COMM_DEAF ) &&
                 !IS_SET( victim->comm, COMM_QUIET ) )
            {
                act_new( CFG_OOC, ch, argument, victim, TO_VICT, MIN_POS_OOC );
            }
        }
    }

    /* Send to the speaker and add to the last queue */
    act_new( CFG_OOC, ch, argument, ch, TO_CHAR, MIN_POS_OOC );
    add2last( CFG_OOC, argument, ch );
}

void do_immtalk( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    /* No argument - toggle on/off */
    if ( argument[0] == '\0' )
    {
        if ( IS_SET( ch->comm, COMM_NOWIZ ) )
        {
            REMOVE_BIT( ch->comm, COMM_NOWIZ );
            send_to_char( "`0Immortal channel is now `GON`0.\n\r", ch );
        }
        else
        {
            SET_BIT( ch->comm, COMM_NOWIZ );
            send_to_char( "`0Immortal channel is now `ROFF`0.\n\r", ch );
        }

        return;
    }

    /* Check for channel being OFF */
    if ( IS_SET( ch->comm, COMM_NOWIZ ) )
    {
        send_to_char
            ( "`0Cannot send to the Immortal channel when you have it turned off!\n\r",
              ch );
        return;
    }

    /* Check for quiet mode */
    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
        send_to_char
            ( "`0Cannot send to the Immortal channel without turning off quiet mode!\n\r",
              ch );
        return;
    }

    /* Check for nochannel */
    if ( IS_SET( ch->comm, COMM_NOCHANNELS ) )
    {
        send_to_char
            ( "The gods have revoked all of your channel priviliges!\n\r", ch );
        return;
    }

    /* Send to everyone that should hear it */
    /* Loop through entire descriptor_list to catch anyone that's switched */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->character;

        if ( victim )
        {
            if ( victim != ch &&
                 d->connected == CON_PLAYING &&
                 !IS_SET( victim->comm, COMM_NOWIZ ) &&
                 !IS_SET( victim->comm, COMM_DEAF ) &&
                 !IS_SET( victim->comm, COMM_QUIET ) &&
                 can_do_immcmd( victim, "immtalk" ) )
            {
                act_new( CFG_IMM, ch, argument, victim, TO_VICT, POS_DEAD );
            }
        }
    }

    /* Send to the speaker and add to the last queue */
    act_new( CFG_IMM, ch, argument, ch, TO_CHAR, POS_DEAD );
    add2last_imm( CFG_IMM, argument, ch );
}

#ifdef USE_ADMINTALK
void do_admintalk( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    /* No argument - toggle on/off */
    if ( argument[0] == '\0' )
    {
        if ( IS_SET( ch->comm, COMM_NOADMIN ) )
        {
            REMOVE_BIT( ch->comm, COMM_NOADMIN );
            send_to_char( "`0Admin channel is now `GON`0.\n\r", ch );
        }
        else
        {
            SET_BIT( ch->comm, COMM_NOADMIN );
            send_to_char( "`0Admin channel is now `ROFF`0.\n\r", ch );
        }

        return;
    }

    /* Check for channel being OFF */
    if ( IS_SET( ch->comm, COMM_NOADMIN ) )
    {
        send_to_char
            ( "`0Cannot send to the Admin channel when you have it turned off!\n\r",
              ch );
        return;
    }

    /* Check for quiet mode */
    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
        send_to_char
            ( "`0Cannot send to the Admin channel without turning off quiet mode!\n\r",
              ch );
        return;
    }

    /* Check for nochannel */
    if ( IS_SET( ch->comm, COMM_NOCHANNELS ) )
    {
        send_to_char
            ( "The gods have revoked all of your channel priviliges!\n\r", ch );
        return;
    }

    /* Send to everyone that should hear it */
    /* Loop through entire descriptor_list to catch anyone that's switched */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->character;

        if ( victim )
        {
            if ( victim != ch &&
                 d->connected == CON_PLAYING &&
                 !IS_SET( victim->comm, COMM_NOADMIN ) &&
                 !IS_SET( victim->comm, COMM_DEAF ) &&
                 !IS_SET( victim->comm, COMM_QUIET ) &&
                 can_do_immcmd( victim, "admintalk" ) )
            {
                act_new( CFG_ADMIN, ch, argument, victim, TO_VICT, POS_DEAD );
            }
        }
    }

    /* Send to the speaker and add to the last queue */
    act_new( CFG_ADMIN, ch, argument, ch, TO_CHAR, POS_DEAD );
    add2last_admin( CFG_ADMIN, argument, ch );
}
#endif
#ifdef USE_HEROTALK
void do_herotalk( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    /* No argument - toggle on/off */
    if ( argument[0] == '\0' )
    {
        if ( IS_SET( ch->comm, COMM_NOHERO ) )
        {
            REMOVE_BIT( ch->comm, COMM_NOHERO );
            send_to_char( "`0HERO channel is now `GON`0.\n\r", ch );
        }
        else
        {
            SET_BIT( ch->comm, COMM_NOHERO );
            send_to_char( "`0HERO channel is now `ROFF`0.\n\r", ch );
        }

        return;
    }

    /* Check for channel being OFF */
    if ( IS_SET( ch->comm, COMM_NOHERO ) )
    {
        send_to_char
            ( "`0Cannot send to the HERO channel when you have it turned off!\n\r",
              ch );
        return;
    }

    /* Check for quiet mode */
    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
        send_to_char
            ( "`0Cannot send to the HERO channel without turning off quiet mode!\n\r",
              ch );
        return;
    }

    /* Check for nochannel */
    if ( IS_SET( ch->comm, COMM_NOCHANNELS ) )
    {
        send_to_char
            ( "The gods have revoked all of your channel priviliges!\n\r", ch );
        return;
    }

    /* Send to everyone that should hear it */
    /* Loop through entire descriptor_list to catch anyone that's switched */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->character;

        if ( victim )
        {
            if ( victim != ch &&
                 d->connected == CON_PLAYING &&
                 !IS_SET( victim->comm, COMM_NOHERO ) &&
                 !IS_SET( victim->comm, COMM_DEAF ) &&
                 !IS_SET( victim->comm, COMM_QUIET ) &&
                 can_do_immcmd( victim, "herotalk" ) )
            {
                act_new( CFG_HERO, ch, argument, victim, TO_VICT, POS_DEAD );
            }
        }
    }

    /* Send to the speaker and add to the last queue */
    act_new( CFG_HERO, ch, argument, ch, TO_CHAR, POS_DEAD );
    add2last_hero( CFG_HERO, argument, ch );
}
#endif

void do_say( CHAR_DATA * ch, char *argument )
{
    if ( argument[0] == '\0' )
    {
        send_to_char( "Say what?\n\r", ch );
        return;
    }

#ifdef SAY_DRUNK
    /* Make the words drunk if needed */
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
        argument = makedrunk( argument, ch );
#endif

    act_new( CFG_SAY, ch, argument, NULL, TO_ROOM, POS_RESTING );
    act_new( CFG_SAY_SELF, ch, argument, NULL, TO_CHAR, POS_RESTING );

    mprog_speech_trigger( argument, ch );

    if ( !ch )
        return;

    rprog_speech_trigger( argument, ch );

    if ( !ch )
        return;

    oprog_speech_trigger( argument, ch );

    return;
}

void do_shout( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( IS_SET( ch->comm, COMM_NOSHOUT ) )
    {
        send_to_char( "You can't shout.\n\r", ch );
        return;
    }

    if ( IS_SET( ch->comm, COMM_DEAF ) )
    {
        send_to_char( "Deaf people can't shout.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Shout what?\n\r", ch );
        return;
    }

    WAIT_STATE( ch, 12 );

#ifdef SHOUT_DRUNK
    /* Make the words drunk if needed */
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
        argument = makedrunk( argument, ch );
#endif

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->character;

        if ( victim != ch &&
             d->connected == CON_PLAYING &&
             !IS_SET( victim->comm, COMM_DEAF ) &&
             !IS_SET( victim->comm, COMM_QUIET ) )
        {
            act_new( CFG_SHOUT, ch, argument, victim, TO_VICT, MIN_POS_SHOUT );
        }
    }

    act_new( CFG_SHOUT_SELF, ch, argument, NULL, TO_CHAR, MIN_POS_SHOUT );
    add2last( CFG_SHOUT, argument, ch );
}

void do_sendinfo( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
        if ( IS_SET( ch->comm, COMM_NOINFO ) )
        {
            send_to_char( "Info channel is now `GON.`w\n\r", ch );
            REMOVE_BIT( ch->comm, COMM_NOINFO );
        }
        else
        {
            send_to_char( "Info channel is now `ROFF.`w\n\r", ch );
            SET_BIT( ch->comm, COMM_NOINFO );
        }

        return;
    }

    /* Loop through entire descriptor_list to catch anyone that's switched */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim = d->character;

        if ( victim )
        {
            if ( d->connected == CON_PLAYING &&
                 !IS_SET( victim->comm, COMM_DEAF ) &&
                 !IS_SET( victim->comm, COMM_NOINFO ) &&
                 !IS_SET( victim->comm, COMM_QUIET ) )
            {
                act_new( CFG_INFO, ch, argument, victim, TO_VICT, POS_DEAD );
            }
        }
    }

    act_new( CFG_INFO, ch, argument, ch, TO_CHAR, POS_DEAD );
    add2last( CFG_INFO, argument, ch );

    return;
}

void do_beep( CHAR_DATA * ch, char *argument )
{
    if ( ch->beep )
    {
        ch->beep = FALSE;
        send_to_char( "`RBeeps are now off\n\r", ch );
        return;
    }
    ch->beep = TRUE;
    send_to_char( "`RBeeps are now on\n\r", ch );
    return;
}

void do_tell( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    bool switched = FALSE;

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
        send_to_char( "Your message didn't get through.\n\r", ch );
        return;
    }

    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
        send_to_char( "You must turn off quiet mode first.\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Tell whom what?\n\r", ch );
        return;
    }

    /*
     * Can tell to PC's anywhere, but NPC's only in same room.
     * -- Furey
     */
    if ( ( victim = get_char_world( ch, arg ) ) == NULL
         || ( IS_NPC( victim ) && victim->in_room != ch->in_room ) )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    /* Check for a dead link */
    if ( victim->desc == NULL && !IS_NPC( victim ) )
    {
        DESCRIPTOR_DATA *d;

        /* Dead link may be because of switch - check that too */
        for ( d = descriptor_list; d != NULL && d->original != victim;
              d = d->next );

        if ( d )
        {
            victim = d->character;
            switched = TRUE;
        }
        else
        {
            act( "$N seems to have misplaced $S link...please try again later.",
                 ch, NULL, victim, TO_CHAR );
            return;
        }
    }

#ifdef TELL_DRUNK
    /* Make the words drunk if needed */
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
        argument = makedrunk( argument, ch );
#endif

    if ( !IS_IMMORTAL( ch ) && ( !IS_AWAKE( victim )
                                 && MIN_POS_TELL == POS_RESTING ) )
    {
        act( "$E can't hear you, but your message has been recorded.",
             ch, 0, victim, TO_CHAR );
        add2queue( ch, victim, argument );
        return;
    }

    if ( ( IS_SET( victim->comm, COMM_QUIET )
           || ( IS_SET( victim->comm, COMM_TELLOFF ) ) ) && !IS_IMMORTAL( ch ) )
    {
        act( "$E is not receiving tells, but your message has been recorded.",
             ch, 0, victim, TO_CHAR );
        add2queue( ch, victim, argument );
        return;
    }

    if ( IS_SET( victim->act, PLR_AFK ) )
    {
        act( "$E is AFK and can't hear you, but your message has been recorded.", ch, 0, victim, TO_CHAR );
        add2queue( ch, victim, argument );
    }

    if ( switched && IS_SET( victim->desc->original->act, PLR_AFK ) )
    {
        act( "$E is AFK and can't hear you, but your message has been recorded.", ch, 0, victim->desc->original, TO_CHAR );
        add2queue( ch, victim->desc->original, argument );
    }

    act_new( COLOR_TELL "`WYou tell $N`Y '$t" COLOR_TELL "`Y'`w", ch, argument,
             switched ? victim->desc->original : victim, TO_CHAR,
             MIN_POS_TELL );

    if ( victim->beep )
        act_new( COLOR_TELL "`W$n \atells you, `Y  '$t" COLOR_TELL "`Y'`w", ch,
                 argument, victim, TO_VICT, MIN_POS_TELL );
    else
        act_new( COLOR_TELL "`W$n tells you, `Y '$t" COLOR_TELL "`Y'`w", ch,
                 argument, victim, TO_VICT, MIN_POS_TELL );

    sprintf( buf, COLOR_TELL "`W%s tells you, `Y'%s" COLOR_TELL "`Y'`w\n\r",
             ch->name, argument );
    add2tell( ch, switched ? victim->desc->original : victim, FALSE, argument );

    victim->reply = ch;
    return;
}

void do_reply( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
        send_to_char( "Your message didn't get through.\n\r", ch );
        return;
    }

    if ( ( victim = ch->reply ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim->desc == NULL && !IS_NPC( victim ) )
    {
        act( "$N seems to have misplaced $S link...please try again later.",
             ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( !IS_IMMORTAL( ch ) && ( !IS_AWAKE( victim )
                                 && MIN_POS_TELL == POS_RESTING ) )
    {
        act( "$E can't hear you, but your message has been recorded.",
             ch, 0, victim, TO_CHAR );
        add2queue( ch, victim, argument );
        return;
    }

    if ( IS_SET( victim->comm, COMM_QUIET ) && !IS_IMMORTAL( ch ) )
    {
        act( "$E is not receiving tells, but your message has been recorded.",
             ch, 0, victim, TO_CHAR );
        add2queue( ch, victim, argument );
        return;
    }

    if ( IS_SET( victim->act, PLR_AFK ) )
    {
        act( "$E is AFK and can't hear you, but your message has been recorded.", ch, 0, victim, TO_CHAR );
        add2queue( ch, victim, argument );
    }

    act_new( COLOR_TELL "`WYou tell $N,`Y '$t" COLOR_TELL "`Y'`w", ch, argument,
             victim, TO_CHAR, MIN_POS_TELL );
    if ( victim->beep )
        act_new( COLOR_TELL "`W$n \atells you,`Y '$t" COLOR_TELL "`Y'`w", ch,
                 argument, victim, TO_VICT, MIN_POS_TELL );
    else
        act_new( COLOR_TELL "`W$n tells you,`Y '$t" COLOR_TELL "`Y'`w", ch,
                 argument, victim, TO_VICT, MIN_POS_TELL );
    sprintf( buf, COLOR_TELL "`W%s tells you,`Y '%s" COLOR_TELL "`Y'`w\n\r",
             ch->name, argument );
    add2tell( ch, victim, FALSE, argument );
    victim->reply = ch;

    return;
}

void add2queue( CHAR_DATA * ch, CHAR_DATA * victim, char *argument )
{
#ifdef DISABLE_MESSAGE_QUEUEING
    return;
#endif
    if IS_NPC
        ( ch ) return;

    if ( victim->pcdata->message == NULL )
    {
        victim->pcdata->message = alloc_mem( sizeof( QUEUE_DATA ) );
        victim->pcdata->message->next = NULL;
        victim->pcdata->message->sender = str_dup( ch->name );
        victim->pcdata->message->text = str_dup( argument );
        victim->pcdata->message->invis = IS_AFFECTED( ch, AFF_INVISIBLE );
        victim->pcdata->message->wizilvl = ch->invis_level;
        victim->pcdata->messages++;
        victim->pcdata->fmessage = victim->pcdata->message;
    }
    else
    {
        victim->pcdata->message->next = alloc_mem( sizeof( QUEUE_DATA ) );
        victim->pcdata->message = victim->pcdata->message->next;
        victim->pcdata->message->next = NULL;
        victim->pcdata->message->sender = str_dup( ch->name );
        victim->pcdata->message->text = str_dup( argument );
        victim->pcdata->message->invis = IS_AFFECTED( ch, AFF_INVISIBLE );
        victim->pcdata->message->wizilvl = ch->invis_level;
        victim->pcdata->messages++;
    }
}

void add2tell( CHAR_DATA * ch, CHAR_DATA * victim, bool group, char *argument )
{
#ifdef DISABLE_MESSAGE_QUEUEING
    return;
#endif
    if IS_NPC
        ( ch ) return;          /* not for NPCs */

    if ( victim->pcdata == NULL )
        return;
    /* Only happens when vict is a switched imm I think */

    if ( victim->pcdata->tells + 1 > MAX_LAST_LENGTH )
    {
        QUEUE_DATA *tmp;
        tmp = victim->pcdata->ftell_q;
        victim->pcdata->ftell_q = victim->pcdata->ftell_q->next;
        victim->pcdata->tells--;
        free_mem( &tmp );
    }

    if ( victim->pcdata->tell_q == NULL )
    {
        victim->pcdata->tell_q = alloc_mem( sizeof( QUEUE_DATA ) );
        victim->pcdata->tell_q->next = NULL;
        victim->pcdata->tell_q->sender = str_dup( ch->name );
        victim->pcdata->tell_q->text = str_dup( argument );
        victim->pcdata->tell_q->invis = IS_AFFECTED( ch, AFF_INVISIBLE );
        victim->pcdata->tell_q->wizilvl = ch->invis_level;
        victim->pcdata->tell_q->group = group;
        victim->pcdata->tells++;
        victim->pcdata->ftell_q = victim->pcdata->tell_q;
    }
    else
    {
        victim->pcdata->tell_q->next = alloc_mem( sizeof( QUEUE_DATA ) );
        victim->pcdata->tell_q = victim->pcdata->tell_q->next;
        victim->pcdata->tell_q->next = NULL;
        victim->pcdata->tell_q->sender = str_dup( ch->name );
        victim->pcdata->tell_q->text = str_dup( argument );
        victim->pcdata->tell_q->invis = IS_AFFECTED( ch, AFF_INVISIBLE );
        victim->pcdata->tell_q->wizilvl = ch->invis_level;
        victim->pcdata->tell_q->group = group;
        victim->pcdata->tells++;
    }
}

void add2last( char *format, char *message, CHAR_DATA * ch )
{
    static int last_length = 0;
    LAST_DATA *tmp;

#if defined(cbuilder)
    ChannelMessage( argument, ch );
#endif

    if ( last_length == MAX_LAST_LENGTH )
    {
        tmp = flast;
        flast = tmp->next;
        last_list->next = tmp;
        last_list = tmp;
        if ( last_list->msg )
            free_string( &last_list->msg );
        if ( last_list->format )
            free_string( &last_list->format );
        if ( last_list->sender )
            free_string( &last_list->sender );
    }
    else
    {
        tmp = alloc_mem( sizeof( LAST_DATA ) );
        if ( last_list != NULL )
            last_list->next = tmp;
        last_list = tmp;
        if ( flast == NULL )
            flast = tmp;
        last_length++;
    }

    last_list->next = NULL;
    last_list->format = str_dup( format );
    last_list->msg = str_dup( message );

    if ( ch != NULL )
    {
        last_list->sender =
            str_dup( IS_NPC( ch ) ? ch->short_descr : ch->name );
        last_list->invis = IS_AFFECTED( ch, AFF_INVISIBLE );
        last_list->wizilvl = ch->invis_level;
    }
    else
    {
        last_list->sender = str_dup( " " );
        last_list->invis = FALSE;
        last_list->wizilvl = 0;
    }

    return;
}

void add2last_imm( char *format, char *message, CHAR_DATA * ch )
{
    static int last_imm_length = 0;
    LAST_DATA *tmp;

#if defined(cbuilder)
    ChannelMessage( argument, ch );
#endif

    if ( last_imm_length == MAX_LAST_LENGTH )
    {
        tmp = flast_imm;
        flast_imm = tmp->next;
        last_imm->next = tmp;
        last_imm = tmp;
        if ( last_imm->msg )
            free_string( &last_imm->msg );
        if ( last_imm->format )
            free_string( &last_imm->format );
        if ( last_imm->sender )
            free_string( &last_imm->sender );
    }
    else
    {
        tmp = alloc_mem( sizeof( LAST_DATA ) );

        if ( last_imm != NULL )
        {
            last_imm->next = tmp;
        }

        last_imm = tmp;

        if ( flast_imm == NULL )
        {
            flast_imm = last_imm;
        }

        ++last_imm_length;
    }

    last_imm->next = NULL;
    last_imm->msg = str_dup( message );
    last_imm->format = str_dup( format );
    last_imm->sender = str_dup( IS_NPC( ch ) ? ch->short_descr : ch->name );
    last_imm->invis = FALSE;
    last_imm->wizilvl = IS_NPC( ch ) ? 0 : ch->invis_level;
}

#ifdef USE_ADMINTALK
void add2last_admin( char *format, char *message, CHAR_DATA * ch )
{
    static int last_admin_length = 0;
    LAST_DATA *tmp;

#if defined(cbuilder)
    ChannelMessage( argument, ch );
#endif

    if ( last_admin_length == MAX_LAST_LENGTH )
    {
        tmp = flast_admin;
        flast_admin = tmp->next;
        last_admin->next = tmp;
        last_admin = tmp;
        if ( last_admin->msg )
            free_string( &last_admin->msg );
        if ( last_admin->format )
            free_string( &last_admin->format );
        if ( last_admin->sender )
            free_string( &last_admin->sender );
    }
    else
    {
        tmp = alloc_mem( sizeof( LAST_DATA ) );

        if ( last_admin != NULL )
        {
            last_admin->next = tmp;
        }

        last_admin = tmp;

        if ( flast_admin == NULL )
        {
            flast_admin = last_admin;
        }

        ++last_admin_length;
    }

    last_admin->next = NULL;
    last_admin->msg = str_dup( message );
    last_admin->format = str_dup( format );
    last_admin->sender = str_dup( IS_NPC( ch ) ? ch->short_descr : ch->name );
    last_admin->invis = FALSE;
    last_admin->wizilvl = IS_NPC( ch ) ? 0 : ch->invis_level;
}
#endif
#ifdef USE_HEROTALK
void add2last_hero( char *format, char *message, CHAR_DATA * ch )
{
    static int last_hero_length = 0;
    LAST_DATA *tmp;

#if defined(cbuilder)
    ChannelMessage( argument, ch );
#endif

    if ( last_hero_length == MAX_LAST_LENGTH )
    {
        tmp = flast_hero;
        flast_hero = tmp->next;
        last_hero->next = tmp;
        last_hero = tmp;
        if ( last_hero->msg )
            free_string( &last_hero->msg );
        if ( last_hero->format )
            free_string( &last_hero->format );
        if ( last_hero->sender )
            free_string( &last_hero->sender );
    }
    else
    {
        tmp = alloc_mem( sizeof( LAST_DATA ) );

        if ( last_hero != NULL )
        {
            last_hero->next = tmp;
        }

        last_hero = tmp;

        if ( flast_hero == NULL )
        {
            flast_hero = last_hero;
        }

        ++last_hero_length;
    }

    last_hero->next = NULL;
    last_hero->msg = str_dup( message );
    last_hero->format = str_dup( format );
    last_hero->sender = str_dup( IS_NPC( ch ) ? ch->short_descr : ch->name );
    last_hero->invis = FALSE;
    last_hero->wizilvl = IS_NPC( ch ) ? 0 : ch->invis_level;
}
#endif

void do_messages( CHAR_DATA * ch, char *argument )
{
    QUEUE_DATA *mq;
    CHAR_DATA  *reply = NULL;
    char buf[MAX_STRING_LENGTH];
    char *sender = NULL;

    if IS_NPC
        ( ch ) return;

    if ( ch->pcdata->fmessage == NULL )
    {
        send_to_char( "You have no messages waiting.\n\r", ch );
        return;
    }

    if ( ch->pcdata->fmessage->wizilvl > get_trust( ch ) )
        sender = "Someone";
    else if ( ch->pcdata->fmessage->invis
              && !IS_AFFECTED( ch, AFF_DETECT_INVIS )
              && !IS_SET( ch->act, PLR_HOLYLIGHT ) )
        sender = "Someone";
    else
        sender = ch->pcdata->fmessage->sender;

    sprintf( buf, "`W%s tells you`Y '%s'`w\n\r", sender,
             ch->pcdata->fmessage->text );
    send_to_char( buf, ch );

    /* Search for the sender's CHAR_DATA.
       Look for an exact match on name instead of using
       get_player_world() / get_char_world() to ensure
       that replies go to the right place. */

    /* Search player_list first 'cause it's shorter and
       it's the most likely place to find a match */
    for ( reply = player_list; reply != NULL; reply = reply->next )
    {
        if ( str_cmp( reply->name, ch->pcdata->fmessage->sender ) == 0 )
        {
            ch->reply = reply;
            break;
        }
    }

    if ( reply == NULL )
    {
        for ( reply = char_list; reply != NULL; reply = reply->next )
        {
            if ( str_cmp( reply->name, ch->pcdata->fmessage->sender ) == 0 )
            {
                ch->reply = reply;
                break;
            }
        }
    }

    /* If no match is found, set ch->reply to NULL */
    if ( reply == NULL ) ch->reply = NULL;

    mq = ch->pcdata->fmessage;
    ch->pcdata->fmessage = ch->pcdata->fmessage->next;
    free_string( &mq->text );
    free_string( &mq->sender );
    free_mem( &mq );
    if ( !ch->pcdata->fmessage )
        ch->pcdata->message = NULL;

    ch->pcdata->messages--;
    if ( ch->pcdata->messages > 0 )
    {
        sprintf( buf, "You still have `C%d`w message%s in your queue.\n\r",
                 ch->pcdata->messages,
                 ( ch->pcdata->messages > 1 ) ? "s" : "" );
        send_to_char( buf, ch );
    }

    return;
}

void do_tq( CHAR_DATA * ch, char *argument )
{
    QUEUE_DATA *mq;
    char buf[MAX_STRING_LENGTH];
    char *sender = NULL;

    if IS_NPC
        ( ch ) return;

    if ( ch->pcdata->ftell_q == NULL )
    {
        send_to_char( "You have recieved no private messages.\n\r", ch );
        return;
    }

    for ( mq = ch->pcdata->ftell_q; mq != NULL; mq = mq->next )
    {
        if ( mq->wizilvl > get_trust( ch ) )
            sender = "Someone";
        else if ( mq->invis && !IS_AFFECTED( ch, AFF_DETECT_INVIS )
                  && !IS_SET( ch->act, PLR_HOLYLIGHT ) )
            sender = "Someone";
        else
            sender = mq->sender;

        if ( mq->group )
        {
            sprintf( buf, "`C%s tells the group '%s`C'.\n\r`w",
                     sender, mq->text );
        }
        else
        {
            sprintf( buf,
                     COLOR_TELL "`W%s tells you, `Y'%s" COLOR_TELL "`Y'`w\n\r",
                     sender, mq->text );
        }

        send_to_char( buf, ch );
    }

    return;
}

void do_last( CHAR_DATA * ch, char *argument )
{
    LAST_DATA *tmp_last;
    char *sender;
    char *format;
    char buf[MAX_STRING_LENGTH];

    if ( last_list == NULL )
    {
        send_to_char( "Nothing has been recorded yet.\n\r", ch );
        return;
    }

    for ( tmp_last = flast; tmp_last != NULL; tmp_last = tmp_last->next )
    {
        if ( tmp_last->wizilvl > get_trust( ch ) )
            sender = "Someone";
        else if ( tmp_last->invis && !IS_AFFECTED( ch, AFF_DETECT_INVIS )
                  && !IS_SET( ch->act, PLR_HOLYLIGHT ) )
            sender = "Someone";
        else
            sender = tmp_last->sender;

        /* Yeah, this parser sucks, but I was in a hurry */
        memset( buf, 0, MAX_STRING_LENGTH );

        format = tmp_last->format;

        while ( *format )
        {
            if ( *format == '$' )
            {
                format++;

                if ( *format == 'n' )
                {
                    strcat( buf, sender );
                }
                else if ( *format == 't' )
                {
                    strcat( buf, tmp_last->msg );
                }
                else
                {
                    buf[strlen( buf ) - 1] = '$';
                    buf[strlen( buf ) - 1] = *format;
                }
            }
            else
            {
                buf[strlen( buf )] = *format;
            }

            format++;
        }

        printf_to_char( ch, "%s\n\r", buf );
    }
}

void do_lastimm( CHAR_DATA * ch, char *argument )
{
    LAST_DATA *tmp_last;
    char *format;
    char *sender;
    char buf[MAX_STRING_LENGTH];

    if ( last_imm == NULL )
    {
        send_to_char( "Nothing has been recorded yet.\n\r", ch );
        return;
    }

    for ( tmp_last = flast_imm; tmp_last != NULL; tmp_last = tmp_last->next )
    {
        if ( tmp_last->wizilvl > get_trust( ch ) )
            sender = "Someone";
        else if ( tmp_last->invis && !IS_AFFECTED( ch, AFF_DETECT_INVIS )
                  && !IS_SET( ch->act, PLR_HOLYLIGHT ) )
            sender = "Someone";
        else
            sender = tmp_last->sender;

        memset( buf, 0, MAX_STRING_LENGTH );

        format = tmp_last->format;

        while ( *format )
        {
            if ( *format == '$' )
            {
                format++;

                if ( *format == 'n' )
                {
                    strcat( buf, sender );
                }
                else if ( *format == 't' )
                {
                    strcat( buf, tmp_last->msg );
                }
                else
                {
                    buf[strlen( buf ) - 1] = '$';
                    buf[strlen( buf ) - 1] = *format;
                }
            }
            else
            {
                buf[strlen( buf )] = *format;
            }

            format++;
        }

        printf_to_char( ch, "%s\n\r", buf );
    }
}

#ifdef USE_ADMINTALK
void do_lastadmin( CHAR_DATA * ch, char *argument )
{
    LAST_DATA *tmp_last;
    char *format;
    char *sender;
    char buf[MAX_STRING_LENGTH];

    if ( last_admin == NULL )
    {
        send_to_char( "Nothing has been recorded yet.\n\r", ch );
        return;
    }

    for ( tmp_last = flast_admin; tmp_last != NULL; tmp_last = tmp_last->next )
    {
        if ( tmp_last->wizilvl > get_trust( ch ) )
            sender = "Someone";
        else if ( tmp_last->invis && !IS_AFFECTED( ch, AFF_DETECT_INVIS )
                  && !IS_SET( ch->act, PLR_HOLYLIGHT ) )
            sender = "Someone";
        else
            sender = tmp_last->sender;

        memset( buf, 0, MAX_STRING_LENGTH );

        format = tmp_last->format;

        while ( *format )
        {
            if ( *format == '$' )
            {
                format++;

                if ( *format == 'n' )
                {
                    strcat( buf, sender );
                }
                else if ( *format == 't' )
                {
                    strcat( buf, tmp_last->msg );
                }
                else
                {
                    buf[strlen( buf ) - 1] = '$';
                    buf[strlen( buf ) - 1] = *format;
                }
            }
            else
            {
                buf[strlen( buf )] = *format;
            }

            format++;
        }

        printf_to_char( ch, "%s\n\r", buf );
    }
}
#endif
#ifdef USE_HEROTALK
void do_lasthero( CHAR_DATA * ch, char *argument )
{
    LAST_DATA *tmp_last;
    char *format;
    char *sender;
    char buf[MAX_STRING_LENGTH];

    if ( last_hero == NULL )
    {
        send_to_char( "Nothing has been recorded yet.\n\r", ch );
        return;
    }

    for ( tmp_last = flast_hero; tmp_last != NULL; tmp_last = tmp_last->next )
    {
        if ( tmp_last->wizilvl > get_trust( ch ) )
            sender = "Someone";
        else if ( tmp_last->invis && !IS_AFFECTED( ch, AFF_DETECT_INVIS )
                  && !IS_SET( ch->act, PLR_HOLYLIGHT ) )
            sender = "Someone";
        else
            sender = tmp_last->sender;

        memset( buf, 0, MAX_STRING_LENGTH );

        format = tmp_last->format;

        while ( *format )
        {
            if ( *format == '$' )
            {
                format++;

                if ( *format == 'n' )
                {
                    strcat( buf, sender );
                }
                else if ( *format == 't' )
                {
                    strcat( buf, tmp_last->msg );
                }
                else
                {
                    buf[strlen( buf ) - 1] = '$';
                    buf[strlen( buf ) - 1] = *format;
                }
            }
            else
            {
                buf[strlen( buf )] = *format;
            }

            format++;
        }

        printf_to_char( ch, "%s\n\r", buf );
    }
}
#endif

void do_yell( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( IS_SET( ch->comm, COMM_NOSHOUT ) )
    {
        send_to_char( "You can't yell.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Yell what?\n\r", ch );
        return;
    }

#ifdef YELL_DRUNK
    /* Make the words drunk if needed */
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
        argument = makedrunk( argument, ch );
#endif

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->character != ch &&
             d->connected == CON_PLAYING &&
             d->character->in_room != NULL &&
             d->character->in_room->area == ch->in_room->area &&
             !IS_SET( d->character->comm, COMM_QUIET ) &&
             !IS_SET( d->character->comm, COMM_DEAF ) )
        {
            act_new( CFG_YELL, ch, argument, d->character, TO_VICT,
                     MIN_POS_YELL );
        }
    }

    act_new( CFG_YELL_SELF, ch, argument, NULL, TO_CHAR, MIN_POS_YELL );
    return;
}

void do_emote( CHAR_DATA * ch, char *argument )
{
    if ( !IS_NPC( ch ) && IS_SET( ch->comm, COMM_NOEMOTE ) )
    {
        send_to_char( "You can't emote.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Emote what?\n\r", ch );
        return;
    }

#ifdef EMOTE_DRUNK              /* Added because a TR player uses emote instead of say as an RP Thing */
    /* Make the words drunk if needed */
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
        argument = makedrunk( argument, ch );
#endif

    act( "$n $T`w", ch, NULL, argument, TO_ROOM );
    act( "$n $T`w", ch, NULL, argument, TO_CHAR );
    return;
}

void do_info( CHAR_DATA * ch )
{
    if ( IS_SET( ch->comm, COMM_NOINFO ) )
    {
        send_to_char( "`BInfo channel is now ON.\n\r`w", ch );
        REMOVE_BIT( ch->comm, COMM_NOINFO );
    }
    else
    {
        send_to_char( "`BInfo channel is now OFF.\n\r`w", ch );
        SET_BIT( ch->comm, COMM_NOINFO );
    }
    return;
}

/*
 * All the posing stuff.
 */
struct pose_table_type {
    char *message[2 * MAX_CLASS];
};

const struct pose_table_type pose_table[] = {
    {
     {
      "You sizzle with energy.",
      "$n sizzles with energy.",
      "You feel very holy.",
      "$n looks very holy.",
      "You perform a small card trick.",
      "$n performs a small card trick.",
      "You show your bulging muscles.",
      "$n shows $s bulging muscles."}
     },

    {
     {
      "You turn into a butterfly, then return to your normal shape.",
      "$n turns into a butterfly, then returns to $s normal shape.",
      "You nonchalantly turn wine into water.",
      "$n nonchalantly turns wine into water.",
      "You wiggle your ears alternately.",
      "$n wiggles $s ears alternately.",
      "You crack nuts between your fingers.",
      "$n cracks nuts between $s fingers."}
     },

    {
     {
      "Blue sparks fly from your fingers.",
      "Blue sparks fly from $n's fingers.",
      "A halo appears over your head.",
      "A halo appears over $n's head.",
      "You nimbly tie yourself into a knot.",
      "$n nimbly ties $mself into a knot.",
      "You grizzle your teeth and look mean.",
      "$n grizzles $s teeth and looks mean."}
     },

    {
     {
      "Little red lights dance in your eyes.",
      "Little red lights dance in $n's eyes.",
      "You recite words of wisdom.",
      "$n recites words of wisdom.",
      "You juggle with daggers, apples, and eyeballs.",
      "$n juggles with daggers, apples, and eyeballs.",
      "You hit your head, and your eyes roll.",
      "$n hits $s head, and $s eyes roll."}
     },

    {
     {
      "A slimy green monster appears before you and bows.",
      "A slimy green monster appears before $n and bows.",
      "Deep in prayer, you levitate.",
      "Deep in prayer, $n levitates.",
      "You steal the underwear off every person in the room.",
      "Your underwear is gone!  $n stole it!",
      "Crunch, crunch -- you munch a bottle.",
      "Crunch, crunch -- $n munches a bottle."}
     },

    {
     {
      "You turn everybody into a little pink elephant.",
      "You are turned into a little pink elephant by $n.",
      "An angel consults you.",
      "An angel consults $n.",
      "The dice roll ... and you win again.",
      "The dice roll ... and $n wins again.",
      "... 98, 99, 100 ... you do pushups.",
      "... 98, 99, 100 ... $n does pushups."}
     },

    {
     {
      "A small ball of light dances on your fingertips.",
      "A small ball of light dances on $n's fingertips.",
      "Your body glows with an unearthly light.",
      "$n's body glows with an unearthly light.",
      "You count the money in everyone's pockets.",
      "Check your money, $n is counting it.",
      "Arnold Schwarzenegger admires your physique.",
      "Arnold Schwarzenegger admires $n's physique."}
     },

    {
     {
      "Smoke and fumes leak from your nostrils.",
      "Smoke and fumes leak from $n's nostrils.",
      "A spot light hits you.",
      "A spot light hits $n.",
      "You balance a pocket knife on your tongue.",
      "$n balances a pocket knife on your tongue.",
      "Watch your feet, you are juggling granite boulders.",
      "Watch your feet, $n is juggling granite boulders."}
     },

    {
     {
      "The light flickers as you rap in magical languages.",
      "The light flickers as $n raps in magical languages.",
      "Everyone levitates as you pray.",
      "You levitate as $n prays.",
      "You produce a coin from everyone's ear.",
      "$n produces a coin from your ear.",
      "Oomph!  You squeeze water out of a granite boulder.",
      "Oomph!  $n squeezes water out of a granite boulder."}
     },

    {
     {
      "Your head disappears.",
      "$n's head disappears.",
      "A cool breeze refreshes you.",
      "A cool breeze refreshes $n.",
      "You step behind your shadow.",
      "$n steps behind $s shadow.",
      "You pick your teeth with a spear.",
      "$n picks $s teeth with a spear."}
     },

    {
     {
      "A fire elemental singes your hair.",
      "A fire elemental singes $n's hair.",
      "The sun pierces through the clouds to illuminate you.",
      "The sun pierces through the clouds to illuminate $n.",
      "Your eyes dance with greed.",
      "$n's eyes dance with greed.",
      "Everyone is swept off their foot by your hug.",
      "You are swept off your feet by $n's hug."}
     },

    {
     {
      "The sky changes color to match your eyes.",
      "The sky changes color to match $n's eyes.",
      "The ocean parts before you.",
      "The ocean parts before $n.",
      "You deftly steal everyone's weapon.",
      "$n deftly steals your weapon.",
      "Your karate chop splits a tree.",
      "$n's karate chop splits a tree."}
     },

    {
     {
      "The stones dance to your command.",
      "The stones dance to $n's command.",
      "A thunder cloud kneels to you.",
      "A thunder cloud kneels to $n.",
      "The Grey Mouser buys you a beer.",
      "The Grey Mouser buys $n a beer.",
      "A strap of your armor breaks over your mighty thews.",
      "A strap of $n's armor breaks over $s mighty thews."}
     },

    {
     {
      "The heavens and grass change colour as you smile.",
      "The heavens and grass change colour as $n smiles.",
      "The Burning Man speaks to you.",
      "The Burning Man speaks to $n.",
      "Everyone's pocket explodes with your fireworks.",
      "Your pocket explodes with $n's fireworks.",
      "A boulder cracks at your frown.",
      "A boulder cracks at $n's frown."}
     },

    {
     {
      "Everyone's clothes are transparent, and you are laughing.",
      "Your clothes are transparent, and $n is laughing.",
      "An eye in a pyramid winks at you.",
      "An eye in a pyramid winks at $n.",
      "Everyone discovers your dagger a centimeter from their eye.",
      "You discover $n's dagger a centimeter from your eye.",
      "Mercenaries arrive to do your bidding.",
      "Mercenaries arrive to do $n's bidding."}
     },

    {
     {
      "A black hole swallows you.",
      "A black hole swallows $n.",
      "Valentine Michael Smith offers you a glass of water.",
      "Valentine Michael Smith offers $n a glass of water.",
      "Where did you go?",
      "Where did $n go?",
      "Four matched Percherons bring in your chariot.",
      "Four matched Percherons bring in $n's chariot."}
     },

    {
     {
      "The world shimmers in time with your whistling.",
      "The world shimmers in time with $n's whistling.",
      "The great gods give you a staff.",
      "The great gods give $n a staff.",
      "Click.",
      "Click.",
      "Atlas asks you to relieve him.",
      "Atlas asks $n to relieve him."}
     }
};

void do_pose( CHAR_DATA * ch, char *argument )
{
    int level;
    int pose;

    if ( IS_NPC( ch ) )
        return;

    level =
        UMIN( ch->level, sizeof( pose_table ) / sizeof( pose_table[0] ) - 1 );
    pose = number_range( 0, level );

    act( pose_table[pose].message[2 * ch->Class + 0], ch, NULL, NULL, TO_CHAR );
    act( pose_table[pose].message[2 * ch->Class + 1], ch, NULL, NULL, TO_ROOM );

    return;
}

void do_bug( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, "%s/%s", sysconfig.area_dir, sysconfig.bug_file );
    append_file( ch, buf, argument );
    send_to_char( "Bug logged.\n\r", ch );
    return;
}

void do_idea( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, "%s/%s", sysconfig.area_dir, sysconfig.idea_file );
    append_file( ch, buf, argument );
    send_to_char( "Idea logged. This is NOT an identify command.\n\r", ch );
    return;
}

void do_typo( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, "%s/%s", sysconfig.area_dir, sysconfig.typo_file );
    append_file( ch, buf, argument );
    send_to_char( "Typo logged.\n\r", ch );
    return;
}

void do_qui( CHAR_DATA * ch, char *argument )
{
    send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
    return;
}

void do_quit( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    DESCRIPTOR_DATA *d_next;
    char buf[MAX_STRING_LENGTH];
    char *name;

    send_to_char( "\n\r", ch );

    if ( IS_NPC( ch ) )
        return;

    if ( ch->position == POS_FIGHTING )
    {
        send_to_char( "No way! You are fighting.\n\r", ch );
        return;
    }

/* Is the player trying to leave while his auction is still goin? Lets
inform him that its not that easy ;) -Lancelight */

    if ( auction_info.high_bidder == ch || auction_info.owner == ch )
    {
        send_to_char( "You still have a stake in the auction!\n\r", ch );
        return;
    }

    if ( ch->position < POS_STUNNED )
    {
        send_to_char( "You're not DEAD yet.\n\r", ch );
        return;
    }
    if ( IS_SET( ch->act, PLR_BUILDING ) );
    {
        REMOVE_BIT( ch->act, PLR_BUILDING );
    }
    send_to_char( CFG_QUIT, ch );
    ch->pcdata->ticks = 0;
    if ( !IS_SET( ch->act, PLR_WIZINVIS ) )
    {
        sprintf( buf, "%s has left the game.", ch->name );
        do_sendinfo( ch, buf );
    }
    act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
    sprintf( log_buf, "%s has quit.", ch->name );
    log_string( log_buf );

#if defined(cbuilder)
    RemoveUser( ch );
#endif

    /*
     * After extract_char the ch is no longer valid!
     */
    save_char_obj( ch );

    /* Free note that might be there somehow */
    if ( ch->pcdata->in_progress )
    {
        free_note( ch->pcdata->in_progress );
    }

    name = strdup( ch->name );
    d = ch->desc;
    extract_char( ch, TRUE );

    if ( d )
    {
        close_socket( d );
    }

    for ( d = descriptor_list; d != NULL; d = d_next )
    {
        CHAR_DATA *tch;

        d_next = d->next;
        tch = d->original ? d->original : d->character;

/*      if ((tch) && (!strcmp(tch->name,name)) && (ch != tch)) */
        if ( ( tch ) && ( !strcmp( tch->name, name ) ) )
        {
            extract_char( tch, TRUE );
            if ( d )
            {
#if defined(cbuilder)
                if ( d->character )
                    RemoveUser( d->character );
#endif
                close_socket( d );
            }
        }
    }

    sprintf( log_buf, "%s left the game", name );
    log_string( log_buf );

    free( name );

    return;
}

void do_save( CHAR_DATA * ch, char *argument )
{
    extern bool chaos;

    if ( IS_NPC( ch ) )
        return;

    if ( chaos )
    {
        send_to_char( "Saving is not allowed during `rC`RH`YA`RO`rS.\n\r`w",
                      ch );
        return;
    }

    save_char_obj( ch );
    send_to_char( "Saving.\n\r", ch );
    return;
}

void do_follow( CHAR_DATA * ch, char *argument )
{
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
    char arg[MAX_INPUT_LENGTH];

    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Follow whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) && ch->master != NULL )
    {
        act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
        return;
    }

    if ( victim == ch )
    {
        if ( ch->master == NULL )
        {
            send_to_char( "You already follow yourself.\n\r", ch );
            return;
        }
        stop_follower( ch );
        return;
    }

    if ( !IS_NPC( victim ) && IS_SET( victim->act, PLR_NOFOLLOW )
         && !IS_HERO( ch ) )
    {
        act( "$N doesn't seem to want any followers.\n\r",
             ch, NULL, victim, TO_CHAR );
        return;
    }

    REMOVE_BIT( ch->act, PLR_NOFOLLOW );

    if ( ch->master != NULL )
        stop_follower( ch );

    add_follower( ch, victim );
    return;
}

void add_follower( CHAR_DATA * ch, CHAR_DATA * master )
{
    if ( ch->master != NULL )
    {
        bug( "Add_follower: non-null master.", 0 );
        return;
    }

    ch->master = master;
    ch->leader = NULL;

    if ( can_see( master, ch ) )
        act( "$n now follows you.", ch, NULL, master, TO_VICT );

    act( "You now follow $N.", ch, NULL, master, TO_CHAR );

    return;
}

void stop_follower( CHAR_DATA * ch )
{
    if ( ch->master == NULL )
    {
        bug( "Stop_follower: null master.", 0 );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
        REMOVE_BIT( ch->affected_by, AFF_CHARM );
        affect_strip( ch, gsn_charm_person );
    }

    if ( can_see( ch->master, ch ) && ch->in_room != NULL )
    {
        act( "$n stops following you.", ch, NULL, ch->master, TO_VICT );
        act( "You stop following $N.", ch, NULL, ch->master, TO_CHAR );
    }
    if ( ch->master->pet == ch )
        ch->master->pet = NULL;

    ch->master = NULL;
    ch->leader = NULL;
    return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA * ch )
{
    CHAR_DATA *pet;

    if ( ( pet = ch->pet ) != NULL )
    {
        stop_follower( pet );
        if ( pet->in_room != NULL )
            act( "$N slowly fades away.", ch, NULL, pet, TO_NOTVICT );
        extract_char( pet, TRUE );
    }
    ch->pet = NULL;

    return;
}

void die_follower( CHAR_DATA * ch )
{
    CHAR_DATA *fch;

    if ( ch->master != NULL )
    {
        if ( ch->master->pet == ch )
            ch->master->pet = NULL;
        stop_follower( ch );
    }

    ch->leader = NULL;

    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
        if ( fch->master == ch )
            stop_follower( fch );
        if ( fch->leader == ch )
            fch->leader = fch;
    }

    return;
}

/* Order modifications made by Raven (Laurie Zenner) */
void do_order( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    char arg2[MAX_INPUT_LENGTH];
    char cmd_vi[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *och;
    CHAR_DATA *och_next;
    bool found;
    bool fAll;
    bool fUnto;

    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Order whom to do what?\n\r", ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
        send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        fAll = TRUE;
        victim = NULL;
    }
    else
    {
        fAll = FALSE;
        if ( ( victim = get_char_room( ch, arg ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }

        if ( victim == ch )
        {
            send_to_char( "Aye aye, right away!\n\r", ch );
            return;
        }

        if ( !IS_AFFECTED( victim, AFF_CHARM ) || victim->master != ch )
        {
            send_to_char( "Do it yourself!\n\r", ch );
            return;
        }
    }

    fUnto = FALSE;
    found = FALSE;
    for ( och = ch->in_room->people; och != NULL; och = och_next )
    {
        och_next = och->next_in_room;

        if ( IS_AFFECTED( och, AFF_CHARM )
             && och->master == ch && ( fAll || och == victim ) )
        {
            found = TRUE;
            strcpy( cmd_vi, argument );
            chk_command( och, cmd_vi );
            if ( cmd_vi[0] == '\0' )
                act( "$N is unable to comply with your order.", ch, NULL, och,
                     TO_CHAR );

            else if ( ( !strcmp( cmd_vi, "delete" ) ) ||
                      ( !strcmp( cmd_vi, "quit" ) ) ||
                      ( !strcmp( cmd_vi, "password" ) ) ||
                      ( !strcmp( cmd_vi, "pk" ) ) )
            {
                act( "I don't think $N appreciated that.", ch, NULL, och,
                     TO_CHAR );
                sprintf( buf, "WATCH PLAYER!: %s attempted to make %s %s!",
                         ch->name, och->name, cmd_vi );
                log_string( buf );

                stop_follower( och );
                if ( !IS_NPC( och ) )
                    fUnto = TRUE;
            }
            else if ( ( ( !strcmp( cmd_vi, "advance" ) ) && ( IS_NPC( och ) ) )
                      || ( !strncmp( cmd_vi, "mp", 2 ) ) )
            {
                sprintf( buf, "WATCH PLAYER!: %s attempted to make %s %s!",
                         ch->name, och->name, cmd_vi );
                log_string( buf );

                act( "$N is unable to comply with your order.", ch, NULL, och,
                     TO_CHAR );

            }
            else
            {
                if ( !IS_NPC( och ) )
                {
                    sprintf( buf, "%s ordered %s to '%s'", ch->name, och->name,
                             argument );
                    log_string( buf );
                }
                sprintf( buf, "$n orders you to '%s'.", argument );
                act( buf, ch, NULL, och, TO_VICT );
                act( "$N does $S best to comply to your order.", ch, NULL, och,
                     TO_CHAR );
                interpret( och, argument );
            }
        }
    }

    if ( !found )
        send_to_char( "You have no followers here.\n\r", ch );

    if ( fUnto )
    {
        /* If I feel especially mean spirited one day, I'll check to see
         * if they tried to changed another's password and, if so, change
         * their's to some random string.
         */
        send_to_char
            ( "The warders of this land have a perverse delight in poetic justice./n/r",
              ch );
        interpret( ch, argument );
    }

    tail_chain(  );
    return;
}

void do_group( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];

    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        CHAR_DATA *gch;
        CHAR_DATA *leader;

        leader = ( ch->leader != NULL ) ? ch->leader : ch;
        sprintf( buf, "`K[`W%s's group`K]`w\n\r", PERS( leader, ch ) );
        send_to_char( buf, ch );

        for ( gch = char_list; gch != NULL; gch = gch->next )
        {
            if ( is_same_group( gch, ch ) )
            {
                sprintf( buf,
                         "`K[`W%3d `G%s`K] `w%-16s `W%4d`K/`W%4d hp %4d`K/`W%4d mana %4d`K/`W%4d mv %5ld xp\n\r",
                         gch->level,
                         IS_NPC( gch ) ? "Mob" : class_table[gch->Class].
                         who_name, capitalize( PERS( gch, ch ) ), gch->hit,
                         gch->max_hit, gch->mana, gch->max_mana, gch->move,
                         gch->max_move, gch->exp );
                send_to_char( buf, ch );
            }
        }
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) )
    {
        send_to_char( "But you are following someone else!\n\r", ch );
        return;
    }

    if ( victim->master != ch && ch != victim )
    {
        act( "$N isn't following you.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( IS_AFFECTED( victim, AFF_CHARM ) )
    {
        send_to_char( "You can't remove charmed mobs from your group.\n\r",
                      ch );
        return;
    }

    if ( IS_AFFECTED( ch, AFF_CHARM ) )
    {
        act( "You like your master too much to leave $m!", ch, NULL, victim,
             TO_VICT );
        return;
    }

    if ( is_same_group( victim, ch ) && ch != victim )
    {
        victim->leader = NULL;
        act( "$n removes $N from $s group.", ch, NULL, victim, TO_NOTVICT );
        act( "$n removes you from $s group.", ch, NULL, victim, TO_VICT );
        act( "You remove $N from your group.", ch, NULL, victim, TO_CHAR );
        return;
    }

    victim->leader = ch;
    act( "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
    act( "You join $n's group.", ch, NULL, victim, TO_VICT );
    act( "$N joins your group.", ch, NULL, victim, TO_CHAR );
    return;
}

/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    CHAR_DATA *gch;
    int members;
    int amount;
    int share;
    int extra;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Split how much?\n\r", ch );
        return;
    }

    amount = atoi( arg );

    if ( amount < 0 )
    {
        send_to_char( "Your group wouldn't like that.\n\r", ch );
        return;
    }

    if ( amount == 0 )
    {
        send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
        return;
    }

    if ( ch->gold < amount )
    {
        send_to_char( "You don't have that much gold.\n\r", ch );
        return;
    }

    members = 0;
    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( is_same_group( gch, ch ) && !IS_AFFECTED( gch, AFF_CHARM ) )
            members++;
    }

    if ( members < 2 )
    {
        send_to_char( "Just keep it all.\n\r", ch );
        return;
    }

    share = amount / members;
    extra = amount % members;

    if ( share == 0 )
    {
        send_to_char( "Don't even bother, cheapskate.\n\r", ch );
        return;
    }

    ch->gold -= amount;
    ch->gold += share + extra;

    sprintf( buf,
             "You split %d gold coins.  Your share is %d gold coins.\n\r",
             amount, share + extra );
    send_to_char( buf, ch );

    sprintf( buf, "$n splits %d gold coins.  Your share is %d gold coins.",
             amount, share );

    for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
    {
        if ( gch != ch && is_same_group( gch, ch )
             && !IS_AFFECTED( gch, AFF_CHARM ) )
        {
            act( buf, ch, NULL, gch, TO_VICT );
            gch->gold += share;
        }
    }

    return;
}

void do_gtell( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    CHAR_DATA *gch;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Tell your group what?\n\r", ch );
        return;
    }

    if ( IS_SET( ch->comm, COMM_NOTELL ) )
    {
        send_to_char( "Your message didn't get through!\n\r", ch );
        return;
    }

#ifdef GTELL_DRUNK
    /* Make the words drunk if needed */
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
        argument = makedrunk( argument, ch );
#endif

    /*
     * Note use of send_to_char, so gtell works on sleepers.
     */
    sprintf( buf, "`C%s tells the group '%s`C'.\n\r`w", ch->name, argument );
    add2tell( ch, ch, TRUE, argument );
    for ( gch = char_list; gch != NULL; gch = gch->next )
    {
        if ( is_same_group( gch, ch ) )
        {
            send_to_char( buf, gch );
            add2tell( ch, gch, TRUE, argument );
        }
    }

    return;
}

/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA * ach, CHAR_DATA * bch )
{
    if ( ach->leader != NULL )
        ach = ach->leader;
    if ( bch->leader != NULL )
        bch = bch->leader;
    return ach == bch;
}

void do_spousetalk( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Umm... Who exactly would mary YOU?", ch );
        return;
    }

    if ( ch->pcdata->spousec == 0 )
    {
        send_to_char( "You aren't even married!\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
        if ( IS_SET( ch->comm, COMM_NOSPOUSETALK ) )
        {
            send_to_char( "`BSpouse channel is now ON.\n\r`w", ch );
            REMOVE_BIT( ch->comm, COMM_NOSPOUSETALK );
        }
        else
        {
            send_to_char( "`BSpouse channel is now OFF.\n\r`w", ch );
            SET_BIT( ch->comm, COMM_NOSPOUSETALK );
        }
        return;
    }

    if ( IS_SET( ch->comm, COMM_QUIET ) )
    {
        send_to_char( "You must turn off quiet mode first.\n\r", ch );
        return;
    }

    if ( IS_SET( ch->comm, COMM_DEAF ) )
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

    /* If they've turned off spouse talk, turn it back on */
    REMOVE_BIT( ch->comm, COMM_NOSPOUSETALK );

#ifdef SPOUSE_DRUNK
    /* Make the words drunk if needed */
    if ( !IS_NPC( ch ) && ch->pcdata->condition[COND_DRUNK] > 10 )
        argument = makedrunk( argument, ch );
#endif

    /*
     * Look through all the currently connected players for the player's
     * spouse.  If we don't find the spouse online let the calling player
     * know.
     */
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        CHAR_DATA *victim;

        victim = d->original ? d->original : d->character;

        /*
         * If we've got a match on the name then check the different statuses
         * and if there's a reason why the spouse can't recieve the message
         * notify the sender.
         */
        if ( victim != ch && d->connected == CON_PLAYING &&
             is_name( victim->pcdata->spouse, ch->name ) )
        {
            if ( IS_SET( victim->comm, COMM_NOSPOUSETALK ) )
            {
                send_to_char
                    ( "Your spouse has their spouse channel turned off, maybe you two should talk?\n\r",
                      ch );
                return;
            }

            if ( IS_SET( victim->comm, COMM_DEAF ) )
            {
                send_to_char
                    ( "Your spouse is having a hearing problem at the moment.\n\r",
                      ch );
                return;
            }

            if ( IS_SET( victim->comm, COMM_QUIET ) )
            {
                send_to_char
                    ( "Your spouse is currently being punished by the gods, no communication is allowed.\n\r",
                      ch );
                return;
            }

            if ( IS_NPC( victim ) )
            {
                send_to_char
                    ( "How in the world did you get married to THAT??\n\r",
                      ch );
                return;
            }

            if ( d->character->position < POS_SLEEPING )
            {
                send_to_char( "Your spouse is a bit DEAD at the moment.\n\r",
                              ch );
                return;
            }

            /* Your spouse is available, send the message */
            printf_to_char( victim,
                            "`W{`R%s`W-`YN`W-`R%s`W}`R<`w%s`R>`w %s`w\n\r",
                            ch->name, victim->name, ch->name, argument );
            printf_to_char( ch, "`W{`R%s`W-`YN`W-`R%s`W}`R<`w%s`R>`w %s`w\n\r",
                            ch->name, victim->name, ch->name, argument );

            return;
        }                       /* if playing & name matches */
    }                           /* end descriptor loop */

    /* The spouse wasn't found, let the user know */
    send_to_char
        ( "Your spouse isn't currently online, please try again later.\n\r",
          ch );
}
