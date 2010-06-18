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

#if defined(WIN32)
#include <windows.h>
#endif
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

extern int _filbuf args( ( FILE * ) );
extern bool chaos;

/* This is called by ban.c -Lancelight */
char *print_flags( int flag )
{
    int count, pos = 0;
    static char buf[52];

    for ( count = 0; count < 32; count++ )
    {
        if ( IS_SET( flag, 1 << count ) )
        {
            if ( count < 26 )
                buf[pos] = 'A' + count;
            else
                buf[pos] = 'a' + ( count - 26 );
            pos++;
        }
    }

    if ( pos == 0 )
    {
        buf[pos] = '0';
        pos++;
    }

    buf[pos] = '\0';

    return buf;
}

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];
extern bool fCopyOver;

/*
 * Local functions.
 */
void fwrite_char args( ( CHAR_DATA * ch, FILE * fp ) );
void fwrite_obj args( ( CHAR_DATA * ch, OBJ_DATA * obj,
                        FILE * fp, int iNest ) );
void fwrite_pet args( ( CHAR_DATA * pet, FILE * fp ) );
void fread_char args( ( CHAR_DATA * ch, FILE * fp ) );
void fread_pet args( ( CHAR_DATA * ch, FILE * fp ) );
void fread_obj args( ( CHAR_DATA * ch, FILE * fp ) );
void fread_imm args( ( CHAR_DATA * ch, FILE * fp ) );

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA * ch )
{
    char strsave[MAX_INPUT_LENGTH];
    char TempFile[MAX_INPUT_LENGTH];
    FILE *fp;
/* #if !defined(WIN32)
    char buf[MAX_STRING_LENGTH];
#endif */

    /* No saving during CHAOS */
    if ( chaos )
        return;

    if ( IS_NPC( ch ) )
        return;
/* This was added for the Win95 version.*/

    sprintf( TempFile, "%s/%d", sysconfig.player_temp, rand(  ) );

    /* Return from switch before saving. */
    if ( ch->desc != NULL && ch->desc->original != NULL )
        ch = ch->desc->original;

    /* create god log */
    if ( IS_HERO( ch ) || ch->level >= LEVEL_HERO )
    {
        fclose( fpReserve );
        sprintf( strsave, "%s/%s", sysconfig.god_dir, capitalize( ch->name ) );
        if ( ( fp = fopen( strsave, "w" ) ) == NULL )
        {
            bug( "Save_char_obj: fopen", 0 );
            send_to_char( "Error saving!", ch );

#if defined(cbuilder)
            return -1;
#else
            perror( strsave );
            return;
#endif
        }

        fprintf( fp, "Lev %2d Trust %2d  %s%s\n",
                 ch->level, get_trust( ch ), ch->name, ch->pcdata->title );
        fclose( fp );
        fpReserve = fopen( NULL_FILE, "r" );
    }

    fclose( fpReserve );
    if ( ( fp = fopen( TempFile, "w" ) ) == NULL )
    {
        bug( "Save_char_obj: fopen", 0 );
        perror( TempFile );
    }
    else
    {
        IMMCMD_TYPE *tmpcmd;
        int c = 0;

        fwrite_char( ch, fp );

        /* Save the objects */
        if ( ch->carrying != NULL )
            fwrite_obj( ch, ch->carrying, fp, 0 );

        /* Save the pets */
        if ( ch->pet != NULL && ch->pet->in_room == ch->in_room )
            fwrite_pet( ch->pet, fp );

        /* Save the imm commands */
        fprintf( fp, "#IMM\n" );

        for ( tmpcmd = ch->pcdata->immcmdlist; tmpcmd != NULL;
              tmpcmd = tmpcmd->next )
        {
            c++;
            fprintf( fp, "%s ", tmpcmd->cmd );
            if ( c == 5 )
            {
                c = 0;
                fprintf( fp, "\n" );
            }
        }
        fprintf( fp, "End\n" );

        /* Save the factions */
        if ( ch->pcdata->faction_standings != NULL )
        {
            fwrite_faction_standings( ch, fp );
        }

        fprintf( fp, "#END\n" );
    }

    if ( fp != NULL )
    {
        sprintf( strsave, "%s/%s", sysconfig.player_dir,
                 capitalize( ch->name ) );
        fclose( fp );
        /* move the file */
#if defined(WIN32)
        if ( !CopyFile( TempFile, strsave, 0 ) )
            perror( "Save: CopyFile" );
        if ( !DeleteFile( TempFile ) )
            perror( "Save: DeleteFile" );
#else
        rename( TempFile, strsave );
/* sprintf( buf, "mv %s %s", TempFile, strsave );
        system( buf ); */
#endif
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA * ch, FILE * fp )
{
    AFFECT_DATA *paf;
    NEWAFFECT_DATA *npaf;
    int sn, gn, pos, i;

    fprintf( fp, "#%s\n", IS_NPC( ch ) ? "MOB" : "PLAYER" );

    fprintf( fp, "Name %s~\n", ch->name );
    fprintf( fp, "Vers %d\n", 4 );
    if ( ch->short_descr[0] != '\0' )
        fprintf( fp, "ShD  %s~\n", ch->short_descr );
    if ( ch->long_descr[0] != '\0' )
        fprintf( fp, "LnD  %s~\n", ch->long_descr );
    if ( ch->description[0] != '\0' )
        fprintf( fp, "Desc %s~\n\r", ch->description );
    fprintf( fp, "Race %s~\n", pc_race_table[ch->race].name );
    fprintf( fp, "Sex  %d\n", ch->sex );
    fprintf( fp, "Cla  %d\n", ch->Class );
    fprintf( fp, "Beep %d\n", ch->beep );
    fprintf( fp, "Anon %d\n", ch->anonymous );
    fprintf( fp, "Levl %d\n", ch->level );
    if ( ch->trust != 0 )
        fprintf( fp, "Tru  %d\n", ch->trust );
    fprintf( fp, "Sec  %d\n", ch->pcdata->security );   /* OLC */
    fprintf( fp, "Logn %d\n", ( int ) ( ch->logon ) );  /* Added for finger command */
    fprintf( fp, "Plyd %d\n",
             ch->played + ( int ) ( current_time - ch->logon ) );
    fprintf( fp, "Note %d\n", ( int ) ch->last_note );
    fprintf( fp, "Scro %d\n", ch->lines );
    fprintf( fp, "Room %d\n",
             ( ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
               && ch->was_in_room != NULL )
             ? ch->was_in_room->vnum
             : ch->in_room == NULL ? 3001 : ch->in_room->vnum );

    fprintf( fp, "HMV  %d %d %d %d %d %d\n",
             ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
             ch->max_move );
    if ( ch->gold > 0 )
        fprintf( fp, "Gold %ld\n", ch->gold );
    else
        fprintf( fp, "Gold %d\n", 0 );
    if ( ch->pcdata->gold_bank > 0 )
        fprintf( fp, "Gold_bank %ld\n", ch->pcdata->gold_bank );
    else
        fprintf( fp, "Gold_bank %d\n", 0 );
    fprintf( fp, "Exp  %ld\n", ch->exp );
    if ( ch->act != 0 )
        fprintf( fp, "Act  %ld\n", ch->act );
    if ( ch->affected_by != 0 )
        fprintf( fp, "AfBy %ld\n", ch->affected_by );
    if ( ch->newaff[0] != 0 )
        fprintf( fp, "NewAfBy %s\n", ch->newaff );
    fprintf( fp, "Comm %ld\n", ch->comm );
    if ( ch->incarnations != 0 )
        fprintf( fp, "Inca %d\n", ch->incarnations );
    if ( ch->invis_level != 0 )
        fprintf( fp, "Invi %d\n", ch->invis_level );
    fprintf( fp, "Pos  %d\n",
             ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
    if ( ch->practice != 0 )
        fprintf( fp, "Prac %d\n", ch->practice );
    if ( ch->train != 0 )
        fprintf( fp, "Trai %d\n", ch->train );
    if ( ch->qp != 0 )
        fprintf( fp, "Qp %d\n", ch->qp );
    if ( ch->saving_throw != 0 )
        fprintf( fp, "Save  %d\n", ch->saving_throw );
    fprintf( fp, "Alig  %d\n", ch->alignment );
    if ( ch->hitroll != 0 )
        fprintf( fp, "Hit   %d\n", ch->hitroll );
    if ( ch->damroll != 0 )
        fprintf( fp, "Dam   %d\n", ch->damroll );
    fprintf( fp, "ACs %d %d %d %d\n",
             ch->armor[0], ch->armor[1], ch->armor[2], ch->armor[3] );
    if ( ch->wimpy != 0 )
        fprintf( fp, "Wimp  %d\n", ch->wimpy );
    fprintf( fp, "Attr %d %d %d %d %d\n",
             ch->perm_stat[STAT_STR],
             ch->perm_stat[STAT_INT],
             ch->perm_stat[STAT_WIS],
             ch->perm_stat[STAT_DEX], ch->perm_stat[STAT_CON] );

    fprintf( fp, "AMod %d %d %d %d %d\n",
             ch->mod_stat[STAT_STR],
             ch->mod_stat[STAT_INT],
             ch->mod_stat[STAT_WIS],
             ch->mod_stat[STAT_DEX], ch->mod_stat[STAT_CON] );

    if ( IS_NPC( ch ) )
    {
        fprintf( fp, "Vnum %d\n", ch->pIndexData->vnum );
    }
    else
    {
        fprintf( fp, "Pass %s~\n", ch->pcdata->pwd );
        if ( ch->pcdata->bamfin[0] != '\0' )
            fprintf( fp, "Bin  %s~\n", ch->pcdata->bamfin );
        if ( ch->pcdata->bamfout[0] != '\0' )
            fprintf( fp, "Bout %s~\n", ch->pcdata->bamfout );
        fprintf( fp, "Titl %s~\n", ch->pcdata->title );
        fprintf( fp, "Pnts %d\n", ch->pcdata->points );
        fprintf( fp, "TSex %d\n", ch->pcdata->true_sex );
        fprintf( fp, "LLev %d\n", ch->pcdata->last_level );
        fprintf( fp, "HMVP %d %d %d\n", ch->pcdata->perm_hit,
                 ch->pcdata->perm_mana, ch->pcdata->perm_move );
        fprintf( fp, "Cond %d %d %d\n",
                 ch->pcdata->condition[0],
                 ch->pcdata->condition[1], ch->pcdata->condition[2] );
        fprintf( fp, "Prom %s~\n", ch->pcdata->prompt );
        fprintf( fp, "Clan %d\n", ch->pcdata->clan );
        fprintf( fp, "Rank %d\n", ch->pcdata->clan_rank );
        fprintf( fp, "Cmnt %s~\n", ch->pcdata->comment );
        fprintf( fp, "Eml  %s~\n", ch->pcdata->email );
        fprintf( fp, "Spoc %d\n", ch->pcdata->spousec );
        fprintf( fp, "Spou %s~\n", ch->pcdata->spouse );
        fprintf( fp, "Neme %s~\n", ch->pcdata->nemesis );
        fprintf( fp, "PKdi %d\n", ch->pcdata->pk_deaths );
        fprintf( fp, "PKki %d\n", ch->pcdata->pk_kills );
        fprintf( fp, "Tick %d\n", ch->pcdata->tick );
        fprintf( fp, "Ticks %d\n", ch->pcdata->ticks );
        if ( ch->pcdata->who_race )
            fprintf( fp, "WRace %s~\n", ch->pcdata->who_race );
        if ( ch->pcdata->who_prefix )
            fprintf( fp, "WPref %s~\n", ch->pcdata->who_prefix );

        /* Save note board status */
        /* Save number of boards in case that number changes */
        fprintf( fp, "Boards       %d ", MAX_BOARD );
        for ( i = 0; i < MAX_BOARD; i++ )
            fprintf( fp, "%s %ld ", boards[i].short_name,
                     ch->pcdata->last_note[i] );
        fprintf( fp, "\n" );

        if ( ch->pcdata->recall_room == NULL )
            ch->pcdata->recall_room = get_room_index( ROOM_VNUM_TEMPLE );
        fprintf( fp, "Recl %d\n", ch->pcdata->recall_room->vnum );

        /* write alias */
        for ( pos = 0; pos < MAX_ALIAS; pos++ )
        {
            if ( ch->pcdata->alias[pos] == NULL
                 || ch->pcdata->alias_sub[pos] == NULL )
                break;

            fprintf( fp, "Alias %s %s~\n", ch->pcdata->alias[pos],
                     ch->pcdata->alias_sub[pos] );
        }

        for ( sn = 0; sn < MAX_SKILL; sn++ )
        {
            if ( skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0 )
            {
                fprintf( fp, "Sk %d '%s'\n",
                         ch->pcdata->learned[sn], skill_table[sn].name );
            }
        }

        for ( gn = 0; gn < MAX_GROUP; gn++ )
        {
            if ( group_table[gn].name != NULL && ch->pcdata->group_known[gn] )
            {
                fprintf( fp, "Gr '%s'\n", group_table[gn].name );
            }
        }
    }

    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
        if ( paf->type < 0 || paf->type >= MAX_SKILL )
            continue;

        fprintf( fp, "AffD '%s' %3d %3d %3d %3d %10d\n",
                 skill_table[paf->type].name,
                 paf->level,
                 paf->duration, paf->modifier, paf->location, paf->bitvector );
    }
    for ( npaf = ch->newaffected; npaf != NULL; npaf = npaf->next )
    {
        if ( npaf->type < 0 || npaf->type >= MAX_SKILL )
            continue;

        fprintf( fp, "NewAffD '%s' %3d %3d %3d %3d %10d\n",
                 skill_table[npaf->type].name,
                 npaf->level,
                 npaf->duration,
                 npaf->modifier, npaf->location, npaf->bitvector );
    }

    fprintf( fp, "End\n\n" );
    return;
}

/* write a pet */
void fwrite_pet( CHAR_DATA * pet, FILE * fp )
{
    AFFECT_DATA *paf;

    fprintf( fp, "#PET\n" );

    fprintf( fp, "Vnum %d\n", pet->pIndexData->vnum );

    fprintf( fp, "Name %s~\n", pet->name );
    if ( pet->short_descr != pet->pIndexData->short_descr )
        fprintf( fp, "ShD  %s~\n", pet->short_descr );
    if ( pet->long_descr != pet->pIndexData->long_descr )
        fprintf( fp, "LnD  %s~\n", pet->long_descr );
    if ( pet->description != pet->pIndexData->description )
        fprintf( fp, "Desc %s~\n", pet->description );
    if ( pet->race != pet->pIndexData->race )
        fprintf( fp, "Race %s~\n", race_table[pet->race].name );
    fprintf( fp, "Sex  %d\n", pet->sex );
    if ( pet->level != pet->pIndexData->level )
        fprintf( fp, "Levl %d\n", pet->level );
    fprintf( fp, "HMV  %d %d %d %d %d %d\n",
             pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move,
             pet->max_move );
    if ( pet->gold > 0 )
        fprintf( fp, "Gold %ld\n", pet->gold );
    if ( pet->exp > 0 )
        fprintf( fp, "Exp  %ld\n", pet->exp );
    if ( pet->act != pet->pIndexData->act )
        fprintf( fp, "Act  %ld\n", pet->act );
    if ( pet->affected_by != pet->pIndexData->affected_by )
        fprintf( fp, "AfBy %ld\n", pet->affected_by );
    if ( pet->comm != 0 )
        fprintf( fp, "Comm %ld\n", pet->comm );
    fprintf( fp, "Pos  %d\n", pet->position =
             POS_FIGHTING ? POS_STANDING : pet->position );
    if ( pet->saving_throw != 0 )
        fprintf( fp, "Save %d\n", pet->saving_throw );
    if ( pet->alignment != pet->pIndexData->alignment )
        fprintf( fp, "Alig %d\n", pet->alignment );
    if ( pet->hitroll != pet->pIndexData->hitroll )
        fprintf( fp, "Hit  %d\n", pet->hitroll );
    if ( pet->damroll != pet->pIndexData->damage[DICE_BONUS] )
        fprintf( fp, "Dam  %d\n", pet->damroll );
    fprintf( fp, "ACs  %d %d %d %d\n",
             pet->armor[0], pet->armor[1], pet->armor[2], pet->armor[3] );
    fprintf( fp, "Attr %d %d %d %d %d\n",
             pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
             pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
             pet->perm_stat[STAT_CON] );
    fprintf( fp, "AMod %d %d %d %d %d\n",
             pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
             pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
             pet->mod_stat[STAT_CON] );

    for ( paf = pet->affected; paf != NULL; paf = paf->next )
    {
        if ( paf->type < 0 || paf->type >= MAX_SKILL )
            continue;

        fprintf( fp, "AffD '%s' %3d %3d %3d %3d %10d\n",
                 skill_table[paf->type].name,
                 paf->level, paf->duration, paf->modifier, paf->location,
                 paf->bitvector );
    }

    fprintf( fp, "End\n" );
    return;
}

/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->next_content != NULL )
        fwrite_obj( ch, obj->next_content, fp, iNest );

    /*
     * Castrate storage characters.
     * keys and maps have special poof-rules
     */
    if ( obj->item_type == ITEM_KEY || obj->item_type == ITEM_MAP )
    {
        if ( obj->value[0] != 0 )
        {
            if ( obj->value[1] > 0 )
            {
                if ( ch->level < obj->value[1] )
                    return;     /* POOF! */
            }

            if ( obj->value[2] > 0 )
            {
                if ( ch->level > obj->value[2] )
                    return;     /* POOF! */
            }
        }
    }

    fprintf( fp, "#O\n" );
    fprintf( fp, "Vnum %d\n", obj->pIndexData->vnum );
    if ( obj->enchanted )
        fprintf( fp, "Enchanted\n" );
    fprintf( fp, "Nest %d\n", iNest );

    /* these data are only used if they do not match the defaults */

    if ( obj->name != obj->pIndexData->name )
        fprintf( fp, "Name %s~\n", obj->name );
    if ( obj->short_descr != obj->pIndexData->short_descr )
        fprintf( fp, "ShD  %s~\n", obj->short_descr );
    if ( obj->description != obj->pIndexData->description )
        fprintf( fp, "Desc %s~\n", obj->description );
    if ( obj->extra_flags != obj->pIndexData->extra_flags )
        fprintf( fp, "ExtF %d\n", obj->extra_flags );
    if ( obj->wear_flags != obj->pIndexData->wear_flags )
        fprintf( fp, "WeaF %d\n", obj->wear_flags );
    if ( obj->item_type != obj->pIndexData->item_type )
        fprintf( fp, "Ityp %d\n", obj->item_type );
    if ( obj->weight != obj->pIndexData->weight )
        fprintf( fp, "Wt   %d\n", obj->weight );

    /* variable data */

    fprintf( fp, "Wear %d\n", obj->wear_loc );
    if ( obj->level != 0 )
        fprintf( fp, "Lev  %d\n", obj->level );
    if ( obj->timer != 0 )
        fprintf( fp, "Time %d\n", obj->timer );
    fprintf( fp, "Cost %d\n", obj->cost );
    if ( obj->value[0] != obj->pIndexData->value[0]
         || obj->value[1] != obj->pIndexData->value[1]
         || obj->value[2] != obj->pIndexData->value[2]
         || obj->value[3] != obj->pIndexData->value[3]
         || obj->value[4] != obj->pIndexData->value[4] )
        fprintf( fp, "Val  %d %d %d %d %d\n",
                 obj->value[0], obj->value[1], obj->value[2], obj->value[3],
                 obj->value[4] );

    switch ( obj->item_type )
    {
    case ITEM_POTION:
    case ITEM_SCROLL:
        if ( obj->value[1] > 0 )
        {
            fprintf( fp, "Spell 1 '%s'\n", skill_table[obj->value[1]].name );
        }

        if ( obj->value[2] > 0 )
        {
            fprintf( fp, "Spell 2 '%s'\n", skill_table[obj->value[2]].name );
        }

        if ( obj->value[3] > 0 )
        {
            fprintf( fp, "Spell 3 '%s'\n", skill_table[obj->value[3]].name );
        }

        break;

    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
        if ( obj->value[3] > 0 )
        {
            fprintf( fp, "Spell 3 '%s'\n", skill_table[obj->value[3]].name );
        }

        break;
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
        if ( paf->type < 0 || paf->type >= MAX_SKILL )
            continue;
        fprintf( fp, "AffD '%s' %d %d %d %d %d\n",
                 skill_table[paf->type].name,
                 paf->level,
                 paf->duration, paf->modifier, paf->location, paf->bitvector );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
        if ( paf->type != -999 )
            continue;
        fprintf( fp, "AfRd %d %d %d %d %d\n",
                 paf->level,
                 paf->duration, paf->modifier, paf->location, paf->bitvector );
    }

    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
        fprintf( fp, "ExDe %s~ %s~\n", ed->keyword, ed->description );
    }

    fprintf( fp, "End\n\n" );

    if ( obj->contains != NULL )
        fwrite_obj( ch, obj->contains, fp, iNest + 1 );

    return;
}

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA * d, char *name )
{
    char buf[MAX_STRING_LENGTH];
    static PC_DATA pcdata_zero;
    char strsave[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    int stat;

    if ( char_free == NULL )
    {
        ch = alloc_perm( sizeof( *ch ) );
    }
    else
    {
        ch = char_free;
        char_free = char_free->next;
    }
    clear_char( ch );

    if ( pcdata_free == NULL )
    {
        ch->pcdata = alloc_perm( sizeof( *ch->pcdata ) );
    }
    else
    {
        ch->pcdata = pcdata_free;
        pcdata_free = pcdata_free->next;
    }
    *ch->pcdata = pcdata_zero;

    d->character = ch;
    ch->desc = d;
    ch->name = str_dup( name );
    ch->version = 0;
    ch->race = race_lookup( "human" );
    ch->affected_by = 0;
    ch->newaff[0] = 0;
    ch->act = PLR_NOSUMMON
        | PLR_AUTOEXIT | PLR_AUTOLOOT | PLR_AUTOSAC | PLR_AUTOGOLD;
    ch->comm = COMM_COMBINE | COMM_PROMPT;
    ch->invis_level = 0;
    ch->practice = 0;
    ch->train = 0;
    ch->hitroll = 0;
    ch->damroll = 0;
    ch->trust = 0;
    ch->wimpy = 0;
    ch->saving_throw = 0;
    ch->exp_stack = 0;
    ch->pcdata->points = 0;
    ch->pcdata->confirm_delete = FALSE;
    ch->pcdata->fmessage = NULL;
    ch->pcdata->message = NULL;
    ch->pcdata->ftell_q = NULL;
    ch->pcdata->tell_q = NULL;
    ch->pcdata->messages = 0;
    ch->pcdata->tells = 0;
    ch->pcdata->board = &boards[DEFAULT_BOARD];
    ch->pcdata->pwd = str_dup( "" );
    ch->pcdata->bamfin = str_dup( "" );
    ch->pcdata->bamfout = str_dup( "" );
    ch->pcdata->title = str_dup( "" );
    for ( stat = 0; stat < MAX_STATS; stat++ )
        ch->perm_stat[stat] = 13;
    ch->pcdata->perm_hit = 0;
    ch->pcdata->perm_mana = 0;
    ch->pcdata->perm_move = 0;
    ch->pcdata->true_sex = 0;
    ch->pcdata->last_level = 0;
    ch->pcdata->condition[COND_THIRST] = 48;
    ch->pcdata->condition[COND_FULL] = 48;
    ch->pcdata->security = 0;   /* OLC */
    ch->pcdata->prompt =
        str_dup( "%i`K/`W%H`w HP %n`K/`W%M`w MP %w`K/`W%V`w MV `K> " );
/*    ch->pcdata->clan			= 0; */
    ch->beep = TRUE;
    ch->anonymous = FALSE;
    ch->pcdata->recall_room = get_room_index( ROOM_VNUM_TEMPLE );
    ch->pcdata->pk_deaths = 0;
    ch->pcdata->pk_kills = 0;
    ch->pcdata->tick = 0;
    ch->pcdata->ticks = 0;
    ch->pcdata->spousec = 0;
    ch->pcdata->spouse = str_dup( "(none)" );
    ch->pcdata->nemesis = str_dup( "(none)" );
    ch->pcdata->email = str_dup( "(none)" );
    ch->pcdata->comment = str_dup( "(none)" );
    ch->pcdata->who_race = NULL;
    ch->pcdata->who_prefix = NULL;
    ch->pcdata->immcmdlist = NULL;
    ch->pcdata->faction_standings = NULL;
    found = FALSE;
    fclose( fpReserve );

    /* decompress if .gz file exists */
    sprintf( strsave, "%s/%s%s", sysconfig.player_dir, capitalize( name ),
             ".gz" );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
        fclose( fp );
        sprintf( buf, "gzip -dfq %s", strsave );
        system( buf );
    }

    sprintf( strsave, "%s/%s", sysconfig.player_dir, capitalize( name ) );
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
        int iNest;

        for ( iNest = 0; iNest < MAX_NEST; iNest++ )
            rgObjNest[iNest] = NULL;

        found = TRUE;
        for ( ;; )
        {
            char letter;
            char *word;

            letter = fread_letter( fp );
            if ( letter == '*' )
            {
                fread_to_eol( fp );
                continue;
            }

            if ( letter != '#' )
            {
                bug( "Load_char_obj: # not found.", 0 );
                break;
            }

            word = fread_word( fp );
            if ( !str_cmp( word, "PLAYER" ) )
                fread_char( ch, fp );
            else if ( !str_cmp( word, "OBJECT" ) )
                fread_obj( ch, fp );
            else if ( !str_cmp( word, "O" ) )
                fread_obj( ch, fp );
            else if ( !str_cmp( word, "PET" ) )
                fread_pet( ch, fp );
            else if ( !str_cmp( word, "FACTIONS" ) )
                fread_faction_standings( ch, fp );
            else if ( !str_cmp( word, "IMM" ) )
                fread_imm( ch, fp );
            else if ( !str_cmp( word, "END" ) )
                break;
            else
            {
                bug( "Load_char_obj: bad section.", 0 );
                break;
            }
        }
        fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );

    /* initialize race */
    if ( found )
    {
        int i;

        if ( ch->race == 0 )
            ch->race = race_lookup( "human" );

        ch->size = pc_race_table[ch->race].size;
        ch->dam_type = 17;      /*punch */

        for ( i = 0; i < 5; i++ )
        {
            if ( pc_race_table[ch->race].skills[i] == NULL )
                break;
            group_add( ch, pc_race_table[ch->race].skills[i], FALSE );
        }
        ch->affected_by = ch->affected_by | race_table[ch->race].aff;
        ch->imm_flags = ch->imm_flags | race_table[ch->race].imm;
        ch->res_flags = ch->res_flags | race_table[ch->race].res;
        ch->vuln_flags = ch->vuln_flags | race_table[ch->race].vuln;
        ch->form = race_table[ch->race].form;
        ch->parts = race_table[ch->race].parts;
    }

    /* RT initialize skills */

    if ( found && ch->version < 2 ) /* need to add the new skills */
    {
        group_add( ch, "rom basics", FALSE );
        group_add( ch, class_table[ch->Class].base_group, FALSE );
        group_add( ch, class_table[ch->Class].default_group, TRUE );
        ch->pcdata->learned[gsn_recall] = 50;
    }

    /* fix levels */
    if ( found && ch->version < 3 && ( ch->level > 35 || ch->trust > 35 ) )
    {
        switch ( ch->level )
        {
        case ( 40 ):
            ch->level = 60;
            break;              /* imp -> imp */
        case ( 39 ):
            ch->level = 58;
            break;              /* god -> supreme */
        case ( 38 ):
            ch->level = 56;
            break;              /* deity -> god */
        case ( 37 ):
            ch->level = 53;
            break;              /* angel -> demigod */
        }

        switch ( ch->trust )
        {
        case ( 40 ):
            ch->trust = 60;
            break;              /* imp -> imp */
        case ( 39 ):
            ch->trust = 58;
            break;              /* god -> supreme */
        case ( 38 ):
            ch->trust = 56;
            break;              /* deity -> god */
        case ( 37 ):
            ch->trust = 53;
            break;              /* angel -> demigod */
        case ( 36 ):
            ch->trust = 51;
            break;              /* hero -> hero */
        }
    }

    return found;
}

