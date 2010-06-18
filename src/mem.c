/***************************************************************************
 *  File: mem.c                                                            *
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

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

/*
 * Globals
 */
extern int top_reset;
extern int top_area;
extern int top_exit;
extern int top_ed;
extern int top_room;
extern int top_mprog;

AREA_DATA *area_free;
EXTRA_DESCR_DATA *extra_descr_free;
EXIT_DATA *exit_free;
ROOM_INDEX_DATA *room_index_free;
OBJ_INDEX_DATA *obj_index_free;
SHOP_DATA *shop_free;
MOB_INDEX_DATA *mob_index_free;
RESET_DATA *reset_free;
HELP_DATA *help_free;

HELP_DATA *help_last;

TODO_DATA *todo_free;
TODO_DATA *todo_last;

RESET_DATA *new_reset_data( void )
{
    RESET_DATA *pReset;

    if ( !reset_free )
    {
        pReset = alloc_perm( sizeof( *pReset ) );
        top_reset++;
    }
    else
    {
        pReset = reset_free;
        reset_free = reset_free->next;
    }

    pReset->next = NULL;
    pReset->command = 'X';
    pReset->vnum = 0;
    pReset->arg2 = 0;
    pReset->arg3 = 0;
    pReset->active_count = 0;

    return pReset;
}

void free_reset_data( RESET_DATA * pReset )
{
    pReset->next = reset_free;
    reset_free = pReset;
    return;
}

 /*
  *  Removes all color codes from the given string and returns a
  *  A string with no color codes.  Used for proper string formating
  *
  *    -Thanatos
  */
char *remove_color( const char *str )
{
    char *nocolor;
    static char nocolor_buf[MAX_STRING_LENGTH];

    bzero( nocolor_buf, MAX_STRING_LENGTH );
    nocolor = nocolor_buf;

    while ( *str )
    {
        if ( *str != '`' )      /* If there's no `, add to the count  */
        {
            *nocolor++ = *str++;
            continue;
        }
        str++;                  /* If there _IS_ a `, check next char */
        switch ( *str )
        {
            /* If it's \0, count the first ` and get outta here */
        case '\0':
            *nocolor++ = '`';
            *nocolor = '\0';
            return nocolor;

            /* If it's a color code, skip over it */
        case 'k':
        case 'K':
        case 'r':
        case 'R':
        case 'b':
        case 'B':
        case 'c':
        case 'C':
        case 'y':
        case 'Y':
        case 'm':
        case 'M':
        case 'w':
        case 'W':
        case 'g':
        case 'G':
            str++;
            break;

            /* If it's not a color code, count the ` and the following char, and advance */
        default:
            *nocolor++ = '`';
            break;
        }
        continue;
    }
    return nocolor_buf;
}

MPROG_DATA *new_mudprog(  )
{
    MPROG_DATA *mprog;

    if ( !mprog_free )
    {
        mprog = ( MPROG_DATA * ) alloc_perm( sizeof( MPROG_DATA ) );
        top_mprog++;
    }
    else
    {
        mprog = mprog_free;
        mprog_free = mprog->next;
        /* mprog->next = NULL  -  added by Zak 1/3/98 */
    }

    mprog->next = NULL;
    mprog->arglist = str_dup( "0" );
    mprog->comlist = str_dup( "break\r\n" );
    mprog->name = str_dup( "(no name)" );
    mprog->description = str_dup( "(no description)" );
    mprog->prog_type = MOB_PROG;
    mprog->trigger_type = RAND_PROG;
    mprog->vnum = 0;

    return mprog;
}

void free_mudprog( MPROG_DATA * mprog )
{
    if ( mprog->name )
        free_string( &mprog->name );
    if ( mprog->description )
        free_string( &mprog->description );
    if ( mprog->arglist )
        free_string( &mprog->arglist );
    if ( mprog->comlist )
        free_string( &mprog->comlist );

    mprog->next = mprog_free;
    mprog_free = mprog;

    return;
}

MPROG_GROUP *new_mudprog_group(  )
{
    MPROG_GROUP *pMprogGroup;

    if ( !mprog_group_free )
    {
        pMprogGroup = ( MPROG_GROUP * ) alloc_perm( sizeof( *pMprogGroup ) );
        top_mprog_group++;
    }
    else
    {
        pMprogGroup = mprog_group_free;
        mprog_group_free = pMprogGroup->next;
    }

    pMprogGroup->next = NULL;
    pMprogGroup->mudprogs = NULL;
    pMprogGroup->prog_type = 0;
    pMprogGroup->vnum = 0;
    pMprogGroup->name = str_dup( "(no name)" );
    pMprogGroup->description = str_dup( "(no description)" );

    return pMprogGroup;
}

