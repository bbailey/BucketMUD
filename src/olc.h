#ifndef _OLC_H_
#define _OLC_H_

/***************************************************************************
 *  File: olc.h                                                            *
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

/*
 * This is a header file for all the OLC files.  Feel free to copy it into
 * merc.h if you wish.  Many of these routines may be handy elsewhere in
 * the code.  -Jason Dinkel
 */

/**************************************************************************
 * Mudprogram's (Mobprogram, Objprogram and Roomprogram) originaly        *
 * by the SMAUG development team                                          *
 * Ported to EmberMUD by Thanatos and Tyrluk of ToED                      *
 * (Temple of Eternal Death)                                              *
 * Tyrluk   - morn@telmaron.com or dajy@mindspring.com                    *
 * Thanatos - morn@telmaron.com or jonathan_w._rose@ffic.com              *
 * Heavily modified by Zane (zane@supernova.org)                          *
 **************************************************************************/

/*
 * The version info.  Please use this info when reporting bugs.
 * It is displayed in the game by typing 'version' while editing.
 * Do not remove these from the code - by request of Jason Dinkel
 */

#define VERSION	"ILAB Online Creation [Beta 1.3]"
#define AUTHOR	"     By Jason(jdinkel@mines.colorado.edu)\n\r" \
                "     Modified for use with ROM 2.3\n\r"        \
                "     By Hans Birkeland (hansbi@ifi.uio.no)"
#define DATE	"     (Apr. 7, 1995 - ROM mod, Apr 16, 1995)"
#define CREDITS "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)"
#define ADDONS  "     Mudprogram's (Mobprogram, Objprogram and Roomprogram) originaly\n\r" \
		"     by the SMAUG development team\n\r" \
		"     Ported to EmberMUD by Thanatos and Tyrluk of ToED\n\r"  \
		"     (Temple of Eternal Death)\n\r"  \
		"     Tyrluk   - morn@telmaron.com or dajy@mindspring.com\n\r" \
		"     Thanatos - morn@telmaron.com or jonathan_w._rose@ffic.com\n\r" \
		"     Heavily modified by Zane (zane@supernova.org) "

#define CURRENT_AREA_VERSION 2

/*
 * New typedefs.
 */
typedef bool OLC_FUN(CHAR_DATA * ch, char *argument);
#define DECLARE_OLC_FUN( fun )	OLC_FUN    fun

/*
 * Structure for an OLC editor command.
 */
struct olc_cmd_type
{
    char *name;
    OLC_FUN *olc_fun;
};

void do_helpedit(CHAR_DATA * ch, char *argument);
extern const struct olc_cmd_type helpsedit_table[];

/* Function prototypes */
void helpsedit(CHAR_DATA * ch, char *argument);
bool helpsedit_show(CHAR_DATA * ch, char *argument);
bool helpsedit_keyword(CHAR_DATA * ch, char *argument);
bool helpsedit_level(CHAR_DATA * ch, char *argument);
bool helpsedit_text(CHAR_DATA * ch, char *argument);
bool helpsedit_create(CHAR_DATA * ch, char *argument);
bool helpsedit_delete(CHAR_DATA * ch, char *argument);

/*
 * Connected states for editor.
 */
#define ED_AREA 	1
#define ED_ROOM 	2
#define ED_OBJECT 	3
#define ED_MOBILE 	4
#define ED_HELP 	6
#define ED_MPROG	7
#define ED_MPGROUP	8
#define ED_CLAN		9
#define ED_FACTION  10
#define ED_HELPOLC  11
#define ED_SOCIAL  12

/*
 * Interpreter Prototypes
 */
void tedit(CHAR_DATA * ch, char *argument);
void hedit(CHAR_DATA * ch, char *argument);
void cedit(CHAR_DATA * ch, char *argument);
void save_clans(void);
void save_area_list(void);
void save_area(AREA_DATA * pArea);
void save_helps(void);
void save_mudprogs(void);
char *fix_string(const char *str);

/*
 * OLC Constants
 */
#define MAX_MOB	1		/* Default maximum number for resetting mobs */

/*
 * Structure for an OLC editor startup command.
 */
struct editor_cmd_type
{
    char *name;
    DO_FUN *do_fun;
};

/*
 * This structure is used in bit.c to lookup flags and stats.
 */
struct flag_type
{
    char *name;
    int bit;
    bool settable;
};

/*
 * Utils.
 */
