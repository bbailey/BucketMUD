#ifndef factions_h__
#define factions_h__

#include "merc.h"
#include "olc.h"

DECLARE_DO_FUN( do_factionedit );
DECLARE_DO_FUN( do_factionfind );
DECLARE_DO_FUN( do_mpchangefaction );
DECLARE_DO_FUN( do_mpsilentchangefaction );
DECLARE_DO_FUN( show_faction_standings );

typedef struct factionlist_data FACTIONLIST_DATA;
typedef struct factionaff_data FACTIONAFF_DATA;
typedef struct factionpc_data FACTIONPC_DATA;

extern FACTIONLIST_DATA *faction_first;
extern FACTIONLIST_DATA *faction_last;

extern const struct olc_cmd_type factionedit_table[];

/* Struct for list of factions available in the MUD */
struct factionlist_data {
    sh_int vnum;
    char *name;
    char *increase_msg;
    char *decrease_msg;
    FACTIONLIST_DATA *next;
};

/* Struct for list of factions affected when a char is killed */
struct factionaff_data {
    FACTIONLIST_DATA *faction;
    sh_int change;
    FACTIONAFF_DATA *next;
};

/* Struct for list of current faction values for a player */
struct factionpc_data {
    FACTIONLIST_DATA *faction;
    sh_int value;
    FACTIONPC_DATA *next;
};

/* Function prototypes */
void load_factionaffs args( ( FILE * fp ) );
void load_factions args( ( FILE * fp ) );
void save_factions args( ( void ) );
void factionedit args( ( CHAR_DATA * ch, char *argument ) );
FACTIONLIST_DATA *new_faction args( ( void ) );
FACTIONLIST_DATA *get_faction_by_vnum args( ( sh_int vnum ) );
bool factedit_show args( ( CHAR_DATA * ch, char *argument ) );
bool factedit_create args( ( CHAR_DATA * ch, char *argument ) );
bool factedit_name args( ( CHAR_DATA * ch, char *argument ) );
bool factedit_increase args( ( CHAR_DATA * ch, char *argument ) );
bool factedit_decrease args( ( CHAR_DATA * ch, char *argument ) );
bool medit_faction args( ( CHAR_DATA * ch, char *argument ) );
void affect_factions args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
void fread_faction_standings args( ( CHAR_DATA * ch, FILE * fp ) );
void fwrite_faction_standings args( ( CHAR_DATA * ch, FILE * fp ) );
void free_faction_standings args( ( FACTIONPC_DATA * pFactPC ) );
char *faction_con_msg args( ( sh_int value ) );
sh_int faction_percentage args( ( sh_int value ) );
sh_int consider_factions
args( ( CHAR_DATA * ch, CHAR_DATA * victim, bool show ) );
void set_faction
args( ( CHAR_DATA * ch, CHAR_DATA * victim, sh_int vnum, sh_int value ) );
void faction_stat args( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
double faction_cost_multiplier
args( ( CHAR_DATA * ch, CHAR_DATA * keeper, bool buy ) );
char *add_to_buf args( ( char *buffer, char *txt ) );

#endif