void free_mudprog_group( MPROG_GROUP * pMprogGroup )
{
    if ( pMprogGroup->name )
        free_string( &pMprogGroup->name );
    if ( pMprogGroup->description )
        free_string( &pMprogGroup->description );

    pMprogGroup->next = mprog_group_free;
    mprog_group_free = pMprogGroup;

    return;
}

AREA_DATA *new_area( void )
{
    char buf[MAX_STRING_LENGTH];
    AREA_DATA *pArea;

    if ( !area_free )
    {
        pArea = alloc_perm( sizeof( *pArea ) );
        top_area++;
    }
    else
    {
        pArea = area_free;
        area_free = area_free->next;
    }

    pArea->next = NULL;
    pArea->name = str_dup( "New area" );
/*    pArea->recall           =   ROOM_VNUM_TEMPLE;      ROM OLC */
    pArea->area_flags = AREA_ADDED;
    pArea->security = 1;
    pArea->builders = str_dup( "None" );
    pArea->lvnum = 0;
    pArea->uvnum = 0;
    pArea->age = 0;
    pArea->nplayer = 0;
    pArea->empty = TRUE;        /* ROM patch */
    sprintf( buf, "area%d.are", pArea->vnum );
    pArea->filename = str_dup( buf );
    pArea->vnum = top_area - 1;

    return pArea;
}

void free_area( AREA_DATA * pArea )
{
    free_string( &pArea->name );
    free_string( &pArea->filename );
    free_string( &pArea->builders );

    pArea->next = area_free->next;
    area_free = pArea;
    return;
}

EXIT_DATA *new_exit( void )
{
    EXIT_DATA *pExit;

    if ( !exit_free )
    {
        pExit = alloc_perm( sizeof( *pExit ) );
        top_exit++;
    }
    else
    {
        pExit = exit_free;
        exit_free = exit_free->next;
    }

    pExit->u1.to_room = NULL;
    pExit->next = NULL;
    pExit->exit_info = 0;
    pExit->key = 0;
    pExit->keyword = &str_empty[0];
    pExit->description = &str_empty[0];
    pExit->rs_flags = 0;
    pExit->orig_door = 5;

    return pExit;
}

void free_exit( EXIT_DATA * pExit )
{
    free_string( &pExit->keyword );
    free_string( &pExit->description );

    pExit->next = exit_free;
    exit_free = pExit;
    return;
}

EXTRA_DESCR_DATA *new_extra_descr( void )
{
    EXTRA_DESCR_DATA *pExtra;

    if ( !extra_descr_free )
    {
        pExtra = alloc_perm( sizeof( *pExtra ) );
        top_ed++;
    }
    else
    {
        pExtra = extra_descr_free;
        extra_descr_free = extra_descr_free->next;
    }

    pExtra->keyword = NULL;
    pExtra->description = NULL;
    pExtra->next = NULL;

    return pExtra;
}

void free_extra_descr( EXTRA_DESCR_DATA * pExtra )
{
    free_string( &pExtra->keyword );
    free_string( &pExtra->description );

    pExtra->next = extra_descr_free;
    extra_descr_free = pExtra;
    return;
}

ROOM_INDEX_DATA *new_room_index( void )
{
    ROOM_INDEX_DATA *pRoom;
    int door;

    if ( !room_index_free )
    {
        pRoom = alloc_perm( sizeof( *pRoom ) );
        top_room++;
    }
    else
    {
        pRoom = room_index_free;
        room_index_free = room_index_free->next;
    }

    pRoom->next = NULL;
    pRoom->people = NULL;
    pRoom->contents = NULL;
    pRoom->extra_descr = NULL;
    pRoom->area = NULL;

    for ( door = 0; door < MAX_DIR; door++ )
        pRoom->exit[door] = NULL;

    pRoom->name = &str_empty[0];
    pRoom->description = &str_empty[0];
    pRoom->vnum = 0;
    pRoom->room_flags = 0;
    pRoom->light = 0;
    pRoom->sector_type = 0;

    return pRoom;
}

