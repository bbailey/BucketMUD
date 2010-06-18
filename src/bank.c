
/* **************************************************************
  January 25th, 1998
  Gothar's Bank Code Version 1.2
  1997 Copyright <* FREEWARE *>

Here is a listing of what the code does:
     1.Give some RP to your thieves!
       - must go to their fence in the thieves guild.
     2.Deposit gold or silver into your account
     3.Withdraw gold or silver into your account
NOTE: This code leaves the the IS_CHANGER stuff in do_give
      so you still can use Otho's services for money exchange.
************************************************************** */
/* N.B: READ this code before you ADD anything!!
 *      This is bank.c plus a patch file for other files.
 */

#if defined(WIN32)
#include <windows.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
/*#include "recycle.h"*/
/* command procedures needed */
DECLARE_DO_FUN( do_help );

void do_account( CHAR_DATA * ch, char *argument )
{
    long gold = 0;              /* silver = 0 */
/*   int diff, bonus = 0;*/
    char buf[MAX_STRING_LENGTH];
    char const *Class = class_table[ch->Class].name;

    gold = ch->pcdata->gold_bank;
    /*silver = ch->pcdata->silver_bank; */
    /* No NPC's, No pets, No imms,
     * No chainmail, No service!
     */
    if ( ( IS_NPC( ch ) || IS_SET( ch->act, ACT_PET ) )
         || ( IS_IMMORTAL( ch ) ) )
    {
        send_to_char( "Only players need money!\n\r", ch );
        return;
    }
    if ( IS_NPC( ch ) )
    {

        return;
    }
    if ( !str_cmp( Class, "thief" ) )
    {
        sprintf( buf,
                 "Your guild's fence tells you.\n\rYou have in your beltpouch:\n\r"
                 "  Gold:   %10ld\n\r" "Under your bedroll:\n\r"
                 "  Gold:   %10ld\n\r  ", ch->gold, gold );
    }
    else
    {
        sprintf( buf,
                 "Your Midgaard Account records show:\n\rYou have in your beltpouch:\n\r"
                 "  Gold:   %10ld\n\r" "In your Account:\n\r"
                 "  Gold:   %10ld\n\r", ch->gold, gold );
    }
    send_to_char( buf, ch );
    return;
}

