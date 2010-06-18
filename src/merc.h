#define EMBER_MUD_VERSION         "`WEmberMUD Release 0.9.47`w\n\r"

#ifndef merc_h__
#define merc_h__
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

/*
 * Accommodate old non-Ansi compilers.
 */

#if defined(WIN32)
#include <stdio.h>
#if defined(unix)
#undef unix
#endif
#endif
#if defined(TRADITIONAL)
#define const
#define args( list )                    ( )
#define DECLARE_DO_FUN( fun )           void fun( )
#define DECLARE_SPELL_FUN( fun )        void fun( )
#define DECLARE_MPROG_FUN( fun )        int  fun( )
#else
#define args( list )                    list
#define DECLARE_DO_FUN( fun )           DO_FUN    fun
#define DECLARE_SPELL_FUN( fun )        SPELL_FUN fun
#define DECLARE_MPROG_FUN( fun )        MPROG_FUN fun
#endif

/* system calls */
#ifndef cbuilder
int unlink(  );
int system(  );
#endif

/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if     !defined(FALSE)
#define FALSE    0
#endif

#if     !defined(TRUE)
#define TRUE     1
#endif

#if     defined(_AIX)
#if     !defined(const)
#define const
#endif
typedef int sh_int;
typedef int bool;
#define unix
#else
typedef short int sh_int;
#if !defined(CPP)
typedef unsigned char bool;
#endif
#endif

/*
 * Structure types.
 */
typedef struct affect_data AFFECT_DATA;
typedef struct newaffect_data NEWAFFECT_DATA;
typedef struct area_data AREA_DATA;
typedef struct auction_data AUCTION_DATA;   /* Added for automated auction. -Lancelight */
typedef struct ban_data BAN_DATA;
typedef struct buf_type BUFFER; /* Added for recycle.h -Lancelight */
typedef struct char_data CHAR_DATA;
typedef struct descriptor_data DESCRIPTOR_DATA;
typedef struct exit_data EXIT_DATA;
typedef struct extra_descr_data EXTRA_DESCR_DATA;
typedef struct help_data HELP_DATA;
typedef struct todo_data TODO_DATA;
typedef struct kill_data KILL_DATA;
typedef struct mob_index_data MOB_INDEX_DATA;
typedef struct note_data NOTE_DATA;
typedef struct obj_data OBJ_DATA;
typedef struct obj_index_data OBJ_INDEX_DATA;
typedef struct pc_data PC_DATA;
typedef struct gen_data GEN_DATA;
typedef struct reset_data RESET_DATA;
typedef struct room_index_data ROOM_INDEX_DATA;
typedef struct shop_data SHOP_DATA;
typedef struct time_info_data TIME_INFO_DATA;
typedef struct weather_data WEATHER_DATA;
typedef struct clan_data CLAN_DATA;
typedef struct mud_prog_data MPROG_DATA;
typedef struct mud_prog_act_list MPROG_ACT_LIST;
typedef struct mud_prog_group MPROG_GROUP;
typedef struct mud_prog_list MPROG_LIST;
typedef struct mud_prog_group_list MPROG_GROUP_LIST;
typedef struct queue_data QUEUE_DATA;
typedef struct last_data LAST_DATA;
typedef struct disabled_data DISABLED_DATA;
typedef struct system_config SYS_CONFIG;
typedef struct hate_data HATE_DATA;
typedef struct race_type RACE_DATA;
typedef struct immcmd_type IMMCMD_TYPE;
typedef struct do_skills_data DO_SKILLS_DATA;
typedef struct do_gain_list_data DO_GAIN_LIST_DATA;

int count_users args( ( OBJ_DATA * obj ) ); /* count users on/in an
                                               obj -Lancelight */
/*
 * Function types.
 */
typedef void DO_FUN args( ( CHAR_DATA * ch, char *argument ) );
typedef void SPELL_FUN args( ( int sn, int level, CHAR_DATA * ch, void *vo ) );
typedef int MPROG_FUN args( ( void *arg ) );

#include "board.h"
#include "config.h"
#include "factions.h"
#include "newbits.h"
#include <signal.h>

/* Just a note....
   I have to take this out to get it to compile on my Linux system.
   I don't know why, but nothing happens when I remove it...
	-Kyle
 */
#if defined(unix)
#include <unistd.h>
#endif

#include <stdarg.h>

struct do_gain_list_data
{
    char *name;
    int level;
    int cost;
};

struct do_skills_data
{
    char *name;
    int level;
    int learned;
};

struct clan_data {
    CLAN_DATA *next;
    sh_int number;
    char *name;
    char *whoname;
    char *leader;
    char *members[MAX_CLAN_MEMBERS];
    char *god;
    sh_int max_members;
    sh_int min_level;
    sh_int num_members;
    long clan_flags;
    long join_flags;
    int cost_gold;
    sh_int recall_room;
    sh_int kills[MAX_CLAN];
    sh_int deaths[MAX_CLAN];
    char *command;
    char *skill;
    char *rank[MAX_RANK];
    int clan_fund;
};

/* Local TODO structures -Lancelight */
struct todo_data {
    TODO_DATA *next;
    sh_int level;
    char *keyword;
    char *text;
};

/* one disabled command */
struct disabled_data {
    DISABLED_DATA *next;        /* pointer to next node */
    struct cmd_type const *command; /* pointer to the command struct */
    char *disabled_by;          /* name of disabler */
    sh_int level;               /* level of disabler */
};
/*
 * Drunk struct
 */

struct struckdrunk {
    int min_drunk_level;
    int number_of_rep;
    char *replacement[11];
};

/*
 * Site ban structure.
 * Added for the new ban functions. -Lancelight
 */
struct ban_data {
    BAN_DATA *next;
    bool valid;
    int ban_flags;
    int level;
    char *name;
};

/* The next couple of buffer structs were added for recyle.h -Lancelight */

struct buf_type {
    BUFFER *next;
    bool valid;
    sh_int state;               /* error state of the buffer */
    sh_int size;                /* size in k */
    char *string;               /* buffer's string */
};

/* buffer procedures */

BUFFER *new_buf args( ( void ) );
BUFFER *new_buf_size args( ( int size ) );
void free_buf args( ( BUFFER * buffer ) );
bool add_buf args( ( BUFFER * buffer, char *string ) );
void clear_buf args( ( BUFFER * buffer ) );
char *buf_string args( ( BUFFER * buffer ) );

/* Class function needed by db.c -Lancelight*/
void load_classes( void );

struct time_info_data {
    int hour;
    int day;
    int month;
    int year;
};

struct weather_data {
    int mmhg;
    int change;
    int sky;
    int sunlight;
};

/*
 * Descriptor (channel) structure.
 */
struct descriptor_data {
    DESCRIPTOR_DATA *next;
    DESCRIPTOR_DATA *snoop_by;
    CHAR_DATA *character;
    CHAR_DATA *original;
    char *host;
    sh_int descriptor;
    sh_int connected;
    bool fcommand;
    bool ansi;                  /* ANSI COLOR ENABLED? */
    char inbuf[4 * MAX_INPUT_LENGTH];
    char incomm[MAX_INPUT_LENGTH];
    char inlast[MAX_INPUT_LENGTH];
    int repeat;
    char *outbuf;
    int outsize;
    int outtop;
    char *showstr_head;
    char *showstr_point;
    void *pEdit;                /* OLC */
    char **pString;             /* OLC */
    int editor;                 /* OLC */
};

/*Auction Structure. 
Uncomment silver_held if you use dual monatry systems -Lancelight*/

struct auction_data {
    OBJ_DATA *item;
    CHAR_DATA *owner;
    CHAR_DATA *high_bidder;
    sh_int status;
    sh_int oldtimer;
    long current_bid;
/*        long            silver_held;*/
    long gold_held;
};
/*
 * Attribute bonus structures.
 */
