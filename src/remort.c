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
 **************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1996 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"

void do_remor( CHAR_DATA * ch, char *argument )
{
    send_to_char( "If you want to REMORT, you must spell it out.\n\r", ch );
    return;
}

void do_remort( CHAR_DATA * ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char strsave[MAX_INPUT_LENGTH], player_name[MAX_INPUT_LENGTH];
    char player_pwd[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
    int player_incarnations;

    if ( IS_NPC( ch ) || ( d = ch->desc ) == NULL )
        return;

    if ( ch->level < LEVEL_HERO )
    {
        sprintf( buf, "You must be level %d.\n\r", LEVEL_HERO );
        send_to_char( buf, ch );
        return;
    }

    if ( ch->level > LEVEL_HERO )
    {
        sprintf( buf,
                 "You must be level %d. So why do you want to that?\n\r",
                 LEVEL_HERO );
        send_to_char( buf, ch );
        return;
    }

    if ( ch->pcdata->confirm_remort )
    {
        if ( argument[0] != '\0' )
        {
            send_to_char( "Remort status removed.\n\r", ch );
            ch->pcdata->confirm_remort = FALSE;
            return;
        }
        else
        {
            /*
             * Get ready to delete the pfile, send a nice informational message.
             */
            sprintf( strsave, "%s/%s", sysconfig.player_dir,
                     capitalize( ch->name ) );
            stop_fighting( ch, TRUE );
            send_to_char
                ( "You have chosen to remort.  You will now be dropped in at the race\n\r",
                  ch );
            send_to_char
                ( "selection section of character creation, and will be allowed to choose from\n\r",
                  ch );
            send_to_char( "a wider selection of races and classes.\n\r\n\r",
                          ch );
            send_to_char
                ( "In the unlikely event that you are disconnected or the MUD\n\r",
                  ch );
            send_to_char
                ( "crashes while you are creating your character, create a new character\n\r",
                  ch );
            send_to_char( "as normal and write a note to 'immortal'.\n\r", ch );
            send_to_char( "\n\r[Hit Enter to Continue]\n\r", ch );
            /*    wiznet( "$N has remorted.", ch, NULL, 0, 0, 0 ); */

            /*
             * I quote:
             * "After extract_char the ch is no longer valid!"
             */
            sprintf( player_name, "%s", capitalize( ch->name ) );
            sprintf( player_pwd, "%s", ch->pcdata->pwd );
            player_incarnations = ++ch->incarnations;
            extract_char( ch, TRUE );

            /*
             * Delete the pfile, but don't boot the character.
             * Instead, do a load_char_obj to get a new ch,
             * saving the password, and the incarnations.  Then,
             * set the PLR_REMORT bit and drop the player in at
             * CON_BEGIN_REMORT.
             */
            unlink( strsave );
            load_char_obj( d, player_name );
            d->character->pcdata->pwd = str_dup( player_pwd );
            d->character->incarnations = player_incarnations;
            if ( !IS_SET( ch->act, PLR_REMORT ) )
                SET_BIT( ch->act, PLR_REMORT );
            d->connected = CON_BEGIN_REMORT;
            return;
        }
    }

    if ( argument[0] != '\0' )
    {
        send_to_char( "Just type remort.  No argument.\n\r", ch );
        return;
    }

    send_to_char( "Type remort again to confirm this command.\n\r", ch );
    send_to_char( "WARNING: This command is irreversible.\n\r", ch );
    send_to_char( "Typing remort with an argument will undo remort status.\n\r",
                  ch );
    ch->pcdata->confirm_remort = TRUE;
    /*  wiznet( "$N is contemplating remorting.",ch,NULL,0,0,get_trust(ch)); */
}