void do_deposit( CHAR_DATA * ch, char *argument )
{
    long amount = 0;
    char const *Class = class_table[ch->Class].name;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    /* No NPC's, No pets, No imms,
     * No chainmail, No service!
     */
    if ( ( IS_NPC( ch ) || IS_SET( ch->act, ACT_PET ) )
         || ( IS_IMMORTAL( ch ) ) )
    {
        send_to_char( "Only players need money!\n\r", ch );
        return;
    }
    if ( ch->in_room != get_room_index( ROOM_VNUM_BANK )
         && str_cmp( Class, "thief" ) )
    {
        send_to_char( "You must be in the Bank to deposit.\n\r", ch );
        return;
    }
    else if ( ch->in_room != get_room_index( ROOM_VNUM_BANK_THIEF )
              && !str_cmp( Class, "thief" ) )
    {
        send_to_char( "You must be at your fence to deposit.\n\r", ch );
        return;
    }
    else                        /* In the Bank */
    {
        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        if ( arg1[0] == '\0' || arg2[0] == '\0' )
        {
            send_to_char( "How much to squirrel away?\n\r", ch );
            if ( !str_cmp( Class, "thief" ) )
            {
                send_to_char( "THIEF: Deposit <value> gold\n\r", ch );
            }
            else
            {
                send_to_char( "GBS: Deposit <value> gold\n\r", ch );
            }
            send_to_char( "For more information Type 'bank'.\n\r", ch );
            return;
        }
        if ( ch->in_room == get_room_index( ROOM_VNUM_BANK ) )
        {
            if ( !str_cmp( Class, "thief" ) )
            {
                act( "A thief lurks in the shadows here.", ch, NULL, NULL,
                     TO_ROOM );
                send_to_char
                    ( "GBS: You aren't allowed in Here, you'd better go!\n\r",
                      ch );
                return;
            }
            else
            {
                if ( is_number( arg1 ) )
                {
                    amount = atoi( arg1 );

                    if ( amount <= 0 )
                    {
                        send_to_char
                            ( "GBS: To deposit you must give money!\n\r", ch );
                        send_to_char
                            ( "     For more information Type 'Bank'.\n\r",
                              ch );
                        return;
                    }
                    if ( !str_cmp( arg2, "gold" ) )
                    {
                        if ( ch->gold < amount )
                        {
                            send_to_char
                                ( "GBS: You don't have that much gold.\n\r",
                                  ch );
                            return;
                        }
                        else
                        {
                            ch->pcdata->gold_bank += amount;
                            ch->gold -= amount;
                            act( "$n deposits gold into $s account.", ch, NULL,
                                 NULL, TO_ROOM );
                            sprintf( buf,
                                     "GBS: You have deposited %ld Gold.\n\r   Account: %10ld.\n\r   Beltpouch: %8ld.\n\r",
                                     amount, ch->pcdata->gold_bank, ch->gold );
                            send_to_char( buf, ch );
                            return;
                        }
                    }
                    /*  if(!str_cmp( arg2, "silver"))
                       {
                       if (ch->silver < amount)
                       {
                       send_to_char("GBS: You don't have that much silver.\n\r",ch);
                       return;
                       }
                       else
                       {
                       ch->pcdata->silver_bank += amount;
                       ch->silver -= amount;
                       act("$n deposits silver into $s account.", ch,NULL,NULL, TO_ROOM);
                       sprintf( buf, "GBS: You have deposited %ld Silver.\n\r   Account: %10ld.\n\r   Beltpouch: %8ld.\n\r", amount, ch->pcdata->silver_bank, ch->silver);
                       send_to_char( buf, ch);
                       return;
                       }
                       } */
                }
            }
        }
        else if ( ch->in_room == get_room_index( ROOM_VNUM_BANK_THIEF ) )
        {
            if ( str_cmp( Class, "thief" ) )
            {
                act( "A raider is Here to kill the Guildmaster.", ch, NULL,
                     NULL, TO_ROOM );
                send_to_char
                    ( "You aren't allowed in Here, you'd better go!\n\r", ch );
                return;
            }
            else
            {
                if ( is_number( arg1 ) )
                {
                    amount = atoi( arg1 );

                    if ( amount <= 0 )
                    {

                        send_to_char
                            ( "THIEF: To deposit you must give money!\n\r",
                              ch );
                        send_to_char
                            ( "       For more information Type 'Bank'.\n\r",
                              ch );
                        return;
                    }
                    if ( !str_cmp( arg2, "gold" ) )
                    {
                        if ( ch->gold < amount )
                        {
                            send_to_char
                                ( "THIEF: You don't have that much gold.\n\r",
                                  ch );
                            return;
                        }
                        else
                        {
                            ch->pcdata->gold_bank += amount;
                            ch->gold -= amount;
                            act( "$n hides gold under a bedroll.", ch, NULL,
                                 NULL, TO_ROOM );
                            sprintf( buf,
                                     "THIEF: You have hidden %ld Gold.\n\r   Bedroll: %10ld.\n\r   Beltpouch: %8ld.\n\r",
                                     amount, ch->pcdata->gold_bank, ch->gold );
                            send_to_char( buf, ch );
                            return;
                        }
                    }
/*               if(!str_cmp( arg2, "silver"))
               {
                  if (ch->silver < amount)
                  {
                     send_to_char("THIEF: You don't have that much silver.\n\r",ch);
                     return;
                  }
                  else
                  {
                     ch->pcdata->silver_bank += amount;
                     ch->silver -= amount;
                     act("$n hides silver under a bedroll.", ch,NULL,NULL, TO_ROOM);
                     sprintf( buf, "THIEF: You have hidden %ld Silver.\n\r   Bedroll: %10ld.\n\r   Beltpouch: %8ld.\n\r", amount, ch->pcdata->silver_bank, ch->silver);
                     send_to_char( buf, ch);
                     return;
                  }
               } */
                }
                else
                {
                    send_to_char
                        ( "You might want to tell me what you want to deposit. For all I know, you might be trying to deposit your mothers ashes.\n\r",
                          ch );
                }
            }
        }
        else
        {
            bug( "Do_deposit: Bank doesn't exist.", 0 );
            send_to_char( "Bank doesn't exist.\n\r", ch );
            return;
        }
    }
    return;
}