struct str_app_type {
    sh_int tohit;
    sh_int todam;
    sh_int carry;
    sh_int wield;
};

struct int_app_type {
    sh_int learn;
};

struct wis_app_type {
    sh_int practice;
};

struct dex_app_type {
    sh_int defensive;
};

struct con_app_type {
    sh_int hitp;
    sh_int shock;
};

/*
 * Help table types.
 */
struct help_data {
    HELP_DATA *next;
    sh_int level;
    char *keyword;
    char *text;
};

struct shop_data {
    SHOP_DATA *next;            /* Next shop in list            */
    sh_int keeper;              /* Vnum of shop keeper mob      */
    sh_int buy_type[MAX_TRADE]; /* Item types shop will buy     */
    sh_int profit_buy;          /* Cost multiplier for buying   */
    sh_int profit_sell;         /* Cost multiplier for selling  */
    sh_int open_hour;           /* First opening hour           */
    sh_int close_hour;          /* First closing hour           */
};

struct class_type {
    char *name;                 /* the full name of the class */
    char who_name[4];           /* Three-letter name for 'who'  */
    sh_int attr_prime;          /* Prime attribute              */
    sh_int weapon;              /* First weapon                 */
    sh_int guild[MAX_GUILD];    /* Vnum of guild rooms          */
    sh_int skill_adept;         /* Maximum skill level          */
    sh_int thac0_00;            /* Thac0 for level  0           */
    sh_int thac0_32;            /* Thac0 for level 32           */
    sh_int hp_min;              /* Min hp gained on leveling    */
    sh_int hp_max;              /* Max hp gained on leveling    */
    bool fMana;                 /* Class gains mana on level    */
    char *base_group;           /* base skills gained           */
    char *default_group;        /* default skills gained        */
    bool remort_class;          /* is this a remort only class? */
};

struct attack_type {
    char *name;                 /* name and message */
    int damage;                 /* damage class */
};

struct race_type {
    char *name;                 /* call name of the race */
    bool pc_race;               /* can be chosen by pcs */
    long act;                   /* act bits for the race */
    long aff;                   /* aff bits for the race */
    long off;                   /* off bits for the race */
    long imm;                   /* imm bits for the race */
    long res;                   /* res bits for the race */
    long vuln;                  /* vuln bits for the race */
    long form;                  /* default form flag for the race */
    long parts;                 /* default parts for the race */
    bool remort_race;           /* is a remort only race */
};

struct pc_race_type             /* additional data for pc races */
{
    char *name;                 /* MUST be in race_type */
    char who_name[11];          /* changed to 11  -Kyle */
    sh_int points;              /* cost in points of the race */
    sh_int class_mult[MAX_CLASS];   /* exp multiplier for class, * 100 */
    char *skills[5];            /* bonus skills for the race */
    sh_int stats[MAX_STATS];    /* starting stats */
    sh_int max_stats[MAX_STATS];    /* maximum stats */
    sh_int size;                /* aff bits for the race */
};

/*
 * Data structure for notes.
 */
struct note_data {
    NOTE_DATA *next;
    char *sender;
    char *date;
    char *to_list;
    char *subject;
    char *text;
    time_t date_stamp;
    time_t expire;
};

/*
 * An affect.
 */
struct affect_data {
    AFFECT_DATA *next;
    sh_int type;
    sh_int level;
    sh_int duration;
    sh_int location;
    sh_int modifier;
    int bitvector;
};

struct newaffect_data {
    NEWAFFECT_DATA *next;
    sh_int type;
    sh_int level;
    sh_int duration;
    sh_int location;
    sh_int modifier;
    int bitvector;              /* Soon to be removed */
};

/*
 * A kill structure (indexed by level).
 */
struct kill_data {
    sh_int number;
    sh_int killed;
};

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct mob_index_data {
    MOB_INDEX_DATA *next;
    SHOP_DATA *pShop;
    AREA_DATA *area;            /* OLC */
    sh_int vnum;
    sh_int count;
    sh_int killed;
    char *player_name;
    char *short_descr;
    char *long_descr;
    char *description;
    long act;
    long affected_by;
    char newaff[( MAX_NEWAFF_BIT / 8 ) + ( MAX_NEWAFF_BIT % 8 ? 1 : 0 )];
    sh_int alignment;
    sh_int level;
    sh_int hitroll;
    sh_int hit[3];
    sh_int mana[3];
    sh_int damage[3];
    sh_int ac[4];
    sh_int dam_type;
    long off_flags;
    long imm_flags;
    long res_flags;
    long vuln_flags;
    sh_int start_pos;
    sh_int default_pos;
    sh_int sex;
    sh_int race;
    long gold;
    long form;
    long parts;
    sh_int size;
    sh_int material;
    MPROG_LIST *mudprogs;
    MPROG_GROUP_LIST *mprog_groups;
    int progtypes;
    sh_int breath_percent;
    sh_int rnd_obj_percent;
    sh_int rnd_obj_num;
    long rnd_obj_types;
    unsigned int path_pos;
    bool path_move;
    FACTIONAFF_DATA *faction_affs;
};
/* Used for formating up the vnum command -Lancelight */
extern OBJ_INDEX_DATA *obj_first;
extern OBJ_INDEX_DATA *obj_last;

/*
 * One character (PC or NPC).
 */
struct char_data {
    CHAR_DATA *next;
    CHAR_DATA *next_player;
    CHAR_DATA *next_in_room;
    CHAR_DATA *next_in_shell;
    CHAR_DATA *master;
    CHAR_DATA *leader;
    CHAR_DATA *fighting;
    CHAR_DATA *reply;
    CHAR_DATA *pet;
    CHAR_DATA *memory;
    HATE_DATA *hate;
    MOB_INDEX_DATA *pIndexData;
    DESCRIPTOR_DATA *desc;
    AFFECT_DATA *affected;
    NEWAFFECT_DATA *newaffected;
    OBJ_DATA *carrying;
    OBJ_DATA *on;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *was_in_room;
    PC_DATA *pcdata;
    GEN_DATA *gen_data;
    char *name;
    bool beep;
    bool anonymous;
    sh_int version;
    char *short_descr;
    char *long_descr;
    char *description;
    char *temp_todo;
    sh_int sex;
    sh_int Class;
    sh_int race;
    sh_int level;
    sh_int trust;
    int played;
    int lines;                  /* for the pager */
    time_t logon;
    time_t last_note;
    sh_int timer;
    sh_int wait;
    sh_int daze;
    sh_int hit;
    sh_int max_hit;
    sh_int mana;
    sh_int max_mana;
    sh_int move;
    sh_int max_move;
    long gold;
    long exp;
    long exp_stack;
    long act;
    long comm;                  /* RT added to pad the vector */
    long imm_flags;
    long res_flags;
    long vuln_flags;
    sh_int invis_level;
    long affected_by;
    char newaff[( MAX_NEWAFF_BIT / 8 ) + ( MAX_NEWAFF_BIT % 8 ? 1 : 0 )];
    sh_int position;
    sh_int practice;
    sh_int train;
    sh_int qp;
    sh_int incarnations;
    sh_int carry_weight;
    sh_int carry_number;
    sh_int saving_throw;
    sh_int alignment;
    sh_int hitroll;
    sh_int breath_percent;
    sh_int rnd_obj_percent;
    sh_int rnd_obj_num;
    long rnd_obj_types;
    sh_int damroll;
    sh_int armor[4];
    sh_int wimpy;
    /* stats */
    sh_int perm_stat[MAX_STATS];
    sh_int mod_stat[MAX_STATS];
    /* parts stuff */
    long form;
    long parts;
    sh_int size;
    sh_int material;
    /* mobile stuff */
    long off_flags;
    sh_int damage[3];
    sh_int dam_type;
    sh_int start_pos;
    sh_int default_pos;
    MPROG_ACT_LIST *mpact;
    int mpactnum;
    int mobinvis;
    sh_int jail_timer;
    int fdpair[2];
    /* mob counter for a reset */
    sh_int *reset_count;
#if defined(cbuilder)
    void *TNode;
#endif

};