AREA_DATA *get_vnum_area(int vnum);
AREA_DATA *get_area_data(int vnum);
int flag_value(const struct flag_type *flag_table, char *argument);
char *flag_string(const struct flag_type *flag_table, int bits);
void add_reset(ROOM_INDEX_DATA * room, RESET_DATA * pReset, int index);

/*
 * Interpreter Table Prototypes
 */
extern const struct olc_cmd_type cedit_table[];

/*
 * Editor Commands.
 */
void do_cedit(CHAR_DATA * ch, char *argument);

/*
 * General Functions
 */
bool show_commands(CHAR_DATA * ch, char *argument);
bool show_help(CHAR_DATA * ch, char *argument);
bool edit_done(CHAR_DATA * ch);
bool show_version(CHAR_DATA * ch, char *argument);

/*
 * Area Editor Prototypes
 */
DECLARE_OLC_FUN(aedit_show);
DECLARE_OLC_FUN(aedit_create);
DECLARE_OLC_FUN(aedit_change);
DECLARE_OLC_FUN(aedit_name);
DECLARE_OLC_FUN(aedit_file);
DECLARE_OLC_FUN(aedit_age);
/* DECLARE_OLC_FUN( aedit_recall	);       ROM OLC */
DECLARE_OLC_FUN(aedit_reset);
DECLARE_OLC_FUN(aedit_security);
DECLARE_OLC_FUN(aedit_builder);
DECLARE_OLC_FUN(aedit_vnum);
DECLARE_OLC_FUN(aedit_lvnum);
DECLARE_OLC_FUN(aedit_uvnum);

/*
 * Room Editor Prototypes
 */
DECLARE_OLC_FUN(redit_show);
DECLARE_OLC_FUN(redit_create);
DECLARE_OLC_FUN(redit_name);
DECLARE_OLC_FUN(redit_desc);
DECLARE_OLC_FUN(redit_ed);
DECLARE_OLC_FUN(redit_format);
DECLARE_OLC_FUN(redit_north);
DECLARE_OLC_FUN(redit_south);
DECLARE_OLC_FUN(redit_east);
DECLARE_OLC_FUN(redit_west);
DECLARE_OLC_FUN(redit_up);
DECLARE_OLC_FUN(redit_down);
DECLARE_OLC_FUN(redit_mreset);
DECLARE_OLC_FUN(redit_oreset);
DECLARE_OLC_FUN(redit_mfreevnums);
DECLARE_OLC_FUN(redit_rfreevnums);
DECLARE_OLC_FUN(redit_ofreevnums);
DECLARE_OLC_FUN(redit_mshow);
DECLARE_OLC_FUN(redit_oshow);

/*
 * Object Editor Prototypes
 */
DECLARE_OLC_FUN(oedit_show);
DECLARE_OLC_FUN(oedit_create);
DECLARE_OLC_FUN(oedit_name);
DECLARE_OLC_FUN(oedit_short);
DECLARE_OLC_FUN(oedit_long);
DECLARE_OLC_FUN(oedit_addaffect);
DECLARE_OLC_FUN(oedit_delaffect);
DECLARE_OLC_FUN(oedit_value0);
DECLARE_OLC_FUN(oedit_value1);
DECLARE_OLC_FUN(oedit_value2);
DECLARE_OLC_FUN(oedit_value3);
DECLARE_OLC_FUN(oedit_value4);	/* ROM */
DECLARE_OLC_FUN(oedit_weight);
DECLARE_OLC_FUN(oedit_cost);
DECLARE_OLC_FUN(oedit_ed);
DECLARE_OLC_FUN(oedit_clan);
DECLARE_OLC_FUN(oedit_extra);	/* ROM */
DECLARE_OLC_FUN(oedit_wear);	/* ROM */
DECLARE_OLC_FUN(oedit_type);	/* ROM */
DECLARE_OLC_FUN(oedit_affect);	/* ROM */
DECLARE_OLC_FUN(oedit_material);	/* ROM */
DECLARE_OLC_FUN(oedit_level);	/* ROM */
DECLARE_OLC_FUN(oedit_condition);	/* ROM */

/*
 * Mobile Editor Prototypes
 */
