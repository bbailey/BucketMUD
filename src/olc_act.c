/***************************************************************************
 *  File: olc_act.c                                                        *
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
 * Mudprogram's (Mobprogram, Objprogram and Roomprogram) originaly        *
 * by the SMAUG development team                                          *
 * Ported to EmberMUD by Thanatos and Tyrluk of ToED                      *
 * (Temple of Eternal Death)                                              *
 * Tyrluk   - morn@telmaron.com or dajy@mindspring.com                    *
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
char arg[MAX_INPUT_LENGTH];

/* Return TRUE if area changed, FALSE if not. */
#define REDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define OEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define MEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define AEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define MPEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define MPGEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
struct olc_help_type {
    char *command;
    const void *structure;
    char *desc;
};

/* Added to add a standard for mob's stats. -Lancelight */
bool show_version( CHAR_DATA * ch, char *argument )
{
    send_to_char( VERSION, ch );
    send_to_char( "\n\r", ch );
    send_to_char( AUTHOR, ch );
    send_to_char( "\n\r", ch );
    send_to_char( DATE, ch );
    send_to_char( "\n\r", ch );
    send_to_char( CREDITS, ch );
    send_to_char( "\n\r", ch );
    send_to_char( ADDONS, ch );
    send_to_char( "\n\r", ch );

    return FALSE;
}

/*
 * This table contains help commands and a brief description of each.
 * ------------------------------------------------------------------
 */
const struct olc_help_type help_table[] = {
    {"area", area_flags, "Area attributes."},
    {"room", room_flags, "Room attributes."},
    {"sector", sector_flags, "Sector types, terrain."},
    {"exit", exit_flags, "Exit types."},
    {"type", type_flags, "Types of objects."},
    {"extra", extra_flags, "Object attributes."},
    {"wear", wear_flags, "Where to wear object."},
    {"sex", sex_flags, "Sexes."},
    {"addaffect", apply_flags, "Addaffect Locations."},
    {"ac", ac_type, "Ac for different attacks."},
    {"act", act_flags, "Mobile attributes."},
    {"affect", affect_flags, "Mobile affects."},
    {"wear-loc", wear_loc_flags, "Where mobile wears object."},
    {"spells", skill_table, "Names of current spells."},
    {"weapon", weapon_flags, "Type of weapon."},
    {"container", container_flags, "Container status."},
    {"liquid", liquid_flags, "Types of liquids."},
    {"mudprogs", mprog_type_flags, "Types of MudProgs."},

/* ROM specific bits: */

    {"form", form_flags, "Mobile body form."},
    {"part", part_flags, "Mobile body parts."},
    {"imm", imm_flags, "Mobile immunity."},
    {"res", res_flags, "Mobile resistance."},
    {"vuln", vuln_flags, "Mobile vlnerability."},
    {"off", off_flags, "Mobile offensive behaviour."},
    {"size", size_flags, "Mobile size."},
    {"position", position_flags, "Mobile positions."},
    {"material", material_type, "Material mob/obj is made from."},
    {"wclass", weapon_class, "Weapon class."},
    {"wtype", weapon_type, "Special weapon type."},
    {"furniture", furniture_flags, "Furniture Flags."},

    {"randobj", rnd_obj_flags, "Random object types."},

/* Clans */

    {"joinflags", clan_join_flags, "Clan join flags."},
    {"clanflags", clan_flags, "Clan flags."},
    {"", 0, ""}
};

char *mprog_type_to_name args( ( int type ) );

/*****************************************************************************
 Name:		show_flag_cmds
 Purpose:	Displays settable flags and stats.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_flag_cmds( CHAR_DATA * ch, const struct flag_type *flag_table )
{
    char buf1[MAX_STRING_LENGTH];
    int flag;
    int col;

    buf1[0] = '\0';
    col = 0;
    for ( flag = 0; flag_table[flag].name[0] != '\0'; flag++ )
    {
        if ( flag_table[flag].settable )
        {
            sprintf( buf, "%-19.18s", flag_table[flag].name );
            strcat( buf1, buf );
            if ( ++col % 4 == 0 )
                strcat( buf1, "\n\r" );
        }
    }

    if ( col % 4 != 0 )
        strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return;
}

/*****************************************************************************
 Name:		show_skill_cmds
 Purpose:	Displays all skill functions.
 		Does remove those damn immortal commands from the list.
 		Could be improved by:
 		(1) Adding a check for a particular class.
 		(2) Adding a check for a level range.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_skill_cmds( CHAR_DATA * ch, int tar )
{
    char buf1[MAX_STRING_LENGTH * 2];
    int sn;
    int col;

    buf1[0] = '\0';
    col = 0;
    for ( sn = 0; sn < MAX_SKILL; sn++ )
    {
        if ( !skill_table[sn].name )
            break;

        if ( !str_cmp( skill_table[sn].name, "reserved" )
             || skill_table[sn].spell_fun == spell_null )
            continue;

        if ( tar == -1 || skill_table[sn].target == tar )
        {
            sprintf( buf, "%-19.18s", skill_table[sn].name );
            strcat( buf1, buf );
            if ( ++col % 4 == 0 )
                strcat( buf1, "\n\r" );
        }
    }

    if ( col % 4 != 0 )
        strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return;
}

/*****************************************************************************
 Name:		show_help
 Purpose:	Displays help for many tables used in OLC.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_help( CHAR_DATA * ch, char *argument )
{
    char spell[MAX_INPUT_LENGTH];
    int cnt;

    argument = one_argument( argument, arg );
    one_argument( argument, spell );

    /*
     * Display syntax.
     */
    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax:  ? [command]\n\r\n\r", ch );
        send_to_char( "[command]  [description]\n\r", ch );
        for ( cnt = 0; help_table[cnt].command[0] != '\0'; cnt++ )
        {
            sprintf( buf, "%-10.10s -%s\n\r",
                     capitalize( help_table[cnt].command ),
                     help_table[cnt].desc );
            send_to_char( buf, ch );
        }
        return FALSE;
    }

    /*
     * Find the command, show changeable data.
     * ---------------------------------------
     */
    for ( cnt = 0; help_table[cnt].command[0]; cnt++ )
    {
        if ( arg[0] == help_table[cnt].command[0]
             && !str_prefix( arg, help_table[cnt].command ) )
        {
            if ( help_table[cnt].structure == skill_table )
            {

                if ( !spell[0] )
                {
                    send_to_char( "Syntax:  ? spells "
                                  "[ignore/attack/defend/self/object/all]\n\r",
                                  ch );
                    return FALSE;
                }

                if ( !str_prefix( spell, "all" ) )
                    show_skill_cmds( ch, -1 );
                else if ( !str_prefix( spell, "ignore" ) )
                    show_skill_cmds( ch, TAR_IGNORE );
                else if ( !str_prefix( spell, "attack" ) )
                    show_skill_cmds( ch, TAR_CHAR_OFFENSIVE );
                else if ( !str_prefix( spell, "defend" ) )
                    show_skill_cmds( ch, TAR_CHAR_DEFENSIVE );
                else if ( !str_prefix( spell, "self" ) )
                    show_skill_cmds( ch, TAR_CHAR_SELF );
                else if ( !str_prefix( spell, "object" ) )
                    show_skill_cmds( ch, TAR_OBJ_INV );
                else
                    send_to_char( "Syntax:  ? spell "
                                  "[ignore/attack/defend/self/object/all]\n\r",
                                  ch );

                return FALSE;
            }
            else
            {
                show_flag_cmds( ch, help_table[cnt].structure );
                return FALSE;
            }
        }
    }

    show_help( ch, "" );
    return FALSE;
}

REDIT( redit_mlist )
{
    MOB_INDEX_DATA *pMobIndex;
    AREA_DATA *pArea;
    char *dupl;

    char buf1[MAX_STRING_LENGTH * 2];
    bool fAll, found;
    int vnum;
    int col = 0;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax:  mlist <all/name>\n\r", ch );
        return FALSE;
    }

    pArea = ch->in_room->area;
    buf1[0] = '\0';
    fAll = !str_cmp( arg, "all" );
    found = FALSE;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
        if ( ( pMobIndex = get_mob_index( vnum ) ) )
        {
            if ( fAll || is_name( arg, pMobIndex->player_name ) )
            {
                found = TRUE;
                dupl = remove_color( pMobIndex->short_descr );
                sprintf( buf, "`B [`K%5d`B]`w %-17.16s",
                         pMobIndex->vnum, dupl );
                strcat( buf1, buf );
                if ( ++col % 3 == 0 )
                    strcat( buf1, "\n\r" );
            }
        }
    }

    if ( !found )
    {
        send_to_char( "Mobile(s) not found in this area.\n\r", ch );
        return FALSE;
    }

    if ( col % 3 != 0 )
        strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return FALSE;
}

/* Free vnums are just the opposite of mlist, olist, olist -Lancelight */
/* Free Vnums section: Mobs */
REDIT( redit_mfreevnums )
{
    MOB_INDEX_DATA *pMobIndex;
    AREA_DATA *pArea;
    char buf1[MAX_STRING_LENGTH * 2];
    bool found;
    int vnum;
    int col = 0;
    char *dummy;
    dummy = "";
    one_argument( argument, arg );
    if ( arg[0] != '\0' )
    {
        send_to_char( "No arguments needed. Syntax: mfree or\n\r", ch );
        return FALSE;
    }
    pArea = ch->in_room->area;
    buf1[0] = '\0';
    found = FALSE;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
        /*bah */
        if ( !( pMobIndex = get_mob_index( vnum ) ) )
        {
            found = TRUE;
            sprintf( buf, "`B [`K%5d`B]`w %-2.3s", vnum, dummy );
            strcat( buf1, buf );
            if ( ++col % 7 == 0 )
                strcat( buf1, "\n\r" );
        }
    }

    if ( !found )
    {
        send_to_char( "There are no free mob vnums in this area!\n\r", ch );
        return FALSE;
    }

    if ( col % 3 != 0 )
        strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return FALSE;
}

/* Free Vnums section: Objs */

REDIT( redit_ofreevnums )
{
    OBJ_INDEX_DATA *pObjIndex;
    AREA_DATA *pArea;
    char buf1[MAX_STRING_LENGTH * 2];
    bool found;
    int vnum;
    int col = 0;
    char *dummy;
    dummy = "";

    one_argument( argument, arg );

    if ( arg[0] != '\0' )
    {
        send_to_char( "No arguments needed. Syntax: mfree or\n\r", ch );
        return FALSE;
    }

    pArea = ch->in_room->area;
    buf1[0] = '\0';
    found = FALSE;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
        if ( !( pObjIndex = get_obj_index( vnum ) ) )
        {
            found = TRUE;
            sprintf( buf, "`B [`K%5d`B]`w %-2.3s", vnum, dummy );
            strcat( buf1, buf );
            if ( ++col % 7 == 0 )
                strcat( buf1, "\n\r" );
        }
    }

    if ( !found )
    {
        send_to_char( "There are no free obj vnums in this area!\n\r", ch );
        return FALSE;
    }

    if ( col % 3 != 0 )
        strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return FALSE;
}

/* Free Vnums section: Rooms */
REDIT( redit_rfreevnums )
{
    ROOM_INDEX_DATA *pRoomIndex;
    AREA_DATA *pArea;
    char buf1[MAX_STRING_LENGTH * 2];
    bool found;
    int vnum;
    int col = 0;
    char *dummy;
    dummy = "";

    one_argument( argument, arg );

    if ( arg[0] != '\0' )
    {
        send_to_char( "No arguments needed. Syntax: rfree or\n\r", ch );
        return FALSE;
    }

    pArea = ch->in_room->area;
    buf1[0] = '\0';
    found = FALSE;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
        /*bah */
        if ( !( pRoomIndex = get_room_index( vnum ) ) )
        {
            found = TRUE;
            sprintf( buf, "`B [`K%5d`B]`w %-2.3s", vnum, dummy );
            strcat( buf1, buf );
            if ( ++col % 7 == 0 )
                strcat( buf1, "\n\r" );
        }
    }

    if ( !found )
    {
        send_to_char( "There are no free room vnums in this area!\n\r", ch );
        return FALSE;
    }

    if ( col % 3 != 0 )
        strcat( buf1, "\n\r" );

    send_to_char( buf1, ch );
    return FALSE;
}

REDIT( redit_olist )
{
    OBJ_INDEX_DATA *pObjIndex;
    AREA_DATA *pArea;
    char buf1[MAX_STRING_LENGTH * 2];
    bool fAll, found;
    char *dupl;
    int vnum;
    int col = 0;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax:  olist <all/name/item_type>\n\r", ch );
        return FALSE;
    }

    pArea = ch->in_room->area;
    buf1[0] = '\0';
    fAll = !str_cmp( arg, "all" );
    found = FALSE;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
        if ( ( pObjIndex = get_obj_index( vnum ) ) )
        {
            if ( fAll || is_name( arg, pObjIndex->name )
                 || flag_value( type_flags, arg ) == pObjIndex->item_type )
            {
                found = TRUE;
                dupl = remove_color( pObjIndex->short_descr );
                sprintf( buf, "`B [`K%5d`B]`w %-30.29s",
                         pObjIndex->vnum, dupl );
                strcat( buf1, buf );
                if ( ++col % 2 == 0 )
                    strcat( buf1, "\n\r" );
            }
        }
    }

    if ( !found )
    {
        send_to_char( "Object(s) not found in this area.\n\r", ch );
        return FALSE;
    }

/*
    if ( col % 3 != 0 )
	strcat( buf1, "\n\r" );
*/
    send_to_char( buf1, ch );
    return FALSE;
}

REDIT( redit_mshow )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( !argument[0] )
    {
        send_to_char( "Syntax:  mshow <vnum>\n\r", ch );
        return FALSE;
    }

    if ( is_number( argument ) )
    {
        value = atoi( argument );
        if ( !( pMob = get_mob_index( value ) ) )
        {
            send_to_char( "REdit:  That mobile does not exist.\n\r", ch );
            return FALSE;
        }

        ch->desc->pEdit = ( void * ) pMob;
    }

    medit_show( ch, argument );
    ch->desc->pEdit = ( void * ) ch->in_room;
    return FALSE;
}

REDIT( redit_oshow )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( !argument[0] )
    {
        send_to_char( "Syntax:  oshow <vnum>\n\r", ch );
        return FALSE;
    }

    if ( !is_number( argument ) )
    {
        send_to_char( "Syntax:  oshow <vnum>\n\r", ch );
        return FALSE;
    }

    if ( is_number( argument ) )
    {
        value = atoi( argument );
        if ( !( pObj = get_obj_index( value ) ) )
        {
            send_to_char( "REdit:  That object does not exist.\n\r", ch );
            return FALSE;
        }

        ch->desc->pEdit = ( void * ) pObj;
    }

    oedit_show( ch, argument );
    ch->desc->pEdit = ( void * ) ch->in_room;
    return FALSE;
}

/*****************************************************************************
 Name:		check_range( lower vnum, upper vnum )
 Purpose:	Ensures the range spans only one area.
 Called by:	aedit_vnum(olc_act.c).
 ****************************************************************************/
bool check_range( int lower, int upper )
{
    AREA_DATA *pArea;
    int cnt = 0;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        /*
         * lower < area < upper
         */
        if ( ( lower <= pArea->lvnum && pArea->lvnum <= upper )
             || ( lower <= pArea->uvnum && pArea->uvnum <= upper ) )
            ++cnt;

        if ( cnt > 1 )
            return FALSE;
    }

    return TRUE;
}

AREA_DATA *get_vnum_area( int vnum )
{
    AREA_DATA *pArea;

    for ( pArea = area_first; pArea; pArea = pArea->next )
    {
        if ( vnum >= pArea->lvnum && vnum <= pArea->uvnum )
            return pArea;
    }

    return 0;
}

/*
 * Area Editor Functions.
 */
AEDIT( aedit_show )
{
    AREA_DATA *pArea;

    EDIT_AREA( ch, pArea );

    sprintf( buf, "Name:     [%5d] %s\n\r", pArea->vnum, pArea->name );
    send_to_char( buf, ch );

#if 0                           /* ROM OLC */
    sprintf( buf, "Recall:   [%5d] %s\n\r", pArea->recall,
             get_room_index( pArea->recall )
             ? get_room_index( pArea->recall )->name : "none" );
    send_to_char( buf, ch );
#endif                          /* ROM */

    sprintf( buf, "File:     %s\n\r", pArea->filename );
    send_to_char( buf, ch );

    sprintf( buf, "Vnums:    [%d-%d]\n\r", pArea->lvnum, pArea->uvnum );
    send_to_char( buf, ch );

    sprintf( buf, "Age:      [%d]\n\r", pArea->age );
    send_to_char( buf, ch );

    sprintf( buf, "Players:  [%d]\n\r", pArea->nplayer );
    send_to_char( buf, ch );

    sprintf( buf, "Security: [%d]\n\r", pArea->security );
    send_to_char( buf, ch );

    sprintf( buf, "Builders: [%s]\n\r", pArea->builders );
    send_to_char( buf, ch );

    sprintf( buf, "Flags:    [%s]\n\r",
             flag_string( area_flags, pArea->area_flags ) );
    send_to_char( buf, ch );

    return FALSE;
}

AEDIT( aedit_reset )
{
    AREA_DATA *pArea;

    EDIT_AREA( ch, pArea );

    reset_area( pArea );
    send_to_char( "Area reset.\n\r", ch );

    return FALSE;
}

AEDIT( aedit_change )
{
    AREA_DATA *pArea;

    EDIT_AREA( ch, pArea );

    if ( !IS_SET( pArea->area_flags, AREA_CHANGED ) )
    {
        SET_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char( "Area flag 'changed' set.\n\r", ch );
    }
    else
    {
        REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
        send_to_char( "Area flag 'changed' removed.\n\r", ch );
    }
    return FALSE;
}

AEDIT( aedit_create )
{
    AREA_DATA *pArea;

    pArea = new_area(  );
    area_last->next = pArea;
    area_last = pArea;          /* Thanks, Walker. */
    ch->desc->pEdit = ( void * ) pArea;

    SET_BIT( pArea->area_flags, AREA_ADDED );
    send_to_char( "Area Created.\n\r", ch );
    return TRUE;
}

AEDIT( aedit_name )
{
    AREA_DATA *pArea;

    EDIT_AREA( ch, pArea );

    if ( !argument[0] )
    {
        send_to_char( "Syntax:   name [$name]\n\r", ch );
        return FALSE;
    }

    free_string( &pArea->name );
    pArea->name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch );
    return TRUE;
}

AEDIT( aedit_file )
{
    AREA_DATA *pArea;
    char file[MAX_INPUT_LENGTH];
    int i, length;

    EDIT_AREA( ch, pArea );

    one_argument( argument, file ); /* Forces Lowercase */

    if ( !argument[0] )
    {
        send_to_char( "Syntax:  filename [$file]\n\r", ch );
        return FALSE;
    }

    /*
     * Simple Syntax Check.
     */
    length = strlen( argument );
    if ( length > 8 )
    {
        send_to_char( "No more than eight characters allowed.\n\r", ch );
        return FALSE;
    }

    /*
     * Allow only letters and numbers.
     */
    for ( i = 0; i < length; i++ )
    {
        if ( !isalnum( file[i] ) )
        {
            send_to_char( "Only letters and numbers are valid.\n\r", ch );
            return FALSE;
        }
    }

    free_string( &pArea->filename );
    strcat( file, ".are" );
    pArea->filename = str_dup( file );

    send_to_char( "Filename set.\n\r", ch );
    return TRUE;
}