struct mud_prog_act_list {
    MPROG_ACT_LIST *next;
    char *buf;
    CHAR_DATA *ch;
    OBJ_DATA *obj;
    void *vo;
};

struct mud_prog_data {
    MPROG_DATA *next;
    char *name;
    char *description;
    sh_int vnum;
    int prog_type;
    int trigger_type;
    char *arglist;
    char *comlist;
};

struct mud_prog_group {
    MPROG_GROUP *next;
    MPROG_LIST *mudprogs;
    char *name;
    char *description;
    sh_int vnum;
    int prog_type;
};

struct mud_prog_list {
    MPROG_LIST *next;
    MPROG_DATA *mudprog;
};

struct mud_prog_group_list {
    MPROG_GROUP_LIST *next;
    MPROG_GROUP *mprog_group;
};

extern bool MOBtrigger;

struct hate_data {
    char *name;
    CHAR_DATA *who;
};

/*
 * Data which only PC's have.
 */
struct pc_data {
    PC_DATA *next;
    BUFFER *buffer;             /* Added for recycle.h -Lancelight */
    QUEUE_DATA *fmessage;
    QUEUE_DATA *message;
    QUEUE_DATA *ftell_q;
    QUEUE_DATA *tell_q;
    BOARD_DATA *board;          /* The current board */
    time_t last_note[MAX_BOARD];    /* last note for the boards */
    NOTE_DATA *in_progress;
    int messages;
    int tells;
    char *pwd;
    char *bamfin;
    char *bamfout;
    char *title;
    sh_int perm_hit;
    sh_int perm_mana;
    sh_int perm_move;
    sh_int true_sex;
    int last_level;
    int security;
    sh_int condition[3];
    sh_int learned[MAX_SKILL];
    bool group_known[MAX_GROUP];
    sh_int points;
    bool confirm_delete;
    bool confirm_remort;
    bool confirm_pk;
    char *prompt;
    ROOM_INDEX_DATA *recall_room;
    int chaos_score;
    char *alias[MAX_ALIAS];
    char *alias_sub[MAX_ALIAS];
    char *nemesis;
    char *comment;
    char *email;
    int pk_deaths;
    int pk_kills;
    int vnum_range[2];
    sh_int clan;
    sh_int join_status;
    CHAR_DATA *clan_ch;
    sh_int join_clan;
    sh_int clan_rank;
    char *who_race;
    char *who_prefix;
    long gold_bank;
    int tick;
    int ticks;
    int spousec;
    char *spouse;
    FACTIONPC_DATA *faction_standings;
    IMMCMD_TYPE *immcmdlist;
};

/* Data for generating characters -- only used during generation */
struct gen_data {
    GEN_DATA *next;
    bool skill_chosen[MAX_SKILL];
    bool group_chosen[MAX_GROUP];
    int points_chosen;
};

/* Message queue data */
struct queue_data {
    QUEUE_DATA *next;
    char *text;
    char *sender;
    sh_int wizilvl;
    bool invis;
    bool group;                 /* was it a group tell? */
};

/* Last Messages list */
struct last_data {
    LAST_DATA *next;
    char *msg;                  /* Text being stored.   */
    char *format;               /* act() format.        */
    bool invis;                 /* Sender invis?        */
    sh_int wizilvl;             /* Sender's wizi level. */
    char *sender;               /* Who was the sender?  */
};

/* A structure to hold all configuration variables read in from the config file */
struct system_config {
    /* Directories */
    char *area_dir;
    char *player_dir;
    char *player_temp;
    char *god_dir;
    char *clan_dir;
    char *note_dir;
    char *log_dir;
    char *class_dir;

    /* Files */
    char *area_list;
    char *help_file;
    char *todo_file;
    char *mudprogs_file;
    char *bug_file;
    char *idea_file;
    char *typo_file;
    char *shutdown_file;
    char *chaos_file;
    char *clans_file;
    char *factions_file;
    char *socials_file;
    char *help_log_file;
    char *ban_file;
    char *disable_file;
};

struct liq_type {
    char *liq_name;
    char *liq_color;
    sh_int liq_affect[3];
};

/*
 * Extra description data for a room or object.
 */
struct extra_descr_data {
    EXTRA_DESCR_DATA *next;     /* Next in list                     */
    char *keyword;              /* Keyword in look/examine          */
    char *description;          /* What to see                      */
};

/*
 * Prototype for an object.
 */
struct obj_index_data {
    OBJ_INDEX_DATA *next;
    EXTRA_DESCR_DATA *extra_descr;
    AFFECT_DATA *affected;
    AREA_DATA *area;            /* OLC */
    MPROG_LIST *mudprogs;       /* mudprogs */
    MPROG_GROUP_LIST *mprog_groups;
    int progtypes;              /* mudprogs */
    char *name;
    char *short_descr;
    char *description;
    sh_int vnum;
    sh_int reset_num;
    sh_int material;
    sh_int item_type;
    int extra_flags;
    sh_int wear_flags;
    sh_int level;
    sh_int condition;
    sh_int count;
    sh_int weight;
    int cost;
    int value[5];
    sh_int clan;
};

/*
 * One object.
 */
struct obj_data {
    OBJ_DATA *next;
    OBJ_DATA *next_content;
    OBJ_DATA *contains;
    OBJ_DATA *in_obj;
    OBJ_DATA *on;
    CHAR_DATA *carried_by;
    EXTRA_DESCR_DATA *extra_descr;
    AFFECT_DATA *affected;
    OBJ_INDEX_DATA *pIndexData;
    ROOM_INDEX_DATA *in_room;
    MPROG_ACT_LIST *mpact;      /* mudprogs */
    int mpactnum;               /* mudprogs */
    bool enchanted;
    char *owner;
    char *name;
    char *short_descr;
    char *description;
    sh_int item_type;
    int extra_flags;
    sh_int wear_flags;
    sh_int wear_loc;
    sh_int weight;
    int cost;
    sh_int level;
    sh_int condition;
    sh_int material;
    sh_int timer;
    int value[5];
    sh_int clan;
};

/*
 * Exit data.
 */
struct exit_data {
    union {
        ROOM_INDEX_DATA *to_room;
        sh_int vnum;
    } u1;
    EXIT_DATA *next;            /* OLC */
    int rs_flags;               /* OLC */
    int orig_door;              /* OLC */
    sh_int exit_info;
    sh_int key;
    char *keyword;
    char *description;
};

/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct reset_data {
    RESET_DATA *next;
    char command;
    sh_int vnum;
    sh_int arg2;
    sh_int arg3;
    sh_int active_count;
};

/*
 * Area definition.
 */
struct area_data {
    AREA_DATA *next;
    RESET_DATA *reset_first;
    RESET_DATA *reset_last;
    char *name;
    sh_int age;
    sh_int nplayer;
    bool empty;                 /* ROM OLC */
    char *filename;             /* OLC */
    char *builders;             /* OLC *//* Listing of */
    int security;               /* OLC *//* Value 1-9  */
    int lvnum;                  /* OLC *//* Lower vnum */
    int uvnum;                  /* OLC *//* Upper vnum */
    int vnum;                   /* OLC *//* Area vnum  */
    int area_flags;             /* OLC */
};

/*
 * Room type.
 */