/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                                }

void fread_char( CHAR_DATA * ch, FILE * fp )
{
    char buf[MAX_STRING_LENGTH];
    char *word = NULL;
    int count = 0;
    bool fMatch = FALSE;

    for ( ;; )
    {
        word = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER( word[0] ) )
        {
        case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

        case 'A':
            KEY( "Act", ch->act, fread_number( fp ) );
            KEY( "AffectedBy", ch->affected_by, fread_number( fp ) );
            KEY( "AfBy", ch->affected_by, fread_number( fp ) );
            KEY( "NewAffectedBy", ch->newaff[0], fread_number( fp ) );
            KEY( "NewAfBy", ch->newaff[0], fread_number( fp ) );
            KEY( "Alignment", ch->alignment, fread_number( fp ) );
            KEY( "Alig", ch->alignment, fread_number( fp ) );
            KEY( "Anon", ch->anonymous, fread_number( fp ) );

            if ( !str_cmp( word, "Alias" ) )
            {
                if ( count >= MAX_ALIAS )
                {
                    fread_to_eol( fp );
                    fMatch = TRUE;
                    break;
                }

                ch->pcdata->alias[count] = str_dup( fread_word( fp ) );
                ch->pcdata->alias_sub[count] = fread_string( fp );
                count++;
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "AC" ) || !str_cmp( word, "Armor" ) )
            {
                fread_to_eol( fp );
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "ACs" ) )
            {
                int i;

                for ( i = 0; i < 4; i++ )
                    ch->armor[i] = fread_number( fp );
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "Aff" )
                 || !str_cmp( word, "AffD" ) )
            {
                AFFECT_DATA *paf;

                if ( affect_free == NULL )
                {
                    paf = alloc_perm( sizeof( *paf ) );
                }
                else
                {
                    paf = affect_free;
                    affect_free = affect_free->next;
                }

                if ( !str_cmp( word, "AffD" ) )
                {
                    int sn;
                    sn = skill_lookup( fread_word( fp ) );
                    if ( sn < 0 )
                        bug( "Fread_char: unknown skill.", 0 );
                    else
                        paf->type = sn;
                }
                else            /* old form */
                    paf->type = fread_number( fp );
                if ( ch->version == 0 )
                    paf->level = ch->level;
                else
                    paf->level = fread_number( fp );
                paf->duration = fread_number( fp );
                paf->modifier = fread_number( fp );
                paf->location = fread_number( fp );
                paf->bitvector = fread_number( fp );
                paf->next = ch->affected;
                ch->affected = paf;
                fMatch = TRUE;
                break;
            }
            if ( !str_cmp( word, "NewAffect" ) || !str_cmp( word, "NewAff" )
                 || !str_cmp( word, "NewAffD" ) )
            {
                NEWAFFECT_DATA *npaf;

                if ( newaffect_free == NULL )
                {
                    npaf = alloc_perm( sizeof( *npaf ) );
                }
                else
                {
                    npaf = newaffect_free;
                    newaffect_free = newaffect_free->next;
                }

                if ( !str_cmp( word, "NewAffD" ) )
                {
                    int sn;
                    sn = skill_lookup( fread_word( fp ) );
                    if ( sn < 0 )
                        bug( "Fread_char: unknown skill.", 0 );
                    else
                        npaf->type = sn;
                }
                else            /* old form */
                    npaf->type = fread_number( fp );
                if ( ch->version == 0 )
                    npaf->level = ch->level;
                else
                    npaf->level = fread_number( fp );
                npaf->duration = fread_number( fp );
                npaf->modifier = fread_number( fp );
                npaf->location = fread_number( fp );
                npaf->bitvector = fread_number( fp );
/*		npaf->next	= ch->newaff[0];
		ch->newaff	= npaf;*/
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "AttrMod" ) || !str_cmp( word, "AMod" ) )
            {
                int stat;
                for ( stat = 0; stat < MAX_STATS; stat++ )
                    ch->mod_stat[stat] = fread_number( fp );
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "AttrPerm" ) || !str_cmp( word, "Attr" ) )
            {
                int stat;

                for ( stat = 0; stat < MAX_STATS; stat++ )
                    ch->perm_stat[stat] = fread_number( fp );
                fMatch = TRUE;
                break;
            }
            break;

        case 'B':
            KEY( "Beep", ch->beep, fread_number( fp ) );
            KEY( "Bamfin", ch->pcdata->bamfin, fread_string( fp ) );
            KEY( "Bamfout", ch->pcdata->bamfout, fread_string( fp ) );
            KEY( "Bin", ch->pcdata->bamfin, fread_string( fp ) );
            KEY( "Bout", ch->pcdata->bamfout, fread_string( fp ) );
            /* Read in board status */
            if ( !str_cmp( word, "Boards" ) )
            {
                int i, num = fread_number( fp );    /* number of boards saved */
                char *boardname;

                for ( ; num; num-- )    /* for each of the board saved */
                {
                    boardname = fread_word( fp );
                    i = board_lookup( boardname );  /* find board number */

                    if ( i == BOARD_NOTFOUND )  /* Does board still exist ? */
                    {
                        sprintf( buf,
                                 "fread_char: %s had unknown board name: %s. Skipped.",
                                 ch->name, boardname );
                        log_string( buf );
                        fread_number( fp ); /* read last_note and skip info */
                    }
                    else        /* Save it */
                        ch->pcdata->last_note[i] = fread_number( fp );
                }               /* for */

                fMatch = TRUE;
            }                   /* Boards */
            break;

        case 'C':
            KEY( "Class", ch->Class, fread_number( fp ) );
            KEY( "Cla", ch->Class, fread_number( fp ) );
            KEY( "Clan", ch->pcdata->clan, fread_number( fp ) );

            if ( !str_cmp( word, "Condition" ) || !str_cmp( word, "Cond" ) )
            {
                ch->pcdata->condition[0] = fread_number( fp );
                ch->pcdata->condition[1] = fread_number( fp );
                ch->pcdata->condition[2] = fread_number( fp );
                fMatch = TRUE;
                break;
            }
            KEY( "Cmnt", ch->pcdata->comment, fread_string( fp ) );
            KEY( "Comm", ch->comm, fread_number( fp ) );

            break;

        case 'D':
            KEY( "Damroll", ch->damroll, fread_number( fp ) );
            KEY( "Dam", ch->damroll, fread_number( fp ) );
            KEY( "Description", ch->description, fread_string( fp ) );
            KEY( "Desc", ch->description, fread_string( fp ) );
            break;

        case 'E':
            if ( !str_cmp( word, "End" ) )
            {
                /* Following two lines overide color setting in pfile */
                if ( ch->desc->ansi && !fCopyOver )
                    SET_BIT( ch->act, PLR_COLOR );
                else if ( !fCopyOver )
                    REMOVE_BIT( ch->act, PLR_COLOR );
                else if ( IS_SET( ch->act, PLR_COLOR ) )
                    ch->desc->ansi = TRUE;
                else
                    ch->desc->ansi = FALSE;
                return;
            }
            KEY( "Eml", ch->pcdata->email, fread_string( fp ) );
            KEY( "Exp", ch->exp, fread_number( fp ) );
            break;

        case 'G':
            KEY( "Gold_bank", ch->pcdata->gold_bank, fread_number( fp ) );
            KEY( "Gold", ch->gold, fread_number( fp ) );
            if ( !str_cmp( word, "Group" ) || !str_cmp( word, "Gr" ) )
            {
                int gn;
                char *temp;

                temp = fread_word( fp );
                gn = group_lookup( temp );
                /* gn    = group_lookup( fread_word( fp ) ); */
                if ( gn < 0 )
                {
                    fprintf( stderr, "%s", temp );
                    bug( "Fread_char: unknown group. ", 0 );
                }
                else
                    gn_add( ch, gn );
                fMatch = TRUE;
            }
            break;

        case 'H':
            KEY( "Hitroll", ch->hitroll, fread_number( fp ) );
            KEY( "Hit", ch->hitroll, fread_number( fp ) );

            if ( !str_cmp( word, "HpManaMove" ) || !str_cmp( word, "HMV" ) )
            {
                ch->hit = fread_number( fp );
                ch->max_hit = fread_number( fp );
                ch->mana = fread_number( fp );
                ch->max_mana = fread_number( fp );
                ch->move = fread_number( fp );
                ch->max_move = fread_number( fp );
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "HpManaMovePerm" )
                 || !str_cmp( word, "HMVP" ) )
            {
                ch->pcdata->perm_hit = fread_number( fp );
                ch->pcdata->perm_mana = fread_number( fp );
                ch->pcdata->perm_move = fread_number( fp );
                fMatch = TRUE;
                break;
            }

            break;

        case 'I':
            KEY( "InvisLevel", ch->invis_level, fread_number( fp ) );
            KEY( "Invi", ch->invis_level, fread_number( fp ) );
            KEY( "Inca", ch->incarnations, fread_number( fp ) );
            break;

        case 'L':
            KEY( "LastLevel", ch->pcdata->last_level, fread_number( fp ) );
            KEY( "LLev", ch->pcdata->last_level, fread_number( fp ) );
            KEY( "Level", ch->level, fread_number( fp ) );
            KEY( "Lev", ch->level, fread_number( fp ) );
            KEY( "Levl", ch->level, fread_number( fp ) );
            KEY( "LongDescr", ch->long_descr, fread_string( fp ) );
            KEY( "LnD", ch->long_descr, fread_string( fp ) );

            if ( ch->version == 3 )
            {
                if ( !str_cmp( word, "Log" ) )
                {
                    fread_number( fp );
                    fMatch = TRUE;
                    break;
                }
            }
            else if ( ch->version == 4 )
            {
                if ( !str_cmp( word, "Logn" ) )
                {
                    fread_number( fp );
                    fMatch = TRUE;
                    break;
                }
            }
            break;

        case 'N':
            KEY( "Name", ch->name, fread_string( fp ) );
            KEY( "Neme", ch->pcdata->nemesis, fread_string( fp ) );
            KEY( "Note", ch->last_note, fread_number( fp ) );
            break;

        case 'P':
            KEY( "Password", ch->pcdata->pwd, fread_string( fp ) );
            KEY( "Pass", ch->pcdata->pwd, fread_string( fp ) );
            KEY( "PKdi", ch->pcdata->pk_deaths, fread_number( fp ) );
            KEY( "PKki", ch->pcdata->pk_kills, fread_number( fp ) );
            KEY( "Played", ch->played, fread_number( fp ) );
            KEY( "Plyd", ch->played, fread_number( fp ) );
            KEY( "Points", ch->pcdata->points, fread_number( fp ) );
            KEY( "Pnts", ch->pcdata->points, fread_number( fp ) );
            KEY( "Position", ch->position, fread_number( fp ) );
            KEY( "Pos", ch->position, fread_number( fp ) );
            KEY( "Practice", ch->practice, fread_number( fp ) );
            KEY( "Prac", ch->practice, fread_number( fp ) );
            KEY( "Prom", ch->pcdata->prompt, fread_string( fp ) );
            break;

        case 'Q':
            KEY( "Qp", ch->qp, fread_number( fp ) );
            break;

        case 'R':
            /* KEY( "Race",        ch->race,    
               race_lookup(fread_string( fp )) ); */
            if ( !str_cmp( word, "Race" ) )
            {
                char *race;
                race = fread_string( fp );
                ch->race = race_lookup( race );
                free_string( &race );
                fMatch = TRUE;
            }

            KEY( "Rank", ch->pcdata->clan_rank, fread_number( fp ) );

            if ( !str_cmp( word, "Recl" ) )
            {
                ch->pcdata->recall_room = get_room_index( fread_number( fp ) );
                if ( ch->pcdata->recall_room == NULL )
                    ch->pcdata->recall_room =
                        get_room_index( ROOM_VNUM_TEMPLE );
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "Room" ) )
            {
                ch->in_room = get_room_index( fread_number( fp ) );
                if ( ch->in_room == NULL )
                    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
                fMatch = TRUE;
                break;
            }
            break;

        case 'S':
            KEY( "SavingThrow", ch->saving_throw, fread_number( fp ) );
            KEY( "Save", ch->saving_throw, fread_number( fp ) );
            KEY( "Scro", ch->lines, fread_number( fp ) );
            KEY( "Sex", ch->sex, fread_number( fp ) );
            KEY( "ShortDescr", ch->short_descr, fread_string( fp ) );
            KEY( "ShD", ch->short_descr, fread_string( fp ) );
            KEY( "Sec", ch->pcdata->security, fread_number( fp ) ); /* OLC */
            KEY( "Spoc", ch->pcdata->spousec, fread_number( fp ) );
            KEY( "Spou", ch->pcdata->spouse, fread_string( fp ) );

            if ( !str_cmp( word, "Skill" ) || !str_cmp( word, "Sk" ) )
            {
                int sn;
                int value;
                char *temp;

                value = fread_number( fp );
                temp = fread_word( fp );
                sn = skill_lookup( temp );
                /* sn    = skill_lookup( fread_word( fp ) ); */
                if ( sn < 0 )
                {
                    fprintf( stderr, "%s", temp );
                    bug( "Fread_char: unknown skill. ", 0 );
                }
                else
                    ch->pcdata->learned[sn] = value;
                fMatch = TRUE;
            }

            break;

        case 'T':
            KEY( "Tick", ch->pcdata->tick, fread_number( fp ) );
            KEY( "Ticks", ch->pcdata->ticks, fread_number( fp ) );
            KEY( "TrueSex", ch->pcdata->true_sex, fread_number( fp ) );
            KEY( "TSex", ch->pcdata->true_sex, fread_number( fp ) );
            KEY( "Trai", ch->train, fread_number( fp ) );
            KEY( "Trust", ch->trust, fread_number( fp ) );
            KEY( "Tru", ch->trust, fread_number( fp ) );

            if ( !str_cmp( word, "Title" ) || !str_cmp( word, "Titl" ) )
            {
                ch->pcdata->title = fread_string( fp );
                if ( ch->pcdata->title[0] != '.' && ch->pcdata->title[0] != ','
                     && ch->pcdata->title[0] != '!'
                     && ch->pcdata->title[0] != '?' )
                {
                    sprintf( buf, " %s", ch->pcdata->title );
                    free_string( &ch->pcdata->title );
                    ch->pcdata->title = str_dup( buf );
                }
                fMatch = TRUE;
                break;
            }

            break;

        case 'V':
            KEY( "Version", ch->version, fread_number( fp ) );
            KEY( "Vers", ch->version, fread_number( fp ) );
            if ( !str_cmp( word, "Vnum" ) )
            {
                ch->pIndexData = get_mob_index( fread_number( fp ) );
                fMatch = TRUE;
                break;
            }
            break;

        case 'W':
            KEY( "Wimpy", ch->wimpy, fread_number( fp ) );
            KEY( "Wimp", ch->wimpy, fread_number( fp ) );
            KEY( "WRace", ch->pcdata->who_race, fread_string( fp ) );
            KEY( "WPref", ch->pcdata->who_prefix, fread_string( fp ) );
            break;
        }

        if ( !fMatch )
        {
            bug( "Fread_char: no match.", 0 );
            fread_to_eol( fp );
        }
    }
}