AEDIT( aedit_age )
{
    AREA_DATA *pArea;
    char age[MAX_INPUT_LENGTH];

    EDIT_AREA( ch, pArea );

    one_argument( argument, age );

    if ( !is_number( age ) || !age[0] )
    {
        send_to_char( "Syntax:  age [#age]\n\r", ch );
        return FALSE;
    }

    pArea->age = atoi( age );

    send_to_char( "Age set.\n\r", ch );
    return TRUE;
}

#if 0                           /* ROM OLC */
AEDIT( aedit_recall )
{
    AREA_DATA *pArea;
    char room[MAX_INPUT_LENGTH];
    int value;

    EDIT_AREA( ch, pArea );

    one_argument( argument, room );

    if ( !is_number( argument ) || !argument[0] )
    {
        send_to_char( "Syntax:  recall [#rvnum]\n\r", ch );
        return FALSE;
    }

    value = atoi( room );

    if ( !get_room_index( value ) )
    {
        send_to_char( "AEdit:  Room vnum does not exist.\n\r", ch );
        return FALSE;
    }

    pArea->recall = value;

    send_to_char( "Recall set.\n\r", ch );
    return TRUE;
}
#endif                          /* ROM OLC */

AEDIT( aedit_security )
{
    AREA_DATA *pArea;
    char sec[MAX_INPUT_LENGTH];
    int value;

    EDIT_AREA( ch, pArea );

    one_argument( argument, sec );

    if ( !is_number( sec ) || !sec[0] )
    {
        send_to_char( "Syntax:  security [#level]\n\r", ch );
        return FALSE;
    }

    value = atoi( sec );

    if ( value > ch->pcdata->security || value < 0 )
    {
        if ( ch->pcdata->security != 0 )
        {
            printf_to_char( ch, "Security is 0-%d.\n\r", ch->pcdata->security );
        }
        else
            send_to_char( "Security is 0 only.\n\r", ch );
        return FALSE;
    }

    pArea->security = value;

    send_to_char( "Security set.\n\r", ch );
    return TRUE;
}

AEDIT( aedit_builder )
{
    AREA_DATA *pArea;
    char name[MAX_INPUT_LENGTH];

    EDIT_AREA( ch, pArea );

    one_argument( argument, name );

    if ( !name[0] )
    {
        send_to_char( "Syntax:  builder [$name]  -toggles builder\n\r", ch );
        send_to_char( "Syntax:  builder All      -allows everyone\n\r", ch );
        return FALSE;
    }

    name[0] = UPPER( name[0] );

    if ( strstr( pArea->builders, name ) )
    {
        pArea->builders = string_replace( pArea->builders, name, "\0" );
        pArea->builders = string_unpad( pArea->builders );

        if ( !pArea->builders[0] )
        {
            free_string( &pArea->builders );
            pArea->builders = str_dup( "None" );
        }

        send_to_char( "Builder removed.\n\r", ch );
        return TRUE;
    }
    else
    {
        buf[0] = '\0';
        if ( strstr( pArea->builders, "None" ) != '\0' )
        {
            pArea->builders = string_replace( pArea->builders, "None", "\0" );
            pArea->builders = string_unpad( pArea->builders );
        }

        if ( pArea->builders[0] != '\0' )
        {
            strcat( buf, pArea->builders );
            strcat( buf, " " );
        }

        strcat( buf, name );
        free_string( &pArea->builders );
        pArea->builders = string_proper( str_dup( buf ) );

        send_to_char( "Builder added.\n\r", ch );
        return TRUE;
    }

    return FALSE;
}

AEDIT( aedit_vnum )
{
    AREA_DATA *pArea;
    char lower[MAX_INPUT_LENGTH];
    char upper[MAX_INPUT_LENGTH];
    int ilower;
    int iupper;

    EDIT_AREA( ch, pArea );

    argument = one_argument( argument, lower );
    one_argument( argument, upper );

    if ( !is_number( lower ) || !lower[0] || !is_number( upper ) || !upper[0] )
    {
        send_to_char( "Syntax:  vnum [#lower] [#upper]\n\r", ch );
        return FALSE;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = atoi( upper ) ) )
    {
        send_to_char( "AEdit:  Upper must be larger then lower.\n\r", ch );
        return FALSE;
    }

    if ( !check_range( atoi( lower ), atoi( upper ) ) )
    {
        send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
        return FALSE;
    }

    if ( get_vnum_area( ilower ) && get_vnum_area( ilower ) != pArea )
    {
        send_to_char( "AEdit:  Lower vnum already assigned.\n\r", ch );
        return FALSE;
    }

    pArea->lvnum = ilower;
    send_to_char( "Lower vnum set.\n\r", ch );

    if ( get_vnum_area( iupper ) && get_vnum_area( iupper ) != pArea )
    {
        send_to_char( "AEdit:  Upper vnum already assigned.\n\r", ch );
        return TRUE;            /* The lower value has been set. */
    }

    pArea->uvnum = iupper;
    send_to_char( "Upper vnum set.\n\r", ch );

    return TRUE;
}

AEDIT( aedit_lvnum )
{
    AREA_DATA *pArea;
    char lower[MAX_INPUT_LENGTH];
    int ilower;
    int iupper;

    EDIT_AREA( ch, pArea );

    one_argument( argument, lower );

    if ( !is_number( lower ) || !lower[0] )
    {
        send_to_char( "Syntax:  lvnum [#lower]\n\r", ch );
        return FALSE;
    }

    if ( ( ilower = atoi( lower ) ) > ( iupper = pArea->uvnum ) )
    {
        send_to_char( "AEdit:  Value must be less than the uvnum.\n\r", ch );
        return FALSE;
    }

    if ( !check_range( ilower, iupper ) )
    {
        send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
        return FALSE;
    }

    if ( get_vnum_area( ilower ) && get_vnum_area( ilower ) != pArea )
    {
        send_to_char( "AEdit:  Lower vnum already assigned.\n\r", ch );
        return FALSE;
    }

    pArea->lvnum = ilower;
    send_to_char( "Lower vnum set.\n\r", ch );
    return TRUE;
}

AEDIT( aedit_uvnum )
{
    AREA_DATA *pArea;
    char upper[MAX_INPUT_LENGTH];
    int ilower;
    int iupper;

    EDIT_AREA( ch, pArea );

    one_argument( argument, upper );

    if ( !is_number( upper ) || !upper[0] )
    {
        send_to_char( "Syntax:  uvnum [#upper]\n\r", ch );
        return FALSE;
    }

    if ( ( ilower = pArea->lvnum ) > ( iupper = atoi( upper ) ) )
    {
        send_to_char( "AEdit:  Upper must be larger then lower.\n\r", ch );
        return FALSE;
    }

    if ( !check_range( ilower, iupper ) )
    {
        send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
        return FALSE;
    }

    if ( get_vnum_area( iupper ) && get_vnum_area( iupper ) != pArea )
    {
        send_to_char( "AEdit:  Upper vnum already assigned.\n\r", ch );
        return FALSE;
    }

    pArea->uvnum = iupper;
    send_to_char( "Upper vnum set.\n\r", ch );

    return TRUE;
}

/*
 * Room Editor Functions.
 */
REDIT( redit_show )
{
    ROOM_INDEX_DATA *pRoom;
    char buf1[2 * MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;
    bool fcnt;

    EDIT_ROOM( ch, pRoom );

    buf1[0] = '\0';

    sprintf( buf, "Description:\n\r%s", pRoom->description );
    strcat( buf1, buf );

    sprintf( buf, "Name:       [%s]\n\rArea:       [%5d] %s\n\r",
             pRoom->name, pRoom->area->vnum, pRoom->area->name );
    strcat( buf1, buf );

    sprintf( buf, "Vnum:       [%5d]\n\rSector:     [%s]\n\r",
             pRoom->vnum, flag_string( sector_flags, pRoom->sector_type ) );
    strcat( buf1, buf );

    sprintf( buf, "Room flags: [%s]\n\r",
             flag_string( room_flags, pRoom->room_flags ) );
    strcat( buf1, buf );

    if ( pRoom->extra_descr )
    {
        EXTRA_DESCR_DATA *ed;

        strcat( buf1, "Desc Kwds:  [" );
        for ( ed = pRoom->extra_descr; ed; ed = ed->next )
        {
            strcat( buf1, ed->keyword );
            if ( ed->next )
                strcat( buf1, " " );
        }
        strcat( buf1, "]\n\r" );
    }

    strcat( buf1, "Characters: [" );
    fcnt = FALSE;
    for ( rch = pRoom->people; rch; rch = rch->next_in_room )
    {
        one_argument( rch->name, buf );
        strcat( buf1, buf );
        strcat( buf1, " " );
        fcnt = TRUE;
    }

    if ( fcnt )
    {
        int end;

        end = strlen( buf1 ) - 1;
        buf1[end] = ']';
        strcat( buf1, "\n\r" );
    }
    else
        strcat( buf1, "none]\n\r" );

    strcat( buf1, "Objects:    [" );
    fcnt = FALSE;
    for ( obj = pRoom->contents; obj; obj = obj->next_content )
    {
        one_argument( obj->name, buf );
        strcat( buf1, buf );
        strcat( buf1, " " );
        fcnt = TRUE;
    }

    if ( fcnt )
    {
        int end;

        end = strlen( buf1 ) - 1;
        buf1[end] = ']';
        strcat( buf1, "\n\r" );
    }
    else
        strcat( buf1, "none]\n\r" );

    for ( door = 0; door < MAX_DIR; door++ )
    {
        EXIT_DATA *pexit;

        if ( ( pexit = pRoom->exit[door] ) )
        {
            char word[MAX_INPUT_LENGTH];
            char reset_state[MAX_STRING_LENGTH];
            char *state;
            int i, length;

            sprintf( buf, "%-5s to [%5d] Key: [%5d]", capitalize( dir_name[door] ), pexit->u1.to_room ? pexit->u1.to_room->vnum : 0,    /* ROM OLC */
                     pexit->key );
            strcat( buf1, buf );

            /*
             * Format up the exit info.
             * Capitalize all flags that are not part of the reset info.
             */
            strcpy( reset_state, flag_string( exit_flags, pexit->rs_flags ) );
            state = flag_string( exit_flags, pexit->exit_info );
            strcat( buf1, " Exit flags: [" );
            for ( ;; )
            {
                state = one_argument( state, word );

                if ( word[0] == '\0' )
                {
                    int end;

                    end = strlen( buf1 ) - 1;
                    buf1[end] = ']';
                    strcat( buf1, "\n\r" );
                    break;
                }

                if ( str_infix( word, reset_state ) )
                {
                    length = strlen( word );
                    for ( i = 0; i < length; i++ )
                        word[i] = UPPER( word[i] );
                }
                strcat( buf1, word );
                strcat( buf1, " " );
            }

            if ( pexit->keyword && pexit->keyword[0] != '\0' )
            {
                sprintf( buf, "Kwds: [%s]\n\r", pexit->keyword );
                strcat( buf1, buf );
            }
            if ( pexit->description && pexit->description[0] != '\0' )
            {
                sprintf( buf, "%s", pexit->description );
                strcat( buf1, buf );
            }
        }
    }

    send_to_char( buf1, ch );
    return FALSE;
}

/* Local function. */
bool change_exit( CHAR_DATA * ch, char *argument, int door )
{
    ROOM_INDEX_DATA *pRoom;
    char command[MAX_INPUT_LENGTH];
    char temp[MAX_INPUT_LENGTH];
    int value;

    EDIT_ROOM( ch, pRoom );

    /*
     * Now parse the arguments.
     */
    argument = one_argument( argument, command );
    one_argument( argument, arg );

    if ( !command[0] && !argument[0] )  /* Move command. */
    {
        move_char( ch, door, TRUE );    /* ROM OLC */
        return FALSE;
    }

    if ( command[0] == '?' )
    {
        do_help( ch, "EXIT" );
        return FALSE;
    }

    if ( !str_cmp( command, "delete" ) )
    {
        ROOM_INDEX_DATA *pToRoom;
        sh_int rev;             /* ROM OLC */

        if ( !pRoom->exit[door] )
        {
            send_to_char( "REdit:  Cannot delete a null exit.\n\r", ch );
            return FALSE;
        }

        /*
         * Remove ToRoom Exit.
         */
        rev = rev_dir[door];
        pToRoom = pRoom->exit[door]->u1.to_room;    /* ROM OLC */

        if ( pToRoom->exit[rev] )
        {
            free_exit( pToRoom->exit[rev] );
            pToRoom->exit[rev] = NULL;
        }

        /*
         * Remove this exit.
         */
        free_exit( pRoom->exit[door] );
        pRoom->exit[door] = NULL;

        send_to_char( "Exit unlinked.\n\r", ch );
        return TRUE;
    }

    if ( !str_cmp( command, "link" ) )
    {
        EXIT_DATA *pExit;

        if ( !arg[0] || !is_number( arg ) )
        {
            send_to_char( "Syntax:  [direction] link [vnum] [flag]\n\r", ch );
            return FALSE;
        }

        value = atoi( arg );

        if ( !get_room_index( value ) )
        {
            send_to_char( "REdit:  Cannot link to non-existant room.\n\r", ch );
            return FALSE;
        }

        if ( !IS_BUILDER( ch, get_room_index( value )->area ) )
        {
            send_to_char( "REdit:  Cannot link to that area.\n\r", ch );
            return FALSE;
        }

        if ( get_room_index( value )->exit[rev_dir[door]] )
        {
            send_to_char( "REdit:  Remote side's exit already exists.\n\r",
                          ch );
            return FALSE;
        }

        if ( !pRoom->exit[door] )
        {
            pRoom->exit[door] = new_exit(  );
        }

        pRoom->exit[door]->u1.to_room = get_room_index( value );    /* ROM OLC */
        pRoom->exit[door]->orig_door = door;
/*	    pRoom->exit[door]->vnum = value;                Can't set vnum in ROM */

        pRoom = get_room_index( value );
        door = rev_dir[door];
        pExit = new_exit(  );
        pExit->u1.to_room = ch->in_room;
/*	    pExit->vnum             = ch->in_room->vnum;    Can't set vnum in ROM */
        pExit->orig_door = door;
        pRoom->exit[door] = pExit;

        send_to_char( "Two-way link established.\n\r", ch );
        return TRUE;
    }

    if ( !str_cmp( command, "dig" ) )
    {
        char buf1[MAX_INPUT_LENGTH];

        if ( !arg[0] || !is_number( arg ) )
        {
            send_to_char( "Syntax: [direction] dig <vnum> [flag]\n\r", ch );
            return FALSE;
        }

        one_argument( arg, temp );

        if ( !is_number( temp ) )
            send_to_char( "vnum is not an integer\n\r", ch );

        redit_create( ch, temp );
        sprintf( buf1, "link %s", temp );
        change_exit( ch, buf1, door );

        return TRUE;
    }

    if ( !str_cmp( command, "room" ) )
    {
        if ( !arg[0] || !is_number( arg ) )
        {
            send_to_char( "Syntax:  [direction] room [vnum]\n\r", ch );
            return FALSE;
        }

        if ( !pRoom->exit[door] )
        {
            pRoom->exit[door] = new_exit(  );
        }

        value = atoi( arg );

        if ( !get_room_index( value ) )
        {
            send_to_char( "REdit:  Cannot link to non-existant room.\n\r", ch );
            return FALSE;
        }

        pRoom->exit[door]->u1.to_room = get_room_index( value );    /* ROM OLC */
        pRoom->exit[door]->orig_door = door;
/*	    pRoom->exit[door]->vnum = value;                 Can't set vnum in ROM */

        send_to_char( "One-way link established.\n\r", ch );
        return TRUE;
    }

    if ( !str_cmp( command, "key" ) )
    {
        if ( !arg[0] || !is_number( arg ) )
        {
            send_to_char( "Syntax:  [direction] key [vnum]\n\r", ch );
            return FALSE;
        }

        if ( !pRoom->exit[door] )
        {
            send_to_char
                ( "REdit:  Cannot make a key for a non-existant exit.\n\r",
                  ch );
            return FALSE;
        }

        value = atoi( arg );

        if ( !get_obj_index( value ) )
        {
            send_to_char( "REdit:  Item doesn't exist.\n\r", ch );
            return FALSE;
        }

        if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY )
        {
            send_to_char( "REdit:  Key doesn't exist.\n\r", ch );
            return FALSE;
        }

        pRoom->exit[door]->key = value;

        send_to_char( "Exit key set.\n\r", ch );
        return TRUE;
    }

    if ( !str_cmp( command, "name" ) )
    {
        if ( !arg[0] )
        {
            send_to_char( "Syntax:  [direction] name [string]\n\r", ch );
            return FALSE;
        }

        if ( !pRoom->exit[door] )
        {
            send_to_char( "REdit:  Cannot name a non-existant exit.\n\r", ch );
            return FALSE;
        }

        free_string( &pRoom->exit[door]->keyword );
        pRoom->exit[door]->keyword = str_dup( arg );

        send_to_char( "Exit name set.\n\r", ch );
        return TRUE;
    }

    if ( !str_prefix( command, "description" ) )
    {
        if ( !arg[0] )
        {
            if ( !pRoom->exit[door] )
            {
                send_to_char
                    ( "REdit:  Cannot describe a non-existant exit.\n\r", ch );
                return FALSE;
            }
            string_append( ch, &pRoom->exit[door]->description );

            return TRUE;
        }

        send_to_char( "Syntax:  [direction] desc\n\r", ch );
        return FALSE;
    }

    /*
       * Set the exit flags, needs full argument.
       * ----------------------------------------
     */
    if ( !str_cmp( command, "exit" ) )
    {
        if ( ( value = flag_value( exit_flags, arg ) ) != NO_FLAG )
        {
            ROOM_INDEX_DATA *pToRoom;
            sh_int rev;         /* ROM OLC */

            if ( !pRoom->exit[door] )
            {
                send_to_char( "REdit:  Cannot flag a non-existant exit.\n\r",
                              ch );
                return FALSE;
            }

            /*
             * This room.
             */
            TOGGLE_BIT( pRoom->exit[door]->rs_flags, value );
            /* Don't toggle exit_info because it can be changed by players. */
            pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;

            /*
             * Connected room.
             */
            pToRoom = pRoom->exit[door]->u1.to_room;    /* ROM OLC */
            rev = rev_dir[door];

            /*
             * If the connected room's exit maps back to us then set it's
             * flags also.  Otherwise ignore it.
             */
            if ( pToRoom->exit[rev] && pToRoom->exit[rev]->u1.to_room == pRoom )
            {
                TOGGLE_BIT( pToRoom->exit[rev]->rs_flags, value );
                TOGGLE_BIT( pToRoom->exit[rev]->exit_info, value );
            }

            send_to_char( "Exit flag toggled.\n\r", ch );
            return TRUE;
        }
    }

    /* If we make it to here, user has typed an invalid command. */

    send_to_char
        ( "That is not a valid exit command!  Type \"help exit\" for help.\n\r",
          ch );
    return FALSE;
}

REDIT( redit_north )
{
    if ( change_exit( ch, argument, DIR_NORTH ) )
        return TRUE;

    return FALSE;
}

REDIT( redit_south )
{
    if ( change_exit( ch, argument, DIR_SOUTH ) )
        return TRUE;

    return FALSE;
}

REDIT( redit_east )
{
    if ( change_exit( ch, argument, DIR_EAST ) )
        return TRUE;

    return FALSE;
}

REDIT( redit_west )
{
    if ( change_exit( ch, argument, DIR_WEST ) )
        return TRUE;

    return FALSE;
}

REDIT( redit_up )
{
    if ( change_exit( ch, argument, DIR_UP ) )
        return TRUE;

    return FALSE;
}