struct room_index_data {
    ROOM_INDEX_DATA *next;
    RESET_DATA *reset_first;    /* OLC */
    RESET_DATA *reset_last;     /* OLC */
    CHAR_DATA *people;
    OBJ_DATA *contents;
    EXTRA_DESCR_DATA *extra_descr;
    AREA_DATA *area;
    EXIT_DATA *exit[6];
    char *name;
    char *description;
    sh_int vnum;
    int room_flags;
    sh_int light;
    sh_int sector_type;
    MPROG_ACT_LIST *mpact;      /* mudprogs */
    int mpactnum;               /* mudprogs */
    MPROG_LIST *mudprogs;
    MPROG_GROUP_LIST *mprog_groups;
    int progtypes;
};

/*
 * Skills include spells as a particular case.
 */
struct skill_type {
    char *name;                 /* Name of skill                */
    sh_int skill_level[MAX_CLASS];  /* Level needed by class        */
    sh_int rating[MAX_CLASS];   /* How hard it is to learn      */
    SPELL_FUN *spell_fun;       /* Spell pointer (for spells)   */
    sh_int target;              /* Legal targets                */
    sh_int minimum_position;    /* Position for caster / user   */
    sh_int *pgsn;               /* Pointer to associated gsn    */
    sh_int slot;                /* Slot for #OBJECT loading     */
    sh_int min_mana;            /* Minimum mana used            */
    sh_int beats;               /* Waiting time after use       */
    char *noun_damage;          /* Damage message               */
    char *msg_off;              /* Wear off message             */
};

struct group_type {
    char *name;
    sh_int rating[MAX_CLASS];
    char *spells[MAX_IN_GROUP];
};

/*
 * These are skill_lookup return values for common skills and spells.
 */
extern sh_int gsn_backstab;
extern sh_int gsn_circle;
extern sh_int gsn_counter;
extern sh_int gsn_grip;
extern sh_int gsn_blind_fighting;
extern sh_int gsn_shield_cleave;
extern sh_int gsn_weapon_cleave;
extern sh_int gsn_dodge;
extern sh_int gsn_hide;
extern sh_int gsn_peek;
extern sh_int gsn_pick_lock;
extern sh_int gsn_sneak;
extern sh_int gsn_steal;

extern sh_int gsn_disarm;
extern sh_int gsn_enhanced_damage;
extern sh_int gsn_kick;
extern sh_int gsn_blackjack;
extern sh_int gsn_parry;
extern sh_int gsn_rescue;
extern sh_int gsn_second_attack;
extern sh_int gsn_third_attack;
extern sh_int gsn_fourth_attack;

extern sh_int gsn_blindness;
extern sh_int gsn_charm_person;
extern sh_int gsn_curse;
extern sh_int gsn_invis;
extern sh_int gsn_mass_invis;
extern sh_int gsn_plague;
extern sh_int gsn_poison;
extern sh_int gsn_sleep;

/* new gsns */
extern sh_int gsn_axe;
extern sh_int gsn_dagger;
extern sh_int gsn_flail;
extern sh_int gsn_mace;
extern sh_int gsn_polearm;
extern sh_int gsn_shield_block;
extern sh_int gsn_spear;
extern sh_int gsn_sword;
extern sh_int gsn_whip;

extern sh_int gsn_bash;
extern sh_int gsn_berserk;
extern sh_int gsn_dirt;
extern sh_int gsn_hand_to_hand;
extern sh_int gsn_trip;

extern sh_int gsn_fast_healing;
extern sh_int gsn_haggle;
extern sh_int gsn_lore;
extern sh_int gsn_meditation;

extern sh_int gsn_scrolls;
extern sh_int gsn_staves;
extern sh_int gsn_wands;
extern sh_int gsn_recall;
#ifdef TRACK_IS_SKILL
extern sh_int gsn_track;
#endif
extern sh_int gsn_brew;
extern sh_int gsn_scribe;

extern DISABLED_DATA *disabled_first;

/* prototypes from db.c */
void load_disabled args( ( void ) );
void save_disabled args( ( void ) );

/*
 * Utility macros.
 */
#define UMIN(a, b)              ((a) < (b) ? (a) : (b))
#define UMAX(a, b)              ((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)         ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)                ((c) >= 'A' && (c) <= 'Z' ? (c)+32 : (c))
#define UPPER(c)                ((c) >= 'a' && (c) <= 'z' ? (c)-32 : (c))
#define IS_SET(flag, bit)       ((flag) & (bit))
#define SET_BIT(var, bit)       ((var) |= (bit))
#define REMOVE_BIT(var, bit)    ((var) &= ~(bit))
#define cls(a)                  write_to_buffer(a,"[2J[1;1H",0)

/*
 * Character macros.
 */
#define IS_NPC(ch)              (IS_SET((ch)->act, ACT_IS_NPC))
#define IS_IMMORTAL(ch)         (get_trust(ch) >= LEVEL_IMMORTAL)
#define CH(d)                   ((d)->original ? (d)->original : (d)->character)
#define IS_HERO(ch)             (get_trust(ch) >= LEVEL_HERO)
#define IS_ADMIN(ch)             (get_trust(ch) >= LEVEL_HERO)
#define IS_TRUSTED(ch,level)    (get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)     (IS_SET((ch)->affected_by, (sn)))
#define IS_NEWAFFECTED(ch, sn)  (IS_NEWAFF_SET((ch)->newaff,(sn)))
#define IS_VALID(data)          ((data) != NULL && (data)->valid)
#define VALIDATE(data)          ((data)->valid = TRUE)
#define INVALIDATE(data)        ((data)->valid = FALSE)

#define GET_AGE(ch)             ((int) (17 + ((ch)->played \
                                    + current_time - (ch)->logon )/72000

#define IS_GOOD(ch)             (ch->alignment >= 350)
#define IS_EVIL(ch)             (ch->alignment <= -350)
#define IS_NEUTRAL(ch)          (!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)            (ch->position > POS_SLEEPING)
#define GET_AC(ch,type)         ((ch)->armor[type]                          \
			+ ( IS_AWAKE(ch)                            \
			? dex_app[get_curr_stat(ch,STAT_DEX)].defensive : 0 ))
#define GET_HITROLL(ch) \
		((ch)->hitroll+str_app[get_curr_stat(ch,STAT_STR)].tohit)
#define GET_DAMROLL(ch) \
		((ch)->damroll+str_app[get_curr_stat(ch,STAT_STR)].todam)

#define IS_OUTSIDE(ch)          (!IS_SET(                                   \
				    (ch)->in_room->room_flags,              \
				    ROOM_INDOORS))

#define WAIT_STATE(ch, npulse)  ((ch)->wait = UMAX((ch)->wait, (npulse)))
#define DAZE_STATE(ch, npulse)  ((ch)->daze = UMAX((ch)->daze, (npulse)))

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)     (IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)  (IS_SET((obj)->extra_flags, (stat)))
#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value[4],(stat)))

/*
 * Description macros.
 */
#define PERS(ch, looker)        ( can_see( looker, (ch) ) ?             \
				( IS_NPC(ch) ? (ch)->short_descr        \
				: (ch)->name ) : "someone" )

/*
 * Structure for a social in the socials table.
 */

struct social_type {
    char name[20];
    char *char_no_arg;
    char *others_no_arg;
    char *char_found;
    char *others_found;
    char *vict_found;
    char *char_not_found;
    char *char_auto;
    char *others_auto;
};

/*
 * Global constants.
 */

extern const struct str_app_type str_app[MAX_ATTAINABLE_STATS + 1];
extern const struct int_app_type int_app[MAX_ATTAINABLE_STATS + 1];
extern const struct wis_app_type wis_app[MAX_ATTAINABLE_STATS + 1];
extern const struct dex_app_type dex_app[MAX_ATTAINABLE_STATS + 1];
extern const struct con_app_type con_app[MAX_ATTAINABLE_STATS + 1];

/* I removed the const from both of these entries to accomodate the new
Askill command. 
	-Lancelight
*/
#if defined(cbuilder)
extern struct class_type class_table[MAX_CLASS + 1];
#else
extern struct class_type class_table[MAX_CLASS];
#endif

