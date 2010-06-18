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

/***************************************************************************
 * MudProgs has been completely rewritten by Zane.  The only thing that    *
 * remains the same is how triggers are handled.  -Zane                    *
 *                                                                         *
 * Original MobProgs by N'Atas-ha.                                         *
 ***************************************************************************/

/* this is a listing of all the commands and command related data */
#ifndef _MUD_PROGS_H_
#define _MUD_PROGS_H_

/*
 * Semi-locals
 */
void *mprog_get_actor args( ( char *arg, char type ) );

extern CHAR_DATA *ProgSource;
extern CHAR_DATA *ProgTriggeredBy;
extern CHAR_DATA *ProgVictim;
extern OBJ_DATA *ProgObjectSource;
extern OBJ_DATA *ProgObjectVictim;
extern ROOM_INDEX_DATA *ProgRoomSource;
extern CHAR_DATA *supermob;
extern int ExecMudProgID;

/*
 * Structure for a command in the command lookup table.
 */
struct mprog_cmd_type {
    char *const name;
    MPROG_FUN *mprog_fun;
    char argtype;
};

/* the command table itself */
extern const struct mprog_cmd_type mprog_cmd_table[];

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_MPROG_FUN( mprog_alignment );
DECLARE_MPROG_FUN( mprog_clan );
DECLARE_MPROG_FUN( mprog_class );
DECLARE_MPROG_FUN( mprog_crimethief );
DECLARE_MPROG_FUN( mprog_fightinroom );
DECLARE_MPROG_FUN( mprog_getrand );
DECLARE_MPROG_FUN( mprog_goldamount );
DECLARE_MPROG_FUN( mprog_hasmemory );
DECLARE_MPROG_FUN( mprog_hitpercent );
DECLARE_MPROG_FUN( mprog_hour );
DECLARE_MPROG_FUN( mprog_immune );
DECLARE_MPROG_FUN( mprog_isawake );
DECLARE_MPROG_FUN( mprog_ischarmed );
DECLARE_MPROG_FUN( mprog_isfight );
DECLARE_MPROG_FUN( mprog_isfollow );
DECLARE_MPROG_FUN( mprog_isimmort );
DECLARE_MPROG_FUN( mprog_isgood );
DECLARE_MPROG_FUN( mprog_isnpc );
DECLARE_MPROG_FUN( mprog_ispc );
DECLARE_MPROG_FUN( mprog_level );
DECLARE_MPROG_FUN( mprog_memory );
DECLARE_MPROG_FUN( mprog_mobvnum );
DECLARE_MPROG_FUN( mprog_objtype );
DECLARE_MPROG_FUN( mprog_objval0 );
DECLARE_MPROG_FUN( mprog_objval1 );
DECLARE_MPROG_FUN( mprog_objval2 );
DECLARE_MPROG_FUN( mprog_objval3 );
DECLARE_MPROG_FUN( mprog_objvnum );
DECLARE_MPROG_FUN( mprog_position );
DECLARE_MPROG_FUN( mprog_rand );
DECLARE_MPROG_FUN( mprog_roomvnum );
DECLARE_MPROG_FUN( mprog_sex );
DECLARE_MPROG_FUN( mprog_sgetrand );
DECLARE_MPROG_FUN( mprog_sreset );
DECLARE_MPROG_FUN( mprog_faction );
DECLARE_MPROG_FUN( mprog_isname );
DECLARE_MPROG_FUN( mprog_isequal );

#endif