REDIT( redit_down )
{
    if ( change_exit( ch, argument, DIR_DOWN ) )
        return TRUE;

    return FALSE;
}

REDIT( redit_ed )
{
    ROOM_INDEX_DATA *pRoom;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    EDIT_ROOM( ch, pRoom );

    argument = one_argument( argument, command );
    one_argument( argument, keyword );

    if ( !command[0] || !keyword[0] )
    {
        send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
        send_to_char( "         ed edit [keyword]\n\r", ch );
        send_to_char( "         ed delete [keyword]\n\r", ch );
        send_to_char( "         ed format [keyword]\n\r", ch );
        return FALSE;
    }

    if ( !str_cmp( command, "add" ) )
    {
        if ( !keyword[0] )
        {
            send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
            return FALSE;
        }

        ed = new_extra_descr(  );
        ed->keyword = str_dup( keyword );
        ed->description = str_dup( "" );
        ed->next = pRoom->extra_descr;
        pRoom->extra_descr = ed;

        string_append( ch, &ed->description );

        return TRUE;
    }

    if ( !str_cmp( command, "edit" ) )
    {
        if ( !keyword[0] )
        {
            send_to_char( "Syntax:  ed edit [keyword]\n\r", ch );
            return FALSE;
        }

        for ( ed = pRoom->extra_descr; ed; ed = ed->next )
        {
            if ( is_name( keyword, ed->keyword ) )
                break;
        }

        if ( !ed )
        {
            send_to_char( "REdit:  Extra description keyword not found.\n\r",
                          ch );
            return FALSE;
        }

        string_append( ch, &ed->description );

        return TRUE;
    }

    if ( !str_cmp( command, "delete" ) )
    {
        EXTRA_DESCR_DATA *ped = NULL;

        if ( !keyword[0] )
        {
            send_to_char( "Syntax:  ed delete [keyword]\n\r", ch );
            return FALSE;
        }

        for ( ed = pRoom->extra_descr; ed; ed = ed->next )
        {
            if ( is_name( keyword, ed->keyword ) )
                break;
            ped = ed;
        }

        if ( !ed )
        {
            send_to_char( "REdit:  Extra description keyword not found.\n\r",
                          ch );
            return FALSE;
        }

        if ( !ped )
            pRoom->extra_descr = ed->next;
        else
            ped->next = ed->next;

        free_extra_descr( ed );

        send_to_char( "Extra description deleted.\n\r", ch );
        return TRUE;
    }

    if ( !str_cmp( command, "format" ) )
    {
        if ( !keyword[0] )
        {
            send_to_char( "Syntax:  ed format [keyword]\n\r", ch );
            return FALSE;
        }

        for ( ed = pRoom->extra_descr; ed; ed = ed->next )
        {
            if ( is_name( keyword, ed->keyword ) )
                break;
        }

        if ( !ed )
        {
            send_to_char( "REdit:  Extra description keyword not found.\n\r",
                          ch );
            return FALSE;
        }

        ed->description = format_string( ed->description );

        send_to_char( "Extra description formatted.\n\r", ch );
        return TRUE;
    }

    redit_ed( ch, "" );
    return FALSE;
}

REDIT( redit_create )
{
    AREA_DATA *pArea;
    ROOM_INDEX_DATA *pRoom;
    int value;
    int iHash;

    EDIT_ROOM( ch, pRoom );

    value = atoi( argument );

    if ( !argument[0] || value <= 0 )
    {
        send_to_char( "Syntax:  create [vnum > 0]\n\r", ch );
        return FALSE;
    }

    pArea = get_vnum_area( value );

    if ( !pArea )
    {
        send_to_char( "REdit:  That vnum is not assigned an area.\n\r", ch );
        return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
        send_to_char( "REdit:  Vnum in an area you cannot build in.\n\r", ch );
        return FALSE;
    }

    if ( get_room_index( value ) )
    {
        send_to_char( "REdit:  Room vnum already exists.\n\r", ch );
        return FALSE;
    }

    pRoom = new_room_index(  );
    pRoom->area = pArea;
    pRoom->vnum = value;

    if ( value > top_vnum_room )
        top_vnum_room = value;

    iHash = value % MAX_KEY_HASH;
    pRoom->next = room_index_hash[iHash];
    room_index_hash[iHash] = pRoom;
    ch->desc->pEdit = ( void * ) pRoom;

    send_to_char( "Room created.\n\r", ch );
    return TRUE;
}

REDIT( redit_name )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM( ch, pRoom );

    if ( !argument[0] )
    {
        send_to_char( "Syntax:  name [name]\n\r", ch );
        return FALSE;
    }

    free_string( &pRoom->name );
    pRoom->name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch );
    return TRUE;
}

REDIT( redit_desc )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM( ch, pRoom );

    if ( !argument[0] )
    {
        string_append( ch, &pRoom->description );
        return TRUE;
    }

    send_to_char( "Syntax:  desc\n\r", ch );
    return FALSE;
}

REDIT( redit_format )
{
    ROOM_INDEX_DATA *pRoom;

    EDIT_ROOM( ch, pRoom );

    pRoom->description = format_string( pRoom->description );

    send_to_char( "String formatted.\n\r", ch );
    return TRUE;
}

REDIT( redit_mreset )
{
    ROOM_INDEX_DATA *pRoom;
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *newmob;

    RESET_DATA *pReset;
    char output[MAX_STRING_LENGTH];

    EDIT_ROOM( ch, pRoom );

    argument = one_argument( argument, arg );

    if ( !arg[0] || !is_number( arg ) )
    {
        send_to_char( "Syntax:  mreset <vnum> <max #>\n\r", ch );
        return FALSE;
    }

    if ( !( pMobIndex = get_mob_index( atoi( arg ) ) ) )
    {
        send_to_char( "REdit: No mobile has that vnum.\n\r", ch );
        return FALSE;
    }

    if ( pMobIndex->area != pRoom->area )
    {
        send_to_char( "REdit: No such mobile in this area.\n\r", ch );
        return FALSE;
    }

    /*
     * Create the mobile reset.
     */
    pReset = new_reset_data(  );
    pReset->command = 'M';
    pReset->vnum = pMobIndex->vnum;
    pReset->arg2 = is_number( argument ) ? atoi( argument ) : MAX_MOB;
    pReset->arg3 = pRoom->vnum;
    add_reset( pRoom, pReset, 0 /* Last slot */  );

    /*
     * Create the mobile.
     */
    newmob = create_mobile( pMobIndex );
    char_to_room( newmob, pRoom );

    sprintf( output, "%s (%d) has been loaded and added to resets.\n\r"
             "There will be a maximum of %d loaded to this room.\n\r",
             capitalize( pMobIndex->short_descr ),
             pMobIndex->vnum, pReset->arg2 );
    send_to_char( output, ch );
    act( "$n has created $N!", ch, NULL, newmob, TO_ROOM );

    return TRUE;
}

struct wear_type {
    int wear_loc;
    int wear_bit;
};

const struct wear_type wear_table[] = {
    {WEAR_NONE, ITEM_TAKE},
    {WEAR_LIGHT, ITEM_LIGHT},
    {WEAR_FINGER_L, ITEM_WEAR_FINGER},
    {WEAR_FINGER_R, ITEM_WEAR_FINGER},
    {WEAR_NECK_1, ITEM_WEAR_NECK},
    {WEAR_NECK_2, ITEM_WEAR_NECK},
    {WEAR_BODY, ITEM_WEAR_BODY},
    {WEAR_HEAD, ITEM_WEAR_HEAD},
    {WEAR_LEGS, ITEM_WEAR_LEGS},
    {WEAR_FEET, ITEM_WEAR_FEET},
    {WEAR_HANDS, ITEM_WEAR_HANDS},
    {WEAR_ARMS, ITEM_WEAR_ARMS},
    {WEAR_SHIELD, ITEM_WEAR_SHIELD},
    {WEAR_ABOUT, ITEM_WEAR_ABOUT},
    {WEAR_WAIST, ITEM_WEAR_WAIST},
    {WEAR_WRIST_L, ITEM_WEAR_WRIST},
    {WEAR_WRIST_R, ITEM_WEAR_WRIST},
    {WEAR_WIELD, ITEM_WIELD},
    {WEAR_HOLD, ITEM_HOLD},
    {NO_FLAG, NO_FLAG}
};

/*****************************************************************************
 Name:		wear_loc
 Purpose:	Returns the location of the bit that matches the count.
 		1 = first match, 2 = second match etc.
 Called by:	oedit_reset(olc_act.c).
 ****************************************************************************/
int wear_loc( int bits, int count )
{
    int flag;

    for ( flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++ )
    {
        if ( IS_SET( bits, wear_table[flag].wear_bit ) && --count < 1 )
            return wear_table[flag].wear_loc;
    }

    return NO_FLAG;
}

/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit( int loc )
{
    int flag;

    for ( flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++ )
    {
        if ( loc == wear_table[flag].wear_loc )
            return wear_table[flag].wear_bit;
    }

    return 0;
}

REDIT( redit_oreset )
{
    ROOM_INDEX_DATA *pRoom;
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *newobj;
    OBJ_DATA *to_obj;
    CHAR_DATA *to_mob;
    char arg2[MAX_INPUT_LENGTH];
    int olevel = 0;

    RESET_DATA *pReset;
    char output[MAX_STRING_LENGTH];

    EDIT_ROOM( ch, pRoom );

    argument = one_argument( argument, arg );
    argument = one_argument( argument, arg2 );

    if ( arg[0] == '\0' || !is_number( arg ) )
    {
        send_to_char( "Syntax:  oreset <vnum> <args>\n\r", ch );
        send_to_char( "        -no_args               = into room\n\r", ch );
        send_to_char( "        -<obj_name>            = into obj\n\r", ch );
        send_to_char( "        -<mob_name> <wear_loc> = into mob\n\r", ch );
        return FALSE;
    }

    if ( !( pObjIndex = get_obj_index( atoi( arg ) ) ) )
    {
        send_to_char( "REdit: No object has that vnum.\n\r", ch );
        return FALSE;
    }

    if ( pObjIndex->area != pRoom->area )
    {
        send_to_char( "REdit: No such object in this area.\n\r", ch );
        return FALSE;
    }

    /*
     * Load into room.
     */
    if ( arg2[0] == '\0' )
    {
        pReset = new_reset_data(  );
        pReset->command = 'O';
        pReset->vnum = pObjIndex->vnum;
        pReset->arg2 = 0;
        pReset->arg3 = pRoom->vnum;
        add_reset( pRoom, pReset, 0 /* Last slot */  );

        newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
        obj_to_room( newobj, pRoom );

        sprintf( output, "%s (%d) has been loaded and added to resets.\n\r",
                 capitalize( pObjIndex->short_descr ), pObjIndex->vnum );
        send_to_char( output, ch );
    }
    else
        /*
         * Load into object's inventory.
         */
    if ( argument[0] == '\0'
             && ( ( to_obj = get_obj_list( ch, arg2, pRoom->contents ) ) !=
                      NULL ) )
    {
        pReset = new_reset_data(  );
        pReset->command = 'P';
        pReset->vnum = pObjIndex->vnum;
        pReset->arg2 = 0;
        pReset->arg3 = to_obj->pIndexData->vnum;
        add_reset( pRoom, pReset, 0 /* Last slot */  );

        newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
        newobj->cost = 0;
        obj_to_obj( newobj, to_obj );

        sprintf( output, "%s (%d) has been loaded into "
                 "%s (%d) and added to resets.\n\r",
                 capitalize( newobj->short_descr ),
                 newobj->pIndexData->vnum,
                 to_obj->short_descr, to_obj->pIndexData->vnum );
        send_to_char( output, ch );
    }
    else
        /*
         * Load into mobile's inventory.
         */
    if ( ( to_mob = get_char_room( ch, arg2 ) ) != NULL )
    {
        int wear_loc;

        /*
         * Make sure the location on mobile is valid.
         */
        if ( ( wear_loc = flag_value( wear_loc_flags, argument ) ) == NO_FLAG )
        {
            send_to_char( "REdit: Invalid wear_loc.  '? wear-loc'\n\r", ch );
            return FALSE;
        }

        /*
         * Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
         */
        if ( !IS_SET( pObjIndex->wear_flags, wear_bit( wear_loc ) ) )
        {
            sprintf( output, "%s (%d) has wear flags: [%s]\n\r",
                     capitalize( pObjIndex->short_descr ), pObjIndex->vnum,
                     flag_string( wear_flags, pObjIndex->wear_flags ) );
            send_to_char( output, ch );
            return FALSE;
        }

        /*
         * Can't load into same position.
         */
        if ( get_eq_char( to_mob, wear_loc ) )
        {
            send_to_char( "REdit:  Object already equipped.\n\r", ch );
            return FALSE;
        }

        pReset = new_reset_data(  );
        pReset->vnum = pObjIndex->vnum;
        pReset->arg2 = wear_loc;
        if ( pReset->arg2 == WEAR_NONE )
            pReset->command = 'G';
        else
            pReset->command = 'E';
        pReset->arg3 = wear_loc;

        add_reset( pRoom, pReset, 0 /* Last slot */  );

        olevel = URANGE( 0, to_mob->level - 2, LEVEL_HERO );
        newobj = create_object( pObjIndex, number_fuzzy( olevel ) );

        if ( to_mob->pIndexData->pShop )    /* Shop-keeper? */
        {
            switch ( pObjIndex->item_type )
            {
            default:
                olevel = 0;
                break;
            case ITEM_PILL:
                olevel = number_range( 0, 10 );
                break;
            case ITEM_POTION:
                olevel = number_range( 0, 10 );
                break;
            case ITEM_SCROLL:
                olevel = number_range( 5, 15 );
                break;
            case ITEM_WAND:
                olevel = number_range( 10, 20 );
                break;
            case ITEM_STAFF:
                olevel = number_range( 15, 25 );
                break;
            case ITEM_ARMOR:
                olevel = number_range( 5, 15 );
                break;
            case ITEM_WEAPON:
                if ( pReset->command == 'G' )
                    olevel = number_range( 5, 15 );
                else
                    olevel = number_fuzzy( olevel );
                break;
            }

            newobj = create_object( pObjIndex, olevel );
            if ( pReset->arg2 == WEAR_NONE )
                SET_BIT( newobj->extra_flags, ITEM_INVENTORY );
        }
        else
            newobj = create_object( pObjIndex, number_fuzzy( olevel ) );

        obj_to_char( newobj, to_mob );
        if ( pReset->command == 'E' )
            equip_char( to_mob, newobj, pReset->arg3 );

        sprintf( output, "%s (%d) has been loaded "
                 "%s of %s (%d) and added to resets.\n\r",
                 capitalize( pObjIndex->short_descr ),
                 pObjIndex->vnum,
                 flag_string( wear_loc_strings, pReset->arg3 ),
                 to_mob->short_descr, to_mob->pIndexData->vnum );
        send_to_char( output, ch );
    }
    else                        /* Display Syntax */
    {
        send_to_char( "REdit:  That mobile isn't here.\n\r", ch );
        return FALSE;
    }

    act( "$n has created $p!", ch, newobj, NULL, TO_ROOM );
    return TRUE;
}

/*
 * Object Editor Functions.
 */
