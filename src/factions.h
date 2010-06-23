#ifndef factions_h__
#define factions_h__

#include "merc.h"
#include "olc.h"

void do_factionedit (CHAR_DATA * ch, char *argument);
void do_factionfind (CHAR_DATA * ch, char *argument);
void do_mpchangefaction (CHAR_DATA * ch, char *argument);
void do_mpsilentchangefaction (CHAR_DATA * ch, char *argument);
void show_faction_standings (CHAR_DATA * ch, char *argument);

typedef struct factionlist_data FACTIONLIST_DATA;
typedef struct factionaff_data FACTIONAFF_DATA;
typedef struct factionpc_data FACTIONPC_DATA;

extern FACTIONLIST_DATA *faction_first;
extern FACTIONLIST_DATA *faction_last;

extern const struct olc_cmd_type factionedit_table[];

/* Struct for list of factions available in the MUD */
struct factionlist_data
{
    sh_int vnum;
    char *name;
    char *increase_msg;
    char *decrease_msg;
    FACTIONLIST_DATA *next;
};

/* Struct for list of factions affected when a char is killed */
struct factionaff_data
{
    FACTIONLIST_DATA *faction;
    sh_int change;
    FACTIONAFF_DATA *next;
};

/* Struct for list of current faction values for a player */
struct factionpc_data
{
    FACTIONLIST_DATA *faction;
    sh_int value;
    FACTIONPC_DATA *next;
};

/* Function prototypes */
void load_factionaffs (FILE * fp);
void load_factions (FILE * fp);
void save_factions (void);
void factionedit (CHAR_DATA * ch, char *argument);
FACTIONLIST_DATA *new_faction (void);
FACTIONLIST_DATA *get_faction_by_vnum (sh_int vnum);
bool factedit_show (CHAR_DATA * ch, char *argument);
bool factedit_create (CHAR_DATA * ch, char *argument);
bool factedit_name (CHAR_DATA * ch, char *argument);
bool factedit_increase (CHAR_DATA * ch, char *argument);
bool factedit_decrease (CHAR_DATA * ch, char *argument);
bool medit_faction (CHAR_DATA * ch, char *argument);
void affect_factions (CHAR_DATA * ch, CHAR_DATA * victim);
void fread_faction_standings (CHAR_DATA * ch, FILE * fp);
void fwrite_faction_standings (CHAR_DATA * ch, FILE * fp);
void free_faction_standings (FACTIONPC_DATA * pFactPC);
char *faction_con_msg (sh_int value);
sh_int faction_percentage (sh_int value);
sh_int consider_factions (CHAR_DATA * ch, CHAR_DATA * victim, bool show);
void set_faction
(CHAR_DATA * ch, CHAR_DATA * victim, sh_int vnum, sh_int value);
void faction_stat (CHAR_DATA * ch, CHAR_DATA * victim);
double faction_cost_multiplier (CHAR_DATA * ch, CHAR_DATA * keeper, bool buy);
char *add_to_buf (char *buffer, char *txt);

#endif