DECLARE_OLC_FUN(medit_show);
DECLARE_OLC_FUN(medit_create);
DECLARE_OLC_FUN(medit_name);
DECLARE_OLC_FUN(medit_short);
DECLARE_OLC_FUN(medit_long);
DECLARE_OLC_FUN(medit_shop);
DECLARE_OLC_FUN(medit_desc);
DECLARE_OLC_FUN(medit_level);
DECLARE_OLC_FUN(medit_align);
DECLARE_OLC_FUN(medit_sex);	/* ROM */
DECLARE_OLC_FUN(medit_ac);	/* ROM */
DECLARE_OLC_FUN(medit_act);	/* ROM */
DECLARE_OLC_FUN(medit_affect);	/* ROM */
DECLARE_OLC_FUN(medit_form);	/* ROM */
DECLARE_OLC_FUN(medit_part);	/* ROM */
DECLARE_OLC_FUN(medit_imm);	/* ROM */
DECLARE_OLC_FUN(medit_res);	/* ROM */
DECLARE_OLC_FUN(medit_vuln);	/* ROM */
DECLARE_OLC_FUN(medit_material);	/* ROM */
DECLARE_OLC_FUN(medit_off);	/* ROM */
DECLARE_OLC_FUN(medit_size);	/* ROM */
DECLARE_OLC_FUN(medit_hitdice);	/* ROM */
DECLARE_OLC_FUN(medit_manadice);	/* ROM */
DECLARE_OLC_FUN(medit_damdice);	/* ROM */
DECLARE_OLC_FUN(medit_damtype);	/* ROM */
DECLARE_OLC_FUN(medit_race);	/* ROM */
DECLARE_OLC_FUN(medit_position);	/* ROM */
DECLARE_OLC_FUN(medit_gold);	/* ROM */
DECLARE_OLC_FUN(medit_hitroll);	/* ROM */
DECLARE_OLC_FUN(medit_bdamage);
/*
 * MudProgs Editor Prototypes
 */
DECLARE_OLC_FUN(mpedit_assign);
DECLARE_OLC_FUN(mpedit_create);
DECLARE_OLC_FUN(mpedit_delete);
DECLARE_OLC_FUN(mpedit_description);
DECLARE_OLC_FUN(mpedit_edit);
DECLARE_OLC_FUN(mpedit_groups);
DECLARE_OLC_FUN(mpedit_name);
DECLARE_OLC_FUN(mpedit_progtype);
DECLARE_OLC_FUN(mpedit_unassign);
DECLARE_OLC_FUN(mpedit_show);
DECLARE_OLC_FUN(mpedit_trigger);
DECLARE_OLC_FUN(mpedit_triggertype);

DECLARE_OLC_FUN(mpgedit_add);
DECLARE_OLC_FUN(mpgedit_create);
DECLARE_OLC_FUN(mpgedit_delete);
DECLARE_OLC_FUN(mpgedit_description);
DECLARE_OLC_FUN(mpgedit_name);
DECLARE_OLC_FUN(mpgedit_remove);
DECLARE_OLC_FUN(mpgedit_show);

/*
 * Clan stuff!
 */
DECLARE_OLC_FUN(cedit_show);
DECLARE_OLC_FUN(cedit_create);
DECLARE_OLC_FUN(cedit_name);
DECLARE_OLC_FUN(cedit_whoname);
DECLARE_OLC_FUN(cedit_rank);
DECLARE_OLC_FUN(cedit_cost);
DECLARE_OLC_FUN(cedit_members);
DECLARE_OLC_FUN(cedit_minlevel);
DECLARE_OLC_FUN(cedit_recall);
DECLARE_OLC_FUN(cedit_leader);
DECLARE_OLC_FUN(cedit_sponsor);
DECLARE_OLC_FUN(cedit_joinflags);
DECLARE_OLC_FUN(cedit_clanflags);

/*
 * Macros
 */

#define IS_SWITCHED( ch )       ( ch->desc->original )	/* ROM OLC */

#define IS_BUILDER(ch, Area)	( ( ch->pcdata->security >= Area->security  \
				|| strstr( Area->builders, ch->name )	    \
				|| strstr( Area->builders, "All" ) )	    \
				&& !IS_SWITCHED( ch ) )

#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))

/* Return pointers to what is being edited. */
#define EDIT_MOB(Ch, Mob)	( Mob   = ( MOB_INDEX_DATA *)Ch->desc->pEdit 	  )
#define EDIT_OBJ(Ch, Obj)	( Obj   = ( OBJ_INDEX_DATA *)Ch->desc->pEdit 	  )
#define EDIT_ROOM(Ch, Room)	( Room  = ( ROOM_INDEX_DATA *)Ch->in_room    	  )
#define EDIT_AREA(Ch, Area)	( Area  = ( AREA_DATA *)Ch->desc->pEdit     	  )
#define EDIT_MPROG(Ch, Mprog)	( Mprog = ( MPROG_DATA *)Ch->desc->pEdit 	  )
#define EDIT_MPGROUP(Ch, Mpgroup) ( Mpgroup = ( MPROG_GROUP *)Ch->desc->pEdit )
#define EDIT_CLAN(Ch, Clan)	( Clan  = ( CLAN_DATA *)Ch->desc->pEdit			  )

