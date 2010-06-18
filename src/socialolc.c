
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

SOCIALLIST_DATA *social_first = NULL;
SOCIALLIST_DATA *social_last = NULL;

sh_int socials_count = 0;

const struct olc_cmd_type socialedit_table[] = {
    {"show", socialedit_show},
    {"create", socialedit_create},
    {"name", socialedit_name},
    {"chnoarg", socialedit_chnoarg},
    {"othersnoarg", socialedit_othersnoarg},
    {"chfound", socialedit_chfound},
    {"othersfound", socialedit_othersfound},
    {"victfound", socialedit_victfound},
    {"chnotfound", socialedit_chnotfound},
    {"chauto", socialedit_chauto},
    {"othersauto", socialedit_othersauto},
    {"?", show_help},
    {"version", show_version},
    {"commands", show_commands},
    {"", 0,}
};

void load_socials( FILE * fp )
{
    SOCIALLIST_DATA *pSocial;
    sh_int vnum;
    char *word;
    char letter;

    for ( ;; )
    {
        word = fread_word( fp );

        /* Exit when "#$" found */
        if ( !str_cmp( word, "#$" ) )
        {
            break;
        }

        /* Start loading when "#SOCIALS" found */
        if ( !str_cmp( word, "#SOCIALS" ) )
        {
            for ( ;; )
            {
                letter = fread_letter( fp );

                /* First thing must be a vnum */
                if ( letter != '#' )
                {
                    bug( "load_socials: # not found while looking for vnum",
                         0 );
                    return;
                }

                vnum = fread_number( fp );

                if ( vnum == 0 )
                {
                    break;
                }

                pSocial = get_social_by_vnum( vnum );

                if ( pSocial != NULL )
                {
                    bug( "load_social: vnum %d duplicated", vnum );
                    return;
                }

                pSocial = new_social(  );

                free_string( &pSocial->name );
                free_string( &pSocial->char_no_arg );
                free_string( &pSocial->others_no_arg );
                free_string( &pSocial->char_found );
                free_string( &pSocial->others_found );
                free_string( &pSocial->vict_found );
                free_string( &pSocial->char_not_found );
                free_string( &pSocial->char_auto );
                free_string( &pSocial->others_auto );

                pSocial->vnum = vnum;
                pSocial->name = fread_string( fp );
                pSocial->char_no_arg = fread_string( fp );
                pSocial->others_no_arg = fread_string( fp );
                pSocial->char_found = fread_string( fp );
                pSocial->others_found = fread_string( fp );
                pSocial->vict_found = fread_string( fp );
                pSocial->char_not_found = fread_string( fp );
                pSocial->char_auto = fread_string( fp );
                pSocial->others_auto = fread_string( fp );

                if ( social_first == NULL )
                {
                    social_first = pSocial;
                    social_last = pSocial;
                }
                else
                {
                    social_last->next = pSocial;
                    social_last = pSocial;
                }
            }

            return;
        }
        else
        {
            bug( "load_socials: bad section name", 0 );
            return;
        }
    }
}

void save_socials( void )
{
    SOCIALLIST_DATA *pSocial;
    FILE *fp;
    char buf[1024];

    sprintf( buf, "%s/%s", sysconfig.area_dir, sysconfig.socials_file );

    fp = fopen( buf, "w" );

    if ( !fp )
    {
        bug( "Cannot write to %s !!!", sysconfig.socials_file );
        return;
    }

    fprintf( fp, "#SOCIALS\n\n" );

    for ( pSocial = social_first; pSocial != NULL; pSocial = pSocial->next )
    {
        fprintf( fp, "#%d\n", pSocial->vnum );
        fprintf( fp, "%s~\n", pSocial->name );
        fprintf( fp, "%s~\n", pSocial->char_no_arg );
        fprintf( fp, "%s~\n", pSocial->others_no_arg );
        fprintf( fp, "%s~\n", pSocial->char_found );
        fprintf( fp, "%s~\n", pSocial->others_found );
        fprintf( fp, "%s~\n", pSocial->vict_found );
        fprintf( fp, "%s~\n", pSocial->char_not_found );
        fprintf( fp, "%s~\n", pSocial->char_auto );
        fprintf( fp, "%s~\n", pSocial->others_auto );
    }

    fprintf( fp, "#0\n" );

    fclose( fp );
}

