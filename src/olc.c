
/***************************************************************************
 *  File: olc.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

/**************************************************************************
 * Mudprogram's (Mobprogram, Objprogram and Roomprogram) originaly 	      *
 * by the SMAUG development team             				              *
 * Ported to EmberMUD by Thanatos and Tyrluk of ToED      		          *
 * (Temple of Eternal Death)    			                     		  *
 * Tyrluk   - morn@telmaron.com or dajy@mindspring.com			          *
 * Thanatos - morn@telmaron.com or jonathan_w._rose@ffic.com              * 
 * Heavily modified by Zane (zane@supernova.org)                          *
 **************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"

char buf[MAX_STRING_LENGTH];

/*
 * Local functions.
 */
AREA_DATA *get_area_data args( ( int vnum ) );
int get_new_mprog_vnum args( ( void ) );

/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor( DESCRIPTOR_DATA * d )
{
    if ( IS_NPC( d->character ) )
        return FALSE;
    switch ( d->editor )
    {
    case ED_AREA:
        aedit( CH( d ), d->incomm );
        break;
    case ED_ROOM:
        redit( CH( d ), d->incomm );
        break;
    case ED_OBJECT:
        oedit( CH( d ), d->incomm );
        break;
    case ED_MOBILE:
        medit( CH( d ), d->incomm );
        break;
    case ED_MPROG:
        mpedit( d->character, d->incomm );
        break;
    case ED_MPGROUP:
        mpgedit( d->character, d->incomm );
        break;
    case ED_CLAN:
        cedit( d->character, d->incomm );
        break;
    case ED_FACTION:
        factionedit( d->character, d->incomm );
        break;
    case ED_SOCIAL:
        socialedit( d->character, d->incomm );
        break;
    case ED_HELPOLC:
        helpsedit( d->character, d->incomm );
        break;
    case ED_TODOOLC:
        todoedit( d->character, d->incomm );
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

char *olc_ed_name( CHAR_DATA * ch )
{
    buf[0] = '\0';

    switch ( ch->desc->editor )
    {
    case ED_AREA:
        sprintf( buf, "AEdit" );
        break;
    case ED_ROOM:
        sprintf( buf, "REdit" );
        break;
    case ED_OBJECT:
        sprintf( buf, "OEdit" );
        break;
    case ED_MOBILE:
        sprintf( buf, "MEdit" );
        break;
    case ED_TODO:
        sprintf( buf, "TEdit" );
        break;
    case ED_HELP:
        sprintf( buf, "HEdit" );
        break;
    case ED_MPROG:
        sprintf( buf, "MPEdit" );
        break;
    case ED_MPGROUP:
        sprintf( buf, "MPGEdit" );
        break;
    case ED_CLAN:
        sprintf( buf, "CEdit" );
        break;
    case ED_FACTION:
        sprintf( buf, "FactionEdit" );
        break;
    case ED_SOCIAL:
        sprintf( buf, "SocialEdit" );
        break;
    case ED_HELPOLC:
        sprintf( buf, "HelpEdit" );
        break;
    default:
        sprintf( buf, " " );
        break;
    }

    return buf;
}

/*****************************************************************************
 Name:		show_olc_cmds
 Purpose:	Format up the commands from given table.
 Called by:	show_commands(olc_act.c).
 ****************************************************************************/
void show_olc_cmds( CHAR_DATA * ch, const struct olc_cmd_type *olc_table )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    int cmd;
    int col;

    buf1[0] = '\0';
    col = 0;
    for ( cmd = 0; olc_table[cmd].name[0] != '\0'; cmd++ )
    {
        sprintf( buf, "%-15.15s", olc_table[cmd].name );
        if ( str_cmp( olc_table[cmd].name, "oclan" ) )
            strcat( buf1, buf );
        if ( ++col % 5 == 0 )
            strcat( buf1, "\n\r" );
    }

    if ( col % 5 != 0 )
        strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return;
}

/*****************************************************************************
 Name:		show_commands
 Purpose:	Display all olc commands.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_commands( CHAR_DATA * ch, char *argument )
{
    switch ( ch->desc->editor )
    {
    case ED_AREA:
        show_olc_cmds( ch, aedit_table );
        break;
    case ED_ROOM:
        show_olc_cmds( ch, redit_table );
        break;
    case ED_OBJECT:
        show_olc_cmds( ch, oedit_table );
        break;
    case ED_MOBILE:
        show_olc_cmds( ch, medit_table );
        break;
    case ED_MPROG:
        show_olc_cmds( ch, mpedit_table );
        break;
    case ED_MPGROUP:
        show_olc_cmds( ch, mpgedit_table );
        break;
    case ED_CLAN:
        show_olc_cmds( ch, cedit_table );
    case ED_FACTION:
        show_olc_cmds( ch, factionedit_table );
    case ED_SOCIAL:
        show_olc_cmds( ch, socialedit_table );
    case ED_HELPOLC:
        show_olc_cmds( ch, helpsedit_table );
    case ED_TODOOLC:
        show_olc_cmds( ch, todoedit_table );
    }

    return FALSE;
}

/*****************************************************************************
 *                           Interpreter Tables.                             *
 *****************************************************************************/
const struct olc_cmd_type aedit_table[] = {
/*  {   command		function	}, */

    {"age", aedit_age},
    {"builder", aedit_builder}, /* s removed -- Hugin */
    {"commands", show_commands},
    {"create", aedit_create},
    {"filename", aedit_file},
    {"name", aedit_name},
/*  {   "recall",	aedit_recall	},   ROM OLC */
    {"reset", aedit_reset},
    {"security", aedit_security},
    {"show", aedit_show},
    {"vnum", aedit_vnum},
    {"lvnum", aedit_lvnum},
    {"uvnum", aedit_uvnum},
    {"change", aedit_change},
    {"?", show_help},
    {"version", show_version},

    {"", 0,}
};

const struct olc_cmd_type redit_table[] = {
/*  {   command		function	}, */

    {"commands", show_commands},
    {"create", redit_create},
    {"desc", redit_desc},
    {"ed", redit_ed},
    {"format", redit_format},
    {"name", redit_name},
    {"show", redit_show},

    {"north", redit_north},
    {"south", redit_south},
    {"east", redit_east},
    {"west", redit_west},
    {"up", redit_up},
    {"down", redit_down},

    /* New reset commands. */
    {"mreset", redit_mreset},
    {"oreset", redit_oreset},
    {"mfreevnums", redit_mfreevnums},
    {"ofreevnums", redit_ofreevnums},
    {"rfreevnums", redit_rfreevnums},
    {"mlist", redit_mlist},
    {"olist", redit_olist},
    {"mshow", redit_mshow},
    {"oshow", redit_oshow},

    {"?", show_help},
    {"version", show_version},

    {"", 0,}
};

const struct olc_cmd_type oedit_table[] = {
/*  {   command		function	}, */

    {"addaffect", oedit_addaffect},
    {"commands", show_commands},
    {"cost", oedit_cost},
    {"create", oedit_create},
    {"delaffect", oedit_delaffect},
    {"ed", oedit_ed},
    {"long", oedit_long},
    {"name", oedit_name},
    {"short", oedit_short},
    {"show", oedit_show},
    {"v0", oedit_value0},
    {"v1", oedit_value1},
    {"v2", oedit_value2},
    {"v3", oedit_value3},
    {"v4", oedit_value4},       /* ROM */
    {"weight", oedit_weight},

    {"extra", oedit_extra},     /* ROM */
    {"wear", oedit_wear},       /* ROM */
    {"type", oedit_type},       /* ROM */
    {"material", oedit_material},   /* ROM */
    {"level", oedit_level},     /* ROM */
    {"condition", oedit_condition}, /* ROM */

    {"?", show_help},
    {"version", show_version},
    {"oclan", oedit_clan},
    {"", 0,}
};

const struct olc_cmd_type medit_table[] = {
/*  {   command		function	}, */

    {"alignment", medit_align},
    {"commands", show_commands},
    {"create", medit_create},
    {"desc", medit_desc},
    {"level", medit_level},
    {"long", medit_long},
    {"name", medit_name},
    {"shop", medit_shop},
    {"short", medit_short},
    {"show", medit_show},

    {"sex", medit_sex},         /* ROM */
    {"ac", medit_ac},           /* ROM */
    {"act", medit_act},         /* ROM */
    {"affect", medit_affect},   /* ROM */
    {"form", medit_form},       /* ROM */
    {"part", medit_part},       /* ROM */
    {"imm", medit_imm},         /* ROM */
    {"res", medit_res},         /* ROM */
    {"vuln", medit_vuln},       /* ROM */
    {"material", medit_material},   /* ROM */
    {"off", medit_off},         /* ROM */
    {"size", medit_size},       /* ROM */
    {"hitdice", medit_hitdice}, /* ROM */
    {"manadice", medit_manadice},   /* ROM */
    {"damdice", medit_damdice}, /* ROM */
    {"damtype", medit_damtype}, /* ROM */
    {"race", medit_race},       /* ROM */
    {"position", medit_position},   /* ROM */
    {"gold", medit_gold},       /* ROM */
    {"breath damage", medit_bdamage},
    {"hitroll", medit_hitroll}, /* ROM */
    {"chance", medit_randpct},  /* ROM */
    {"number", medit_randnum},  /* ROM */
    {"types", medit_randtype},  /* ROM */

    {"faction", medit_faction},

    {"?", show_help},
    {"version", show_version},

    {"", 0,}
};

const struct olc_cmd_type mpedit_table[] = {
/*  {   command,		function	}, */

    {"commands", show_commands},
    {"assign", mpedit_assign},
    {"create", mpedit_create},
    {"delete", mpedit_delete},
    {"description", mpedit_description},
    {"edit", mpedit_edit},
    {"groups", mpedit_groups},
    {"name", mpedit_name},
    {"progtype", mpedit_progtype},
    {"unassign", mpedit_unassign},
    {"show", mpedit_show},
    {"trigger", mpedit_trigger},
    {"triggertype", mpedit_triggertype},

    {"?", show_help},
    {"version", show_version},

    {"", 0,}
};

const struct olc_cmd_type mpgedit_table[] = {
/*  {   command		function	}, */

    {"add", mpgedit_add},
    {"commands", show_commands},
    {"create", mpgedit_create},
    {"delete", mpgedit_delete},
    {"description", mpgedit_description},
    {"name", mpgedit_name},
    {"remove", mpgedit_remove},
    {"show", mpgedit_show},

    {"?", show_help},
    {"version", show_version},

    {"", 0,}
};

/*****************************************************************************
 *                          End Interpreter Tables.                          *
 *****************************************************************************/

/*****************************************************************************
 Name:		get_area_data
 Purpose:	Returns pointer to area with given vnum.
 Called by:	do_aedit(olc.c).
 ****************************************************************************/
AREA_DATA *get_area_data( int vnum )
{
    AREA_DATA *pArea;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        if ( pArea->vnum == vnum )
            return pArea;
    }

    return 0;
}