void do_withdraw( CHAR_DATA * ch, char *argument )
{
    long amount = 0;
    char const *Class = class_table[ch->Class].name;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    /* No NPC's, No pets, No imms,
     * No chainmail, No service!
     */
    if ( ( IS_NPC( ch ) || IS_SET( ch->act, ACT_PET ) )
         || ( IS_IMMORTAL( ch ) ) )
    {
        send_to_char( "Only players need money!\n\r", ch );
        return;
    }
    if ( ch->in_room != get_room_index( ROOM_VNUM_BANK )
         && str_cmp( Class, "thief" ) )
    {
        send_to_char( "You must be in the Bank to withdraw.\n\r", ch );
        return;
    }
    else if ( ch->in_room != get_room_index( ROOM_VNUM_BANK_THIEF )
              && !str_cmp( Class, "thief" ) )
    {
        send_to_char( "You must be at your fence to withdraw.\n\r", ch );
        return;
    }
    else                        /* In the Bank */
    {
        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        if ( arg1[0] == '\0' || arg2[0] == '\0' )
        {
            send_to_char( "How much to withdraw away?\n\r", ch );
            if ( !str_cmp( Class, "thief" ) )
            {
                send_to_char( "THIEF: Withdraw <value> gold\n\r", ch );
            }
            else
            {
                send_to_char( "GBS: Withdraw <value> gold\n\r", ch );
            }
            send_to_char( "For more information Type 'Bank'.\n\r", ch );
            return;
        }
        if ( ch->in_room == get_room_index( ROOM_VNUM_BANK ) )
        {
            if ( !str_cmp( Class, "thief" ) )
            {
                act( "A thief lurks in the shadows here.", ch, NULL, NULL,
                     TO_ROOM );
                send_to_char
                    ( "You aren't allowed in Here, you'd better go!\n\r", ch );
                return;
            }
            else
            {
                if ( is_number( arg1 ) )
                {
                    amount = atoi( arg1 );

                    if ( amount <= 0 )
                    {
                        send_to_char
                            ( "GBS: To withdraw you must give an amount!\n\r",
                              ch );
                        send_to_char( "     For information Type 'Bank'.\n\r",
                                      ch );
                        return;
                    }
                    if ( !str_cmp( arg2, "gold" ) )
                    {
                        if ( ch->pcdata->gold_bank < amount )
                        {
                            send_to_char
                                ( "GBS: You don't have that much gold squirreled away.\n\r",
                                  ch );
                            return;
                        }
                        else
                        {
                            ch->pcdata->gold_bank -= amount;
                            ch->gold += amount;
                            act( "$n withdraws gold from $s account.", ch, NULL,
                                 NULL, TO_ROOM );
                            sprintf( buf,
                                     "GBS: You have withdrawn %ld Gold.\n\r   Account: %10ld.\n\r   Beltpouch: %8ld.\n\r",
                                     amount, ch->pcdata->gold_bank, ch->gold );
                            send_to_char( buf, ch );
                            return;
                        }
                    }
/*               if(!str_cmp( arg2, "silver"))
               {
                  if (ch->pcdata->silver_bank < amount)
                  {
                     send_to_char("GBS: You don't have that much silver squirreled away.\n\r",ch);
                     return;
                  }
                  else
                  {
                     ch->pcdata->silver_bank -= amount;
                     ch->silver += amount;
                     act("$n withdraws silver from $s account.", ch,NULL,NULL, TO_ROOM);
                     sprintf( buf, "GBS: You have withdrawn %ld Silver.\n\r   Account: %10ld.\n\r   Beltpouch: %8ld.\n\r", amount, ch->pcdata->silver_bank, ch->silver);
                     send_to_char( buf, ch);
                     return;
                  }
               }*/
                }
                else
                {
                    send_to_char
                        ( "Are you trying to withdraw gold? Or corpses?\n\r",
                          ch );
                }
            }
        }
        else if ( ch->in_room == get_room_index( ROOM_VNUM_BANK_THIEF ) )
        {
            if ( str_cmp( Class, "thief" ) )
            {
                act( "A raider is Here to kill the Guildmaster.", ch, NULL,
                     NULL, TO_ROOM );
                send_to_char
                    ( "You aren't allowed in Here, you'd better go!\n\r", ch );
                return;
            }
            else
            {
                if ( is_number( arg1 ) )
                {
                    amount = atoi( arg1 );

                    if ( amount <= 0 )
                    {
                        send_to_char
                            ( "THIEF: To withdraw you must give an amount!\n\r",
                              ch );
                        return;
                    }
                    if ( !str_cmp( arg2, "gold" ) )
                    {
                        if ( ch->pcdata->gold_bank < amount )
                        {
                            send_to_char
                                ( "THIEF: You don't have that much gold squirreled away.\n\r",
                                  ch );
                            return;
                        }
                        else
                        {
                            ch->pcdata->gold_bank -= amount;
                            ch->gold += amount;
                            act( "$n grabs gold from under a bedroll.", ch,
                                 NULL, NULL, TO_ROOM );
                            sprintf( buf,
                                     "THIEF: You have grabbed %ld Gold.\n\r   Bedroll: %10ld.\n\r   Beltpouch: %8ld.\n\r",
                                     amount, ch->pcdata->gold_bank, ch->gold );
                            send_to_char( buf, ch );
                            return;
                        }
                    }
/*               if(!str_cmp( arg2, "silver"))
               {
                  if (ch->pcdata->silver_bank < amount)
                  {
                     send_to_char("THIEF: You don't have that much silver squirreled away.\n\r",ch);
                     return;
                  }
                  else
                  {
                     ch->pcdata->silver_bank -= amount;
                     ch->silver += amount;
                     act("$n grabs silver from under a bedroll.", ch,NULL,NULL, TO_ROOM);
                     sprintf( buf, "THIEF: You have grabbed %ld Silver.\n\r   Bedroll: %10ld.\n\r   Beltpouch: %8ld.\n\r", amount, ch->pcdata->silver_bank, ch->silver);
                     send_to_char( buf, ch);
                     return;
                  }
               }*/
                }
                else
                {
                    send_to_char
                        ( "I will give you nothing if you dont tell me what you want!\n\r",
                          ch );
                }
            }
        }
        else
        {
            bug( "Do_withdraw: Bank doesn't exist.", 0 );
            send_to_char( "Bank doesn't exist.\n\r", ch );
            return;
        }
    }
    return;
}

void do_bank( CHAR_DATA * ch, char *argument )
{
    send_to_char
        ( " In order to use the bank, you must first be at the bank. Once your at the\n\r  bank, you can type withdraw, account or deposit. Since the bank does not want\n\r to do business with thieving people, thieves and the like must depend on their\n\r guild masters in order to store their loot.\n\r",
          ch );
    return;
}
