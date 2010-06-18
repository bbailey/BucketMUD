
#if defined(WIN32)
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "merc.h"
#include "olc.h"

HELP_DATA *help_first = NULL;
HELP_DATA *help_last = NULL;

const struct olc_cmd_type helpsedit_table[] = {
    {"show", helpsedit_show},
    {"create", helpsedit_create},
    {"keyword", helpsedit_keyword},
    {"level", helpsedit_level},
    {"text", helpsedit_text},
    {"?", show_help},
    {"version", show_version},
    {"commands", show_commands},
    {"", 0,}
};

HELP_DATA *new_help( void )
{
    HELP_DATA *pHelp;

    pHelp = alloc_perm( sizeof( HELP_DATA ) );

    if ( !pHelp )
    {
        bug( "new help: Call to alloc_perm failed!", 0 );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    pHelp->next = NULL;
    pHelp->keyword = str_dup( "CHANGE_ME!" );
    pHelp->level = 0;
    pHelp->text =
        str_dup( "Someone didnt change the text for this help, the bonheads." );

    return pHelp;
}

HELP_DATA *get_help( char *argument )
{
    HELP_DATA *pHelp;
    char arg[MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg );
    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )

        if ( str_cmp( arg, pHelp->keyword ) )
        {
            break;
        }

    return pHelp;
}

void do_helpedit( CHAR_DATA * ch, char *argument )
{
    HELP_DATA *pHelp;
    char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while ( argument[0] != '\0' )
    {
        argument = one_argument( argument, argone );
        if ( argall[0] != '\0' )
            strcat( argall, " " );
        strcat( argall, argone );
    }

    if ( IS_NPC( ch ) )
    {
        send_to_char( "Why does an NPC need to write helps?!\n\r", ch );
        return;
    }

    if ( get_trust( ch ) < HELP_EDIT_LEVEL )
    {
        send_to_char( "Your not powerful enough to do this.\n\r", ch );
        return;
    }

    if ( argall[0] == '\0' )
    {
        send_to_char
            ( "Syntax: edit help createhelp\nSyntax: edit help <keyword>\n\r",
              ch );
        return;
    }

    for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )

        if ( is_name( argall, pHelp->keyword ) )
        {
            ch->desc->pEdit = ( void * ) pHelp;
            ch->desc->editor = ED_HELPOLC;
            SET_BIT( ch->act, PLR_BUILDING );
            act( "$n has entered the Help Editor.", ch, NULL, NULL, TO_ROOM );
            return;
        }
    if ( is_name( argall, "createhelp" ) )
    {
        if ( helpsedit_create( ch, argument ) )
        {
            ch->desc->editor = ED_HELPOLC;
            SET_BIT( ch->act, PLR_BUILDING );
            act( "$n has entered the Help Editor.", ch, NULL, NULL, TO_ROOM );
        }

        return;
    }
    else
    {
        send_to_char( "There is no default help to edit.\n\r", ch );
        return;
    }
}

void helpsedit( CHAR_DATA * ch, char *argument )
{
    HELP_DATA *pHelp;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde( argument );
    strncpy( arg, argument, sizeof( arg ) - 1 );
    arg[sizeof( arg )] = '\0';
    argument = one_argument( argument, command );

    pHelp = ( HELP_DATA * ) ch->desc->pEdit;

    if ( !str_cmp( command, "done" ) )
    {
        edit_done( ch );
        do_asave( ch, "helps" );
        return;
    }

    if ( get_trust( ch ) < HELP_EDIT_LEVEL )
    {
        send_to_char( "Insufficient security to modify helps.\n\r", ch );
        interpret( ch, arg );
        return;
    }

    if ( command[0] == '\0' )
    {
        helpsedit_show( ch, argument );
        return;
    }

    if ( !pHelp )
    {
        send_to_char( "You are not currently editing a help.\n\r", ch );
        interpret( ch, arg );
        return;
    }

    /* Search table and dispatch command */
    for ( cmd = 0; helpsedit_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( !str_prefix( command, helpsedit_table[cmd].name ) )
        {
            if ( ( *helpsedit_table[cmd].olc_fun ) ( ch, argument ) )
            {
                return;
            }
            else
            {
                return;
            }
        }
    }

    /* If command not found, default to the standard interpreter. */
    interpret( ch, arg );
    return;
}

bool helpsedit_show( CHAR_DATA * ch, char *argument )
{
    HELP_DATA *pHelp;

    pHelp = ( HELP_DATA * ) ch->desc->pEdit;

    if ( !pHelp )
    {
        send_to_char( "You are not currently editing a help.\n\r", ch );
    }
    else
    {
        printf_to_char( ch, "Keywords:             %s\n\r", pHelp->keyword );
        printf_to_char( ch, "Level: %d\n\r", pHelp->level );
        printf_to_char( ch,
                        "Text: Type text to see the text (Some are VERY long, so they are not shown here.)" );
    }

    return FALSE;
}

bool helpsedit_create( CHAR_DATA * ch, char *argument )
{
    HELP_DATA *pHelp = new_help(  );

    if ( !pHelp )
    {
        send_to_char( "ERROR!  Could not create a new help!\n\r", ch );
        return FALSE;
    }

    if ( help_first == NULL )
    {
        help_first = pHelp;
        help_last = pHelp;
    }
    else
    {
        help_last->next = pHelp;
        help_last = pHelp;
    }

    send_to_char( "Help created.\n\r", ch );

    ch->desc->pEdit = ( void * ) pHelp;

    return TRUE;
}

bool helpsedit_keyword( CHAR_DATA * ch, char *argument )
{
    HELP_DATA *pHelp;

    pHelp = ( HELP_DATA * ) ch->desc->pEdit;

    if ( !pHelp )
    {
        send_to_char( "You are not currently editing a help.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  keyword <keywords for this help>\n\r", ch );
        return FALSE;
    }

    if ( strlen( argument ) != str_len( argument ) )
    {
        send_to_char( "ERROR!  Color codes are not allowed in keywords.\n\r",
                      ch );
        return FALSE;
    }

    if ( strlen( argument ) > 60 )
    {
        send_to_char( "ERROR! Keywords must be 60 characters or less.\n\r",
                      ch );
        return FALSE;
    }

    free_string( &pHelp->keyword );
    pHelp->keyword = str_dup( argument );

    send_to_char( "Help keywords set.\n\r", ch );

    return TRUE;
}

bool helpsedit_level( CHAR_DATA * ch, char *argument )
{
    HELP_DATA *pHelp;

    pHelp = ( HELP_DATA * ) ch->desc->pEdit;

    if ( !pHelp )
    {
        send_to_char( "You are not currently editing a help.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax:  level [number]\n\r", ch );
        return FALSE;
    }

    pHelp->level = atoi( argument );

    send_to_char( "Level set.\n\r", ch );
    return TRUE;

}

bool helpsedit_text( CHAR_DATA * ch, char *argument )
{
    HELP_DATA *pHelp;

    pHelp = ( HELP_DATA * ) ch->desc->pEdit;

    if ( !pHelp )
    {
        send_to_char( "You are not currently editing a help.\n\r", ch );
        return FALSE;
    }
    if ( argument[0] == '\0' )
    {
        string_append( ch, &pHelp->text );
        return TRUE;
    }

    send_to_char( "Syntax:  text    (No arguments allowed)\n\r", ch );
    return FALSE;
}