extern const struct attack_type attack_table[];
extern const struct race_type race_table[];
extern const struct pc_race_type pc_race_table[];
extern const struct liq_type liq_table[LIQ_MAX];
extern struct skill_type skill_table[MAX_SKILL];
extern const struct group_type group_table[MAX_GROUP];
extern struct social_type social_table[MAX_SOCIALS];

/*
 * Global variables.
 */
/* from handler.c */
extern char thetime[7];
extern char thedate[10];

extern HELP_DATA *help_first;
extern TODO_DATA *todo_first;
extern SHOP_DATA *shop_first;
extern CLAN_DATA *clan_first;
extern CLAN_DATA *clan_last;

extern BAN_DATA *ban_list;
extern CHAR_DATA *char_list;
extern CHAR_DATA *player_list;
extern CHAR_DATA *shell_char_list;
extern MPROG_DATA *mudprog_first;
extern MPROG_DATA *mudprog_last;
extern MPROG_GROUP *mprog_group_first;
extern MPROG_GROUP *mprog_group_last;
extern DESCRIPTOR_DATA *descriptor_list;
extern NOTE_DATA *note_list;
extern OBJ_DATA *object_list;

extern MPROG_DATA *mprog_free;
extern MPROG_GROUP *mprog_group_free;
extern AFFECT_DATA *affect_free;
extern NEWAFFECT_DATA *newaffect_free;
extern BAN_DATA *ban_free;
extern CHAR_DATA *char_free;
extern DESCRIPTOR_DATA *descriptor_free;
extern EXTRA_DESCR_DATA *extra_descr_free;
extern LAST_DATA *flast;
extern LAST_DATA *last_list;
extern LAST_DATA *flast_imm;
extern LAST_DATA *last_imm;
extern LAST_DATA *flast_admin;
extern LAST_DATA *last_admin;
extern LAST_DATA *flast_hero;
extern LAST_DATA *last_hero;
extern NOTE_DATA *note_free;
extern OBJ_DATA *obj_free;
extern PC_DATA *pcdata_free;

extern char bug_buf[];
extern time_t current_time;
extern bool fLogAll;
extern FILE *fpReserve;
extern KILL_DATA kill_table[];
extern char log_buf[];
extern TIME_INFO_DATA time_info;
extern WEATHER_DATA weather_info;
extern AUCTION_DATA auction_info;   /* Needed for autoauction -Lancelight */

/* System configuration structure */

extern SYS_CONFIG sysconfig;

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */

#if     defined(_AIX)
char *crypt args( ( const char *key, const char *salt ) );
#endif

#if     defined(apollo)
int atoi args( ( const char *string ) );
void *calloc args( ( unsigned nelem, size_t size ) );
char *crypt args( ( const char *key, const char *salt ) );
#endif

#if     defined(hpux)
char *crypt args( ( const char *key, const char *salt ) );
#endif

#if     defined(linux)
char *crypt args( ( const char *key, const char *salt ) );
#endif

#if             defined(WIN32)

int gettimeofday( struct timeval *, void * );   /* Second parameter is for compatibility with Unix GetTimeOfDay */
void bzero( char *data, unsigned long length );
char *crypt( char *key, char *salt );

#define SIGTRAP -1              /* CBuilder doesn't have SIGTRAP, setting */
            /* it to match SIG_ERR                                        */

#endif

#if             defined(cbuilder)
void update_gui( void );        /* Update GUI stats and run GUI commands. */
void SetStatus( char *Message );
#endif

#if     defined(MIPS_OS)
char *crypt args( ( const char *key, const char *salt ) );
#endif

#if     defined(NeXT)
char *crypt args( ( const char *key, const char *salt ) );
#endif

#if     defined(sequent)
char *crypt args( ( const char *key, const char *salt ) );
int fclose args( ( FILE * stream ) );
int fprintf args( ( FILE * stream, const char *format, ... ) );
int fread args( ( void *ptr, int size, int n, FILE * stream ) );
int fseek args( ( FILE * stream, long offset, int ptrname ) );
void perror args( ( const char *s ) );
int ungetc args( ( int c, FILE * stream ) );
#endif

#if     defined(sun)
char *crypt args( ( const char *key, const char *salt ) );
int fclose args( ( FILE * stream ) );
int fprintf args( ( FILE * stream, const char *format, ... ) );
#if     defined(SYSV)
siz_t fread args( ( void *ptr, size_t size, size_t n, FILE * stream ) );
#else
int fread args( ( void *ptr, int size, int n, FILE * stream ) );
#endif
int fseek args( ( FILE * stream, long offset, int ptrname ) );
void perror args( ( const char *s ) );
int ungetc args( ( int c, FILE * stream ) );
#endif

#if     defined(ultrix)
char *crypt args( ( const char *key, const char *salt ) );
#endif

/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 * United States to foreign countries.
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if     defined(NOCRYPT)
#define crypt(s1, s2)   (s1)
#endif

/*
 * Our function prototypes.
 * One big lump ... this is every global function in Ember.
 */
#define CD      CHAR_DATA
#define MID     MOB_INDEX_DATA
#define OD      OBJ_DATA
#define OID     OBJ_INDEX_DATA
#define RID     ROOM_INDEX_DATA
#define AD      AFFECT_DATA
#define NEWAD   NEWAFFECT_DATA

/* pty.c */
int master_pty( char *ptyname );
int slave_tty( char *ptyname, char *ttyname );

/* route_io.c */
int route_io( int fd1, int fd2 );

/* act_comm.c */
bool is_note_to args( ( CHAR_DATA * ch, NOTE_DATA * pnote ) );
void check_sex args( ( CHAR_DATA * ch ) );
void add_follower args( ( CHAR_DATA * ch, CHAR_DATA * master ) );
void stop_follower args( ( CHAR_DATA * ch ) );
void nuke_pets args( ( CHAR_DATA * ch ) );
void die_follower args( ( CHAR_DATA * ch ) );
bool is_same_group args( ( CHAR_DATA * ach, CHAR_DATA * bch ) );

extern bool reap_shells;
extern const char echo_off_str[];
extern const char echo_on_str[];
extern const char go_ahead_str[];
extern const char will_suppress_ga_str[];
extern const char wont_suppress_ga_str[];

/* act_info.c */
void set_title args( ( CHAR_DATA * ch, char *title ) );
void do_look args( ( CHAR_DATA * ch, char *argument ) );

/* auction.c */
void auction_update args( ( void ) );
void auction_channel args( ( char *vismsg, char *notvismsg ) );

/* act_move.c */
void move_char args( ( CHAR_DATA * ch, int door, bool follow ) );
int find_door args( ( CHAR_DATA * ch, char *arg ) );

/* act_obj.c */
bool can_loot args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
void get_obj args( ( CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container ) );
bool shopkeeper_kick_ch
args( ( CHAR_DATA * ch, CHAR_DATA * keeper, char *actstr ) );

/* act_wiz.c */

/* alias.c */
void substitute_alias args( ( DESCRIPTOR_DATA * d, char *input ) );
/*ban.c*/
bool check_ban args( ( char *site, int type ) );

/* comm.c */
void show_string args( ( struct descriptor_data * d, char *input ) );
void close_socket args( ( DESCRIPTOR_DATA * dclose ) );
void write_to_buffer args( ( DESCRIPTOR_DATA * d, const char *txt,
                             int length ) );
void send_to_char args( ( const char *txt, CHAR_DATA * ch ) );
void page_to_char args( ( const char *txt, CHAR_DATA * ch ) );
void printf_to_char args( ( CHAR_DATA * ch, char *fmt, ... ) );
void act args( ( const char *format, CHAR_DATA * ch,
                 const void *arg1, const void *arg2, int type ) );
char *act_new args( ( const char *format, CHAR_DATA * ch,
                      const void *arg1, const void *arg2, int type,
                      int min_pos ) );