void show_obj_values( CHAR_DATA * ch, OBJ_INDEX_DATA * obj )
{
    switch ( obj->item_type )
    {
    default:                   /* No values. */
        break;

    case ITEM_LIGHT:
        if ( obj->value[2] == -1 || obj->value[2] == 999 )  /* ROM OLC */
            sprintf( buf, "[v2] Light:  Infinite[-1]\n\r" );
        else
            sprintf( buf, "[v2] Light:  [%d]\n\r", obj->value[2] );
        send_to_char( buf, ch );
        break;

    case ITEM_WAND:
    case ITEM_STAFF:
        sprintf( buf,
                 "[v0] Level:          [%d]\n\r"
                 "[v1] Charges Total:  [%d]\n\r"
                 "[v2] Charges Left:   [%d]\n\r"
                 "[v3] Spell:          %s\n\r",
                 obj->value[0],
                 obj->value[1],
                 obj->value[2],
                 obj->value[3] != -1 ? skill_table[obj->value[3]].name
                 : "none" );
        send_to_char( buf, ch );
        break;
    case ITEM_FURNITURE:
        sprintf( buf,
                 "[v0] Max people:      [%d]\n\r"
                 "[v1] Max weight:      [%d]\n\r"
                 "[v2] Furniture Flags: %s\n\r"
                 "[v3] Heal bonus:      [%d]\n\r"
                 "[v4] Mana bonus:      [%d]\n\r",
                 obj->value[0],
                 obj->value[1],
                 flag_string( furniture_flags, obj->value[2] ),
                 obj->value[3], obj->value[4] );
        send_to_char( buf, ch );
        break;

    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
        sprintf( buf,
                 "[v0] Level:  [%d]\n\r"
                 "[v1] Spell:  %s\n\r"
                 "[v2] Spell:  %s\n\r"
                 "[v3] Spell:  %s\n\r",
                 obj->value[0],
                 obj->value[1] != -1 ? skill_table[obj->value[1]].name
                 : "none",
                 obj->value[2] != -1 ? skill_table[obj->value[2]].name
                 : "none",
                 obj->value[3] != -1 ? skill_table[obj->value[3]].name
                 : "none" );
        send_to_char( buf, ch );
        break;

/* ARMOR for ROM */

    case ITEM_ARMOR:
        sprintf( buf,
                 "[v0] Ac pierce       [%d]\n\r"
                 "[v1] Ac bash         [%d]\n\r"
                 "[v2] Ac slash        [%d]\n\r"
                 "[v3] Ac exotic       [%d]\n\r",
                 obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
        send_to_char( buf, ch );
        break;

/* WEAPON changed in ROM: */
/* I had to split the output here, I have no idea why, but it helped -- Hugin */
/* It somehow fixed a bug in showing scroll/pill/potions too ?! */
    case ITEM_WEAPON:
        sprintf( buf, "[v0] Weapon class:   %s\n\r",
                 flag_string( weapon_class, obj->value[0] ) );
        send_to_char( buf, ch );
        sprintf( buf, "[v1] Number of dice: [%d]\n\r", obj->value[1] );
        send_to_char( buf, ch );
        sprintf( buf, "[v2] Type of dice:   [%d]\n\r", obj->value[2] );
        send_to_char( buf, ch );
        sprintf( buf, "[v3] Type:           %s\n\r",
                 flag_string( weapon_flags, obj->value[3] ) );
        send_to_char( buf, ch );
        sprintf( buf, "[v4] Special type:   %s\n\r",
                 flag_string( weapon_type, obj->value[4] ) );
        send_to_char( buf, ch );
        break;

    case ITEM_CONTAINER:
        sprintf( buf,
                 "[v0] Weight: [%d kg]\n\r"
                 "[v1] Flags:  [%s]\n\r"
                 "[v2] Key:    %s [%d]\n\r",
                 obj->value[0],
                 flag_string( container_flags, obj->value[1] ),
                 get_obj_index( obj->value[2] )
                 ? get_obj_index( obj->value[2] )->short_descr
                 : "none", obj->value[2] );
        send_to_char( buf, ch );
        break;

    case ITEM_DRINK_CON:
        sprintf( buf,
                 "[v0] Liquid Total: [%d]\n\r"
                 "[v1] Liquid Left:  [%d]\n\r"
                 "[v2] Liquid:       %s\n\r"
                 "[v3] Poisoned:     %s\n\r",
                 obj->value[0],
                 obj->value[1],
                 flag_string( liquid_flags, obj->value[2] ),
                 obj->value[3] != 0 ? "Yes" : "No" );
        send_to_char( buf, ch );
        break;

    case ITEM_FOOD:
        sprintf( buf,
                 "[v0] Food hours: [%d]\n\r"
                 "[v3] Poisoned:   %s\n\r",
                 obj->value[0], obj->value[3] != 0 ? "Yes" : "No" );
        send_to_char( buf, ch );
        break;

    case ITEM_MONEY:
        sprintf( buf, "[v0] Gold:   [%d]\n\r", obj->value[0] );
        send_to_char( buf, ch );
        break;

    case ITEM_PORTAL:
        sprintf( buf, "[v0] Vnum:   [%d]\n\r", obj->value[0] );
        send_to_char( buf, ch );
        break;

    }

    return;
}

bool set_obj_values( CHAR_DATA * ch, OBJ_INDEX_DATA * pObj, int value_num,
                     char *argument )
{
    switch ( pObj->item_type )
    {
    default:
        break;

    case ITEM_LIGHT:
        switch ( value_num )
        {
        default:
            do_help( ch, "ITEM_LIGHT" );
            return FALSE;
        case 2:
            send_to_char( "HOURS OF LIGHT SET.\n\r\n\r", ch );
            pObj->value[2] = atoi( argument );
            break;
        }
        break;

    case ITEM_WAND:
    case ITEM_STAFF:
        switch ( value_num )
        {
        default:
            do_help( ch, "ITEM_STAFF_WAND" );
            return FALSE;
        case 0:
            send_to_char( "SPELL LEVEL SET.\n\r\n\r", ch );
            pObj->value[0] = atoi( argument );
            break;
        case 1:
            send_to_char( "TOTAL NUMBER OF CHARGES SET.\n\r\n\r", ch );
            pObj->value[1] = atoi( argument );
            break;
        case 2:
            send_to_char( "CURRENT NUMBER OF CHARGES SET.\n\r\n\r", ch );
            pObj->value[2] = atoi( argument );
            break;
        case 3:
            send_to_char( "SPELL TYPE SET.\n\r", ch );
            pObj->value[3] = skill_lookup( argument );
            break;
        }
        break;
    case ITEM_FURNITURE:
        switch ( value_num )
        {
        default:
            do_help( ch, "ITEM_FURNITURE" );
            return FALSE;

        case 0:
            send_to_char( "NUMBER OF PEOPLE SET.\n\r\n\r", ch );
            pObj->value[0] = atoi( argument );
            break;
        case 1:
            send_to_char( "MAX WEIGHT SET.\n\r\n\r", ch );
            pObj->value[1] = atoi( argument );
            break;
        case 2:
            send_to_char( "FURNITURE FLAGS TOGGLED.\n\r\n\r", ch );
            pObj->value[2] ^=
                ( flag_value( furniture_flags, argument ) !=
                  NO_FLAG ? flag_value( furniture_flags, argument ) : 0 );
            break;
        case 3:
            send_to_char( "HEAL BONUS SET.\n\r\n\r", ch );
            pObj->value[3] = atoi( argument );
            break;
        case 4:
            send_to_char( "MANA BONUS SET.\n\r\n\r", ch );
            pObj->value[4] = atoi( argument );
            break;
        }
        break;

    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
        switch ( value_num )
        {
        default:
            do_help( ch, "ITEM_SCROLL_POTION_PILL" );
            return FALSE;
        case 0:
            send_to_char( "SPELL LEVEL SET.\n\r\n\r", ch );
            pObj->value[0] = atoi( argument );
            break;
        case 1:
            send_to_char( "SPELL TYPE 1 SET.\n\r\n\r", ch );
            pObj->value[1] = skill_lookup( argument );
            break;
        case 2:
            send_to_char( "SPELL TYPE 2 SET.\n\r\n\r", ch );
            pObj->value[2] = skill_lookup( argument );
            break;
        case 3:
            send_to_char( "SPELL TYPE 3 SET.\n\r\n\r", ch );
            pObj->value[3] = skill_lookup( argument );
            break;
        }
        break;

/* ARMOR for ROM: */

    case ITEM_ARMOR:
        switch ( value_num )
        {
        default:
            do_help( ch, "ITEM_ARMOR" );
            return FALSE;
        case 0:
            send_to_char( "AC PIERCE SET.\n\r\n\r", ch );
            pObj->value[0] = atoi( argument );
            break;
        case 1:
            send_to_char( "AC BASH SET.\n\r\n\r", ch );
            pObj->value[1] = atoi( argument );
            break;
        case 2:
            send_to_char( "AC SLASH SET.\n\r\n\r", ch );
            pObj->value[2] = atoi( argument );
            break;
        case 3:
            send_to_char( "AC EXOTIC SET.\n\r\n\r", ch );
            pObj->value[3] = atoi( argument );
            break;
        }
        break;

/* WEAPONS changed in ROM */

    case ITEM_WEAPON:
        switch ( value_num )
        {
        default:
            do_help( ch, "ITEM_WEAPON" );
            return FALSE;
        case 0:
            send_to_char( "WEAPON CLASS SET.\n\r\n\r", ch );
            pObj->value[0] = flag_value( weapon_class, argument );
            break;
        case 1:
            send_to_char( "NUMBER OF DICE SET.\n\r\n\r", ch );
            pObj->value[1] = atoi( argument );
            break;
        case 2:
            send_to_char( "TYPE OF DICE SET.\n\r\n\r", ch );
            pObj->value[2] = atoi( argument );
            break;
        case 3:
            send_to_char( "WEAPON TYPE SET.\n\r\n\r", ch );
            pObj->value[3] = flag_value( weapon_flags, argument );
            break;
        case 4:
            send_to_char( "SPECIAL WEAPON TYPE SET.\n\r\n\r", ch );
            pObj->value[4] = flag_value( weapon_type, argument );
            break;
        }
        break;

    case ITEM_CONTAINER:
        switch ( value_num )
        {
            int value;

        default:
            do_help( ch, "ITEM_CONTAINER" );
            return FALSE;
        case 0:
            send_to_char( "WEIGHT CAPACITY SET.\n\r\n\r", ch );
            pObj->value[0] = atoi( argument );
            break;
        case 1:
            if ( ( value = flag_value( container_flags, argument ) )
                 != NO_FLAG )
                TOGGLE_BIT( pObj->value[1], value );
            else
            {
                do_help( ch, "ITEM_CONTAINER" );
                return FALSE;
            }
            send_to_char( "CONTAINER TYPE SET.\n\r\n\r", ch );
            break;
        case 2:
            if ( atoi( argument ) != 0 )
            {
                if ( !get_obj_index( atoi( argument ) ) )
                {
                    send_to_char( "THERE IS NO SUCH ITEM.\n\r\n\r", ch );
                    return FALSE;
                }

                if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY )
                {
                    send_to_char( "THAT ITEM IS NOT A KEY.\n\r\n\r", ch );
                    return FALSE;
                }
            }
            send_to_char( "CONTAINER KEY SET.\n\r\n\r", ch );
            pObj->value[2] = atoi( argument );
            break;
        }
        break;

    case ITEM_DRINK_CON:
        switch ( value_num )
        {
        default:
            do_help( ch, "ITEM_DRINK" );
/* OLC		    do_help( ch, "liquids" );    */
            return FALSE;
        case 0:
            send_to_char( "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch );
            pObj->value[0] = atoi( argument );
            break;
        case 1:
            send_to_char( "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch );
            pObj->value[1] = atoi( argument );
            break;
        case 2:
            send_to_char( "LIQUID TYPE SET.\n\r\n\r", ch );
            pObj->value[2] = flag_value( liquid_flags, argument );
            break;
        case 3:
            send_to_char( "POISON VALUE TOGGLED.\n\r\n\r", ch );
            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
            break;
        }
        break;

    case ITEM_FOOD:
        switch ( value_num )
        {
        default:
            do_help( ch, "ITEM_FOOD" );
            return FALSE;
        case 0:
            send_to_char( "HOURS OF FOOD SET.\n\r\n\r", ch );
            pObj->value[0] = atoi( argument );
            break;
        case 3:
            send_to_char( "POISON VALUE TOGGLED.\n\r\n\r", ch );
            pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
            break;
        }
        break;

    case ITEM_MONEY:
        switch ( value_num )
        {
        default:
            do_help( ch, "ITEM_MONEY" );
            return FALSE;
        case 0:
            send_to_char( "GOLD AMOUNT SET.\n\r\n\r", ch );
            pObj->value[0] = atoi( argument );
            break;
        }
        break;

    case ITEM_PORTAL:
        switch ( value_num )
        {
        default:
            do_help( ch, "ITEM_PORTAL" );
            return FALSE;
        case 0:
            send_to_char( "VNUM SET.\n\r\n\r", ch );
            pObj->value[0] = atoi( argument );
            break;
        }
        break;

    }

    show_obj_values( ch, pObj );

    return TRUE;
}

OEDIT( oedit_show )
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *paf;
    int cnt;

    EDIT_OBJ( ch, pObj );

    sprintf( buf, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
             pObj->name,
             !pObj->area ? -1 : pObj->area->vnum,
             !pObj->area ? "No Area" : pObj->area->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum:        [%5d]\n\rType:        [%s]\n\r",
             pObj->vnum, flag_string( type_flags, pObj->item_type ) );
    send_to_char( buf, ch );

    sprintf( buf, "Level:       [%5d]\n\r", pObj->level );
    send_to_char( buf, ch );

    sprintf( buf, "Wear flags:  [%s]\n\r",
             flag_string( wear_flags, pObj->wear_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Extra flags: [%s]\n\r",
             flag_string( extra_flags, pObj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Material:    [%s]\n\r",  /* ROM */
             flag_string( material_type, pObj->material ) );
    send_to_char( buf, ch );

    sprintf( buf, "Condition:   [%5d]\n\r", /* ROM */
             pObj->condition );
    send_to_char( buf, ch );

    sprintf( buf, "Weight:      [%5d]\n\rCost:        [%5d]\n\r",
             pObj->weight, pObj->cost );
    send_to_char( buf, ch );

    if ( pObj->extra_descr )
    {
        EXTRA_DESCR_DATA *ed;

        send_to_char( "Ex desc kwd: ", ch );

        for ( ed = pObj->extra_descr; ed; ed = ed->next )
        {
            send_to_char( "[", ch );
            send_to_char( ed->keyword, ch );
            send_to_char( "]", ch );
        }

        send_to_char( "\n\r", ch );
    }

    sprintf( buf, "Short desc:  %s\n\rLong desc:\n\r     %s\n\r",
             pObj->short_descr, pObj->description );
    send_to_char( buf, ch );

    for ( cnt = 0, paf = pObj->affected; paf; paf = paf->next )
    {
        if ( cnt == 0 )
        {
            send_to_char( "Number Modifier Affects\n\r", ch );
            send_to_char( "------ -------- -------\n\r", ch );
        }
        sprintf( buf, "[%4d] %-8d %s\n\r", cnt,
                 paf->modifier, flag_string( apply_flags, paf->location ) );
        send_to_char( buf, ch );
        cnt++;
    }

    show_obj_values( ch, pObj );
/* Only IMPs can see an object's clan.
   This is because only IMPs can know the tre number of a clan.
 */
    if ( get_trust( ch ) >= MAX_LEVEL && pObj->clan != 0 )
        printf_to_char( ch, "Clan: %d\n\r", pObj->clan );

    return FALSE;
}

/*
 * Need to issue warning if flag isn't valid. -- does so now -- Hugin.
 */
OEDIT( oedit_addaffect )
{
    int value;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];

    EDIT_OBJ( ch, pObj );

    argument = one_argument( argument, loc );
    one_argument( argument, mod );

    if ( loc[0] == '\0' || mod[0] == '\0' || !is_number( mod ) )
    {
        send_to_char( "Syntax:  addaffect [location] [#mod]\n\r", ch );
        return FALSE;
    }

    if ( ( value = flag_value( apply_flags, loc ) ) == NO_FLAG )    /* Hugin */
    {
        send_to_char( "Valid affects are:\n\r", ch );
        show_help( ch, "? affect" );
        return FALSE;
    }

    pAf = new_affect(  );
    pAf->location = value;
    pAf->modifier = atoi( mod );
    pAf->type = -1;
    pAf->duration = -1;
    pAf->bitvector = 0;
    pAf->next = pObj->affected;
    pObj->affected = pAf;

    send_to_char( "Affect added.\n\r", ch );
    return TRUE;
}

/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
OEDIT( oedit_delaffect )
{
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    AFFECT_DATA *pAf_next;
    char affect[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;

    EDIT_OBJ( ch, pObj );

    one_argument( argument, affect );

    if ( !is_number( affect ) || affect[0] == '\0' )
    {
        send_to_char( "Syntax:  delaffect [#affect]\n\r", ch );
        return FALSE;
    }

    value = atoi( affect );

    if ( value < 0 )
    {
        send_to_char( "Only non-negative affect-numbers allowed.\n\r", ch );
        return FALSE;
    }

    if ( !( pAf = pObj->affected ) )
    {
        send_to_char( "OEdit:  Non-existant affect.\n\r", ch );
        return FALSE;
    }

    if ( value == 0 )           /* First case: Remove first affect */
    {
        pAf = pObj->affected;
        pObj->affected = pAf->next;
        free_affect( pAf );
    }
    else                        /* Affect to remove is not the first */
    {
        while ( ( pAf_next = pAf->next ) && ( ++cnt < value ) )
            pAf = pAf_next;

        if ( pAf_next )         /* See if it's the next affect */
        {
            pAf->next = pAf_next->next;
            free_affect( pAf_next );
        }
        else                    /* Doesn't exist */
        {
            send_to_char( "No such affect.\n\r", ch );
            return FALSE;
        }
    }

    send_to_char( "Affect removed.\n\r", ch );
    return TRUE;
}

OEDIT( oedit_name )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ( ch, pObj );

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  name [string]\n\r", ch );
        return FALSE;
    }

    free_string( &pObj->name );
    pObj->name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch );
    return TRUE;
}

OEDIT( oedit_short )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ( ch, pObj );

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  short [string]\n\r", ch );
        return FALSE;
    }

    free_string( &pObj->short_descr );
    pObj->short_descr = str_dup( argument );
    pObj->short_descr[0] = LOWER( pObj->short_descr[0] );

    send_to_char( "Short description set.\n\r", ch );
    return TRUE;
}

OEDIT( oedit_long )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ( ch, pObj );

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  long [string]\n\r", ch );
        return FALSE;
    }

    free_string( &pObj->description );
    pObj->description = str_dup( argument );
    pObj->description[0] = UPPER( pObj->description[0] );

    send_to_char( "Long description set.\n\r", ch );
    return TRUE;
}

bool set_value( CHAR_DATA * ch, OBJ_INDEX_DATA * pObj, char *argument,
                int value )
{
    if ( argument[0] == '\0' )
    {
        set_obj_values( ch, pObj, -1, "" ); /* '\0' changed to "" -- Hugin */
        return FALSE;
    }

    if ( set_obj_values( ch, pObj, value, argument ) )
        return TRUE;

    return FALSE;
}

/*****************************************************************************
 Name:		oedit_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool oedit_values( CHAR_DATA * ch, char *argument, int value )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ( ch, pObj );

    if ( set_value( ch, pObj, argument, value ) )
        return TRUE;

    return FALSE;
}

OEDIT( oedit_value0 )
{
    if ( oedit_values( ch, argument, 0 ) )
        return TRUE;

    return FALSE;
}

OEDIT( oedit_value1 )
{
    if ( oedit_values( ch, argument, 1 ) )
        return TRUE;

    return FALSE;
}

OEDIT( oedit_value2 )
{
    if ( oedit_values( ch, argument, 2 ) )
        return TRUE;

    return FALSE;
}

OEDIT( oedit_value3 )
{
    if ( oedit_values( ch, argument, 3 ) )
        return TRUE;

    return FALSE;
}

OEDIT( oedit_value4 )
{
    if ( oedit_values( ch, argument, 4 ) )
        return TRUE;

    return FALSE;
}

OEDIT( oedit_clan )
{
    OBJ_INDEX_DATA *pObj;
    int clan;

    EDIT_OBJ( ch, pObj );

/* We can't have low level gods knowing the actual numbers of the clans */
    if ( get_trust( ch ) < MAX_LEVEL )
    {
        interpret( ch, "oclan" );   /* this _should_ produce an error message */
        return FALSE;
    }

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax:  oclan [number]\n\r", ch );
        return FALSE;
    }

    clan = atoi( argument );
    if ( clan > MAX_CLAN || clan < 0 )
    {
        send_to_char( "Syntax:  oclan [number]\n\r", ch );
        return FALSE;
    }

    pObj->clan = clan;

    send_to_char( "Clan set.\n\r", ch );
    return TRUE;
}

OEDIT( oedit_weight )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ( ch, pObj );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax:  weight [number]\n\r", ch );
        return FALSE;
    }

    pObj->weight = atoi( argument );

    send_to_char( "Weight set.\n\r", ch );
    return TRUE;
}

OEDIT( oedit_cost )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ( ch, pObj );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax:  cost [number]\n\r", ch );
        return FALSE;
    }

    pObj->cost = atoi( argument );

    send_to_char( "Cost set.\n\r", ch );
    return TRUE;
}

OEDIT( oedit_create )
{
    OBJ_INDEX_DATA *pObj;
    AREA_DATA *pArea;
    int value;
    int iHash;

    value = atoi( argument );

    if ( argument[0] == '\0' || value == 0 )
    {
        send_to_char( "Syntax:  oedit create [vnum]\n\r", ch );
        return FALSE;
    }

    pArea = get_vnum_area( value );

    if ( !pArea )
    {
        send_to_char( "OEdit:  That vnum is not assigned an area.\n\r", ch );
        return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
        send_to_char( "OEdit:  Vnum in an area you cannot build in.\n\r", ch );
        return FALSE;
    }

    if ( get_obj_index( value ) )
    {
        send_to_char( "OEdit:  Object vnum already exists.\n\r", ch );
        return FALSE;
    }

    pObj = new_obj_index(  );
    pObj->vnum = value;
    pObj->area = pArea;

    if ( value > top_vnum_obj )
        top_vnum_obj = value;

    iHash = value % MAX_KEY_HASH;
    pObj->next = obj_index_hash[iHash];
    obj_index_hash[iHash] = pObj;
    ch->desc->pEdit = ( void * ) pObj;

    send_to_char( "Object Created.\n\r", ch );
    return TRUE;
}