void free_room_index( ROOM_INDEX_DATA * pRoom )
{
    int door;
    EXTRA_DESCR_DATA *pExtra;
    RESET_DATA *pReset;
    MPROG_LIST *pMprogList;
    MPROG_GROUP_LIST *pMprogGroupList;

    free_string( &pRoom->name );
    free_string( &pRoom->description );

    for ( door = 0; door < MAX_DIR; door++ )
        if ( pRoom->exit[door] )
            free_exit( pRoom->exit[door] );

    for ( pExtra = pRoom->extra_descr; pExtra; pExtra = pExtra->next )
        free_extra_descr( pExtra );

    for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next )
        free_reset_data( pReset );

    for ( pMprogList = pRoom->mudprogs; pMprogList;
          pMprogList = pMprogList->next )
        free_mem( &pMprogList );

    for ( pMprogGroupList = pRoom->mprog_groups; pMprogGroupList;
          pMprogGroupList = pMprogGroupList->next )
        free_mem( &pMprogGroupList );

    pRoom->next = room_index_free;
    room_index_free = pRoom;
    return;
}

AFFECT_DATA *new_affect( void )
{
    AFFECT_DATA *pAf;

    if ( !affect_free )
    {
        pAf = alloc_perm( sizeof( *pAf ) );
        top_affect++;
    }
    else
    {
        pAf = affect_free;
        affect_free = affect_free->next;
    }

    pAf->next = NULL;
    pAf->location = 0;
    pAf->modifier = 0;
    pAf->type = 0;
    pAf->duration = 0;
    pAf->bitvector = 0;

    return pAf;
}

void free_affect( AFFECT_DATA * pAf )
{
    pAf->next = affect_free;
    affect_free = pAf;
    return;
}

NEWAFFECT_DATA *new_newaffect( void )
{
    NEWAFFECT_DATA *npAf;

    if ( !newaffect_free )
    {
        npAf = alloc_perm( sizeof( *npAf ) );
        top_newaffect++;
    }
    else
    {
        npAf = newaffect_free;
        newaffect_free = newaffect_free->next;
    }

    npAf->next = NULL;
    npAf->location = 0;
    npAf->modifier = 0;
    npAf->type = 0;
    npAf->duration = 0;
    npAf->bitvector = 0;

    return npAf;
}

void free_newaffect( NEWAFFECT_DATA * npAf )
{
    npAf->next = newaffect_free;
    newaffect_free = npAf;
    return;
}

SHOP_DATA *new_shop( void )
{
    SHOP_DATA *pShop;
    int buy;

    if ( !shop_free )
    {
        pShop = alloc_perm( sizeof( *pShop ) );
        top_shop++;
    }
    else
    {
        pShop = shop_free;
        shop_free = shop_free->next;
    }

    pShop->next = NULL;
    pShop->keeper = 0;

    for ( buy = 0; buy < MAX_TRADE; buy++ )
        pShop->buy_type[buy] = 0;

    pShop->profit_buy = 100;
    pShop->profit_sell = 100;
    pShop->open_hour = 0;
    pShop->close_hour = 23;

    return pShop;
}

void free_shop( SHOP_DATA * pShop )
{
    pShop->next = shop_free;
    shop_free = pShop;
    return;
}

OBJ_INDEX_DATA *new_obj_index( void )
{
    OBJ_INDEX_DATA *pObj;
    int value;

    if ( !obj_index_free )
    {
        pObj = alloc_perm( sizeof( *pObj ) );
        top_obj_index++;
    }
    else
    {
        pObj = obj_index_free;
        obj_index_free = obj_index_free->next;
    }

    pObj->next = NULL;
    pObj->extra_descr = NULL;
    pObj->affected = NULL;
    pObj->area = NULL;
    pObj->name = str_dup( "(no name)" );
    pObj->short_descr = str_dup( "(no short description)" );
    pObj->description = str_dup( "(no description)" );
    pObj->vnum = 0;
    pObj->item_type = ITEM_TRASH;
    pObj->extra_flags = 0;
    pObj->wear_flags = 0;
    pObj->count = 0;
    pObj->weight = 0;
    pObj->cost = 0;
    pObj->material = material_lookup( "" ); /* ROM */
    pObj->condition = 100;      /* ROM */
    for ( value = 0; value < 5; value++ )   /* 5 - ROM */
        pObj->value[value] = 0;

    return pObj;
}

void free_obj_index( OBJ_INDEX_DATA * pObj )
{
    EXTRA_DESCR_DATA *pExtra;
    MPROG_LIST *pMprogList;
    MPROG_GROUP_LIST *pMprogGroupList;
    AFFECT_DATA *pAf;

    free_string( &pObj->name );
    free_string( &pObj->short_descr );
    free_string( &pObj->description );

    for ( pAf = pObj->affected; pAf; pAf = pAf->next )
        free_affect( pAf );

    for ( pExtra = pObj->extra_descr; pExtra; pExtra = pExtra->next )
        free_extra_descr( pExtra );

    for ( pMprogList = pObj->mudprogs; pMprogList;
          pMprogList = pMprogList->next )
        free_mem( &pMprogList );

    for ( pMprogGroupList = pObj->mprog_groups; pMprogGroupList;
          pMprogGroupList = pMprogGroupList->next )
        free_mem( &pMprogGroupList );

    pObj->next = obj_index_free;
    obj_index_free = pObj;
    return;
}