/* load an #IMM section from a pfile */
void fread_imm( CHAR_DATA * ch, FILE * fp )
{
    char *word;
    IMMCMD_TYPE *tmp;

    for ( ;; )
    {
        word = feof( fp ) ? "End" : fread_word( fp );

        if ( !str_cmp( word, "End" ) )
        {
            break;
        }

        tmp = malloc( sizeof( IMMCMD_TYPE ) );
        tmp->cmd = str_dup( word );
        tmp->next = ch->pcdata->immcmdlist;
        ch->pcdata->immcmdlist = tmp;
    }
}

/* load a pet from the forgotten reaches */
void fread_pet( CHAR_DATA * ch, FILE * fp )
{
    char *word;
    CHAR_DATA *pet;
    bool fMatch;

    /* first entry had BETTER be the vnum or we barf */
    word = feof( fp ) ? "END" : fread_word( fp );
    if ( !str_cmp( word, "Vnum" ) )
    {
        int vnum;

        vnum = fread_number( fp );
        if ( get_mob_index( vnum ) == NULL )
        {
            bug( "Fread_pet: bad vnum %d.", vnum );
            pet = create_mobile( get_mob_index( MOB_VNUM_FIDO ) );
        }
        else
            pet = create_mobile( get_mob_index( vnum ) );
    }
    else
    {
        bug( "Fread_pet: no vnum in file.", 0 );
        pet = create_mobile( get_mob_index( MOB_VNUM_FIDO ) );
    }

    for ( ;; )
    {
        word = feof( fp ) ? "END" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER( word[0] ) )
        {
        case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

        case 'A':
            KEY( "Act", pet->act, fread_number( fp ) );
            KEY( "AfBy", pet->affected_by, fread_number( fp ) );
            KEY( "Alig", pet->alignment, fread_number( fp ) );

            if ( !str_cmp( word, "ACs" ) )
            {
                int i;

                for ( i = 0; i < 4; i++ )
                    pet->armor[i] = fread_number( fp );
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "AffD" ) )
            {
                AFFECT_DATA *paf;
                int sn;

                if ( affect_free == NULL )
                    paf = alloc_perm( sizeof( *paf ) );
                else
                {
                    paf = affect_free;
                    affect_free = affect_free->next;
                }

                sn = skill_lookup( fread_word( fp ) );
                if ( sn < 0 )
                    bug( "Fread_char: unknown skill.", 0 );
                else
                    paf->type = sn;

                paf->level = fread_number( fp );
                paf->duration = fread_number( fp );
                paf->modifier = fread_number( fp );
                paf->location = fread_number( fp );
                paf->bitvector = fread_number( fp );
                paf->next = pet->affected;
                pet->affected = paf;
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "AMod" ) )
            {
                int stat;

                for ( stat = 0; stat < MAX_STATS; stat++ )
                    pet->mod_stat[stat] = fread_number( fp );
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "Attr" ) )
            {
                int stat;

                for ( stat = 0; stat < MAX_STATS; stat++ )
                    pet->perm_stat[stat] = fread_number( fp );
                fMatch = TRUE;
                break;
            }
            break;

        case 'C':
            KEY( "Comm", pet->comm, fread_number( fp ) );
            break;

        case 'D':
            KEY( "Dam", pet->damroll, fread_number( fp ) );
            KEY( "Desc", pet->description, fread_string( fp ) );
            break;

        case 'E':
            if ( !str_cmp( word, "End" ) )
            {
                pet->leader = ch;
                pet->master = ch;
                ch->pet = pet;
                return;
            }
            KEY( "Exp", pet->exp, fread_number( fp ) );
            break;

        case 'G':
            KEY( "Gold", pet->gold, fread_number( fp ) );
            break;

        case 'H':
            KEY( "Hit", pet->hitroll, fread_number( fp ) );

            if ( !str_cmp( word, "HMV" ) )
            {
                pet->hit = fread_number( fp );
                pet->max_hit = fread_number( fp );
                pet->mana = fread_number( fp );
                pet->max_mana = fread_number( fp );
                pet->move = fread_number( fp );
                pet->max_move = fread_number( fp );
                fMatch = TRUE;
                break;
            }
            break;

        case 'L':
            KEY( "Levl", pet->level, fread_number( fp ) );
            KEY( "LnD", pet->long_descr, fread_string( fp ) );
            break;

        case 'N':
            KEY( "Name", pet->name, fread_string( fp ) );
            break;

        case 'P':
            KEY( "Pos", pet->position, fread_number( fp ) );
            break;

        case 'R':
            /* KEY( "Race", pet->race, race_lookup(fread_string(fp))); */
            if ( !str_cmp( word, "Race" ) )
            {
                char *race;
                race = fread_string( fp );
                ch->race = race_lookup( race );
                free_string( &race );
                fMatch = TRUE;
            }

            break;

        case 'S':
            KEY( "Save", pet->saving_throw, fread_number( fp ) );
            KEY( "Sex", pet->sex, fread_number( fp ) );
            KEY( "ShD", pet->short_descr, fread_string( fp ) );
            break;

            if ( !fMatch )
            {
                bug( "Fread_pet: no match.", 0 );
                fread_to_eol( fp );
            }

        }
    }

}