OEDIT( oedit_ed )
{
    OBJ_INDEX_DATA *pObj;
    EXTRA_DESCR_DATA *ed;
    char command[MAX_INPUT_LENGTH];
    char keyword[MAX_INPUT_LENGTH];

    EDIT_OBJ( ch, pObj );

    argument = one_argument( argument, command );
    one_argument( argument, keyword );

    if ( command[0] == '\0' )
    {
        send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
        send_to_char( "         ed delete [keyword]\n\r", ch );
        send_to_char( "         ed edit [keyword]\n\r", ch );
        send_to_char( "         ed format [keyword]\n\r", ch );
        return FALSE;
    }

    if ( !str_cmp( command, "add" ) )
    {
        if ( keyword[0] == '\0' )
        {
            send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
            return FALSE;
        }

        ed = new_extra_descr(  );
        ed->keyword = str_dup( keyword );
        ed->next = pObj->extra_descr;
        pObj->extra_descr = ed;

        string_append( ch, &ed->description );

        return TRUE;
    }

    if ( !str_cmp( command, "edit" ) )
    {
        if ( keyword[0] == '\0' )
        {
            send_to_char( "Syntax:  ed edit [keyword]\n\r", ch );
            return FALSE;
        }

        for ( ed = pObj->extra_descr; ed; ed = ed->next )
        {
            if ( is_name( keyword, ed->keyword ) )
                break;
        }

        if ( !ed )
        {
            send_to_char( "OEdit:  Extra description keyword not found.\n\r",
                          ch );
            return FALSE;
        }

        string_append( ch, &ed->description );

        return TRUE;
    }

    if ( !str_cmp( command, "delete" ) )
    {
        EXTRA_DESCR_DATA *ped = NULL;

        if ( keyword[0] == '\0' )
        {
            send_to_char( "Syntax:  ed delete [keyword]\n\r", ch );
            return FALSE;
        }

        for ( ed = pObj->extra_descr; ed; ed = ed->next )
        {
            if ( is_name( keyword, ed->keyword ) )
                break;
            ped = ed;
        }

        if ( !ed )
        {
            send_to_char( "OEdit:  Extra description keyword not found.\n\r",
                          ch );
            return FALSE;
        }

        if ( !ped )
            pObj->extra_descr = ed->next;
        else
            ped->next = ed->next;

        free_extra_descr( ed );

        send_to_char( "Extra description deleted.\n\r", ch );
        return TRUE;
    }

    if ( !str_cmp( command, "format" ) )
    {
        EXTRA_DESCR_DATA *ped = NULL;

        if ( keyword[0] == '\0' )
        {
            send_to_char( "Syntax:  ed format [keyword]\n\r", ch );
            return FALSE;
        }

        for ( ed = pObj->extra_descr; ed; ed = ed->next )
        {
            if ( is_name( keyword, ed->keyword ) )
                break;
            ped = ed;
        }

        if ( !ed )
        {
            send_to_char( "OEdit:  Extra description keyword not found.\n\r",
                          ch );
            return FALSE;
        }

        ed->description = format_string( ed->description );

        send_to_char( "Extra description formatted.\n\r", ch );
        return TRUE;
    }

    oedit_ed( ch, "" );
    return FALSE;
}

/* ROM object functions : */

OEDIT( oedit_extra )            /* Moved out of oedit() due to naming conflicts -- Hugin */
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_OBJ( ch, pObj );

        if ( ( value = flag_value( extra_flags, argument ) ) != NO_FLAG )
        {
            TOGGLE_BIT( pObj->extra_flags, value );

            send_to_char( "Extra flag toggled.\n\r", ch );
            return TRUE;
        }
    }

    send_to_char( "Syntax:  extra [flag]\n\r"
                  "Type '? extra' for a list of flags.\n\r", ch );
    return FALSE;
}

OEDIT( oedit_wear )             /* Moved out of oedit() due to naming conflicts -- Hugin */
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_OBJ( ch, pObj );

        if ( ( value = flag_value( wear_flags, argument ) ) != NO_FLAG )
        {
            TOGGLE_BIT( pObj->wear_flags, value );

            send_to_char( "Wear flag toggled.\n\r", ch );
            return TRUE;
        }
    }

    send_to_char( "Syntax:  wear [flag]\n\r"
                  "Type '? wear' for a list of flags.\n\r", ch );
    return FALSE;
}

OEDIT( oedit_type )             /* Moved out of oedit() due to naming conflicts -- Hugin */
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_OBJ( ch, pObj );

        if ( ( value = flag_value( type_flags, argument ) ) != NO_FLAG )
        {
            pObj->item_type = value;

            send_to_char( "Type set.\n\r", ch );

            /*
             * Clear the values.
             */
            pObj->value[0] = 0;
            pObj->value[1] = 0;
            pObj->value[2] = 0;
            pObj->value[3] = 0;
            pObj->value[4] = 0; /* ROM */

            return TRUE;
        }
    }

    send_to_char( "Syntax:  type [flag]\n\r"
                  "Type '? type' for a list of flags.\n\r", ch );
    return FALSE;
}

OEDIT( oedit_material )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_OBJ( ch, pObj );

        if ( ( value = flag_value( material_type, argument ) ) != NO_FLAG )
        {
            pObj->material = value;
            send_to_char( "Material type set.\n\r", ch );
            return TRUE;
        }
    }

    send_to_char( "Syntax:  material [material-name]\n\r"
                  "Type '? material' for a list of materials.\n\r", ch );
    return FALSE;
}

OEDIT( oedit_level )
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ( ch, pObj );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax:  level [number]\n\r", ch );
        return FALSE;
    }

    pObj->level = atoi( argument );

    send_to_char( "Level set.\n\r", ch );
    return TRUE;
}

OEDIT( oedit_condition )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( argument[0] != '\0'
         && ( value = atoi( argument ) ) >= 0 && ( value <= 100 ) )
    {
        EDIT_OBJ( ch, pObj );

        pObj->condition = value;
        send_to_char( "Condition set.\n\r", ch );

        return TRUE;
    }

    send_to_char( "Syntax:  condition [number]\n\r"
                  "Where number can range from 0 (ruined) to 100 (perfect).\n\r",
                  ch );
    return FALSE;
}

/*
 * Mobile Editor Functions.
 */
MEDIT( medit_show )
{
    MOB_INDEX_DATA *pMob;
    FACTIONAFF_DATA *pFactAff;
    int x;

    EDIT_MOB( ch, pMob );

    sprintf( buf, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
             pMob->player_name,
             !pMob->area ? -1 : pMob->area->vnum,
             !pMob->area ? "No Area" : pMob->area->name );
    send_to_char( buf, ch );

    sprintf( buf, "Act:         [%s]\n\r",
             flag_string( act_flags, pMob->act ) );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum:        [%5d]\n\rSex:         [%s]\n\r", pMob->vnum, pMob->sex == SEX_MALE ? "male" : pMob->sex == SEX_FEMALE ? "female" : pMob->sex == 3 ? "random" : "neutral" ); /* ROM magic number */
    send_to_char( buf, ch );

    sprintf( buf, "Race:        [%s]\n\r",  /* ROM OLC */
             race_table[pMob->race].name );
    send_to_char( buf, ch );

    sprintf( buf,
             "Level:       [%2d]\n\rAlign:       [%4d]\n\r",
             pMob->level, pMob->alignment );
    send_to_char( buf, ch );

/* ROM values: */

    sprintf( buf, "Hitroll:     [%d]\n\r", pMob->hitroll );
    send_to_char( buf, ch );

    sprintf( buf, "Hit dice:    [%2dd%-3d+%4d]\n\r",
             pMob->hit[DICE_NUMBER],
             pMob->hit[DICE_TYPE], pMob->hit[DICE_BONUS] );
    send_to_char( buf, ch );

    sprintf( buf, "Damage dice: [%2dd%-3d+%4d]\n\r",
             pMob->damage[DICE_NUMBER],
             pMob->damage[DICE_TYPE], pMob->damage[DICE_BONUS] );
    send_to_char( buf, ch );

    sprintf( buf, "Mana dice:   [%2dd%-3d+%4d]\n\r",
             pMob->mana[DICE_NUMBER],
             pMob->mana[DICE_TYPE], pMob->mana[DICE_BONUS] );
    send_to_char( buf, ch );

    sprintf( buf, "Damage Type: [%s]\n\r", attack_table[pMob->dam_type].name );
    send_to_char( buf, ch );

/* ROM values end */

    sprintf( buf, "Affected by: [%s]\n\r",
             flag_string( affect_flags, pMob->affected_by ) );
    send_to_char( buf, ch );
    send_to_char( "NEWAffected by: [ ", ch );
    for ( x = 1; x <= MAX_NEWAFF_BIT; x++ )
    {
        if ( IS_NEWAFF_SET( pMob->newaff, x ) )
            printf_to_char( ch, "%s ", newaff_table[x].name );
    }
    send_to_char( "]\n\r", ch );

/* ROM values: */

    sprintf( buf,
             "Armor:       [pierce: %d  bash: %d  slash: %d  magic: %d]\n\r",
             pMob->ac[AC_PIERCE], pMob->ac[AC_BASH], pMob->ac[AC_SLASH],
             pMob->ac[AC_EXOTIC] );
    send_to_char( buf, ch );

    sprintf( buf, "Form:        [%s]\n\r",
             flag_string( form_flags, pMob->form ) );
    send_to_char( buf, ch );

    sprintf( buf, "Parts:       [%s]\n\r",
             flag_string( part_flags, pMob->parts ) );
    send_to_char( buf, ch );

    sprintf( buf, "Imm:         [%s]\n\r",
             flag_string( imm_flags, pMob->imm_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Res:         [%s]\n\r",
             flag_string( res_flags, pMob->res_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Vuln:        [%s]\n\r",
             flag_string( vuln_flags, pMob->vuln_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Off:         [%s]\n\r",
             flag_string( off_flags, pMob->off_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Size:        [%s]\n\r",
             flag_string( size_flags, pMob->size ) );
    send_to_char( buf, ch );

    sprintf( buf, "Start pos.   [%s]\n\r",
             flag_string( position_flags, pMob->start_pos ) );
    send_to_char( buf, ch );

    sprintf( buf, "Default pos  [%s]\n\r",
             flag_string( position_flags, pMob->default_pos ) );
    send_to_char( buf, ch );

    sprintf( buf, "Gold:        [%5ld]\n\r", pMob->gold );
    send_to_char( buf, ch );

    sprintf( buf, "Breath weapon strength:      [%4d]\n\r",
             pMob->breath_percent );
    send_to_char( buf, ch );

    sprintf( buf, "Random objects:\n\rChance:      [%4d]\n\r"
             "Number:      [%4d]\n\r",
             pMob->rnd_obj_percent, pMob->rnd_obj_num );
    send_to_char( buf, ch );

    sprintf( buf, "Types:       [%s]\n\r",
             flag_string( rnd_obj_flags, pMob->rnd_obj_types ) );
    send_to_char( buf, ch );

/* ROM values end */

    sprintf( buf, "Short descr: %s\n\rLong descr:\n\r%s",
             pMob->short_descr, pMob->long_descr );
    send_to_char( buf, ch );

    sprintf( buf, "Description:\n\r%s", pMob->description );
    send_to_char( buf, ch );

    if ( pMob->pShop )
    {
        SHOP_DATA *pShop;
        int iTrade;

        pShop = pMob->pShop;

        sprintf( buf,
                 "Shop data for [%5d]:\n\r"
                 "  Markup for purchaser: %d%%\n\r"
                 "  Markdown for seller:  %d%%\n\r",
                 pShop->keeper, pShop->profit_buy, pShop->profit_sell );
        send_to_char( buf, ch );
        sprintf( buf, "  Hours: %d to %d.\n\r",
                 pShop->open_hour, pShop->close_hour );
        send_to_char( buf, ch );

        for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
        {
            if ( pShop->buy_type[iTrade] != 0 )
            {
                if ( iTrade == 0 )
                {
                    send_to_char( "  Number Trades Type\n\r", ch );
                    send_to_char( "  ------ -----------\n\r", ch );
                }
                sprintf( buf, "  [%4d] %s\n\r", iTrade,
                         flag_string( type_flags, pShop->buy_type[iTrade] ) );
                send_to_char( buf, ch );
            }
        }
    }

    send_to_char( "\n\rFactions affected:\n\r", ch );

    for ( pFactAff = pMob->faction_affs;
          pFactAff != NULL; pFactAff = pFactAff->next )
    {
        printf_to_char( ch, "%s will be %s by %d points.\n\r",
                        pFactAff->faction->name,
                        ( pFactAff->change > 0 ) ? "increased" : "decreased",
                        abs( pFactAff->change ) );
    }

    return FALSE;
}

MEDIT( medit_create )
{
    MOB_INDEX_DATA *pMob;
    AREA_DATA *pArea;
    int value;
    int iHash;

    value = atoi( argument );
    if ( argument[0] == '\0' || value == 0 )
    {
        send_to_char( "Syntax:  medit create [vnum]\n\r", ch );
        return FALSE;
    }

    pArea = get_vnum_area( value );

    if ( !pArea )
    {
        send_to_char( "MEdit:  That vnum is not assigned an area.\n\r", ch );
        return FALSE;
    }

    if ( !IS_BUILDER( ch, pArea ) )
    {
        send_to_char( "MEdit:  Vnum in an area you cannot build in.\n\r", ch );
        return FALSE;
    }

    if ( get_mob_index( value ) )
    {
        send_to_char( "MEdit:  Mobile vnum already exists.\n\r", ch );
        return FALSE;
    }

    pMob = new_mob_index(  );
    pMob->vnum = value;
    pMob->area = pArea;

    if ( value > top_vnum_mob )
        top_vnum_mob = value;

    pMob->act = ACT_IS_NPC;
    pMob->hit[DICE_NUMBER] = 1;
    pMob->hit[DICE_TYPE] = 1;
    pMob->hit[DICE_BONUS] = 1;
    iHash = value % MAX_KEY_HASH;
    pMob->next = mob_index_hash[iHash];
    mob_index_hash[iHash] = pMob;
    ch->desc->pEdit = ( void * ) pMob;

    send_to_char( "Mobile Created.\n\r", ch );
    return TRUE;
}

MEDIT( medit_align )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax:  alignment [number]\n\r", ch );
        return FALSE;
    }

    pMob->alignment = atoi( argument );

    send_to_char( "Alignment set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_level )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax:  level [number]\n\r", ch );
        return FALSE;
    }

    pMob->level = atoi( argument );

    send_to_char( "Level set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_desc )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
        string_append( ch, &pMob->description );
        return TRUE;
    }

    send_to_char( "Syntax:  desc    - line edit\n\r", ch );
    return FALSE;
}

MEDIT( medit_long )
{
    MOB_INDEX_DATA *pMob;
    char buf[MAX_STRING_LENGTH];

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  long [string]\n\r", ch );
        return FALSE;
    }

    free_string( &pMob->long_descr );
    sprintf( buf, "%s\n\r", argument );
    pMob->long_descr = str_dup( buf );
    pMob->long_descr[0] = UPPER( pMob->long_descr[0] );

    send_to_char( "Long description set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_short )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  short [string]\n\r", ch );
        return FALSE;
    }

    free_string( &pMob->short_descr );
    pMob->short_descr = str_dup( argument );

    send_to_char( "Short description set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_name )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  name [string]\n\r", ch );
        return FALSE;
    }

    free_string( &pMob->player_name );
    pMob->player_name = str_dup( argument );

    send_to_char( "Name set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_shop )
{
    MOB_INDEX_DATA *pMob;
    char command[MAX_INPUT_LENGTH];

    argument = one_argument( argument, command );
    argument = one_argument( argument, arg );

    EDIT_MOB( ch, pMob );

    if ( command[0] == '\0' )
    {
        send_to_char( "Syntax:  shop hours [#opening] [#closing]\n\r", ch );
        send_to_char( "         shop profit [#buying%] [#selling%]\n\r", ch );
        send_to_char( "         shop type [#0-4] [item type]\n\r", ch );
        send_to_char( "         shop delete [#0-4]\n\r", ch );
        return FALSE;
    }

    if ( !str_cmp( command, "hours" ) )
    {
        if ( arg[0] == '\0' || !is_number( arg )
             || argument[0] == '\0' || !is_number( argument ) )
        {
            send_to_char( "Syntax:  shop hours [#opening] [#closing]\n\r", ch );
            return FALSE;
        }

        if ( !pMob->pShop )
        {
            pMob->pShop = new_shop(  );
            pMob->pShop->keeper = pMob->vnum;
            shop_last->next = pMob->pShop;
        }

        pMob->pShop->open_hour = atoi( arg );
        pMob->pShop->close_hour = atoi( argument );

        send_to_char( "Shop hours set.\n\r", ch );
        return TRUE;
    }

    if ( !str_cmp( command, "profit" ) )
    {
        if ( arg[0] == '\0' || !is_number( arg )
             || argument[0] == '\0' || !is_number( argument ) )
        {
            send_to_char( "Syntax:  shop profit [#buying%] [#selling%]\n\r",
                          ch );
            return FALSE;
        }

        if ( !pMob->pShop )
        {
            pMob->pShop = new_shop(  );
            pMob->pShop->keeper = pMob->vnum;
            shop_last->next = pMob->pShop;
        }

        pMob->pShop->profit_buy = atoi( arg );
        pMob->pShop->profit_sell = atoi( argument );

        send_to_char( "Shop profit set.\n\r", ch );
        return TRUE;
    }

    if ( !str_cmp( command, "type" ) )
    {
        int value;

        if ( arg[0] == '\0' || !is_number( arg ) || argument[0] == '\0' )
        {
            send_to_char( "Syntax:  shop type [#0-4] [item type]\n\r", ch );
            return FALSE;
        }

        if ( atoi( arg ) >= MAX_TRADE )
        {
            sprintf( buf, "REdit:  May sell %d items max.\n\r", MAX_TRADE );
            send_to_char( buf, ch );
            return FALSE;
        }

        if ( ( value = flag_value( type_flags, argument ) ) == NO_FLAG )
        {
            send_to_char( "REdit:  That type of item is not known.\n\r", ch );
            return FALSE;
        }

        if ( !pMob->pShop )
        {
            pMob->pShop = new_shop(  );
            pMob->pShop->keeper = pMob->vnum;
            shop_last->next = pMob->pShop;
        }

        pMob->pShop->buy_type[atoi( arg )] = value;

        send_to_char( "Shop type set.\n\r", ch );
        return TRUE;
    }

    if ( !str_cmp( command, "delete" ) )
    {
        SHOP_DATA *pShop;
        SHOP_DATA *pShop_next;
        int value;
        int cnt = 0;

        if ( arg[0] == '\0' || !is_number( arg ) )
        {
            send_to_char( "Syntax:  shop delete [#0-4]\n\r", ch );
            return FALSE;
        }

        value = atoi( argument );

        if ( !pMob->pShop )
        {
            send_to_char( "REdit:  Non-existant shop.\n\r", ch );
            return FALSE;
        }

        if ( value == 0 )
        {
            pShop = pMob->pShop;
            pMob->pShop = pMob->pShop->next;
            free_shop( pShop );
        }
        else
            for ( pShop = pMob->pShop, cnt = 0; pShop;
                  pShop = pShop_next, cnt++ )
            {
                pShop_next = pShop->next;
                if ( cnt + 1 == value )
                {
                    pShop->next = pShop_next->next;
                    free_shop( pShop_next );
                    break;
                }
            }

        send_to_char( "Shop deleted.\n\r", ch );
        return TRUE;
    }

    medit_shop( ch, "" );
    return FALSE;
}

/* ROM medit functions: */

MEDIT( medit_sex )              /* Moved out of medit() due to naming conflicts -- Hugin */
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_MOB( ch, pMob );

        if ( ( value = flag_value( sex_flags, argument ) ) != NO_FLAG )
        {
            pMob->sex = value;

            send_to_char( "Sex set.\n\r", ch );
            return TRUE;
        }
    }

    send_to_char( "Syntax: sex [sex]\n\r"
                  "Type '? sex' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_act )              /* Moved out of medit() due to naming conflicts -- Hugin */
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_MOB( ch, pMob );

        if ( ( value = flag_value( act_flags, argument ) ) != NO_FLAG )
        {
            pMob->act ^= value;
            SET_BIT( pMob->act, ACT_IS_NPC );

            send_to_char( "Act flag toggled.\n\r", ch );
            return TRUE;
        }
    }

    send_to_char( "Syntax: act [flag]\n\r"
                  "Type '? act' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_affect )           /* Moved out of medit() due to naming conflicts -- Hugin */
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_MOB( ch, pMob );

        if ( ( value = flag_value( affect_flags, argument ) ) != NO_FLAG )
        {
            pMob->affected_by ^= value;

            send_to_char( "Affect flag toggled.\n\r", ch );
            return TRUE;
        }
    }

    send_to_char( "Syntax: affect [flag]\n\r"
                  "Type '? affect' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_ac )
{
    MOB_INDEX_DATA *pMob;
    int pierce, bash, slash, exotic;

    do                          /* So that I can use break and send the syntax in one place */
    {
        if ( argument[0] == '\0' )
            break;

        EDIT_MOB( ch, pMob );
        argument = one_argument( argument, arg );

        if ( !is_number( arg ) )
            break;
        pierce = atoi( arg );
        argument = one_argument( argument, arg );

        if ( arg[0] != '\0' )
        {
            if ( !is_number( arg ) )
                break;
            bash = atoi( arg );
            argument = one_argument( argument, arg );
        }
        else
            bash = pMob->ac[AC_BASH];

        if ( arg[0] != '\0' )
        {
            if ( !is_number( arg ) )
                break;
            slash = atoi( arg );
            argument = one_argument( argument, arg );
        }
        else
            slash = pMob->ac[AC_SLASH];

        if ( arg[0] != '\0' )
        {
            if ( !is_number( arg ) )
                break;
            exotic = atoi( arg );
        }
        else
            exotic = pMob->ac[AC_EXOTIC];

        pMob->ac[AC_PIERCE] = pierce;
        pMob->ac[AC_BASH] = bash;
        pMob->ac[AC_SLASH] = slash;
        pMob->ac[AC_EXOTIC] = exotic;

        send_to_char( "Ac set.\n\r", ch );
        return TRUE;
    }
    while ( FALSE );            /* Just do it once.. */

    send_to_char( "Syntax:  ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n\r"
                  "help MOB_AC  gives a list of reasonable ac-values.\n\r",
                  ch );
    return FALSE;
}

MEDIT( medit_form )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_MOB( ch, pMob );

        if ( ( value = flag_value( form_flags, argument ) ) != NO_FLAG )
        {
            pMob->form ^= value;
            send_to_char( "Form toggled.\n\r", ch );
            return TRUE;
        }
    }

    send_to_char( "Syntax: form [flags]\n\r"
                  "Type '? form' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_part )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_MOB( ch, pMob );

        if ( ( value = flag_value( part_flags, argument ) ) != NO_FLAG )
        {
            pMob->parts ^= value;
            send_to_char( "Parts toggled.\n\r", ch );
            return TRUE;
        }
    }

    send_to_char( "Syntax: part [flags]\n\r"
                  "Type '? part' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_imm )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_MOB( ch, pMob );

        if ( ( value = flag_value( imm_flags, argument ) ) != NO_FLAG )
        {
            pMob->imm_flags ^= value;
            send_to_char( "Immunity toggled.\n\r", ch );
            return TRUE;
        }
    }

    send_to_char( "Syntax: imm [flags]\n\r"
                  "Type '? imm' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_res )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_MOB( ch, pMob );

        if ( ( value = flag_value( res_flags, argument ) ) != NO_FLAG )
        {
            pMob->res_flags ^= value;
            send_to_char( "Resistance toggled.\n\r", ch );
            return TRUE;
        }
    }

    send_to_char( "Syntax: res [flags]\n\r"
                  "Type '? res' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_vuln )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_MOB( ch, pMob );

        if ( ( value = flag_value( vuln_flags, argument ) ) != NO_FLAG )
        {
            pMob->vuln_flags ^= value;
            send_to_char( "Vulnerability toggled.\n\r", ch );
            return TRUE;
        }
    }

    send_to_char( "Syntax: vuln [flags]\n\r"
                  "Type '? vuln' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_material )
{
    MOB_INDEX_DATA *pMob;
    int value;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax:  material [material-name]\n\r"
                      "Type '? material' for a list of materials.\n\r", ch );
        return FALSE;
    }

    if ( ( value = flag_value( material_type, argument ) ) != NO_FLAG )
    {
        pMob->material = value;
        send_to_char( "Material type set.\n\r", ch );
        return TRUE;
    }

    send_to_char( "Unknown material type, '? material' for a list.\n\r", ch );
    return FALSE;
}