int figure_difference args( ( int points ) );
void do_color args( ( register char *inbuf, int inlen, register char *outbuf,
                      int outlen, bool color ) );

/* db.c */

#if !defined(CPP)
char *remove_color args( ( const char *str ) );
#endif

/* Added for ban.c -Lancelight */
char *print_flags args( ( int flag ) );

int boot_db args( (  ) );
void update_last args( ( char *line1, char *line2, char *line3 ) );
void area_update args( ( void ) );
CD *create_mobile args( ( MOB_INDEX_DATA * pMobIndex ) );
void clone_mobile args( ( CHAR_DATA * parent, CHAR_DATA * clone ) );
OD *create_object args( ( OBJ_INDEX_DATA * pObjIndex, int level ) );
void clone_object args( ( OBJ_DATA * parent, OBJ_DATA * clone ) );
void clear_char args( ( CHAR_DATA * ch ) );
void free_char args( ( CHAR_DATA * ch ) );
char *get_extra_descr args( ( const char *name, EXTRA_DESCR_DATA * ed ) );
MID *get_mob_index args( ( int vnum ) );
OID *get_obj_index args( ( int vnum ) );
RID *get_room_index args( ( int vnum ) );
MPROG_DATA *get_mprog_by_vnum args( ( int vnum ) );
MPROG_GROUP *get_mprog_group_by_vnum args( ( int vnum ) );
MPROG_DATA *new_mudprog args( ( void ) );
void free_mudprog args( ( MPROG_DATA * mprog ) );
MPROG_GROUP *new_mudprog_group args( ( void ) );
void free_mudprog_group args( ( MPROG_GROUP * pMprogGroup ) );
char fread_letter args( ( FILE * fp ) );
int fread_number args( ( FILE * fp ) );
long fread_flag args( ( FILE * fp ) );
void fread_to_eol args( ( FILE * fp ) );
char *get_word args( ( FILE * fp ) );
char *fread_word args( ( FILE * fp ) );
long flag_convert args( ( char letter ) );
void *alloc_mem args( ( int sMem ) );
void *alloc_perm args( ( int sMem ) );
void free_mem args( ( void *pMemPtr ) );
unsigned int str_len args( ( const char *str ) );
#if defined(_MSC_VER)
void snprintf args( ( char *buffer, size_t count, const char *format, ... ) );
#endif
char *str_str args( ( char *str1, char *str2 ) );
char *str_upr args( ( char *str ) );
int number_fuzzy args( ( int number ) );
int number_range args( ( int from, int to ) );
int number_percent args( ( void ) );
int number_door args( ( void ) );
int number_bits args( ( int width ) );
int number_mm args( ( void ) );
int dice args( ( int number, int size ) );
int interpolate args( ( int level, int value_00, int value_32 ) );
void smash_tilde args( ( char *str ) );
bool str_cmp args( ( const char *astr, const char *bstr ) );
bool str_prefix args( ( const char *astr, const char *bstr ) );
bool str_infix args( ( const char *astr, const char *bstr ) );
bool str_suffix args( ( const char *astr, const char *bstr ) );
char *capitalize args( ( const char *str ) );
void append_file args( ( CHAR_DATA * ch, char *file, char *str ) );
void bug args( ( const char *str, ... ) );
void bug_trace args( ( const char *str ) );
#if !defined(CPP)
void logf_string args( ( const char *str, ... ) );
void log_string args( ( const char *str ) );
#endif
void tail_chain args( ( void ) );
bool assign_mobprog args( ( MOB_INDEX_DATA * pMob, MPROG_DATA * pMudProg,
                            MPROG_GROUP * pMprogGroup ) );
bool assign_objprog args( ( OBJ_INDEX_DATA * pObj, MPROG_DATA * pMudProg,
                            MPROG_GROUP * pMprogGroup ) );
bool assign_roomprog args( ( ROOM_INDEX_DATA * pRoom, MPROG_DATA * pMudProg,
                             MPROG_GROUP * pMprogGroup ) );
bool unassign_mobprog args( ( MOB_INDEX_DATA * pMob, MPROG_DATA * pMudProg,
                              MPROG_GROUP * pMprogGroup ) );
bool unassign_objprog args( ( OBJ_INDEX_DATA * pObj, MPROG_DATA * pMudProg,
                              MPROG_GROUP * pMprogGroup ) );
bool unassign_roomprog args( ( ROOM_INDEX_DATA * pRoom, MPROG_DATA * pMudProg,
                               MPROG_GROUP * pMprogGroup ) );

/* fight.c */
bool is_safe args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool is_safe_spell args( ( CHAR_DATA * ch, CHAR_DATA * victim, bool area ) );
void violence_update args( ( void ) );
void multi_hit args( ( CHAR_DATA * ch, CHAR_DATA * victim, int dt ) );
bool damage
args( ( CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * weapon, int dam, int dt,
        int Class ) );
bool new_damage
args( ( CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * weapon, int dam, int dt,
        int Class, bool show ) );
void update_pos args( ( CHAR_DATA * victim ) );
void stop_fighting args( ( CHAR_DATA * ch, bool fBoth ) );
bool is_hating args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void stop_hating args( ( CHAR_DATA * ch ) );
void stop_fighting args( ( CHAR_DATA * ch, bool fBoth ) );