/*****************************************************************************
 Name:		edit_done
 Purpose:	Resets builder information on completion.
 Called by:	aedit, redit, oedit, medit(olc.c)
 ****************************************************************************/
bool edit_done( CHAR_DATA * ch )
{
    REMOVE_BIT( ch->act, PLR_BUILDING );
    ch->desc->pEdit = NULL;
    ch->desc->editor = 0;

    if ( OLC_AUTOSAVE != 0 )
    {
        do_asave( ch, "changed" );
    }

    act( "$n has exited an OLC Editor.", ch, NULL, NULL, TO_ROOM );
    return FALSE;
}

/*****************************************************************************
 *                              Interpreters.                                *
 *****************************************************************************/

/* Area Interpreter, called by do_aedit. */
void aedit( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    AREA_DATA *pArea;
    char command[MAX_INPUT_LENGTH];
    int cmd;
    int value;

    EDIT_AREA( ch, pArea );
    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( !IS_BUILDER( ch, pArea ) )
        send_to_char( "AEdit:  Insufficient security to modify area.\n\r", ch );

    if ( command[0] == '\0' )
    {
        aedit_show( ch, argument );
        return;
    }

    if ( !str_cmp( command, "done" ) )
    {
        edit_done( ch );
        return;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
        interpret( ch, arg );
        return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; aedit_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( !str_prefix( command, aedit_table[cmd].name ) )
        {
            if ( ( *aedit_table[cmd].olc_fun ) ( ch, argument ) )
                SET_BIT( pArea->area_flags, AREA_CHANGED );

            return;
        }
    }

    if ( ( value = flag_value( area_flags, arg ) ) != NO_FLAG )
    {
        TOGGLE_BIT( pArea->area_flags, value );

        if ( value != AREA_CHANGED )
            SET_BIT( pArea->area_flags, AREA_CHANGED );

        send_to_char( "Flag toggled.\n\r", ch );
        return;
    }

    interpret( ch, arg );

    /* Default to Standard Interpreter. */

    return;
}