/*
 * Prototypes
 */
/* mem.c - memory prototypes. */
#define ED	EXTRA_DESCR_DATA
RESET_DATA *new_reset_data(void);
void free_reset_data(RESET_DATA * pReset);
AREA_DATA *new_area(void);
void free_area(AREA_DATA * pArea);
EXIT_DATA *new_exit(void);
void free_exit(EXIT_DATA * pExit);
ED *new_extra_descr(void);
void free_extra_descr(ED * pExtra);
ROOM_INDEX_DATA *new_room_index(void);
void free_room_index(ROOM_INDEX_DATA * pRoom);
AFFECT_DATA *new_affect(void);
NEWAFFECT_DATA *new_newaffect(void);
void free_affect(AFFECT_DATA * pAf);
SHOP_DATA *new_shop(void);
void free_shop(SHOP_DATA * pShop);
OBJ_INDEX_DATA *new_obj_index(void);
void free_obj_index(OBJ_INDEX_DATA * pObj);
MOB_INDEX_DATA *new_mob_index(void);
void free_mob_index(MOB_INDEX_DATA * pMob);
/* Olc.c */
int mprog_count(MOB_INDEX_DATA * pMob);
MPROG_DATA *edit_mprog(CHAR_DATA * ch, MOB_INDEX_DATA * pMobProg);
void delete_mprog(CHAR_DATA * ch, int pnum);

int rprog_count(ROOM_INDEX_DATA * pRoom);
MPROG_DATA *edit_rprog(CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomProg);
void delete_rprog(CHAR_DATA * ch, int pnum);
void show_rprog(CHAR_DATA * ch, MPROG_DATA * pRoomProg);
int oprog_count(OBJ_INDEX_DATA * pObj);
MPROG_DATA *edit_oprog(CHAR_DATA * ch, OBJ_INDEX_DATA * pObjProg);
void delete_oprog(CHAR_DATA * ch, int pnum);
void show_oprog(CHAR_DATA * ch, MPROG_DATA * pObjProg);
/* Clan.c */
CLAN_DATA *new_clan(void);
char *fwrite_flag(long flags, char buf[]);
void clan_log(CLAN_DATA * clan, char *str, ...);

void do_socialedit(CHAR_DATA * ch, char *argument);
void do_socialfind(CHAR_DATA * ch, char *argument);

typedef struct sociallist_data SOCIALLIST_DATA;

extern SOCIALLIST_DATA *social_first;
extern SOCIALLIST_DATA *social_last;

extern const struct olc_cmd_type socialedit_table[];

/* Struct for list of socials available in the MUD */
struct sociallist_data
{
    sh_int vnum;
    char *name;
    char *char_no_arg;
    char *others_no_arg;
    char *char_found;
    char *others_found;
    char *vict_found;
    char *char_not_found;
    char *char_auto;
    char *others_auto;
    SOCIALLIST_DATA *next;
};

/* Function prototypes */
void load_socials(FILE * fp);
void save_socials(void);
void socialedit(CHAR_DATA * ch, char *argument);
SOCIALLIST_DATA *new_social(void);
SOCIALLIST_DATA *get_social_by_vnum(sh_int vnum);
bool socialedit_show(CHAR_DATA * ch, char *argument);
bool socialedit_create(CHAR_DATA * ch, char *argument);
bool socialedit_name(CHAR_DATA * ch, char *argument);
bool socialedit_chnoarg(CHAR_DATA * ch, char *argument);
bool socialedit_othersnoarg(CHAR_DATA * ch, char *argument);
bool socialedit_chfound(CHAR_DATA * ch, char *argument);
bool socialedit_othersfound(CHAR_DATA * ch, char *argument);
bool socialedit_victfound(CHAR_DATA * ch, char *argument);
bool socialedit_chnotfound(CHAR_DATA * ch, char *argument);
bool socialedit_chauto(CHAR_DATA * ch, char *argument);
bool socialedit_othersauto(CHAR_DATA * ch, char *argument);

#undef	ED

#endif