/* handler.c */
char *get_curdate(  );
char *get_curtime(  );
char *get_time(  );
char *get_date(  );
AD *affect_find args( ( AFFECT_DATA * paf, int sn ) );
int check_immune args( ( CHAR_DATA * ch, int dam_type ) );
int material_lookup args( ( const char *name ) );
char *material_name args( ( sh_int num ) );
int race_lookup args( ( const char *name ) );
int class_lookup args( ( const char *name ) );
int get_skill args( ( CHAR_DATA * ch, int sn ) );
int get_weapon_sn args( ( CHAR_DATA * ch ) );
int get_second_weapon_sn args( ( CHAR_DATA * ch ) );
int get_weapon_skill args( ( CHAR_DATA * ch, int sn ) );
int get_age args( ( CHAR_DATA * ch ) );
void reset_char args( ( CHAR_DATA * ch ) );
int get_trust args( ( CHAR_DATA * ch ) );
int get_curr_stat args( ( CHAR_DATA * ch, int stat ) );
int get_max_train args( ( CHAR_DATA * ch, int stat ) );
int can_carry_n args( ( CHAR_DATA * ch ) );
int can_carry_w args( ( CHAR_DATA * ch ) );
bool is_name args( ( char *str, char *namelist ) );
bool is_full_name args( ( const char *str, char *namelist ) );
bool is_exact_name args( ( char *str, char *namelist ) );
void affect_to_char args( ( CHAR_DATA * ch, AFFECT_DATA * paf ) );
void newaffect_to_char args( ( CHAR_DATA * ch, NEWAFFECT_DATA * paf ) );
void affect_to_obj args( ( OBJ_DATA * obj, AFFECT_DATA * paf ) );
void affect_remove args( ( CHAR_DATA * ch, AFFECT_DATA * paf ) );
void newaffect_remove args( ( CHAR_DATA * ch, NEWAFFECT_DATA * paf ) );
void affect_remove_obj args( ( OBJ_DATA * obj, AFFECT_DATA * paf ) );
void affect_strip args( ( CHAR_DATA * ch, int sn ) );
void newaffect_strip args( ( CHAR_DATA * ch, int sn ) );
bool is_affected args( ( CHAR_DATA * ch, int sn ) );
bool is_newaffected args( ( CHAR_DATA * ch, int sn ) );
void affect_join args( ( CHAR_DATA * ch, AFFECT_DATA * paf ) );
void newaffect_join args( ( CHAR_DATA * ch, NEWAFFECT_DATA * paf ) );
void char_from_room args( ( CHAR_DATA * ch ) );
void char_to_room args( ( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex ) );
void obj_to_char args( ( OBJ_DATA * obj, CHAR_DATA * ch ) );
void obj_from_char args( ( OBJ_DATA * obj ) );
int apply_ac args( ( OBJ_DATA * obj, int iWear, int type ) );
OD *get_eq_char args( ( CHAR_DATA * ch, int iWear ) );
void equip_char args( ( CHAR_DATA * ch, OBJ_DATA * obj, int iWear ) );
void unequip_char args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
int count_obj_list args( ( OBJ_INDEX_DATA * obj, OBJ_DATA * list ) );
void obj_from_room args( ( OBJ_DATA * obj ) );
void obj_to_room args( ( OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex ) );
void obj_to_obj args( ( OBJ_DATA * obj, OBJ_DATA * obj_to ) );
void obj_from_obj args( ( OBJ_DATA * obj ) );
void extract_obj args( ( OBJ_DATA * obj ) );
void extract_char args( ( CHAR_DATA * ch, bool fPull ) );
void pk_extract_char args( ( CHAR_DATA * ch, bool fPull ) );
CD *get_char_room args( ( CHAR_DATA * ch, char *argument ) );
CD *get_mob_room args( ( CHAR_DATA * ch, char *argument ) );
CD *get_player_room args( ( CHAR_DATA * ch, char *argument ) );
CD *get_char_world args( ( CHAR_DATA * ch, char *argument ) );
CD *get_mob_world args( ( CHAR_DATA * ch, char *argument ) );
CD *get_player_world args( ( CHAR_DATA * ch, char *argument ) );
OD *get_obj_type args( ( OBJ_INDEX_DATA * pObjIndexData ) );
OD *get_obj_list args( ( CHAR_DATA * ch, char *argument, OBJ_DATA * list ) );
OD *get_obj_carry args( ( CHAR_DATA * ch, char *argument ) );
OD *get_obj_wear args( ( CHAR_DATA * ch, char *argument ) );
OD *get_obj_here args( ( CHAR_DATA * ch, char *argument ) );
OD *get_obj_world args( ( CHAR_DATA * ch, char *argument ) );
OD *create_money args( ( int amount ) );
int get_obj_number args( ( OBJ_DATA * obj ) );
int get_obj_weight args( ( OBJ_DATA * obj ) );
bool room_is_dark args( ( ROOM_INDEX_DATA * pRoomIndex ) );
bool room_is_private args( ( ROOM_INDEX_DATA * pRoomIndex ) );
bool can_see args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
bool can_see_obj args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
bool can_see_room args( ( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex ) );
bool can_drop_obj args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
char *item_type_name args( ( OBJ_DATA * obj ) );
char *affect_loc_name args( ( int location ) );
char *affect_bit_name args( ( int vector ) );
char *newaffect_bit_name args( ( int vector ) );
char *extra_bit_name args( ( int extra_flags ) );
char *wear_bit_name args( ( int wear_flags ) );
char *act_bit_name args( ( int act_flags ) );
char *off_bit_name args( ( int off_flags ) );
char *imm_bit_name args( ( int imm_flags ) );
char *form_bit_name args( ( int form_flags ) );
char *part_bit_name args( ( int part_flags ) );
char *weapon_bit_name args( ( int weapon_flags ) );
char *comm_bit_name args( ( int comm_flags ) );

/* interp.c */
int cmd_level args( ( char *argument ) );
void interpret args( ( CHAR_DATA * ch, char *argument ) );
void mpinterpret args( ( CHAR_DATA * ch, char *argument ) );
bool is_number args( ( char *arg ) );
int number_argument args( ( char *argument, char *arg ) );
char *one_argument args( ( char *argument, char *arg_first ) );
char *one_argument2 args( ( char *argument, char *arg_first ) );
void chk_command args( ( CHAR_DATA * ch, char *argument ) );

/* magic.c */
int mana_cost( CHAR_DATA * ch, int min_mana, int level );
int skill_lookup args( ( const char *name ) );
int slot_lookup args( ( int slot ) );
bool saves_spell args( ( int level, CHAR_DATA * victim ) );
void obj_cast_spell args( ( int sn, int level, CHAR_DATA * ch,
                            CHAR_DATA * victim, OBJ_DATA * obj ) );

/* save.c */
void save_char_obj args( ( CHAR_DATA * ch ) );
bool load_char_obj args( ( DESCRIPTOR_DATA * d, char *name ) );

/* skills.c */
bool parse_gen_groups args( ( CHAR_DATA * ch, char *argument ) );
void list_group_costs args( ( CHAR_DATA * ch ) );
void list_group_known args( ( CHAR_DATA * ch ) );
long exp_per_level args( ( CHAR_DATA * ch, int points ) );
void check_improve args( ( CHAR_DATA * ch, int sn, bool success,
                           int multiplier ) );
int group_lookup args( ( const char *name ) );
void gn_add args( ( CHAR_DATA * ch, int gn ) );
void gn_remove args( ( CHAR_DATA * ch, int gn ) );
void group_add args( ( CHAR_DATA * ch, const char *name, bool deduct ) );
void group_remove args( ( CHAR_DATA * ch, const char *name ) );

/* ssm.c */
char *str_dup args( ( const char *str ) );
char *fread_string args( ( FILE * fp ) );
char *fread_string_eol args( ( FILE * fp ) );
void free_string args( ( char **strptr ) );
void temp_fread_string args( ( FILE * fp, char *outbuf ) );
int temp_fread_string_eol args( ( FILE * fp, char *outbuf ) );

/* update.c */
void advance_level args( ( CHAR_DATA * ch ) );
void gain_exp args( ( CHAR_DATA * ch, int gain ) );
void gain_condition args( ( CHAR_DATA * ch, int iCond, int value ) );
void update_handler args( ( void ) );

/* drunk.c */
char *makedrunk args( ( char *string, CHAR_DATA * ch ) );

#undef  CD
#undef  MID
#undef  OD
#undef  OID
#undef  RID

/*****************************************************************************
 *                                    OLC                                    *
 *****************************************************************************/

/*
 * Interp.c
 */
DECLARE_DO_FUN( do_olc );
DECLARE_DO_FUN( do_asave );
DECLARE_DO_FUN( do_alist );
DECLARE_DO_FUN( do_resets );
DECLARE_DO_FUN( do_sendinfo );
DECLARE_DO_FUN( do_disable );

/*
 * Global Constants
 */
extern char *const dir_name[];
extern const sh_int rev_dir[];  /* sh_int - ROM OLC */

/*
 * Global variables
 */
extern AREA_DATA *area_first;
extern AREA_DATA *area_last;
extern SHOP_DATA *shop_last;

extern int top_mprog;
extern int top_mprog_group;
extern int top_affect;
extern int top_newaffect;
extern int top_area;
extern int top_ed;
extern int top_exit;
extern int top_help;
extern int top_mob_index;
extern int top_obj_index;
extern int top_reset;
extern int top_room;
extern int top_shop;
extern int top_clan;

extern int top_vnum_mob;
extern int top_vnum_obj;
extern int top_vnum_room;

extern char str_empty[1];

extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

/* act_comm.c */
void add2queue args( ( CHAR_DATA * ch, CHAR_DATA * victim, char *argument ) );
void add2tell
args( ( CHAR_DATA * ch, CHAR_DATA * victim, bool group, char *argument ) );
void add2last args( ( char *format, char *message, CHAR_DATA * ch ) );
void add2last_imm args( ( char *format, char *message, CHAR_DATA * ch ) );
void add2last_admin args( ( char *format, char *message, CHAR_DATA * ch ) );
void add2last_hero args( ( char *format, char *message, CHAR_DATA * ch ) );

/* act_wiz.c */
/*
ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg );
*/
/* db.c */
void reset_area args( ( AREA_DATA * pArea ) );
void reset_room args( ( ROOM_INDEX_DATA * pRoom ) );

