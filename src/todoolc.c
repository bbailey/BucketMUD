/* Header for no real reason */
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

TODO_DATA *todo_first = NULL;
TODO_DATA *todo_last = NULL;

const struct olc_cmd_type todoedit_table[] = {
    {"show", todoedit_show},
    {"create", todoedit_create},
    {"keyword", todoedit_keyword},
    {"level", todoedit_level},
    {"text", todoedit_text},
    {"?", show_help},
    {"version", show_version},
    {"commands", show_commands},
    {"", 0,}
};

TODO_DATA *new_todo( void )
{
    TODO_DATA *pTodo;

    pTodo = alloc_perm( sizeof( TODO_DATA ) );

    if ( !pTodo )
    {
        bug( "new todo: Call to alloc_perm failed!", 0 );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    pTodo->next = NULL;
    pTodo->keyword = str_dup( "CHANGE_ME!" );
    pTodo->level = 0;
    pTodo->text =
        str_dup( "Someone didnt change the text for this todo, the bonheads." );

    return pTodo;
}

TODO_DATA *get_todo( char *argument )
{
    TODO_DATA *pTodo;
    char arg[MAX_INPUT_LENGTH];
    argument = one_argument( argument, arg );
    for ( pTodo = todo_first; pTodo != NULL; pTodo = pTodo->next )

        if ( str_cmp( arg, pTodo->keyword ) )
        {
            break;
        }

    return pTodo;
}

void do_todoedit( CHAR_DATA * ch, char *argument )
{
    TODO_DATA *pTodo;
    char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];

    /* this parts handles todo a b so that it returns todo 'a b' */
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
        send_to_char( "Why does an NPC need to write todo?!\n\r", ch );
        return;
    }

    if ( get_trust( ch ) < TODO_EDIT_LEVEL )
    {
        send_to_char( "Your not powerful enough to do this.\n\r", ch );
        return;
    }

    if ( argall[0] == '\0' )
    {
        send_to_char
            ( "Syntax: edit todo createtodo\nSyntax: edit todo <keyword>\n\r",
              ch );
        return;
    }

    for ( pTodo = todo_first; pTodo != NULL; pTodo = pTodo->next )

        if ( is_name( argall, pTodo->keyword ) )
        {
            ch->desc->pEdit = ( void * ) pTodo;
            ch->desc->editor = ED_TODOOLC;
            SET_BIT( ch->act, PLR_BUILDING );
            act( "$n has entered the Todo Editor.", ch, NULL, NULL, TO_ROOM );
            return;
        }
    if ( is_name( argall, "createtodo" ) )
    {
        if ( todoedit_create( ch, argument ) )
        {
            ch->desc->editor = ED_TODOOLC;
            SET_BIT( ch->act, PLR_BUILDING );
            act( "$n has entered the Todo Editor.", ch, NULL, NULL, TO_ROOM );
        }

        return;
    }
    else
    {
        send_to_char( "There is no default todo to edit.\n\r", ch );
        return;
    }
}

void todoedit( CHAR_DATA * ch, char *argument )
{
    TODO_DATA *pTodo;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde( argument );
    strncpy( arg, argument, sizeof( arg ) - 1 );
    arg[sizeof( arg )] = '\0';
    argument = one_argument( argument, command );

    pTodo = ( TODO_DATA * ) ch->desc->pEdit;

    if ( !str_cmp( command, "done" ) )
    {
        edit_done( ch );
        do_asave( ch, "todo" );
        return;
    }

    if ( get_trust( ch ) < TODO_EDIT_LEVEL )
    {
        send_to_char( "Insufficient security to modify todo.\n\r", ch );
        interpret( ch, arg );
        return;
    }

    if ( command[0] == '\0' )
    {
        todoedit_show( ch, argument );
        return;
    }

    if ( !pTodo )
    {
        send_to_char( "You are not currently editing a todo.\n\r", ch );
        interpret( ch, arg );
        return;
    }

    /* Search table and dispatch command */
    for ( cmd = 0; todoedit_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( !str_prefix( command, todoedit_table[cmd].name ) )
        {
            if ( ( *todoedit_table[cmd].olc_fun ) ( ch, argument ) )
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

bool todoedit_show( CHAR_DATA * ch, char *argument )
{
    TODO_DATA *pTodo;

    pTodo = ( TODO_DATA * ) ch->desc->pEdit;

    if ( !pTodo )
    {
        send_to_char( "You are not currently editing a todo.\n\r", ch );
    }
    else
    {
        printf_to_char( ch, "Keywords:             %s\n\r", pTodo->keyword );
        printf_to_char( ch, "Level: %d\n\r", pTodo->level );
        printf_to_char( ch,
                        "Text: Type text to see the text (Some are VERY long, so they are not shown here.)" );
    }

    return FALSE;
}

bool todoedit_create( CHAR_DATA * ch, char *argument )
{
    TODO_DATA *pTodo = new_todo(  );

    if ( !pTodo )
    {
        send_to_char( "ERROR!  Could not create a new todo!\n\r", ch );
        return FALSE;
    }

    if ( todo_first == NULL )
    {
        todo_first = pTodo;
        todo_last = pTodo;
    }
    else
    {
        todo_last->next = pTodo;
        todo_last = pTodo;
    }

    send_to_char( "Todo created.\n\r", ch );

    ch->desc->pEdit = ( void * ) pTodo;

    return TRUE;
}

bool todoedit_keyword( CHAR_DATA * ch, char *argument )
{
    TODO_DATA *pTodo;

    pTodo = ( TODO_DATA * ) ch->desc->pEdit;

    if ( !pTodo )
    {
        send_to_char( "You are not currently editing a todo.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  keyword <keywords for this todo>\n\r", ch );
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

    free_string( &pTodo->keyword );
    pTodo->keyword = str_dup( argument );

    send_to_char( "Todo keywords set.\n\r", ch );

    return TRUE;
}

bool todoedit_level( CHAR_DATA * ch, char *argument )
{
    TODO_DATA *pTodo;

    pTodo = ( TODO_DATA * ) ch->desc->pEdit;

    if ( !pTodo )
    {
        send_to_char( "You are not currently editing a todo.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax:  level [number]\n\r", ch );
        return FALSE;
    }

    pTodo->level = atoi( argument );

    send_to_char( "Level set.\n\r", ch );
    return TRUE;

}

bool todoedit_text( CHAR_DATA * ch, char *argument )
{
    TODO_DATA *pTodo;

    pTodo = ( TODO_DATA * ) ch->desc->pEdit;

    if ( !pTodo )
    {
        send_to_char( "You are not currently editing a Todo.\n\r", ch );
        return FALSE;
    }
    if ( argument[0] == '\0' )
    {
        string_append( ch, &pTodo->text );
        return TRUE;
    }

    send_to_char( "Syntax:  text    (No arguments allowed)\n\r", ch );
    return FALSE;
}
