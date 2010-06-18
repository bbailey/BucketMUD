
#if defined(WIN32)
#include <windows.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"

DECLARE_DO_FUN( do_save );

void do_marry( CHAR_DATA * ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *victim2;

    /*    char buf[MAX_STRING_LENGTH]; */
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ( ch->level >= LEVEL_IMMORTAL ) )
    {
        if ( arg1[0] == '\0' || arg2[0] == '\0' )
        {
            send_to_char( "Syntax: marry <char1> <char2>\n\r", ch );
            return;
        }

        if ( !str_cmp( arg1, arg2 ) )
        {
            send_to_char( "A person cannot be married to themselves!\n\r", ch );
            return;
        }

        if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
        {
            send_to_char( "The first person mentioned isn't playing.\n\r", ch );
            return;
        }

        if ( ( victim2 = get_char_world( ch, arg2 ) ) == NULL )
        {
            send_to_char( "The second person mentioned isn't playing.\n\r",
                          ch );
            return;
        }

        if ( victim == victim2 )
        {
            send_to_char
                ( "I don't think marrying someone to themselves is a very good idea.\n\r",
                  ch );
            return;
        }

        if ( IS_NPC( victim ) || IS_NPC( victim2 ) )
        {
            send_to_char
                ( "I don't think they want to be Married to the Mob.\n\r", ch );
            return;
        }

        if ( !IS_SET( victim->act, PLR_CONSENT )
             || !IS_SET( victim2->act, PLR_CONSENT ) )
        {
            send_to_char( "They do not give consent.\n\r", ch );
            return;
        }

        if ( !is_name( victim->pcdata->spouse, "(none)" )
             || !is_name( victim2->pcdata->spouse, "(none)" ) )
        {
            send_to_char( "They are already married! \n\r", ch );
            return;
        }

        if ( victim->level < 1 || victim2->level < 1 )
        {
            send_to_char( "They are not of the proper level to marry.\n\r",
                          ch );
            return;
        }

        send_to_char( "You pronounce them man and wife!\n\r", ch );
        send_to_char( "You say the big 'I do.'\n\r", victim );
        send_to_char( "You say the big 'I do.'\n\r", victim2 );

        victim->pcdata->spousec += 1;
        victim2->pcdata->spousec += 1;
        free_string( &victim->pcdata->spouse );
        victim->pcdata->spouse = str_dup( victim2->name );
        free_string( &victim2->pcdata->spouse );
        victim2->pcdata->spouse = str_dup( victim->name );

        do_save( victim, "" );
        do_save( victim2, "" );

        /*    victim->pcdata->spouse = victim2->name;
           victim2->pcdata->spouse = victim->name; */

        return;
    }

    /*   else
       {
       send_to_char( "You do not have marrying power.\n\r", ch);
       return;
       } */
}

void do_divorce( CHAR_DATA * ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *victim2;
    /*    char buf[MAX_STRING_LENGTH]; */

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( ch->level >= LEVEL_IMMORTAL )
    {
        if ( arg1[0] == '\0' || arg2[0] == '\0' )
        {
            send_to_char( "Syntax: divorce <char1> <char2>\n\r", ch );
            return;
        }

        if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
        {
            send_to_char( "The first person mentioned isn't playing.\n\r", ch );
            return;
        }

        if ( ( victim2 = get_char_world( ch, arg2 ) ) == NULL )
        {
            send_to_char( "The second person mentioned isn't playing.\n\r",
                          ch );
            return;
        }

        if ( victim == victim2 )
        {
            send_to_char
                ( "I don't think divorcing someone without the other knowing is a very good idea.\n\r",
                  ch );
            return;
        }

        if ( IS_NPC( victim ) || IS_NPC( victim2 ) )
        {
            send_to_char( "I don't think they're Married to the Mob...\n\r",
                          ch );
            return;
        }

        if ( !IS_SET( victim->act, PLR_CONSENT )
             || !IS_SET( victim2->act, PLR_CONSENT ) )
        {
            send_to_char( "They do not give consent.\n\r", ch );
            return;
        }

        if ( !is_name( victim->pcdata->spouse, victim2->name ) )
        {
            send_to_char( "They aren't even married!!\n\r", ch );
            return;
        }

        send_to_char( "You hand them their papers.\n\r", ch );
        send_to_char( "Your divorce is final.\n\r", victim );
        send_to_char( "Your divorce is final.\n\r", victim2 );

        victim->pcdata->spousec -= 1;
        victim2->pcdata->spousec -= 1;
        free_string( &victim->pcdata->spouse );
        victim->pcdata->spouse = str_dup( "(none)" );
        free_string( &victim2->pcdata->spouse );
        victim2->pcdata->spouse = str_dup( "(none)" );

        do_save( victim, "" );
        do_save( victim2, "" );

        /*
           victim->pcdata->spouse = str_dup("none") ;
           victim2->pcdata->spouse = str_dup("none"); */

        return;
    }
    else
    {
        send_to_char( "You do not have divorcing power.\n\r", ch );
        return;
    }
}

void do_consent( CHAR_DATA * ch )
{
    if ( IS_NPC( ch ) )
        return;

    if ( IS_SET( ch->act, PLR_CONSENT ) )
    {
        send_to_char( "You no longer give consent.\n\r", ch );
        REMOVE_BIT( ch->act, PLR_CONSENT );
        return;
    }

    send_to_char( "You now give consent to be Married!\n\r", ch );
    SET_BIT( ch->act, PLR_CONSENT );

    return;
}