MEDIT( medit_off )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_MOB( ch, pMob );

        if ( ( value = flag_value( off_flags, argument ) ) != NO_FLAG )
        {
            pMob->off_flags ^= value;
            send_to_char( "Offensive behaviour toggled.\n\r", ch );
            return TRUE;
        }
    }

    send_to_char( "Syntax: off [flags]\n\r"
                  "Type '? off' for a list of flags.\n\r", ch );
    return FALSE;
}

MEDIT( medit_size )
{
    MOB_INDEX_DATA *pMob;
    int value;

    if ( argument[0] != '\0' )
    {
        EDIT_MOB( ch, pMob );

        if ( ( value = flag_value( size_flags, argument ) ) != NO_FLAG )
        {
            pMob->size = value;
            send_to_char( "Size set.\n\r", ch );
            return TRUE;
        }
    }

    send_to_char( "Syntax: size [size]\n\r"
                  "Type '? size' for a list of sizes.\n\r", ch );
    return FALSE;
}

MEDIT( medit_hitdice )
{
    static char syntax[] = "Syntax:  hitdice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
        send_to_char( syntax, ch );
        return FALSE;
    }

    num = cp = argument;

    while ( isdigit( *cp ) )
        ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )
        *( cp++ ) = '\0';

    type = cp;

    while ( isdigit( *cp ) )
        ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )
        *( cp++ ) = '\0';

    bonus = cp;

    while ( isdigit( *cp ) )
        ++cp;
    if ( *cp != '\0' )
        *cp = '\0';

    if ( ( !is_number( num ) || atoi( num ) < 1 )
         || ( !is_number( type ) || atoi( type ) < 1 )
         || ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
        send_to_char( syntax, ch );
        return FALSE;
    }

    pMob->hit[DICE_NUMBER] = atoi( num );
    pMob->hit[DICE_TYPE] = atoi( type );
    pMob->hit[DICE_BONUS] = atoi( bonus );

    send_to_char( "Hitdice set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_manadice )
{
    static char syntax[] = "Syntax:  manadice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
        send_to_char( syntax, ch );
        return FALSE;
    }

    num = cp = argument;

    while ( isdigit( *cp ) )
        ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )
        *( cp++ ) = '\0';

    type = cp;

    while ( isdigit( *cp ) )
        ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )
        *( cp++ ) = '\0';

    bonus = cp;

    while ( isdigit( *cp ) )
        ++cp;
    if ( *cp != '\0' )
        *cp = '\0';

    if ( !( is_number( num ) && is_number( type ) && is_number( bonus ) ) )
    {
        send_to_char( syntax, ch );
        return FALSE;
    }

    if ( ( !is_number( num ) || atoi( num ) < 1 )
         || ( !is_number( type ) || atoi( type ) < 1 )
         || ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
        send_to_char( syntax, ch );
        return FALSE;
    }

    pMob->mana[DICE_NUMBER] = atoi( num );
    pMob->mana[DICE_TYPE] = atoi( type );
    pMob->mana[DICE_BONUS] = atoi( bonus );

    send_to_char( "Manadice set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_damdice )
{
    static char syntax[] = "Syntax:  damdice <number> d <type> + <bonus>\n\r";
    char *num, *type, *bonus, *cp;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
        send_to_char( syntax, ch );
        return FALSE;
    }

    num = cp = argument;

    while ( isdigit( *cp ) )
        ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )
        *( cp++ ) = '\0';

    type = cp;

    while ( isdigit( *cp ) )
        ++cp;
    while ( *cp != '\0' && !isdigit( *cp ) )
        *( cp++ ) = '\0';

    bonus = cp;

    while ( isdigit( *cp ) )
        ++cp;
    if ( *cp != '\0' )
        *cp = '\0';

    if ( !( is_number( num ) && is_number( type ) && is_number( bonus ) ) )
    {
        send_to_char( syntax, ch );
        return FALSE;
    }

    if ( ( !is_number( num ) || atoi( num ) < 1 )
         || ( !is_number( type ) || atoi( type ) < 1 )
         || ( !is_number( bonus ) || atoi( bonus ) < 0 ) )
    {
        send_to_char( syntax, ch );
        return FALSE;
    }

    pMob->damage[DICE_NUMBER] = atoi( num );
    pMob->damage[DICE_TYPE] = atoi( type );
    pMob->damage[DICE_BONUS] = atoi( bonus );

    send_to_char( "Damdice set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_damtype )
{
    static char syntax[] = "Syntax:  damtype <damage type>\n\r";
    int x;
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' )
    {
        send_to_char( syntax, ch );
        return FALSE;
    }

    for ( x = 0; x <= MAX_ATTACK_TYPE + 1; x++ )
    {
        if ( x == MAX_ATTACK_TYPE + 1 )
        {
            send_to_char( "Invalid damage type, do 'help DAMTYPE'.\n\r", ch );
            return FALSE;
        }
        if ( !strcmp( attack_table[x].name, argument ) )
        {
            send_to_char( "Damage type set.\n\r", ch );
            pMob->dam_type = x;
            return TRUE;
        }
    }

    return FALSE;
}

MEDIT( medit_race )
{
    MOB_INDEX_DATA *pMob;
    int race;

    if ( argument[0] != '\0' && ( race = race_lookup( argument ) ) != 0 )
    {
        EDIT_MOB( ch, pMob );

        pMob->race = race;
        pMob->off_flags |= race_table[race].off;
        pMob->imm_flags |= race_table[race].imm;
        pMob->res_flags |= race_table[race].res;
        pMob->vuln_flags |= race_table[race].vuln;
        pMob->form |= race_table[race].form;
        pMob->parts |= race_table[race].parts;

        send_to_char( "Race set.\n\r", ch );
        return TRUE;
    }

    if ( argument[0] == '?' )
    {
        send_to_char( "Available races are:", ch );

        for ( race = 0; race_table[race].name != NULL; race++ )
        {
            if ( ( race % 3 ) == 0 )
                send_to_char( "\n\r", ch );
            sprintf( buf, " %-15s", race_table[race].name );
            send_to_char( buf, ch );
        }

        send_to_char( "\n\r", ch );
        return FALSE;
    }

    send_to_char( "Syntax:  race [race]\n\r"
                  "Type 'race ?' for a list of races.\n\r", ch );
    return FALSE;
}

MEDIT( medit_position )
{
    MOB_INDEX_DATA *pMob;
    int value;

    argument = one_argument( argument, arg );

    switch ( arg[0] )
    {
    default:
        break;

    case 'S':
    case 's':
        if ( str_prefix( arg, "start" ) )
            break;

        if ( ( value = flag_value( position_flags, argument ) ) == NO_FLAG )
            break;

        EDIT_MOB( ch, pMob );

        pMob->start_pos = value;
        send_to_char( "Start position set.\n\r", ch );
        return TRUE;

    case 'D':
    case 'd':
        if ( str_prefix( arg, "default" ) )
            break;

        if ( ( value = flag_value( position_flags, argument ) ) == NO_FLAG )
            break;

        EDIT_MOB( ch, pMob );

        pMob->default_pos = value;
        send_to_char( "Default position set.\n\r", ch );
        return TRUE;
    }

    send_to_char( "Syntax:  position [start/default] [position]\n\r"
                  "Type '? position' for a list of positions.\n\r", ch );
    return FALSE;
}

MEDIT( medit_gold )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax:  gold [number]\n\r", ch );
        return FALSE;
    }

    pMob->gold = atoi( argument );

    send_to_char( "Gold set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_hitroll )
{
    MOB_INDEX_DATA *pMob;

    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax:  hitroll [number]\n\r", ch );
        return FALSE;
    }

    pMob->hitroll = atoi( argument );

    send_to_char( "Hitroll set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_randpct )
{
    MOB_INDEX_DATA *pMob;
    int pctbuf = 0;
    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax: chance [number]\n\r", ch );
        return FALSE;
    }
    pctbuf = atoi( argument );

    if ( pctbuf < 0 || pctbuf > 100 )
    {
        send_to_char( "Number must be from 0 to 100.\n\r", ch );
        return FALSE;
    }

    pMob->rnd_obj_percent = pctbuf;
    send_to_char( "Random object loading percent set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_bdamage )
{
    MOB_INDEX_DATA *pMob;
    int percentbuf;
    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax: breath [number]\n\r", ch );
        return FALSE;
    }

    percentbuf = atoi( argument );

    if ( percentbuf < -1000 || percentbuf > 1000 )
    {
        send_to_char( "Number must be between -1000 and 1000.\n\r", ch );
        return FALSE;
    }

    pMob->breath_percent = percentbuf;

    send_to_char( "Breath weapon percentage set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_randnum )
{
    MOB_INDEX_DATA *pMob;
    int randbuf;
    EDIT_MOB( ch, pMob );

    if ( argument[0] == '\0' || !is_number( argument ) )
    {
        send_to_char( "Syntax:  number [number]\n\r", ch );
        return FALSE;
    }

    randbuf = atoi( argument );
    if ( randbuf < 0 || randbuf > 10 )
    {
        send_to_char( "Number must be from 0 to 10.\n\r", ch );
        return FALSE;
    }
    pMob->rnd_obj_num = randbuf;

    send_to_char( "Number of random objects possible set.\n\r", ch );
    return TRUE;
}

MEDIT( medit_randtype )
{
    MOB_INDEX_DATA *pMob;
    int value;
    EDIT_MOB( ch, pMob );

    if ( argument[0] != '\0' )
    {
        if ( ( value = flag_value( rnd_obj_flags, argument ) ) != NO_FLAG )
        {
            pMob->rnd_obj_types ^= value;
            send_to_char( "Type set.\n\r", ch );
            return TRUE;
        }
        send_to_char( "Syntax:  types [flag]\n\r"
                      "Type '? randobj' for a list of flags.\n\r", ch );
        return FALSE;
    }

    send_to_char( "Syntax:  types [flag]\n\r"
                  "Type '? randobj' for a list of flags.\n\r", ch );
    return FALSE;
}

/*
 * MudProg Editor Functions.
 */

MPEDIT( mpedit_create )
{
    MPROG_DATA *pMudProg;

    if ( ch->desc->pEdit )
    {
        send_to_char( "You're already editing a MudProg.\n\r", ch );
        return FALSE;
    }

    if ( *argument == '\0' || ( str_prefix( argument, "mobprog" )
                                && str_prefix( argument, "objprog" )
                                && str_prefix( argument, "roomprog" ) ) )
    {
        send_to_char
            ( "Syntax:  edit mprog create [mobprog||objprog||roomprog]\n\r",
              ch );
        return FALSE;
    }

    pMudProg = new_mudprog(  );

    if ( !str_prefix( argument, "mobprog" ) )
        pMudProg->prog_type = MOB_PROG;

    if ( !str_prefix( argument, "objprog" ) )
        pMudProg->prog_type = OBJ_PROG;

    if ( !str_prefix( argument, "roomprog" ) )
        pMudProg->prog_type = ROOM_PROG;

    ch->desc->pEdit = ( void * ) pMudProg;

    send_to_char( "MudProg Created.\n\r", ch );
    return TRUE;
}

MPEDIT( mpedit_delete )
{
    MPROG_DATA *pMudProg;
    MPROG_DATA *pTempProg;
    MPROG_GROUP *pMprogGroup;
    MPROG_LIST *pList;
    OBJ_INDEX_DATA *pObj;
    MOB_INDEX_DATA *pMob;
    ROOM_INDEX_DATA *pRoom;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int vnum;
    int nMatch;

    argument = one_argument( argument, arg );

    if ( *arg != '\0' )
    {
        if ( ( vnum = atoi( arg ) ) < 1 )
        {
            send_to_char( "Syntax:  Delete [MudProg Vnum]\n\r", ch );
            return FALSE;
        }

        if ( ( pMudProg = get_mprog_by_vnum( vnum ) ) == NULL )
        {
            send_to_char( "Invalid vnum.\n\r", ch );
            return FALSE;
        }
    }
    else
        EDIT_MPROG( ch, pMudProg );

    /*
     * This mudprog is new and hasn't been added to the list, just free it.
     */
    if ( pMudProg->vnum == 0 )
    {
        sprintf( buf, "MudProg [%4d] %s - deleted.\n\r", pMudProg->vnum,
                 pMudProg->name );
        free_mudprog( pMudProg );
        send_to_char( buf, ch );
        edit_done( ch );
        return TRUE;
    }

    /*
     * Check to see if this mudprog is in use or is in a group.  If either is
     * true then don't allow the deletion.  -Zane
     */
    switch ( pMudProg->prog_type )
    {
    case MOB_PROG:
        nMatch = 0;
        for ( vnum = 0; nMatch < top_mob_index; vnum++ )
            if ( ( pMob = get_mob_index( vnum ) ) != NULL )
            {
                if ( IS_SET( pMob->progtypes, pMudProg->trigger_type ) )
                    for ( pList = pMob->mudprogs; pList; pList = pList->next )
                        if ( pList->mudprog->vnum == pMudProg->vnum )
                        {
                            send_to_char
                                ( "This prog is in use, you cannot delete it.\n\r",
                                  ch );
                            return FALSE;
                        }
                nMatch++;
            }
        break;
    case ROOM_PROG:
        nMatch = 0;
        for ( vnum = 0; nMatch < top_room; vnum++ )
            if ( ( pRoom = get_room_index( vnum ) ) != NULL )
            {
                if ( IS_SET( pRoom->progtypes, pMudProg->trigger_type ) )
                    for ( pList = pRoom->mudprogs; pList; pList = pList->next )
                        if ( pList->mudprog->vnum == pMudProg->vnum )
                        {
                            send_to_char
                                ( "This prog is in use, you cannot delete it.\n\r",
                                  ch );
                            return FALSE;
                        }
                nMatch++;
            }
        break;
    case OBJ_PROG:
        nMatch = 0;
        for ( vnum = 0; nMatch < top_obj_index; vnum++ )
            if ( ( pObj = get_obj_index( vnum ) ) != NULL )
            {
                if ( IS_SET( pObj->progtypes, pMudProg->trigger_type ) )
                    for ( pList = pObj->mudprogs; pList; pList = pList->next )
                        if ( pList->mudprog->vnum == pMudProg->vnum )
                        {
                            send_to_char
                                ( "This prog is in use, you cannot delete it.\n\r",
                                  ch );
                            return FALSE;
                        }
                nMatch++;
            }
        break;
    }

    /* Check groups */
    for ( pMprogGroup = mprog_group_first; pMprogGroup;
          pMprogGroup = pMprogGroup->next )
        if ( pMprogGroup->prog_type == pMudProg->prog_type )
            for ( pList = pMprogGroup->mudprogs; pList; pList = pList->next )
                if ( pList->mudprog->vnum == pMudProg->vnum )
                {
                    send_to_char
                        ( "This prog belongs to a group, it cannot be deleted.\n\r",
                          ch );
                    return FALSE;
                }

    sprintf( buf, "MudProg [%4d] %s - deleted.\n\r", pMudProg->vnum,
             pMudProg->name );

    /* Remove mudprog from the list if it's in there */
    if ( mudprog_first == pMudProg )
        mudprog_first = pMudProg->next;
    else
    {
        for ( pTempProg = mudprog_first; pTempProg->next != pMudProg;
              pTempProg = pTempProg->next )
            ;

        pTempProg->next = pMudProg->next;
    }

    if ( mudprog_last == pMudProg )
    {
        for ( pTempProg = mudprog_first; !pTempProg->next;
              pTempProg = pTempProg->next )
            ;

        mudprog_last = pTempProg;
    }

    free_mudprog( pMudProg );
    send_to_char( buf, ch );
    edit_done( ch );
    return TRUE;
}

MPEDIT( mpedit_description )
{
    MPROG_DATA *pMudProg;
    char colorbuf[MAX_STRING_LENGTH];

    EDIT_MPROG( ch, pMudProg );

    if ( *argument != '\0' )
    {
        if ( *pMudProg->description != '\0' )
            free_string( &pMudProg->description );

        do_color( argument, strlen( argument ), colorbuf, sizeof( colorbuf ),
                  FALSE );
        pMudProg->description = str_dup( colorbuf );
        send_to_char( "MudProg description set.\n\r", ch );
    }
    else
    {
        send_to_char( "Syntax:  Description <new description>\n\r", ch );
        return FALSE;
    }

    return TRUE;
}

MPEDIT( mpedit_edit )
{
    MPROG_DATA *pMudProg;

    EDIT_MPROG( ch, pMudProg );

    string_append( ch, &pMudProg->comlist );
    return TRUE;
}

/*
 * List all the groups a MudProg belongs to. - Zane
 */
MPEDIT( mpedit_groups )
{
    MPROG_DATA *pMudProg;
    MPROG_LIST *pList;
    MPROG_GROUP *pMprogGroup;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    *buf = '\0';

    EDIT_MPROG( ch, pMudProg );

    /* If it's new then just return */
    if ( pMudProg->vnum == 0 )
    {
        send_to_char( "This program does not belong to any groups.\n\r", ch );
        return TRUE;
    }

    for ( pMprogGroup = mprog_group_first; pMprogGroup;
          pMprogGroup = pMprogGroup->next )
        if ( pMprogGroup->prog_type == pMudProg->prog_type )
            for ( pList = pMprogGroup->mudprogs; pList; pList = pList->next )
                if ( pList->mudprog->vnum == pMudProg->vnum )
                {
                    sprintf( buf2, "[%4d] %s\n\r",
                             pMprogGroup->vnum, pMprogGroup->name );

                    if ( strlen( buf ) + strlen( buf2 ) >= sizeof( buf ) )
                    {
                        page_to_char( buf, ch );
                        *buf = '\0';
                    }
                    strcat( buf, buf2 );
                }

    if ( *buf == '\0' )
        send_to_char( "This program does not belong to any groups.\n\r", ch );
    else
        page_to_char( buf, ch );

    return TRUE;
}

MPEDIT( mpedit_name )
{
    MPROG_DATA *pMudProg;
    char colorbuf[MAX_STRING_LENGTH];

    EDIT_MPROG( ch, pMudProg );

    if ( str_len( argument ) > 20 )
    {
        send_to_char( "MudProg name cannot be longer than 20 characters.\n\r",
                      ch );
        return FALSE;
    }

    if ( *argument != '\0' )
    {
        if ( *pMudProg->name != '\0' )
            free_string( &pMudProg->name );

        do_color( argument, strlen( argument ), colorbuf, sizeof( colorbuf ),
                  FALSE );
        pMudProg->name = str_dup( colorbuf );
        send_to_char( "MudProg name set.\n\r", ch );
    }
    else
        send_to_char( "Syntax:  Name <new name>\n\r", ch );

    return TRUE;
}

MPEDIT( mpedit_progtype )
{
    MPROG_DATA *pMudProg;
    MPROG_LIST *pList;
    MPROG_GROUP *pMprogGroup;
    MOB_INDEX_DATA *pMob;
    ROOM_INDEX_DATA *pRoom;
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_STRING_LENGTH];
    int nMatch;
    int vnum;

    EDIT_MPROG( ch, pMudProg );
    argument = one_argument( argument, arg );

    if ( !str_prefix( arg, "mobprog" ) || !str_prefix( arg, "roomprog" )
         || !str_prefix( arg, "objprog" ) )
    {
        /*
         * Check to see if the prog is in use anywhere (or in any groups).
         * If it is, don't allow the type to be changed. -Zane
         */
        if ( pMudProg->vnum )
        {
            switch ( pMudProg->prog_type )
            {
            case MOB_PROG:
                nMatch = 0;
                for ( vnum = 0; nMatch < top_mob_index; vnum++ )
                    if ( ( pMob = get_mob_index( vnum ) ) != NULL )
                    {
                        if ( IS_SET( pMob->progtypes, pMudProg->trigger_type ) )
                            for ( pList = pMob->mudprogs; pList;
                                  pList = pList->next )
                                if ( pList->mudprog->vnum == pMudProg->vnum )
                                {
                                    send_to_char
                                        ( "This prog is in use, you cannot change it's prog type.\n\r",
                                          ch );
                                    return FALSE;
                                }
                        nMatch++;
                    }
                break;
            case ROOM_PROG:
                nMatch = 0;
                for ( vnum = 0; nMatch < top_room; vnum++ )
                    if ( ( pRoom = get_room_index( vnum ) ) != NULL )
                    {
                        if ( IS_SET
                             ( pRoom->progtypes, pMudProg->trigger_type ) )
                            for ( pList = pRoom->mudprogs; pList;
                                  pList = pList->next )
                                if ( pList->mudprog->vnum == pMudProg->vnum )
                                {
                                    send_to_char
                                        ( "This prog is in use, you cannot change it's prog type.\n\r",
                                          ch );
                                    return FALSE;
                                }
                        nMatch++;
                    }
                break;
            case OBJ_PROG:
                nMatch = 0;
                for ( vnum = 0; nMatch < top_obj_index; vnum++ )
                    if ( ( pObj = get_obj_index( vnum ) ) != NULL )
                    {
                        if ( IS_SET( pObj->progtypes, pMudProg->trigger_type ) )
                            for ( pList = pObj->mudprogs; pList;
                                  pList = pList->next )
                                if ( pList->mudprog->vnum == pMudProg->vnum )
                                {
                                    send_to_char
                                        ( "This prog is in use, you cannot change it's prog type.\n\r",
                                          ch );
                                    return FALSE;
                                }
                        nMatch++;
                    }
                break;
            }

            /* Check groups */
            for ( pMprogGroup = mprog_group_first; pMprogGroup;
                  pMprogGroup = pMprogGroup->next )
                if ( pMprogGroup->prog_type == pMudProg->prog_type )
                    for ( pList = pMprogGroup->mudprogs; pList;
                          pList = pList->next )
                        if ( pList->mudprog->vnum == pMudProg->vnum )
                        {
                            send_to_char
                                ( "This prog belongs to a group, it's type cannot be changed.\n\r",
                                  ch );
                            return FALSE;
                        }
        }

        /* We passed the checks, assign the type */
        if ( !str_prefix( arg, "mobprog" ) )
            pMudProg->prog_type = MOB_PROG;

        if ( !str_prefix( arg, "objprog" ) )
            pMudProg->prog_type = OBJ_PROG;

        if ( !str_prefix( arg, "roomprog" ) )
            pMudProg->prog_type = ROOM_PROG;

        send_to_char( "MudProg type set.\n\r", ch );
        return TRUE;
    }
    else
    {
        send_to_char( "Syntax:  Progtype <mobprog||objprog||roomprog>\n\r",
                      ch );
        return FALSE;
    }

    return TRUE;
}

MPEDIT( mpedit_show )
{
    MPROG_DATA *pMudProg;

    EDIT_MPROG( ch, pMudProg );
    show_mprog( ch, pMudProg );

    return TRUE;
}

MPEDIT( mpedit_trigger )
{
    MPROG_DATA *pMudProg;
    char arg[MAX_STRING_LENGTH];

    argument = one_argument( argument, arg );
    EDIT_MPROG( ch, pMudProg );

    if ( *arg != '\0' )
    {
        if ( *pMudProg->arglist != '\0' )
            free_string( &pMudProg->arglist );

        pMudProg->arglist = str_dup( arg );
        send_to_char( "MudProg trigger set.\n\r", ch );
    }
    else
    {
        send_to_char( "Syntax:  Trigger <Trigger value>\n\r", ch );
        return FALSE;
    }

    return TRUE;
}

MPEDIT( mpedit_triggertype )
{
    MPROG_DATA *pMudProg;
    MOB_INDEX_DATA *pMob;
    ROOM_INDEX_DATA *pRoom;
    OBJ_INDEX_DATA *pObj;
    char arg[MAX_STRING_LENGTH];
    int value;
    int vnum;
    int nMatch;

    argument = one_argument( argument, arg );

    if ( *arg == '\0' )
    {
        send_to_char( "Syntax:  triggertype <trigger name> [arguments]\n\r",
                      ch );
        return FALSE;
    }

    /* Set mudprog type */
    if ( ( value = flag_value( mprog_type_flags, arg ) ) != NO_FLAG )
    {
        EDIT_MPROG( ch, pMudProg );

        /*
         * If this isn't a new MudProg then search all mobs/objs/rooms to see
         * if this prog is in use and if it is change their progtypes to reflect
         * the new prog type.  Clunky but shouldn't happen too often. -Zane
         */
        if ( pMudProg->vnum )
        {
            switch ( pMudProg->prog_type )
            {
            case MOB_PROG:
                nMatch = 0;
                for ( vnum = 0; nMatch < top_mob_index; vnum++ )
                    if ( ( pMob = get_mob_index( vnum ) ) != NULL )
                    {
                        if ( IS_SET( pMob->progtypes, pMudProg->trigger_type ) )
                        {
                            REMOVE_BIT( pMob->progtypes,
                                        pMudProg->trigger_type );
                            SET_BIT( pMob->progtypes, value );
                        }
                        nMatch++;
                    }
                break;
            case ROOM_PROG:
                nMatch = 0;
                for ( vnum = 0; nMatch < top_room; vnum++ )
                    if ( ( pRoom = get_room_index( vnum ) ) != NULL )
                    {
                        if ( IS_SET
                             ( pRoom->progtypes, pMudProg->trigger_type ) )
                        {
                            REMOVE_BIT( pRoom->progtypes,
                                        pMudProg->trigger_type );
                            SET_BIT( pRoom->progtypes, value );
                        }
                        nMatch++;
                    }
                break;
            case OBJ_PROG:
                nMatch = 0;
                for ( vnum = 0; nMatch < top_obj_index; vnum++ )
                    if ( ( pObj = get_obj_index( vnum ) ) != NULL )
                    {
                        if ( IS_SET( pObj->progtypes, pMudProg->trigger_type ) )
                        {
                            REMOVE_BIT( pObj->progtypes,
                                        pMudProg->trigger_type );
                            SET_BIT( pObj->progtypes, value );
                        }
                        nMatch++;
                    }
                break;
            }
        }
        pMudProg->trigger_type = value;

        argument = one_argument( argument, arg );

        if ( *arg != '\0' )
        {
            if ( *pMudProg->arglist != '\0' )
                free_string( &pMudProg->arglist );

            pMudProg->arglist = str_dup( arg );
        }

        send_to_char( "MudProg trigger type set.\n\r", ch );
    }
    else
    {
        send_to_char( "Unknown trigger type.\n\r", ch );
        return FALSE;
    }

    return TRUE;
}

MPEDIT( mpedit_assign )
{
    MPROG_DATA *pMudProg;
    MPROG_GROUP *pMprogGroup;
    MOB_INDEX_DATA *pMob;
    OBJ_INDEX_DATA *pObj;
    ROOM_INDEX_DATA *pRoom;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    int targetvnum;
    int progvnum;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( *arg1 == '\0' || *arg2 == '\0' || !is_number( arg1 )
         || !is_number( arg2 ) )
    {
        send_to_char
            ( "Syntax:  edit mprog assign <target vnum> <mudprog vnum>\n\r",
              ch );
        return FALSE;
    }

    targetvnum = atoi( arg1 );
    progvnum = atoi( arg2 );

    if ( ( pMudProg = get_mprog_by_vnum( progvnum ) ) != NULL )
        switch ( pMudProg->prog_type )
        {
        case MOB_PROG:
            if ( ( pMob = get_mob_index( targetvnum ) ) == NULL )
            {
                send_to_char
                    ( "The MudProg you specified is a MobProg and no mob matches the vnum you supplied.\n\r",
                      ch );
                return FALSE;
            }

            assign_mobprog( pMob, pMudProg, NULL );
            if ( pMob->area )
                SET_BIT( pMob->area->area_flags, AREA_CHANGED );

            break;
        case OBJ_PROG:
            if ( ( pObj = get_obj_index( targetvnum ) ) == NULL )
            {
                send_to_char
                    ( "The MudProg you specified is a ObjProg and no object matches the vnum you supplied.\n\r",
                      ch );
                return FALSE;
            }

            assign_objprog( pObj, pMudProg, NULL );
            if ( pObj->area )
                SET_BIT( pObj->area->area_flags, AREA_CHANGED );

            break;
        case ROOM_PROG:
            if ( ( pRoom = get_room_index( targetvnum ) ) == NULL )
            {
                send_to_char
                    ( "The MudProg you specified is a RoomProg and no room matches the vnum you supplied.\n\r",
                      ch );
                return FALSE;
            }

            assign_roomprog( pRoom, pMudProg, NULL );
            if ( pRoom->area )
                SET_BIT( pRoom->area->area_flags, AREA_CHANGED );

            break;
        }
    else if ( ( pMprogGroup = get_mprog_group_by_vnum( progvnum ) ) != NULL )
        switch ( pMprogGroup->prog_type )
        {
        case MOB_PROG:
            if ( ( pMob = get_mob_index( targetvnum ) ) == NULL )
            {
                send_to_char
                    ( "The MudProg you specified is a MobProg and no mob matches the vnum you supplied.\n\r",
                      ch );
                return FALSE;
            }

            assign_mobprog( pMob, NULL, pMprogGroup );
            if ( pMob->area )
                SET_BIT( pMob->area->area_flags, AREA_CHANGED );

            break;
        case OBJ_PROG:
            if ( ( pObj = get_obj_index( targetvnum ) ) == NULL )
            {
                send_to_char
                    ( "The MudProg you specified is a ObjProg and no object matches the vnum you supplied.\n\r",
                      ch );
                return FALSE;
            }

            assign_objprog( pObj, NULL, pMprogGroup );
            if ( pObj->area )
                SET_BIT( pObj->area->area_flags, AREA_CHANGED );

            break;
        case ROOM_PROG:
            if ( ( pRoom = get_room_index( targetvnum ) ) == NULL )
            {
                send_to_char
                    ( "The MudProg you specified is a RoomProg and no room matches the vnum you supplied.\n\r",
                      ch );
                return FALSE;
            }

            assign_roomprog( pRoom, NULL, pMprogGroup );
            if ( pRoom->area )
                SET_BIT( pRoom->area->area_flags, AREA_CHANGED );

            break;
        }
    else
    {
        sprintf( arg1, "Invalid MudProg vnum [%4d].\n\r", progvnum );
        send_to_char( arg1, ch );
        return FALSE;
    }

    send_to_char( "MudProg assigned.\n\r", ch );
    return TRUE;
}

MPEDIT( mpedit_unassign )
{
    MPROG_DATA *pMudProg;
    MPROG_GROUP *pMprogGroup;
    MOB_INDEX_DATA *pMob;
    OBJ_INDEX_DATA *pObj;
    ROOM_INDEX_DATA *pRoom;
    char arg1[MAX_STRING_LENGTH];
    char arg2[MAX_STRING_LENGTH];
    int targetvnum;
    int progvnum;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( *arg1 == '\0' || *arg2 == '\0' || !is_number( arg1 )
         || !is_number( arg2 ) )
    {
        send_to_char
            ( "Syntax:  edit mprog unassign <target vnum> <mudprog vnum>\n\r",
              ch );
        return FALSE;
    }

    targetvnum = atoi( arg1 );
    progvnum = atoi( arg2 );

    if ( ( pMudProg = get_mprog_by_vnum( progvnum ) ) != NULL )
        switch ( pMudProg->prog_type )
        {
        case MOB_PROG:
            if ( ( pMob = get_mob_index( targetvnum ) ) == NULL )
            {
                send_to_char
                    ( "No matching MudProg is assigned to this mob.\n\r", ch );
                return FALSE;
            }

            if ( !unassign_mobprog( pMob, pMudProg, NULL ) )
            {
                send_to_char
                    ( "MudProg is part of a group assigned to that mob, please remove the group"
                      " or remove the Prog from that group.\n\r", ch );
                return FALSE;
            }

            if ( pMob->area )
                SET_BIT( pMob->area->area_flags, AREA_CHANGED );

            break;
        case OBJ_PROG:
            if ( ( pObj = get_obj_index( targetvnum ) ) == NULL )
            {
                send_to_char
                    ( "No matching MudProg is assigned to this object.\n\r",
                      ch );
                return FALSE;
            }

            if ( !unassign_objprog( pObj, pMudProg, NULL ) )
            {
                send_to_char
                    ( "MudProg is part of a group assigned to that mob, please remove the group"
                      " or remove the Prog from that group.\n\r", ch );
                return FALSE;
            }

            if ( pObj->area )
                SET_BIT( pObj->area->area_flags, AREA_CHANGED );

            break;
        case ROOM_PROG:
            if ( ( pRoom = get_room_index( targetvnum ) ) == NULL )
            {
                send_to_char
                    ( "No matching MudProg is assigned to this room.\n\r", ch );
                return FALSE;
            }

            if ( !unassign_roomprog( pRoom, pMudProg, NULL ) )
            {
                send_to_char
                    ( "MudProg is part of a group assigned to that mob, please remove the group"
                      " or remove the Prog from that group.\n\r", ch );
                return FALSE;
            }

            if ( pRoom->area )
                SET_BIT( pRoom->area->area_flags, AREA_CHANGED );

            break;
        }
    else if ( ( pMprogGroup = get_mprog_group_by_vnum( progvnum ) ) != NULL )
        switch ( pMprogGroup->prog_type )
        {
        case MOB_PROG:
            if ( ( pMob = get_mob_index( targetvnum ) ) == NULL )
            {
                send_to_char
                    ( "No matching MudProg is assigned to this mob.\n\r", ch );
                return FALSE;
            }

            unassign_mobprog( pMob, NULL, pMprogGroup );
            if ( pMob->area )
                SET_BIT( pMob->area->area_flags, AREA_CHANGED );

            break;
        case OBJ_PROG:
            if ( ( pObj = get_obj_index( targetvnum ) ) == NULL )
            {
                send_to_char
                    ( "No matching MudProg is assigned to this object.\n\r",
                      ch );
                return FALSE;
            }

            unassign_objprog( pObj, NULL, pMprogGroup );
            if ( pObj->area )
                SET_BIT( pObj->area->area_flags, AREA_CHANGED );

            break;
        case ROOM_PROG:
            if ( ( pRoom = get_room_index( targetvnum ) ) == NULL )
            {
                send_to_char
                    ( "No matching MudProg is assigned to this room.\n\r", ch );
                return FALSE;
            }

            unassign_roomprog( pRoom, NULL, pMprogGroup );
            if ( pRoom->area )
                SET_BIT( pRoom->area->area_flags, AREA_CHANGED );

            break;
        }
    else
    {
        sprintf( arg1, "Invalid MudProg vnum [%4d].\n\r", progvnum );
        send_to_char( arg1, ch );
        return FALSE;
    }

    send_to_char( "MudProg unassigned.\n\r", ch );
    return TRUE;
}

void show_mprog( CHAR_DATA * ch, MPROG_DATA * pMudProg )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if ( !pMudProg )
        return;

    switch ( pMudProg->prog_type )
    {
    case MOB_PROG:
        sprintf( buf2, "MobProg" );
        break;
    case OBJ_PROG:
        sprintf( buf2, "ObjProg" );
        break;
    case ROOM_PROG:
        sprintf( buf2, "RoomProg" );
        break;
    }
    sprintf( buf, "[%4d] %-20s   -   ProgType: %s\n\rDesc: %s\n\r>%s %s~\n\r",
             pMudProg->vnum, pMudProg->name, buf2,
             pMudProg->description,
             mprog_type_to_name( pMudProg->trigger_type ),
             pMudProg->arglist ? pMudProg->arglist : "NULL" );

    sprintf( buf2, "%s~\n\r",
             pMudProg->comlist ? pMudProg->comlist : "NULL\n\r" );

    if ( strlen( buf ) + strlen( buf2 ) >= sizeof( buf ) )
    {
        page_to_char( buf, ch );
        page_to_char( buf2, ch );
        return;
    }
    else
    {
        strcat( buf, buf2 );
        page_to_char( buf, ch );
        return;
    }
}

MPGEDIT( mpgedit_add )
{
    MPROG_GROUP *pGroup;
    MPROG_DATA *pProg;
    MPROG_GROUP_LIST *pGroupList;
    MPROG_LIST *pList;
    MOB_INDEX_DATA *pMob;
    OBJ_INDEX_DATA *pObj;
    ROOM_INDEX_DATA *pRoom;
    int progvnum;
    int vnum;
    int nMatch;
    int iFound;
    bool updated;

    EDIT_MPGROUP( ch, pGroup );
    updated = FALSE;
    progvnum = 0;

    if ( *argument == '\0' || ( ( progvnum = atoi( argument ) ) == 0 ) )
    {
        send_to_char( "Syntax:  add <prog vnum>\n\r", ch );
        return FALSE;
    }

    if ( ( pProg = get_mprog_by_vnum( progvnum ) ) == NULL )
    {
        send_to_char( "Invalid vnum.\n\r", ch );
        return FALSE;
    }

    /* Check to make sure it's not already in the group */
    for ( pList = pGroup->mudprogs; pList; pList = pList->next )
        if ( pList->mudprog->vnum == progvnum )
        {
            send_to_char( "That MudProg is already in this group.\n\r", ch );
            return FALSE;
        }

    if ( pProg->prog_type != pGroup->prog_type && pGroup->mudprogs )
    {
        send_to_char
            ( "Unable to add MudProg, MudProg Group contains Progs of a different type.\n\r",
              ch );
        return FALSE;
    }
    else
        pGroup->prog_type = pProg->prog_type;

    /*
     * Add the prog to this group.
     */
    pList = alloc_perm( sizeof( *pList ) );
    pList->mudprog = pProg;
    pList->next = pGroup->mudprogs;
    pGroup->mudprogs = pList;

    /*
     * Search through all mobs/rooms/objs and add this prog to their list if they
     * have this group.
     */
    switch ( pGroup->prog_type )
    {
    case MOB_PROG:
        nMatch = 0;
        for ( vnum = 0; nMatch < top_mob_index; vnum++ )
            if ( ( pMob = get_mob_index( vnum ) ) != NULL )
            {
                iFound = 0;

                for ( pGroupList = pMob->mprog_groups; pGroupList;
                      pGroupList = pGroupList->next )
                    if ( pGroupList->mprog_group->vnum == pGroup->vnum )
                    {
                        iFound = 0;

                        for ( pList = pMob->mudprogs; pList;
                              pList = pList->next )
                            if ( pList->mudprog->vnum == pProg->vnum )
                                iFound = 1;

                        if ( !iFound )
                        {
                            updated = TRUE;
                            pList = alloc_perm( sizeof( *pList ) );
                            pList->mudprog = pProg;
                            pList->next = pMob->mudprogs;
                            pMob->mudprogs = pList;
                            SET_BIT( pMob->progtypes, pProg->trigger_type );
                        }

                        break;
                    }

                nMatch++;
            }
        break;
    case ROOM_PROG:
        nMatch = 0;
        for ( vnum = 0; nMatch < top_room; vnum++ )
            if ( ( pRoom = get_room_index( vnum ) ) != NULL )
            {
                for ( pGroupList = pRoom->mprog_groups; pGroupList;
                      pGroupList = pGroupList->next )
                    if ( pGroupList->mprog_group->vnum == pGroup->vnum )
                    {
                        iFound = 0;

                        for ( pList = pRoom->mudprogs; pList;
                              pList = pList->next )
                            if ( pList->mudprog->vnum == pProg->vnum )
                                iFound = 1;

                        if ( !iFound )
                        {
                            updated = TRUE;
                            pList = alloc_perm( sizeof( *pList ) );
                            pList->mudprog = pProg;
                            pList->next = pRoom->mudprogs;
                            pRoom->mudprogs = pList;
                            SET_BIT( pRoom->progtypes, pProg->trigger_type );
                        }

                        break;
                    }

                nMatch++;
            }
        break;
    case OBJ_PROG:
        nMatch = 0;
        for ( vnum = 0; nMatch < top_obj_index; vnum++ )
            if ( ( pObj = get_obj_index( vnum ) ) != NULL )
            {
                for ( pGroupList = pObj->mprog_groups; pGroupList;
                      pGroupList = pGroupList->next )
                    if ( pGroupList->mprog_group->vnum == pGroup->vnum )
                    {
                        iFound = 0;

                        for ( pList = pObj->mudprogs; pList;
                              pList = pList->next )
                            if ( pList->mudprog->vnum == pProg->vnum )
                                iFound = 1;

                        if ( !iFound )
                        {
                            updated = TRUE;
                            pList = alloc_perm( sizeof( *pList ) );
                            pList->mudprog = pProg;
                            pList->next = pObj->mudprogs;
                            pObj->mudprogs = pList;
                            SET_BIT( pObj->progtypes, pProg->trigger_type );
                        }

                        break;
                    }

                nMatch++;
            }
        break;
    }

    if ( updated )
        send_to_char
            ( "Prog added to group and existing obj/mob/rooms updated.\n\r",
              ch );
    else
        send_to_char( "Prog added to group.\n\r", ch );

    return TRUE;
}

MPGEDIT( mpgedit_create )
{
    MPROG_GROUP *pMprogGroup;

    if ( ch->desc->pEdit )
    {
        send_to_char( "You're already editing.\n\r", ch );
        return FALSE;
    }

    if ( *argument == '\0' && !str_prefix( argument, "mobprog" )
         && !str_prefix( argument, "objprog" )
         && !str_prefix( argument, "roomprog" ) )
    {
        send_to_char
            ( "Syntax:  edit mpgroup create [mobprog||objprog||roomprog]\n\r",
              ch );
        return FALSE;
    }

    pMprogGroup = new_mudprog_group(  );

    if ( !str_prefix( argument, "mobprog" ) )
        pMprogGroup->prog_type = MOB_PROG;

    if ( !str_prefix( argument, "objprog" ) )
        pMprogGroup->prog_type = OBJ_PROG;

    if ( !str_prefix( argument, "roomprog" ) )
        pMprogGroup->prog_type = ROOM_PROG;

    ch->desc->pEdit = ( void * ) pMprogGroup;

    send_to_char( "MudProg Group Created.\n\r", ch );
    return TRUE;
}

MPGEDIT( mpgedit_delete )
{
    MPROG_GROUP *pTempGroup;
    MPROG_GROUP *pMprogGroup;
    MPROG_GROUP_LIST *pList;
    OBJ_INDEX_DATA *pObj;
    MOB_INDEX_DATA *pMob;
    ROOM_INDEX_DATA *pRoom;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int vnum;
    int nMatch;

    argument = one_argument( argument, arg );

    if ( *arg != '\0' )
    {
        if ( ( vnum = atoi( arg ) ) < 1 )
        {
            send_to_char( "Syntax:  Delete [MudProg Group Vnum]\n\r", ch );
            return FALSE;
        }

        if ( ( pMprogGroup = get_mprog_group_by_vnum( vnum ) ) == NULL )
        {
            send_to_char( "Invalid vnum.\n\r", ch );
            return FALSE;
        }
    }
    else
        EDIT_MPGROUP( ch, pMprogGroup );

    /*
     * This mudprog group is new and hasn't been added to the list, just free it.
     */
    if ( pMprogGroup->vnum == 0 )
    {
        sprintf( buf, "MudProg Group [%4d] %s - deleted.\n\r",
                 pMprogGroup->vnum, pMprogGroup->name );
        free_mudprog_group( pMprogGroup );
        send_to_char( buf, ch );
        edit_done( ch );
        return TRUE;
    }

    /*
     * Check to see if this mudprog group is in use.  If it is
     * then don't allow the deletion.  -Zane
     */
    switch ( pMprogGroup->prog_type )
    {
    case MOB_PROG:
        nMatch = 0;
        for ( vnum = 0; nMatch < top_mob_index; vnum++ )
            if ( ( pMob = get_mob_index( vnum ) ) != NULL )
            {
                for ( pList = pMob->mprog_groups; pList; pList = pList->next )
                    if ( pList->mprog_group->vnum == pMprogGroup->vnum )
                    {
                        send_to_char
                            ( "This group is in use, you cannot delete it.\n\r",
                              ch );
                        return FALSE;
                    }
                nMatch++;
            }
        break;
    case ROOM_PROG:
        nMatch = 0;
        for ( vnum = 0; nMatch < top_room; vnum++ )
            if ( ( pRoom = get_room_index( vnum ) ) != NULL )
            {
                for ( pList = pRoom->mprog_groups; pList; pList = pList->next )
                    if ( pList->mprog_group->vnum == pMprogGroup->vnum )
                    {
                        send_to_char
                            ( "This group is in use, you cannot delete it.\n\r",
                              ch );
                        return FALSE;
                    }
                nMatch++;
            }
        break;
    case OBJ_PROG:
        nMatch = 0;
        for ( vnum = 0; nMatch < top_obj_index; vnum++ )
            if ( ( pObj = get_obj_index( vnum ) ) != NULL )
            {
                for ( pList = pObj->mprog_groups; pList; pList = pList->next )
                    if ( pList->mprog_group->vnum == pMprogGroup->vnum )
                    {
                        send_to_char
                            ( "This prog is in use, you cannot delete it.\n\r",
                              ch );
                        return FALSE;
                    }
                nMatch++;
            }
        break;
    }

    sprintf( buf, "MudProg Group [%4d] %s - deleted.\n\r", pMprogGroup->vnum,
             pMprogGroup->name );

    /* Remove mudprog from the list if it's in there */
    if ( mprog_group_first == pMprogGroup )
        mprog_group_first = pMprogGroup->next;
    else
    {
        for ( pTempGroup = mprog_group_first; pTempGroup->next != pMprogGroup;
              pTempGroup = pTempGroup->next )
            ;

        pTempGroup->next = pMprogGroup->next;
    }

    if ( mprog_group_last == pMprogGroup )
    {
        for ( pTempGroup = mprog_group_first; !pTempGroup->next;
              pTempGroup = pTempGroup->next )
            ;

        mprog_group_last = pTempGroup;
    }

    free_mudprog_group( pMprogGroup );
    send_to_char( buf, ch );
    edit_done( ch );
    return TRUE;
}

MPGEDIT( mpgedit_description )
{
    MPROG_GROUP *pMprogGroup;
    char colorbuf[MAX_STRING_LENGTH];

    EDIT_MPGROUP( ch, pMprogGroup );

    if ( *argument != '\0' )
    {
        if ( *pMprogGroup->description != '\0' )
            free_string( &pMprogGroup->description );

        do_color( argument, strlen( argument ), colorbuf, sizeof( colorbuf ),
                  FALSE );
        pMprogGroup->description = str_dup( colorbuf );
        send_to_char( "MudProg Group description set.\n\r", ch );
    }
    else
    {
        send_to_char( "Syntax:  Description <new description>\n\r", ch );
        return FALSE;
    }

    return TRUE;
}

MPGEDIT( mpgedit_name )
{
    MPROG_GROUP *pMprogGroup;
    char colorbuf[MAX_STRING_LENGTH];

    EDIT_MPGROUP( ch, pMprogGroup );

    if ( str_len( argument ) > 20 )
    {
        send_to_char
            ( "MudProg Group name cannot be longer than 20 characters.\n\r",
              ch );
        return FALSE;
    }

    if ( *argument != '\0' )
    {
        if ( *pMprogGroup->name != '\0' )
            free_string( &pMprogGroup->name );

        do_color( argument, strlen( argument ), colorbuf, sizeof( colorbuf ),
                  FALSE );
        pMprogGroup->name = str_dup( colorbuf );
        send_to_char( "MudProg Group name set.\n\r", ch );
    }
    else
        send_to_char( "Syntax:  Name <new name>\n\r", ch );

    return TRUE;
}

MPGEDIT( mpgedit_remove )
{
    MPROG_GROUP *pGroup;
    MPROG_GROUP_LIST *pGroupList;
    MPROG_LIST *pList;
    MPROG_LIST *pInnerList;
    MOB_INDEX_DATA *pMob;
    OBJ_INDEX_DATA *pObj;
    ROOM_INDEX_DATA *pRoom;
    int progvnum;
    int vnum;
    int nMatch;
    int iFound;
    bool updated;

    EDIT_MPGROUP( ch, pGroup );
    updated = FALSE;
    progvnum = 0;

    if ( *argument == '\0' || ( ( progvnum = atoi( argument ) ) == 0 ) )
    {
        send_to_char( "Syntax:  remove <prog vnum>\n\r", ch );
        return FALSE;
    }

    /* Remove the MudProg from the Group if it's in there. */
    for ( pList = pGroup->mudprogs; pList; pList = pList->next )
        if ( pList->mudprog->vnum == progvnum )
        {
            if ( pList == pGroup->mudprogs )
                pGroup->mudprogs = pList->next;
            else
            {
                for ( pInnerList = pGroup->mudprogs; pInnerList->next != pList;
                      pInnerList = pInnerList->next )
                    ;

                pInnerList->next = pList->next;
            }

            free_mem( &pList );
            updated = TRUE;
            break;
        }

    /* If no matching prog was found throw an error */
    if ( !updated )
    {
        send_to_char( "That MudProg does not exist in this group.\n\r", ch );
        return FALSE;
    }
    else
        updated = FALSE;

    /*
     * Search through all mobs/rooms/objs and remove this prog form their list if they
     * have this group.
     */
    switch ( pGroup->prog_type )
    {
    case MOB_PROG:
        nMatch = 0;
        for ( vnum = 0; nMatch < top_mob_index; vnum++ )
            if ( ( pMob = get_mob_index( vnum ) ) != NULL )
            {
                iFound = 0;

                for ( pGroupList = pMob->mprog_groups; pGroupList;
                      pGroupList = pGroupList->next )
                    if ( pGroupList->mprog_group->vnum == pGroup->vnum )
                        iFound = 1;

                /*
                 * We found the group, count how many groups use this MudProg.
                 */
                if ( iFound )
                {
                    iFound = 0;

                    for ( pGroupList = pMob->mprog_groups; pGroupList;
                          pGroupList = pGroupList->next )
                        for ( pList = pGroupList->mprog_group->mudprogs; pList;
                              pList = pList->next )
                            if ( pList->mudprog->vnum == progvnum )
                                iFound++;

                    /* No references to that prog were found, remove it. */
                    if ( iFound < 1 )
                    {
                        for ( pList = pMob->mudprogs;
                              pList && pList->mudprog->vnum != progvnum;
                              pList = pList->next )
                            ;

                        if ( pList == pMob->mudprogs )
                            pMob->mudprogs = pList->next;
                        else
                        {
                            for ( pInnerList = pMob->mudprogs;
                                  pInnerList->next != pList;
                                  pInnerList = pInnerList->next )
                                ;

                            pInnerList->next = pList->next;
                        }

                        free_mem( &pList );
                        updated = TRUE;
                    }
                }

                nMatch++;
            }
        break;
    case ROOM_PROG:
        nMatch = 0;
        for ( vnum = 0; nMatch < top_room; vnum++ )
            if ( ( pRoom = get_room_index( vnum ) ) != NULL )
            {
                iFound = 0;

                for ( pGroupList = pRoom->mprog_groups; pGroupList;
                      pGroupList = pGroupList->next )
                    if ( pGroupList->mprog_group->vnum == pGroup->vnum )
                        iFound = 1;

                /*
                 * We found the group, count how many groups use this MudProg.
                 */
                if ( iFound )
                {
                    iFound = 0;

                    for ( pGroupList = pRoom->mprog_groups; pGroupList;
                          pGroupList = pGroupList->next )
                        for ( pList = pGroupList->mprog_group->mudprogs; pList;
                              pList = pList->next )
                            if ( pList->mudprog->vnum == progvnum )
                                iFound++;

                    /* Only one reference to that prog was found, remove it. */
                    if ( iFound == 1 )
                    {
                        for ( pList = pRoom->mudprogs;
                              pList && pList->mudprog->vnum != progvnum;
                              pList = pList->next )
                            ;

                        if ( pList == pRoom->mudprogs )
                            pRoom->mudprogs = pList->next;
                        else
                        {
                            for ( pInnerList = pRoom->mudprogs;
                                  pInnerList->next != pList;
                                  pInnerList = pInnerList->next )
                                ;

                            pInnerList->next = pList->next;
                        }

                        free_mem( &pList );
                        updated = TRUE;
                    }
                }

                nMatch++;
            }
        break;
    case OBJ_PROG:
        nMatch = 0;
        for ( vnum = 0; nMatch < top_obj_index; vnum++ )
            if ( ( pObj = get_obj_index( vnum ) ) != NULL )
            {
                iFound = 0;

                for ( pGroupList = pObj->mprog_groups; pGroupList;
                      pGroupList = pGroupList->next )
                    if ( pGroupList->mprog_group->vnum == pGroup->vnum )
                        iFound = 1;

                /*
                 * We found the group, count how many groups use this MudProg.
                 */
                if ( iFound )
                {
                    iFound = 0;

                    for ( pGroupList = pObj->mprog_groups; pGroupList;
                          pGroupList = pGroupList->next )
                        for ( pList = pGroupList->mprog_group->mudprogs; pList;
                              pList = pList->next )
                            if ( pList->mudprog->vnum == progvnum )
                                iFound++;

                    /* Only one reference to that prog was found, remove it. */
                    if ( iFound == 1 )
                    {
                        for ( pList = pObj->mudprogs;
                              pList && pList->mudprog->vnum != progvnum;
                              pList = pList->next )
                            ;

                        if ( pList == pObj->mudprogs )
                            pObj->mudprogs = pList->next;
                        else
                        {
                            for ( pInnerList = pObj->mudprogs;
                                  pInnerList->next != pList;
                                  pInnerList = pInnerList->next )
                                ;

                            pInnerList->next = pList->next;
                        }

                        free_mem( &pList );
                        updated = TRUE;
                    }
                }

                nMatch++;
            }
        break;
    }

    if ( updated )
        send_to_char
            ( "Prog removed from group and existing obj/mob/rooms updated.\n\r",
              ch );
    else
        send_to_char( "Prog removed from group.\n\r", ch );

    return TRUE;
}

MPGEDIT( mpgedit_show )
{
    MPROG_GROUP *pGroup;

    EDIT_MPGROUP( ch, pGroup );
    show_mpgroup( ch, pGroup );

    return TRUE;
}

void show_mpgroup( CHAR_DATA * ch, MPROG_GROUP * pGroup )
{
    MPROG_LIST *pList;
    char buf[MAX_STRING_LENGTH];

    if ( !pGroup )
        return;

    *buf = '\0';

    sprintf( buf, "** [%4d] Group - %s\n\r** Desc: %s\n\r\n\r", pGroup->vnum,
             pGroup->name, pGroup->description );
    page_to_char( buf, ch );

    for ( pList = pGroup->mudprogs; pList; pList = pList->next )
        show_mprog( ch, pList->mudprog );

    sprintf( buf, "\n\r** [%4d] End of group.\n\r\n\r", pGroup->vnum );
    page_to_char( buf, ch );
}