/* Room Interpreter, called by do_redit. */
void redit( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *pRoom;
    AREA_DATA *pArea;
    char command[MAX_INPUT_LENGTH];
    int cmd;
    int value;

    EDIT_ROOM( ch, pRoom );
    pArea = pRoom->area;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    if ( !IS_BUILDER( ch, pArea ) )
        send_to_char( "REdit:  Insufficient security to modify room.\n\r", ch );

    if ( command[0] == '\0' )
    {
        redit_show( ch, argument );
        return;
    }

    if ( !str_cmp( command, "done" ) )
    {
        edit_done( ch );
        return;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
        interpret( ch, arg );
        return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; redit_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( !str_prefix( command, redit_table[cmd].name ) )
        {
            if ( ( *redit_table[cmd].olc_fun ) ( ch, argument ) )
                SET_BIT( pArea->area_flags, AREA_CHANGED );

            return;
        }
    }

    if ( ( value = flag_value( room_flags, arg ) ) != NO_FLAG )
    {
        TOGGLE_BIT( pRoom->room_flags, value );

        SET_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char( "Room flag toggled.\n\r", ch );
        return;
    }

    if ( ( value = flag_value( sector_flags, arg ) ) != NO_FLAG )
    {
        pRoom->sector_type = value;

        SET_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char( "Sector type set.\n\r", ch );
        return;
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}

/* Object Interpreter, called by do_oedit. */
void oedit( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    AREA_DATA *pArea;
    OBJ_INDEX_DATA *pObj;
    char command[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_OBJ( ch, pObj );
    pArea = pObj->area;

    if ( !IS_BUILDER( ch, pArea ) )
        send_to_char( "OEdit: Insufficient security to modify area.\n\r", ch );

    if ( command[0] == '\0' )
    {
        oedit_show( ch, argument );
        return;
    }

    if ( !str_cmp( command, "done" ) )
    {
        /* 
         * Check to make sure the damage type is set if the item is a weapon.
         * If it is, tell the user to set it and don't allow exiting the editor.
         */
        if ( pObj->item_type == ITEM_WEAPON && pObj->value[3] == 0 )
        {
            send_to_char
                ( "OEdit: You must set the weapon's damage type (v3) before exiting.\n\r",
                  ch );
            return;
        }

        edit_done( ch );
        return;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
        interpret( ch, arg );
        return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; oedit_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( !str_prefix( command, oedit_table[cmd].name ) )
        {
            if ( ( *oedit_table[cmd].olc_fun ) ( ch, argument ) )
                SET_BIT( pArea->area_flags, AREA_CHANGED );

            return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}

/* Mobile Interpreter, called by do_medit. */
void medit( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    AREA_DATA *pArea;
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

    EDIT_MOB( ch, pMob );
    pArea = pMob->area;

    if ( !IS_BUILDER( ch, pArea ) )
        send_to_char( "MEdit: Insufficient security to modify area.\n\r", ch );

    if ( command[0] == '\0' )
    {
        medit_show( ch, argument );
        return;
    }

    if ( !str_cmp( command, "done" ) )
    {
        edit_done( ch );
        return;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
        interpret( ch, arg );
        return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; medit_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( !str_prefix( command, medit_table[cmd].name ) )
        {
            if ( ( *medit_table[cmd].olc_fun ) ( ch, argument ) )
                SET_BIT( pArea->area_flags, AREA_CHANGED );

            return;
        }
    }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}

/* MudProg Interpreter, called by do_mpedit. */
void mpedit( CHAR_DATA * ch, char *argument )
{
    MPROG_DATA *pMudProg;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int cmd;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

#ifdef REQUIRE_MUDPROG_PERMISSION
    if ( !IS_SET( ch->act, PLR_CAN_PROG ) )
    {
        send_to_char( "You don't have permission.\n\r", ch );
        return;
    }
#endif

    if ( command[0] == '\0' )
    {
        mpedit_show( ch, "" );
        return;
    }

    if ( !str_cmp( command, "done" ) )
    {
        EDIT_MPROG( ch, pMudProg );
        if ( !pMudProg->vnum )
        {
            pMudProg->vnum = get_new_mprog_vnum(  );
            if ( mudprog_last )
            {
                mudprog_last->next = pMudProg;
                mudprog_last = pMudProg;
            }
            else
            {
                mudprog_last = pMudProg;
                mudprog_first = pMudProg;
            }
            pMudProg->next = NULL;

            sprintf( buf, "MudProg assigned vnum [%4d].\n\r", pMudProg->vnum );
            send_to_char( buf, ch );
        }
        edit_done( ch );
        return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; mpedit_table[cmd].name[0] != '\0'; cmd++ )
        if ( !str_prefix( command, mpedit_table[cmd].name ) )
        {
            ( *mpedit_table[cmd].olc_fun ) ( ch, argument );
            return;
        }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}

/* MudProg Group Interpreter, called by do_mpgedit. */
void mpgedit( CHAR_DATA * ch, char *argument )
{
    MPROG_GROUP *pGroup;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int cmd;

    smash_tilde( argument );
    strcpy( arg, argument );
    argument = one_argument( argument, command );

#ifdef REQUIRE_MUDPROG_PERMISSION
    if ( !IS_SET( ch->act, PLR_CAN_PROG ) )
    {
        send_to_char( "You don't have permission.\n\r", ch );
        return;
    }
#endif

    if ( command[0] == '\0' )
    {
        mpgedit_show( ch, "" );
        return;
    }

    if ( !str_cmp( command, "done" ) )
    {
        EDIT_MPGROUP( ch, pGroup );
        if ( !pGroup->vnum )
        {
            pGroup->vnum = get_new_mprog_vnum(  );

            if ( mprog_group_last )
            {
                mprog_group_last->next = pGroup;
                mprog_group_last = pGroup;
            }
            else
            {
                mprog_group_first = pGroup;
                mprog_group_last = pGroup;
            }
            pGroup->next = NULL;

            sprintf( buf, "MudProg Group assigned vnum [%4d].\n\r",
                     pGroup->vnum );
            send_to_char( buf, ch );
        }
        edit_done( ch );
        return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; mpgedit_table[cmd].name[0] != '\0'; cmd++ )
        if ( !str_prefix( command, mpgedit_table[cmd].name ) )
        {
            ( *mpgedit_table[cmd].olc_fun ) ( ch, argument );
            return;
        }

    /* Default to Standard Interpreter. */
    interpret( ch, arg );
    return;
}

/* Entry point for editing mud_prog_data. */
void do_mpedit( CHAR_DATA * ch, char *argument )
{
    MPROG_DATA *pMudProg;
    int value;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    if ( IS_NPC( ch ) )
        return;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( is_number( arg1 ) )
    {
        value = atoi( arg1 );
        if ( !( pMudProg = get_mprog_by_vnum( value ) ) )
        {
            send_to_char( "MPEdit:  That vnum does not exist.\n\r", ch );
            return;
        }

        ch->desc->pEdit = ( void * ) pMudProg;
        ch->desc->editor = ED_MPROG;
        SET_BIT( ch->act, PLR_BUILDING );
        act( "$n has entered the MudProg Editor.", ch, NULL, NULL, TO_ROOM );

        return;
    }
    else
    {
        if ( !str_cmp( arg1, "create" ) )
        {
            if ( mpedit_create( ch, argument ) )
                ch->desc->editor = ED_MPROG;

            return;
        }

        if ( !str_cmp( arg1, "assign" ) )
        {
            mpedit_assign( ch, argument );
            return;
        }

        if ( !str_cmp( arg1, "unassign" ) )
        {
            mpedit_unassign( ch, argument );
            return;
        }

    }

    send_to_char
        ( "Syntax:  edit mprog <vnum> or edit mprog create [mobprog||objprog||roomprog]\n\r"
          "         edit mprog assign   <target vnum> <mudprog vnum>\n\r"
          "         edit mprog unassign <target vnum> <mudprog vnum>\n\r", ch );
    return;
}

/* Entry point for editing mud_prog_group. */
void do_mpgedit( CHAR_DATA * ch, char *argument )
{
    MPROG_GROUP *pGroup;
    int value;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    if ( IS_NPC( ch ) )
        return;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( is_number( arg1 ) )
    {
        value = atoi( arg1 );
        if ( !( pGroup = get_mprog_group_by_vnum( value ) ) )
        {
            send_to_char( "MPGEdit:  That vnum does not exist.\n\r", ch );
            return;
        }

        ch->desc->pEdit = ( void * ) pGroup;
        ch->desc->editor = ED_MPGROUP;
        SET_BIT( ch->act, PLR_BUILDING );
        act( "$n has entered the MudProg Groups Editor.", ch, NULL, NULL,
             TO_ROOM );
        return;
    }
    else
    {
        if ( !str_cmp( arg1, "create" ) )
        {
            if ( mpgedit_create( ch, argument ) )
                ch->desc->editor = ED_MPGROUP;

            return;
        }
    }

    send_to_char
        ( "Syntax:  edit mpgroup <vnum> or edit mpgroup create [mobprog||objprog||roomprog]\n\r",
          ch );
    return;
}

const struct editor_cmd_type editor_table[] = {
/*  {   command		function	}, */

    {"area", do_aedit},
    {"room", do_redit},
    {"object", do_oedit},
    {"mobile", do_medit},
    {"mprog", do_mpedit},
    {"mpgroup", do_mpgedit},
    {"clan", do_cedit},
    {"faction", do_factionedit},
    {"social", do_socialedit},
    {"help", do_helpedit},
    {"todo", do_todoedit},
    {"", 0,}
};

/* Entry point for all editors. */
void do_olc( CHAR_DATA * ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    int cmd;

#ifdef REQUIRE_EDIT_PERMISSION
    if ( !IS_SET( ch->act, PLR_CAN_EDIT ) )
    {
        send_to_char( "You don't have permission.\n\r", ch );
        return;
    }
#endif

/*     if ( ch->pcdata &&
       ( ch->in_room->vnum < ch->pcdata->vnum_range[0]
       || ch->in_room->vnum > ch->pcdata->vnum_range[1] ) )
       {
          send_to_char( "You're not authorized to use this command here.\n\r", ch );
          return;
       }       */

    argument = one_argument( argument, command );

    if ( command[0] == '\0' )
    {
        do_help( ch, "olc" );
        return;
    }
    else if ( command[0] == '\0' )
    {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    /* Search Table and Dispatch Command. */
    for ( cmd = 0; editor_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( !str_prefix( command, editor_table[cmd].name ) )
        {
            ( *editor_table[cmd].do_fun ) ( ch, argument );
            return;
        }
    }

    /* Invalid command, send help. */
    do_help( ch, "olc" );
    return;
}

/* Entry point for editing area_data. */
void do_aedit( CHAR_DATA * ch, char *argument )
{
    AREA_DATA *pArea;
    int value;

    pArea = ch->in_room->area;

    if ( is_number( argument ) )
    {
        value = atoi( argument );

        if ( !( pArea = get_area_data( value ) ) )
        {
            send_to_char( "That area vnum does not exist.\n\r", ch );
            return;
        }
    }
    else
    {
        if ( !str_cmp( argument, "create" ) )
        {
            pArea = new_area(  );
            area_last->next = pArea;
            area_last = pArea;  /* Thanks, Walker. */
            SET_BIT( pArea->area_flags, AREA_ADDED );
        }
    }

    SET_BIT( ch->act, PLR_BUILDING );
    act( "$n has entered the Area Editor.", ch, NULL, NULL, TO_ROOM );

    ch->desc->pEdit = ( void * ) pArea;
    ch->desc->editor = ED_AREA;
    return;
}

/* Entry point for editing room_index_data. */
void do_redit( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *pRoom;

    argument = one_argument( argument, arg );

    pRoom = ch->in_room;

    if ( !str_cmp( arg, "reset" ) )
    {
        reset_room( pRoom );
        send_to_char( "Room reset.\n\r", ch );
        return;
    }
    else if ( !str_cmp( arg, "create" ) )
    {
        if ( argument[0] == '\0' || atoi( argument ) == 0 )
        {
            send_to_char( "Syntax:  edit room create [vnum]\n\r", ch );
            return;
        }

        if ( redit_create( ch, argument ) )
        {
            char_from_room( ch );
            char_to_room( ch, ch->desc->pEdit );
            SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
        }
    }

    ch->desc->editor = ED_ROOM;
    SET_BIT( ch->act, PLR_BUILDING );
    act( "$n has entered the Room Editor.", ch, NULL, NULL, TO_ROOM );

    return;
}

/* Entry point for editing obj_index_data. */
void do_oedit( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    int value;

    argument = one_argument( argument, arg );

    if ( is_number( arg ) )
    {
        value = atoi( arg );
        if ( !( pObj = get_obj_index( value ) ) )
        {
            send_to_char( "OEdit:  That vnum does not exist.\n\r", ch );
            return;
        }

        ch->desc->pEdit = ( void * ) pObj;
        ch->desc->editor = ED_OBJECT;
        return;
    }
    else
    {
        if ( !str_cmp( arg, "create" ) )
        {
            value = atoi( argument );
            if ( argument[0] == '\0' || value == 0 )
            {
                send_to_char( "Syntax:  edit object create [vnum]\n\r", ch );
                return;
            }

            pArea = get_vnum_area( value );

            if ( oedit_create( ch, argument ) )
            {
                SET_BIT( pArea->area_flags, AREA_CHANGED );
                ch->desc->editor = ED_OBJECT;
                SET_BIT( ch->act, PLR_BUILDING );
                act( "$n has entered the Object Editor.", ch, NULL, NULL,
                     TO_ROOM );
            }

            return;
        }
    }

    send_to_char( "OEdit:  There is no default object to edit.\n\r", ch );
    return;
}

/* Entry point for editing mob_index_data. */
void do_medit( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int value;

    argument = one_argument( argument, arg );

    if ( is_number( arg ) )
    {
        value = atoi( arg );
        if ( !( pMob = get_mob_index( value ) ) )
        {
            send_to_char( "MEdit:  That vnum does not exist.\n\r", ch );
            return;
        }

        ch->desc->pEdit = ( void * ) pMob;
        ch->desc->editor = ED_MOBILE;
        SET_BIT( ch->act, PLR_BUILDING );
        act( "$n has entered the Mobile Editor.", ch, NULL, NULL, TO_ROOM );

        return;
    }
    else
    {
        if ( !str_cmp( arg, "create" ) )
        {
            value = atoi( argument );
            if ( arg[0] == '\0' || value == 0 )
            {
                send_to_char( "Syntax:  edit mobile create [vnum]\n\r", ch );
                return;
            }

            pArea = get_vnum_area( value );

            if ( medit_create( ch, argument ) )
            {
                SET_BIT( pArea->area_flags, AREA_CHANGED );
                ch->desc->editor = ED_MOBILE;
                SET_BIT( ch->act, PLR_BUILDING );
                act( "$n has entered the Mobile Editor.", ch, NULL, NULL,
                     TO_ROOM );
            }

            return;
        }
    }

    send_to_char( "MEdit:  There is no default mobile to edit.\n\r", ch );
    return;
}

void display_resets( CHAR_DATA * ch )
{
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *pRoom;
    RESET_DATA *pReset;
    MOB_INDEX_DATA *pMob = NULL;
    char final[MAX_STRING_LENGTH];
    int iReset = 0;

    EDIT_ROOM( ch, pRoom );
    final[0] = '\0';

    send_to_char
        ( " No.  Loads    Description       Location         Vnum    Max  Description"
          "\n\r"
          "==== ======== ============= =================== ======== ===== ==========="
          "\n\r", ch );

    for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
    {
        OBJ_INDEX_DATA *pObj;
        MOB_INDEX_DATA *pMobIndex;
        OBJ_INDEX_DATA *pObjIndex;
        OBJ_INDEX_DATA *pObjToIndex;
        ROOM_INDEX_DATA *pRoomIndex;

        final[0] = '\0';
        sprintf( final, "[%2d] ", ++iReset );

        switch ( pReset->command )
        {
        default:
            sprintf( buf, "Bad reset command: %c.", pReset->command );
            strcat( final, buf );
            break;

        case 'M':
            if ( !( pMobIndex = get_mob_index( pReset->vnum ) ) )
            {
                sprintf( buf, "Load Mobile - Bad Mob %d\n\r", pReset->vnum );
                strcat( final, buf );
                continue;
            }

            if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
            {
                sprintf( buf, "Load Mobile - Bad Room %d\n\r", pReset->arg3 );
                strcat( final, buf );
                continue;
            }

            pMob = pMobIndex;
            sprintf( buf,
                     "M[%5d] %-13.13s in room             R[%5d] [%3d] %-15.15s\n\r",
                     pReset->vnum, pMob->short_descr, pReset->arg3,
                     ( pReset->arg2 == 0 ) ? 1 : pReset->arg2,
                     pRoomIndex->name );
            strcat( final, buf );

            /*
             * Check for pet shop.
             * -------------------
             */
            {
                ROOM_INDEX_DATA *pRoomIndexPrev;

                pRoomIndexPrev = get_room_index( pRoomIndex->vnum - 1 );
                if ( pRoomIndexPrev
                     && IS_SET( pRoomIndexPrev->room_flags, ROOM_PET_SHOP ) )
                    final[5] = 'P';
            }

            break;

        case 'O':
            if ( !( pObjIndex = get_obj_index( pReset->vnum ) ) )
            {
                sprintf( buf, "Load Object - Bad Object %d\n\r", pReset->vnum );
                strcat( final, buf );
                continue;
            }

            pObj = pObjIndex;

            if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
            {
                sprintf( buf, "Load Object - Bad Room %d\n\r", pReset->arg3 );
                strcat( final, buf );
                continue;
            }

            sprintf( buf, "O[%5d] %-13.13s in room             "
                     "R[%5d]       %-15.15s\n\r",
                     pReset->vnum, pObj->short_descr,
                     pReset->arg3, pRoomIndex->name );
            strcat( final, buf );

            break;

        case 'P':
            if ( !( pObjIndex = get_obj_index( pReset->vnum ) ) )
            {
                sprintf( buf, "Put Object - Bad Object %d\n\r", pReset->vnum );
                strcat( final, buf );
                continue;
            }

            pObj = pObjIndex;

            if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
            {
                sprintf( buf, "Put Object - Bad To Object %d\n\r",
                         pReset->arg3 );
                strcat( final, buf );
                continue;
            }

            sprintf( buf,
                     "O[%5d] %-13.13s inside              O[%5d]       %-15.15s\n\r",
                     pReset->vnum,
                     pObj->short_descr,
                     pReset->arg3, pObjToIndex->short_descr );
            strcat( final, buf );

            break;

        case 'G':
        case 'E':
            if ( !( pObjIndex = get_obj_index( pReset->vnum ) ) )
            {
                sprintf( buf, "Give/Equip Object - Bad Object %d\n\r",
                         pReset->vnum );
                strcat( final, buf );
                continue;
            }

            pObj = pObjIndex;

            if ( !pMob )
            {
                sprintf( buf, "Give/Equip Object - No Previous Mobile\n\r" );
                strcat( final, buf );
                break;
            }

            if ( pMob->pShop )
            {
                sprintf( buf,
                         "O[%5d] %-13.13s in the inventory of S[%5d]       %-15.15s\n\r",
                         pReset->vnum,
                         pObj->short_descr, pMob->vnum, pMob->short_descr );
            }
            else
                sprintf( buf,
                         "O[%5d] %-13.13s %-19.19s M[%5d]       %-15.15s\n\r",
                         pReset->vnum,
                         pObj->short_descr,
                         ( pReset->command == 'G' ) ?
                         flag_string( wear_loc_strings, WEAR_NONE )
                         : flag_string( wear_loc_strings, pReset->arg3 ),
                         pMob->vnum, pMob->short_descr );
            strcat( final, buf );

            break;

            /*
             * Doors are set in rs_flags don't need to be displayed.
             * If you want to display them then uncomment the new_reset
             * line in the case 'D' in load_resets in db.c and here.
             */
        case 'D':
            pRoomIndex = get_room_index( pReset->vnum );
            sprintf( buf, "R[%5d] %s door of %-19.19s reset to %s\n\r",
                     pReset->vnum,
                     capitalize( dir_name[pReset->arg2] ),
                     pRoomIndex->name,
                     flag_string( door_resets, pReset->arg3 ) );
            strcat( final, buf );

            break;
            /*
             * End Doors Comment.
             */
        case 'R':
            if ( !( pRoomIndex = get_room_index( pReset->vnum ) ) )
            {
                sprintf( buf, "Randomize Exits - Bad Room %d\n\r",
                         pReset->vnum );
                strcat( final, buf );
                continue;
            }

            sprintf( buf, "R[%5d] Exits are randomized in %s\n\r",
                     pReset->vnum, pRoomIndex->name );
            strcat( final, buf );

            break;
        }
        send_to_char( final, ch );
    }

    return;
}

/*****************************************************************************
 Name:		add_reset
 Purpose:	Inserts a new reset in the given index slot.
 Called by:	do_resets(olc.c).
 ****************************************************************************/
void add_reset( ROOM_INDEX_DATA * room, RESET_DATA * pReset, int index )
{
    RESET_DATA *reset;
    int iReset = 0;

    if ( !room->reset_first )
    {
        room->reset_first = pReset;
        room->reset_last = pReset;
        pReset->next = NULL;
        return;
    }

    index--;

    if ( index == 0 )           /* First slot (1) selected. */
    {
        pReset->next = room->reset_first;
        room->reset_first = pReset;
        return;
    }

    /*
     * If negative slot( <= 0 selected) then this will find the last.
     */
    for ( reset = room->reset_first; reset->next; reset = reset->next )
    {
        if ( ++iReset == index )
            break;
    }

    pReset->next = reset->next;
    reset->next = pReset;

    if ( !pReset->next )
        room->reset_last = pReset;

    SET_BIT( room->area->area_flags, AREA_CHANGED );
    return;
}

void do_resets( CHAR_DATA * ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char arg5[MAX_INPUT_LENGTH];
    RESET_DATA *pReset = NULL;

/*    if ( ch->pcdata &&
       ( ch->in_room->vnum < ch->pcdata->vnum_range[0]
       || ch->in_room->vnum > ch->pcdata->vnum_range[1] ) )
       {
          send_to_char( "You're not authorized to use this command here.\n\r", ch );
          return;
       }    */

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    argument = one_argument( argument, arg4 );
    argument = one_argument( argument, arg5 );

    /*
     * Display resets in current room.
     * -------------------------------
     */
    if ( arg[0] == '\0' )
    {
        if ( ch->in_room->reset_first )
        {
            send_to_char( "Resets: M = mobile, R = room, O = object, "
                          "P = pet, S = shopkeeper\n\r", ch );
            display_resets( ch );
        }
        else
            send_to_char( "No resets in this room.\n\r", ch );
    }

    if ( !IS_BUILDER( ch, ch->in_room->area ) )
    {
        send_to_char( "Resets: Invalid security for editing this area.\n\r",
                      ch );
        return;
    }

    /*
     * Take index number and search for commands.
     * ------------------------------------------
     */
    if ( is_number( arg ) )
    {
        ROOM_INDEX_DATA *pRoom = ch->in_room;

        /*
         * Delete a reset.
         * ---------------
         */
        if ( !str_cmp( arg2, "delete" ) )
        {
            int insert_loc = atoi( arg );

            if ( !ch->in_room->reset_first )
            {
                send_to_char( "No resets in this room.\n\r", ch );
                return;
            }

            if ( insert_loc - 1 <= 0 )
            {
                pReset = pRoom->reset_first;
                pRoom->reset_first = pRoom->reset_first->next;
                if ( !pRoom->reset_first )
                    pRoom->reset_last = NULL;
                SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
            }
            else
            {
                int iReset = 0;
                RESET_DATA *prev = NULL;

                for ( pReset = pRoom->reset_first; pReset;
                      pReset = pReset->next )
                {
                    if ( ++iReset == insert_loc )
                        break;

                    prev = pReset;
                }

                if ( !pReset )
                {
                    send_to_char( "Reset not found.\n\r", ch );
                    return;
                }

                if ( prev )
                    prev->next = prev->next->next;
                else
                    pRoom->reset_first = pRoom->reset_first->next;

                for ( pRoom->reset_last = pRoom->reset_first;
                      pRoom->reset_last->next;
                      pRoom->reset_last = pRoom->reset_last->next );
            }

            free_reset_data( pReset );
            SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
            send_to_char( "Reset deleted.\n\r", ch );
        }
        else
            /*
             * Add a reset.
             * ------------
             */
        if ( ( !str_cmp( arg2, "mob" ) && is_number( arg3 ) )
                 || ( !str_cmp( arg2, "obj" ) && is_number( arg3 ) ) )
        {
            /*
             * Check to see if the mob or object to be reset exists
             * in the game.  This is added because of Rajic's fat 
             * fingers - Dorzak Tempestuous Realms
             */
            if ( !str_cmp( arg2, "mob" ) )
            {
                MOB_INDEX_DATA *pMobIndex;
                if ( ( pMobIndex = get_mob_index( atoi( arg3 ) ) ) == NULL )
                {
                    send_to_char( "No mob has that vnum.- Invalid Reset\n\r",
                                  ch );
                    return;
                }
            }
            if ( !str_cmp( arg2, "obj" ) )
            {
                OBJ_INDEX_DATA *pObjIndex;
                if ( ( pObjIndex = get_obj_index( atoi( arg3 ) ) ) == NULL )
                {
                    send_to_char( "No object has that vnum.\n\r", ch );
                    return;
                }
            }

            /*
             * Check for Mobile reset.
             * -----------------------
             */
            if ( !str_cmp( arg2, "mob" ) )
            {
                pReset = new_reset_data(  );
                pReset->command = 'M';
                pReset->vnum = atoi( arg3 );
                pReset->arg2 = is_number( arg4 ) ? atoi( arg4 ) : 1;    /* Max # */
                pReset->arg3 = ch->in_room->vnum;
            }
            else
                /*
                 * Check for Object reset.
                 * -----------------------
                 */
            if ( !str_cmp( arg2, "obj" ) )
            {
                pReset = new_reset_data(  );
                pReset->vnum = atoi( arg3 );
                /*
                 * Inside another object.
                 * ----------------------
                 */
                if ( !str_prefix( arg4, "inside" ) )
                {
                    pReset->command = 'P';
                    pReset->arg2 = 0;
                    pReset->arg3 = is_number( arg5 ) ? atoi( arg5 ) : 1;
                }
                else
                    /*
                     * Inside the room.
                     * ----------------
                     */
                if ( !str_cmp( arg4, "room" ) )
                {
                    pReset->command = 'O';
                    pReset->arg2 = 0;
                    pReset->arg3 = ch->in_room->vnum;
                }
                else
                    /*
                     * Into a Mobile's inventory.
                     * --------------------------
                     */
                {
                    if ( flag_value( wear_loc_flags, arg4 ) == NO_FLAG )
                    {
                        send_to_char( "Resets: '? wear-loc'\n\r", ch );
                        return;
                    }

                    pReset->arg3 = flag_value( wear_loc_flags, arg4 );
                    if ( pReset->arg3 == WEAR_NONE )
                        pReset->command = 'G';
                    else
                        pReset->command = 'E';
                }
            }

            SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );

            add_reset( ch->in_room, pReset, atoi( arg ) );
            send_to_char( "Reset added.\n\r", ch );
        }
        else
        {
            send_to_char( "Syntax: RESET <number> OBJ <vnum> <wear_loc>\n\r",
                          ch );
            send_to_char( "        RESET <number> OBJ <vnum> in <vnum>\n\r",
                          ch );
            send_to_char( "        RESET <number> OBJ <vnum> room\n\r", ch );
            send_to_char( "        RESET <number> MOB <vnum> [<max #>]\n\r",
                          ch );
            send_to_char( "        RESET <number> DELETE\n\r", ch );
        }
    }

    return;
}

/*****************************************************************************
 Name:		do_alist
 Purpose:	Normal command to list areas and display area information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_alist( CHAR_DATA * ch, char *argument )
{
    char result[MAX_STRING_LENGTH * 2]; /* May need tweaking. */
    char buf[MAX_STRING_LENGTH];
    AREA_DATA *pArea;

    sprintf( result, "[%3s] [%-27s] (%-5s-%5s) [%-10s] %3s [%-10s]\n\r",
             "Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec",
             "Builders" );

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        sprintf( buf, "[%3d] %-29.29s (%-5d-%5d) %-12.12s [%d] [%-10.10s]\n\r",
                 pArea->vnum,
                 &pArea->name[8],
                 pArea->lvnum,
                 pArea->uvnum,
                 pArea->filename, pArea->security, pArea->builders );
        strcat( result, buf );
    }

    send_to_char( result, ch );
    return;
}

int get_new_mprog_vnum( void )
{
    MPROG_DATA *pMudProg;
    MPROG_GROUP *pMprogGroup;
    int vnum;

    vnum = 0;

    for ( pMudProg = mudprog_first; pMudProg; pMudProg = pMudProg->next )
        if ( pMudProg->vnum > vnum )
            vnum = pMudProg->vnum;

    for ( pMprogGroup = mprog_group_first; pMprogGroup;
          pMprogGroup = pMprogGroup->next )
        if ( pMprogGroup->vnum > vnum )
            vnum = pMprogGroup->vnum;

    return ++vnum;
}