MOB_INDEX_DATA *new_mob_index( void )
{
    MOB_INDEX_DATA *pMob;

    if ( !mob_index_free )
    {
        pMob = alloc_perm( sizeof( *pMob ) );
        top_mob_index++;
    }
    else
    {
        pMob = mob_index_free;
        mob_index_free = mob_index_free->next;
    }

    pMob->next = NULL;
    pMob->pShop = NULL;
    pMob->area = NULL;
    pMob->player_name = str_dup( "no name" );
    pMob->short_descr = str_dup( "(no short description)" );
    pMob->long_descr = str_dup( "(no long description)\n\r" );
    pMob->description = &str_empty[0];
    pMob->vnum = 0;
    pMob->count = 0;
    pMob->killed = 0;
    pMob->sex = 0;
    pMob->level = 0;
    pMob->act = ACT_IS_NPC;
    pMob->affected_by = 0;
    memset( pMob->newaff, 0,
            ( MAX_NEWAFF_BIT / 8 ) + ( MAX_NEWAFF_BIT % 8 ? 1 : 0 ) );
    pMob->alignment = 0;
    pMob->hitroll = 0;
    pMob->race = race_lookup( "human" );    /* - Hugin */
    pMob->form = 0;             /* ROM patch -- Hugin */
    pMob->parts = 0;            /* ROM patch -- Hugin */
    pMob->imm_flags = 0;        /* ROM patch -- Hugin */
    pMob->res_flags = 0;        /* ROM patch -- Hugin */
    pMob->vuln_flags = 0;       /* ROM patch -- Hugin */
    pMob->material = material_lookup( "" ); /* -- Hugin */
    pMob->off_flags = 0;        /* ROM patch -- Hugin */
    pMob->size = SIZE_MEDIUM;   /* ROM patch -- Hugin */
    pMob->ac[AC_PIERCE] = 0;    /* ROM patch -- Hugin */
    pMob->ac[AC_BASH] = 0;      /* ROM patch -- Hugin */
    pMob->ac[AC_SLASH] = 0;     /* ROM patch -- Hugin */
    pMob->ac[AC_EXOTIC] = 0;    /* ROM patch -- Hugin */
    pMob->hit[DICE_NUMBER] = 0; /* ROM patch -- Hugin */
    pMob->hit[DICE_TYPE] = 0;   /* ROM patch -- Hugin */
    pMob->hit[DICE_BONUS] = 0;  /* ROM patch -- Hugin */
    pMob->mana[DICE_NUMBER] = 0;    /* ROM patch -- Hugin */
    pMob->mana[DICE_TYPE] = 0;  /* ROM patch -- Hugin */
    pMob->mana[DICE_BONUS] = 0; /* ROM patch -- Hugin */
    pMob->damage[DICE_NUMBER] = 0;  /* ROM patch -- Hugin */
    pMob->damage[DICE_TYPE] = 0;    /* ROM patch -- Hugin */
    pMob->damage[DICE_NUMBER] = 0;  /* ROM patch -- Hugin */
    pMob->start_pos = POS_STANDING; /*  -- Hugin */
    pMob->default_pos = POS_STANDING;   /*  -- Hugin */
    pMob->gold = 0;
    pMob->path_move = FALSE;
    pMob->path_pos = 0;

    return pMob;
}

void free_mob_index( MOB_INDEX_DATA * pMob )
{
    MPROG_LIST *pMprogList;
    MPROG_GROUP_LIST *pMprogGroupList;

    free_string( &pMob->player_name );
    free_string( &pMob->short_descr );
    free_string( &pMob->long_descr );
    free_string( &pMob->description );

    free_shop( pMob->pShop );

    for ( pMprogList = pMob->mudprogs; pMprogList;
          pMprogList = pMprogList->next )
        free_mem( &pMprogList );

    for ( pMprogGroupList = pMob->mprog_groups; pMprogGroupList;
          pMprogGroupList = pMprogGroupList->next )
        free_mem( &pMprogGroupList );

    pMob->next = mob_index_free;
    mob_index_free = pMob;
    return;
}