void fread_obj( CHAR_DATA * ch, FILE * fp )
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;
    char *word;
    int iNest;
    bool fMatch = FALSE;
    bool fNest;
    bool fVnum;
    bool first;
    bool new_format;            /* to prevent errors */
    bool make_new;              /* update object */

    fVnum = FALSE;
    obj = NULL;
    first = TRUE;               /* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    word = feof( fp ) ? "End" : fread_word( fp );
    if ( !str_cmp( word, "Vnum" ) )
    {
        int vnum;
        first = FALSE;          /* fp will be in right place */

        vnum = fread_number( fp );
        if ( get_obj_index( vnum ) == NULL )
        {
            bug( "Fread_obj: bad vnum %d.", vnum );
        }
        else
        {
            obj = create_object( get_obj_index( vnum ), -1 );
            new_format = TRUE;
        }

    }

    if ( obj == NULL )          /* either not found or old style */
    {
        if ( obj_free == NULL )
        {
            obj = alloc_perm( sizeof( *obj ) );
        }
        else
        {
            obj = obj_free;
            obj_free = obj_free->next;
        }

        *obj = obj_zero;
        obj->name = str_dup( "" );
        obj->short_descr = str_dup( "" );
        obj->description = str_dup( "" );
    }

    fNest = FALSE;
    fVnum = TRUE;
    iNest = 0;

    for ( ;; )
    {
        if ( first )
            first = FALSE;
        else
            word = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER( word[0] ) )
        {
        case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;

        case 'A':
            if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "Aff" )
                 || !str_cmp( word, "AffD" ) )
            {
                AFFECT_DATA *paf;

                if ( affect_free == NULL )
                {
                    paf = alloc_perm( sizeof( *paf ) );
                }
                else
                {
                    paf = affect_free;
                    affect_free = affect_free->next;
                }

                if ( !str_cmp( word, "AffD" ) )
                {
                    int sn;
                    sn = skill_lookup( fread_word( fp ) );
                    if ( sn < 0 )
                        bug( "Fread_obj: unknown skill.", 0 );
                    else
                        paf->type = sn;
                }
                else            /* old form */
                    paf->type = fread_number( fp );
                if ( ch->version == 0 )
                    paf->level = 20;
                else
                    paf->level = fread_number( fp );
                paf->duration = fread_number( fp );
                paf->modifier = fread_number( fp );
                paf->location = fread_number( fp );
                paf->bitvector = fread_number( fp );
                paf->next = obj->affected;
                obj->affected = paf;
                fMatch = TRUE;
                break;
            }

            else if ( !str_cmp( word, "AfRd" ) )
            {
                AFFECT_DATA *paf;

                if ( affect_free == NULL )
                {
                    paf = alloc_perm( sizeof( *paf ) );
                }
                else
                {
                    paf = affect_free;
                    affect_free = affect_free->next;
                }
                paf->type = -999;
                paf->level = fread_number( fp );
                paf->duration = fread_number( fp );
                paf->modifier = fread_number( fp );
                paf->location = fread_number( fp );
                paf->bitvector = fread_number( fp );
                paf->next = obj->affected;
                obj->affected = paf;
                fMatch = TRUE;
                break;
            }

            break;

        case 'C':
            KEY( "Cost", obj->cost, fread_number( fp ) );
            break;

        case 'D':
            KEY( "Description", obj->description, fread_string( fp ) );
            KEY( "Desc", obj->description, fread_string( fp ) );
            break;

        case 'E':

            if ( !str_cmp( word, "Enchanted" ) )
            {
                obj->enchanted = TRUE;
                fMatch = TRUE;
                break;
            }

            KEY( "ExtraFlags", obj->extra_flags, fread_number( fp ) );
            KEY( "ExtF", obj->extra_flags, fread_number( fp ) );

            if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp( word, "ExDe" ) )
            {
                EXTRA_DESCR_DATA *ed;

                if ( extra_descr_free == NULL )
                {
                    ed = alloc_perm( sizeof( *ed ) );
                }
                else
                {
                    ed = extra_descr_free;
                    extra_descr_free = extra_descr_free->next;
                }

                ed->keyword = fread_string( fp );
                ed->description = fread_string( fp );
                ed->next = obj->extra_descr;
                obj->extra_descr = ed;
                fMatch = TRUE;
            }

            if ( !str_cmp( word, "End" ) )
            {
                if ( !fNest || ( fVnum && obj->pIndexData == NULL ) )
                {
                    bug( "Fread_obj: incomplete object.", 0 );
                    free_string( &obj->name );
                    free_string( &obj->description );
                    free_string( &obj->short_descr );
                    obj->next = obj_free;
                    obj_free = obj;
                    return;
                }
                else
                {
                    if ( !fVnum )
                    {
                        free_string( &obj->name );
                        free_string( &obj->description );
                        free_string( &obj->short_descr );
                        obj->next = obj_free;
                        obj_free = obj;

                        obj =
                            create_object( get_obj_index( OBJ_VNUM_DUMMY ), 0 );
                    }

                    if ( !new_format )
                    {
                        obj->next = object_list;
                        object_list = obj;
                        obj->pIndexData->count++;
                    }

                    if ( make_new )
                    {
                        int wear;

                        wear = obj->wear_loc;
                        extract_obj( obj );

                        obj = create_object( obj->pIndexData, 0 );
                        obj->wear_loc = wear;
                    }
                    if ( iNest == 0 || rgObjNest[iNest] == NULL )
                        obj_to_char( obj, ch );
                    else
                        obj_to_obj( obj, rgObjNest[iNest - 1] );
                    return;
                }
            }
            break;

        case 'I':
            KEY( "ItemType", obj->item_type, fread_number( fp ) );
            KEY( "Ityp", obj->item_type, fread_number( fp ) );
            break;

        case 'L':
            KEY( "Level", obj->level, fread_number( fp ) );
            KEY( "Lev", obj->level, fread_number( fp ) );
            break;

        case 'N':
            KEY( "Name", obj->name, fread_string( fp ) );

            if ( !str_cmp( word, "Nest" ) )
            {
                iNest = fread_number( fp );
                if ( iNest < 0 || iNest >= MAX_NEST )
                {
                    bug( "Fread_obj: bad nest %d.", iNest );
                }
                else
                {
                    rgObjNest[iNest] = obj;
                    fNest = TRUE;
                }
                fMatch = TRUE;
            }
            break;

        case 'O':
            if ( !str_cmp( word, "Oldstyle" ) )
            {
                if ( obj->pIndexData != NULL )
                    make_new = TRUE;
                fMatch = TRUE;
            }
            break;

        case 'S':
            KEY( "ShortDescr", obj->short_descr, fread_string( fp ) );
            KEY( "ShD", obj->short_descr, fread_string( fp ) );

            if ( !str_cmp( word, "Spell" ) )
            {
                int iValue;
                int sn;

                iValue = fread_number( fp );
                sn = skill_lookup( fread_word( fp ) );
                if ( iValue < 0 || iValue > 3 )
                {
                    bug( "Fread_obj: bad iValue %d.", iValue );
                }
                else if ( sn < 0 )
                {
                    bug( "Fread_obj: unknown skill.", 0 );
                }
                else
                {
                    obj->value[iValue] = sn;
                }
                fMatch = TRUE;
                break;
            }

            break;

        case 'T':
            KEY( "Timer", obj->timer, fread_number( fp ) );
            KEY( "Time", obj->timer, fread_number( fp ) );
            break;

        case 'V':
            if ( !str_cmp( word, "Values" ) || !str_cmp( word, "Vals" ) )
            {
                obj->value[0] = fread_number( fp );
                obj->value[1] = fread_number( fp );
                obj->value[2] = fread_number( fp );
                obj->value[3] = fread_number( fp );
                if ( obj->item_type == ITEM_WEAPON && obj->value[0] == 0 )
                    obj->value[0] = obj->pIndexData->value[0];
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "Val" ) )
            {
                obj->value[0] = fread_number( fp );
                obj->value[1] = fread_number( fp );
                obj->value[2] = fread_number( fp );
                obj->value[3] = fread_number( fp );
                obj->value[4] = fread_number( fp );
                fMatch = TRUE;
                break;
            }

            if ( !str_cmp( word, "Vnum" ) )
            {
                int vnum;

                vnum = fread_number( fp );
                if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
                    bug( "Fread_obj: bad vnum %d.", vnum );
                else
                    fVnum = TRUE;
                fMatch = TRUE;
                break;
            }
            break;

        case 'W':
            KEY( "WearFlags", obj->wear_flags, fread_number( fp ) );
            KEY( "WeaF", obj->wear_flags, fread_number( fp ) );
            KEY( "WearLoc", obj->wear_loc, fread_number( fp ) );
            KEY( "Wear", obj->wear_loc, fread_number( fp ) );
            KEY( "Weight", obj->weight, fread_number( fp ) );
            KEY( "Wt", obj->weight, fread_number( fp ) );
            break;

        }

        if ( !fMatch )
        {
            bug( "Fread_obj: no match.", 0 );
            fread_to_eol( fp );
        }
    }
}