SOCIALLIST_DATA *new_social( void )
{
    SOCIALLIST_DATA *pSocial;

    pSocial = malloc( sizeof( SOCIALLIST_DATA ) );

    if ( !pSocial )
    {
        bug( "new_social: Call to alloc_perm failed!", 0 );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    socials_count++;

    pSocial->next = NULL;
    pSocial->vnum = socials_count;
    pSocial->name = str_dup( "none" );
    pSocial->char_no_arg = str_dup( "none" );
    pSocial->others_no_arg = str_dup( "none" );
    pSocial->char_found = str_dup( "none" );
    pSocial->others_found = str_dup( "none" );
    pSocial->vict_found = str_dup( "none" );
    pSocial->char_not_found = str_dup( "none" );
    pSocial->char_auto = str_dup( "none" );
    pSocial->others_auto = str_dup( "none" );

    return pSocial;
}

SOCIALLIST_DATA *get_social_by_vnum( sh_int vnum )
{
    SOCIALLIST_DATA *pSocial;

    for ( pSocial = social_first; pSocial != NULL; pSocial = pSocial->next )
    {
        if ( pSocial->vnum == vnum )
            break;
    }

    return pSocial;
}

void do_socialedit( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    SOCIALLIST_DATA *pSocial;
    sh_int vnum;

    if ( IS_NPC( ch ) )
    {
        send_to_char( "NPCs can't do that!\n\r", ch );
        return;
    }

    argument = one_argument( argument, arg );
    vnum = atoi( arg );
    for ( pSocial = social_first; pSocial != NULL; pSocial = pSocial->next )
/*    if ( is_number( arg ) )*/
        if ( is_name( arg, pSocial->name ) || atoi( arg ) == pSocial->vnum )
        {

/*	pSocial = get_social_by_vnum( vnum );
	
	if ( !pSocial )
	{
	    send_to_char( "That social number does not exist!\n\r", ch );
	    return;
	}*/

            ch->desc->pEdit = ( void * ) pSocial;
            ch->desc->editor = ED_SOCIAL;
            SET_BIT( ch->act, PLR_BUILDING );
            act( "$n has entered the Socials Editor.", ch, NULL, NULL,
                 TO_ROOM );

            return;
        }
        else
        {
            if ( !str_cmp( arg, "create" ) )
            {
                if ( socialedit_create( ch, argument ) )
                {
                    ch->desc->editor = ED_SOCIAL;
                    SET_BIT( ch->act, PLR_BUILDING );
                    act( "$n has entered the Socials Editor.", ch, NULL, NULL,
                         TO_ROOM );

                }

                return;
            }
        }

    send_to_char( "There is no default social to edit.\n\r", ch );
    return;
}

void socialedit( CHAR_DATA * ch, char *argument )
{
    SOCIALLIST_DATA *pSocial;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde( argument );
    strncpy( arg, argument, sizeof( arg ) - 1 );
    arg[sizeof( arg )] = '\0';
    argument = one_argument( argument, command );

    pSocial = ( SOCIALLIST_DATA * ) ch->desc->pEdit;

    if ( !str_cmp( command, "done" ) )
    {
        edit_done( ch );
        do_asave( ch, "socials" );
        return;
    }

    if ( get_trust( ch ) < SOCIAL_EDIT_LEVEL )
    {
        send_to_char( "Insufficient security to modify socials.\n\r", ch );
        interpret( ch, arg );
        return;
    }

    if ( command[0] == '\0' )
    {
        socialedit_show( ch, argument );
        return;
    }

    if ( !pSocial )
    {
        send_to_char( "You are not currently editing a social.\n\r", ch );
        interpret( ch, arg );
        return;
    }

    /* Search table and dispatch command */
    for ( cmd = 0; socialedit_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( !str_prefix( command, socialedit_table[cmd].name ) )
        {
            if ( ( *socialedit_table[cmd].olc_fun ) ( ch, argument ) )
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

bool socialedit_show( CHAR_DATA * ch, char *argument )
{
    SOCIALLIST_DATA *pSocial;

    pSocial = ( SOCIALLIST_DATA * ) ch->desc->pEdit;

    if ( !pSocial )
    {
        send_to_char( "You are not currently editing a social.\n\r", ch );
    }
    else
    {
        printf_to_char( ch, "VNum       :             [%-5d]\n\r",
                        pSocial->vnum );
        printf_to_char( ch, "Name       :             %s\n\r", pSocial->name );
        printf_to_char( ch, "Chnoarg    :             %s\n\r",
                        pSocial->char_no_arg );
        printf_to_char( ch, "Othersnoarg:             %s\n\r",
                        pSocial->others_no_arg );
        printf_to_char( ch, "Chfound    :             %s\n\r",
                        pSocial->char_found );
        printf_to_char( ch, "Othersfound:             %s\n\r",
                        pSocial->others_found );
        printf_to_char( ch, "Victfound  :             %s\n\r",
                        pSocial->vict_found );
        printf_to_char( ch, "Chnotfound :             %s\n\r",
                        pSocial->char_not_found );
        printf_to_char( ch, "Chauto     :             %s\n\r",
                        pSocial->char_auto );
        printf_to_char( ch, "Othersauto :             %s\n\r",
                        pSocial->others_auto );
    }

    return FALSE;
}

bool socialedit_create( CHAR_DATA * ch, char *argument )
{
    SOCIALLIST_DATA *pSocial = new_social(  );

    if ( !pSocial )
    {
        send_to_char( "ERROR!  Could not create a new social!\n\r", ch );
        return FALSE;
    }

    if ( social_first == NULL )
    {
        social_first = pSocial;
        social_last = pSocial;
    }
    else
    {
        social_last->next = pSocial;
        social_last = pSocial;
    }

    send_to_char( "Social created.\n\r", ch );

    ch->desc->pEdit = ( void * ) pSocial;

    return TRUE;
}

bool socialedit_name( CHAR_DATA * ch, char *argument )
{
    SOCIALLIST_DATA *pSocial;

    pSocial = ( SOCIALLIST_DATA * ) ch->desc->pEdit;

    if ( !pSocial )
    {
        send_to_char( "You are not currently editing a social.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  name <social name>\n\r", ch );
        return FALSE;
    }

    if ( strlen( argument ) != str_len( argument ) )
    {
        send_to_char
            ( "ERROR!  Color codes are not allowed in social names.\n\r", ch );
        return FALSE;
    }

    if ( strlen( argument ) > 20 )
    {
        send_to_char( "ERROR!  Social names must be 20 characters or less.\n\r",
                      ch );
        return FALSE;
    }

    free_string( &pSocial->name );
    pSocial->name = str_dup( argument );

    send_to_char( "Social name set.\n\r", ch );

    return TRUE;
}

bool socialedit_chnoarg( CHAR_DATA * ch, char *argument )
{
    SOCIALLIST_DATA *pSocial;

    pSocial = ( SOCIALLIST_DATA * ) ch->desc->pEdit;

    if ( !pSocial )
    {
        send_to_char( "You are not currently editing a social.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  chnoarg <chnoarg message>\n\r", ch );
        return FALSE;
    }

    free_string( &pSocial->char_no_arg );
    pSocial->char_no_arg = str_dup( argument );

    send_to_char( "Chnoarg message set.\n\r", ch );

    return TRUE;
}

bool socialedit_othersnoarg( CHAR_DATA * ch, char *argument )
{
    SOCIALLIST_DATA *pSocial;

    pSocial = ( SOCIALLIST_DATA * ) ch->desc->pEdit;

    if ( !pSocial )
    {
        send_to_char( "You are not currently editing a social.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  chothersarg <chothersarg message>\n\r", ch );
        return FALSE;
    }

    free_string( &pSocial->others_no_arg );
    pSocial->others_no_arg = str_dup( argument );

    send_to_char( "Othersnoarg message set.\n\r", ch );

    return TRUE;
}

bool socialedit_chfound( CHAR_DATA * ch, char *argument )
{
    SOCIALLIST_DATA *pSocial;

    pSocial = ( SOCIALLIST_DATA * ) ch->desc->pEdit;

    if ( !pSocial )
    {
        send_to_char( "You are not currently editing a social.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  chfound <chfound message>\n\r", ch );
        return FALSE;
    }

    free_string( &pSocial->char_found );
    pSocial->char_found = str_dup( argument );

    send_to_char( "Chfound message set.\n\r", ch );

    return TRUE;
}

bool socialedit_othersfound( CHAR_DATA * ch, char *argument )
{
    SOCIALLIST_DATA *pSocial;

    pSocial = ( SOCIALLIST_DATA * ) ch->desc->pEdit;

    if ( !pSocial )
    {
        send_to_char( "You are not currently editing a social.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  othersfound <othersfound message>\n\r", ch );
        return FALSE;
    }

    free_string( &pSocial->others_found );
    pSocial->others_found = str_dup( argument );

    send_to_char( "Othersfound message set.\n\r", ch );

    return TRUE;
}

bool socialedit_victfound( CHAR_DATA * ch, char *argument )
{
    SOCIALLIST_DATA *pSocial;

    pSocial = ( SOCIALLIST_DATA * ) ch->desc->pEdit;

    if ( !pSocial )
    {
        send_to_char( "You are not currently editing a social.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  victfound <victfound message>\n\r", ch );
        return FALSE;
    }

    free_string( &pSocial->vict_found );
    pSocial->vict_found = str_dup( argument );

    send_to_char( "Victfound message set.\n\r", ch );

    return TRUE;
}

bool socialedit_chnotfound( CHAR_DATA * ch, char *argument )
{
    SOCIALLIST_DATA *pSocial;

    pSocial = ( SOCIALLIST_DATA * ) ch->desc->pEdit;

    if ( !pSocial )
    {
        send_to_char( "You are not currently editing a social.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  chnotfound <chnoarg message>\n\r", ch );
        return FALSE;
    }

    free_string( &pSocial->char_not_found );
    pSocial->char_not_found = str_dup( argument );

    send_to_char( "Chnotfound message set.\n\r", ch );

    return TRUE;
}

bool socialedit_chauto( CHAR_DATA * ch, char *argument )
{
    SOCIALLIST_DATA *pSocial;

    pSocial = ( SOCIALLIST_DATA * ) ch->desc->pEdit;

    if ( !pSocial )
    {
        send_to_char( "You are not currently editing a social.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  chauto <chauto message>\n\r", ch );
        return FALSE;
    }

    free_string( &pSocial->char_auto );
    pSocial->char_auto = str_dup( argument );

    send_to_char( "Chauto message set.\n\r", ch );

    return TRUE;
}

bool socialedit_othersauto( CHAR_DATA * ch, char *argument )
{
    SOCIALLIST_DATA *pSocial;

    pSocial = ( SOCIALLIST_DATA * ) ch->desc->pEdit;

    if ( !pSocial )
    {
        send_to_char( "You are not currently editing a social.\n\r", ch );
        return FALSE;
    }

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  othersauto <othersauto message>\n\r", ch );
        return FALSE;
    }

    free_string( &pSocial->others_auto );
    pSocial->others_auto = str_dup( argument );

    send_to_char( "Othersauto message set.\n\r", ch );

    return TRUE;
}

void do_socialfind( CHAR_DATA * ch, char *argument )
{
    SOCIALLIST_DATA *pSocial;
    char *buffer = NULL;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;
    int count;
    count = 0;
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        arg[0] = atoi( "all" );
    }

    for ( pSocial = social_first; pSocial != NULL; pSocial = pSocial->next )
    {
        if ( ( is_name( argument, pSocial->name ) )
             || ( !str_cmp( argument, "all" ) ) )
        {
            count++;
            if ( IS_IMMORTAL( ch ) )
            {
                if ( count > 4 )
                {
                    sprintf( buf, "`Y%d.`w%-10s\n\r", pSocial->vnum,
                             pSocial->name );
                    buffer = add_to_buf( buffer, buf );
                    count = 0;
                }
                else
                {
                    sprintf( buf, "`Y%d.`w%-10s", pSocial->vnum,
                             pSocial->name );
                    buffer = add_to_buf( buffer, buf );
                }
            }
            else
            {
                if ( count > 6 )
                {
                    sprintf( buf, "`w%10s\n\r", pSocial->name );
                    buffer = add_to_buf( buffer, buf );
                    count = 0;
                }
                else
                {
                    sprintf( buf, "`w%10s", pSocial->name );
                    buffer = add_to_buf( buffer, buf );
                }
            }

            found = TRUE;
        }
    }

    if ( !found )
    {
        send_to_char( "No socials found by that name.\n\r", ch );
    }
    else
    {
        page_to_char( buffer, ch );
    }
}