/* string.c */
void string_edit args( ( CHAR_DATA * ch, char **pString ) );
void string_append args( ( CHAR_DATA * ch, char **pString ) );
char *string_replace args( ( char *orig, char *old, char *New ) );
void string_add args( ( CHAR_DATA * ch, char *argument ) );
char *format_string args( ( char *oldstring /*, bool fSpace */  ) );
char *first_arg args( ( char *argument, char *arg_first, bool fCase ) );
char *string_unpad args( ( char *argument ) );
char *string_proper args( ( char *argument ) );

/* ban data recycling */
#define BD BAN_DATA
/* ban data recycling */
#define BD BAN_DATA
BD *new_ban args( ( void ) );
void free_ban args( ( BAN_DATA * ban ) );
#undef BD

/* olc.c */
bool run_olc_editor args( ( DESCRIPTOR_DATA * d ) );
char *olc_ed_name args( ( CHAR_DATA * ch ) );
char *olc_ed_vnum args( ( CHAR_DATA * ch ) );

/* olc_act.c */
void show_mprog args( ( CHAR_DATA * ch, MPROG_DATA * pMobProg ) );
void show_mpgroup args( ( CHAR_DATA * ch, MPROG_GROUP * pGroup ) );

/* clan.c */
CLAN_DATA *get_clan args( ( int clannum ) );

/* bit.c */
extern const struct flag_type area_flags[];
extern const struct flag_type sex_flags[];
extern const struct flag_type exit_flags[];
extern const struct flag_type door_resets[];
extern const struct flag_type room_flags[];
extern const struct flag_type sector_flags[];
extern const struct flag_type type_flags[];
extern const struct flag_type extra_flags[];
extern const struct flag_type wear_flags[];
extern const struct flag_type act_flags[];
extern const struct flag_type affect_flags[];
extern const struct flag_type apply_flags[];
extern const struct flag_type wear_loc_strings[];
extern const struct flag_type wear_loc_flags[];
extern const struct flag_type weapon_flags[];
extern const struct flag_type container_flags[];
extern const struct flag_type liquid_flags[];
extern const struct flag_type mprog_type_flags[];

/* ROM OLC: */

extern const struct flag_type material_type[];
extern const struct flag_type form_flags[];
extern const struct flag_type part_flags[];
extern const struct flag_type ac_type[];
extern const struct flag_type size_flags[];
extern const struct flag_type off_flags[];
extern const struct flag_type imm_flags[];
extern const struct flag_type res_flags[];
extern const struct flag_type vuln_flags[];
extern const struct flag_type position_flags[];
extern const struct flag_type weapon_class[];
extern const struct flag_type weapon_type[];
extern const struct flag_type furniture_flags[];

/*   Added by EmberMUD */

extern const struct flag_type clan_flags[];
extern const struct flag_type clan_join_flags[];
extern const struct flag_type rnd_obj_flags[];

struct rand_bag_material {
    char *name;
    bool isprefix;              /* before or after the type.name? */
    sh_int size;                /* is the material big or small? */
};

struct rand_bag_type {
    char *name;
    sh_int capacity;            /* big or small? */
    long wear_loc;              /* can it be worn? */
};

/* struct rand_light_type
 * {
 *  char *  name;
 *  sh_int  duration;       
 * };
 */

struct rand_weapon_type {
    char *name;
    sh_int magic_mod;
    sh_int damage_mod;
    sh_int weapon_type;
};

struct rand_armor_type {
    char *name;
    sh_int wearloc;
    sh_int protect_mod;         /* + or - AC ? */
    sh_int magic_mod;           /* better or worse for magic? */
    sh_int p_type;
};

struct rand_item_material {
    char *name;
    sh_int coolness;            /* better AC for armor, better damroll for weapons, etc. */
    sh_int magic_mod;
    sh_int level;               /* level where this material is normal (i.e. iron is 1, dilithium is 80, etc.) */
};

struct rand_ring_type {
    char *name;
    sh_int coolness;            /* "exquisite gold" rings are cooler than "dirty copper" */
    long extra_flags;           /* if it's "glowing" set it to glow, etc. */
};

/*****************************************************************************
 *                                 OLC END                                   *
 *****************************************************************************/

/*************************
 * Mob Programs Funcs    *
 *************************/

/* act_wiz.c */
ROOM_INDEX_DATA *find_location args( ( CHAR_DATA * ch, char *arg ) );

/* fight.c */
void death_cry args( ( CHAR_DATA * ch ) );
int hit_xp_compute args( ( CHAR_DATA * gch, CHAR_DATA * victim,
                           int total_levels, int members, int dam ) );
int cast_xp_compute args( ( CHAR_DATA * gch, CHAR_DATA * victim,
                            int total_levels, int members, int dam ) );

/* mud_prog.c */
#ifdef DUNNO_STRSTR
char *strstr args( ( const char *s1, const char *s2 ) );
#endif

void mprog_act_trigger args( ( char *txt, CHAR_DATA * ch ) );
void mprog_bribe_trigger args( ( CHAR_DATA * mob, CHAR_DATA * ch,
                                 int amount ) );
void mprog_entry_trigger args( ( CHAR_DATA * mob ) );
void mprog_give_trigger args( ( CHAR_DATA * mob, CHAR_DATA * ch,
                                OBJ_DATA * obj ) );
void mprog_greet_trigger args( ( CHAR_DATA * mob ) );
void mprog_fight_trigger args( ( CHAR_DATA * mob, CHAR_DATA * ch ) );
void mprog_hitprcnt_trigger args( ( CHAR_DATA * mob, CHAR_DATA * ch ) );
void mprog_death_trigger args( ( CHAR_DATA * mob ) );
void mprog_random_trigger args( ( CHAR_DATA * mob ) );
void mprog_speech_trigger args( ( char *txt, CHAR_DATA * mob ) );
/* added by Zak */
bool mprog_command_trigger args( ( char *txt, CHAR_DATA * ch, char *extra ) );
/* added by Zane */
void mprog_fightgroup_trigger args( ( CHAR_DATA * mob ) );

/* room progs */
void rprog_act_trigger args( ( char *txt, CHAR_DATA * ch ) );
void rprog_leave_trigger args( ( CHAR_DATA * ch ) );
void rprog_enter_trigger args( ( CHAR_DATA * ch ) );
void rprog_sleep_trigger args( ( CHAR_DATA * ch ) );
void rprog_rest_trigger args( ( CHAR_DATA * ch ) );
void rprog_rfight_trigger args( ( CHAR_DATA * ch ) );
void rprog_death_trigger args( ( CHAR_DATA * ch ) );
void rprog_speech_trigger args( ( char *txt, CHAR_DATA * ch ) );
void rprog_random_trigger args( ( CHAR_DATA * ch ) );

/* object progs */
void oprog_speech_trigger args( ( char *txt, CHAR_DATA * ch ) );
void oprog_random_trigger args( ( OBJ_DATA * obj ) );
void oprog_wear_trigger args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
bool oprog_use_trigger args( ( CHAR_DATA * ch, OBJ_DATA * obj, CHAR_DATA * vict,
                               OBJ_DATA * targ ) );
void oprog_remove_trigger args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
void oprog_sac_trigger args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
void oprog_damage_trigger args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
void oprog_repair_trigger args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
void oprog_drop_trigger args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
void oprog_zap_trigger args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
void oprog_get_trigger args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
void oprog_examine_trigger args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
void oprog_greet_trigger args( ( CHAR_DATA * ch ) );
void oprog_act_trigger args( ( char *txt, CHAR_DATA * ch ) );
/* added by Zane */
void oprog_hit_trigger args( ( CHAR_DATA * ch, CHAR_DATA * victim,
                               OBJ_DATA * obj ) );

#endif
