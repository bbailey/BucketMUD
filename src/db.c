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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#if defined(WIN32)
#include <process.h>
#endif

#include "merc.h"
#include "olc.h"
#include "db.h"
#include "interp.h"

#if defined(unix)
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
/* extern int getrlimit(int resource, struct rlimit *rlp); */
/* extern int setrlimit(int resource, struct rlimit *rlp); */
#endif

extern int _filbuf args( ( FILE * ) );

/*
 * Globals.
 */

SYS_CONFIG sysconfig;

HELP_DATA *help_first;
HELP_DATA *help_last;

TODO_DATA *todo_first;
TODO_DATA *todo_last;

SHOP_DATA *shop_first;
SHOP_DATA *shop_last;

CHAR_DATA *char_free;
EXTRA_DESCR_DATA *extra_descr_free;
NOTE_DATA *note_free;
OBJ_DATA *obj_free;
PC_DATA *pcdata_free;

char bug_buf[2 * MAX_INPUT_LENGTH];
CHAR_DATA *char_list;
CHAR_DATA *player_list = NULL;
CHAR_DATA *shell_char_list = NULL;
MPROG_DATA *mudprog_first;
MPROG_DATA *mudprog_last;
MPROG_GROUP *mprog_group_first;
MPROG_GROUP *mprog_group_last;
char *help_greeting;
char *ansi_greeting;
char log_buf[2 * MAX_INPUT_LENGTH];
KILL_DATA kill_table[MAX_LEVEL];
OBJ_DATA *object_list;
CLAN_DATA *clan_first;
CLAN_DATA *clan_last;
TIME_INFO_DATA time_info;
WEATHER_DATA weather_info;
AUCTION_DATA auction_info;      /* Define the auction stuff here. -Lancelight */
/*struct  sociallist_data     social_table    [MAX_SOCIALS];*/
int social_count = 0;

sh_int gsn_backstab;
sh_int gsn_counter;
sh_int gsn_circle;
sh_int gsn_grip;
sh_int gsn_shield_cleave;
sh_int gsn_weapon_cleave;
sh_int gsn_blind_fighting;
sh_int gsn_dodge;
sh_int gsn_hide;
sh_int gsn_peek;
sh_int gsn_pick_lock;
sh_int gsn_sneak;
sh_int gsn_steal;

sh_int gsn_disarm;
sh_int gsn_enhanced_damage;
sh_int gsn_kick;
sh_int gsn_blackjack;
sh_int gsn_parry;
sh_int gsn_rescue;
sh_int gsn_second_attack;
sh_int gsn_third_attack;
sh_int gsn_fourth_attack;

sh_int gsn_blindness;
sh_int gsn_charm_person;
sh_int gsn_curse;
sh_int gsn_invis;
sh_int gsn_mass_invis;
sh_int gsn_poison;
sh_int gsn_plague;
sh_int gsn_sleep;

sh_int gsn_axe;
sh_int gsn_dagger;
sh_int gsn_flail;
sh_int gsn_mace;
sh_int gsn_polearm;
sh_int gsn_shield_block;
sh_int gsn_spear;
sh_int gsn_sword;
sh_int gsn_whip;

sh_int gsn_bash;
sh_int gsn_berserk;
sh_int gsn_dirt;
sh_int gsn_hand_to_hand;
sh_int gsn_trip;

sh_int gsn_fast_healing;
sh_int gsn_haggle;
sh_int gsn_lore;
sh_int gsn_meditation;

sh_int gsn_scrolls;
sh_int gsn_staves;
sh_int gsn_wands;
sh_int gsn_recall;
#ifdef TRACK_IS_SKILL
sh_int gsn_track;
#endif
sh_int gsn_brew;
sh_int gsn_scribe;

/*
 * Locals.
 */
MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
int load_config_file args( ( void ) );
char *get_config_value args( ( char *inbuf, char *outbuf ) );
MPROG_DATA *new_mudprog args( ( void ) );
MPROG_DATA *mprog_free;
MPROG_GROUP *mprog_group_free;
AREA_DATA *area_first;
AREA_DATA *area_last;
char *string_space;
char *top_string;
char str_empty[1];

int top_mprog;
int top_mprog_group;
int top_affect;
int top_newaffect;
int top_area;
int top_ed;
int top_exit;
int top_help;
int top_todo;
int top_mob_index;
int top_obj_index;
int top_reset;
int top_room;
int top_shop;
int top_clan;
int top_vnum_room;              /* OLC */
int top_vnum_mob;               /* OLC */
int top_vnum_obj;               /* OLC */
int mobile_count = 0;

/*
 * MudProg locals
*/

int mprog_name_to_type args( ( char *name ) );
void load_mudprogs args( ( FILE * fp ) );
void load_progs args( ( FILE * fp ) );

/* Ban Locals
 * -Lancelight
 */

void load_bans args( ( void ) );

/*
 * Memory management.
 * Tune only if you understand what you're doing.
 */
#define                 MAX_PERM_BLOCK  131072
#define                 MAX_MEM_LIST    11

void *rgFreeList[MAX_MEM_LIST];
const int rgSizeList[MAX_MEM_LIST] = {
    16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768 - 64
};

int nAllocString;
int sAllocString;
int nAllocPerm;
int sAllocPerm;

/*
 * Semi-locals.
 */
bool fBootDb;
FILE *fpArea;
char strArea[MAX_INPUT_LENGTH];
char strPath[MAX_STRING_LENGTH];
extern int MAX_STRING;

void init_string_space( void );
void boot_done( void );

/*
 * Local booting procedures.
*/
void init_mm args( ( void ) );
void skip_section args( ( FILE * fp, char *section ) );
void load_clans args( ( FILE * fp ) );
void load_area args( ( FILE * fp ) );   /* OLC */
void load_helps args( ( FILE * fp ) );
void load_todo args( ( FILE * fp ) );
void load_mobiles args( ( FILE * fp ) );
void load_objects args( ( FILE * fp ) );
void load_resets args( ( FILE * fp ) );
void load_rooms args( ( FILE * fp ) );
void load_shops args( ( FILE * fp ) );
void load_socials args( ( FILE * fp ) );
void fix_exits args( ( void ) );
void init_supermob args( ( void ) );
void reset_area args( ( AREA_DATA * pArea ) );
void load_random_objs args( ( CHAR_DATA * mob, MOB_INDEX_DATA * mobIndex ) );

#define MSL MAX_STRING_LENGTH
#define MIL MAX_INPUT_LENGTH
/* This is the handy CH() macro. I think that it was Tom Adriansen (sp?) */

bool write_to_descriptor args( ( int desc, char *txt, int length ) );

#if defined(WIN32)
#define COPYOVER_FILE  ".\\CopyOver.tmp"
#if defined(cbuilder)
#define EXE_FILE       "..\\src\\embergui.exe"
#else
#define EXE_FILE       "..\\src\\ember.exe"
#endif
#else
#define COPYOVER_FILE  "TEMP"
#define EXE_FILE       "../src/ember"
#endif

extern int port, control;
extern bool fCopyOver;

void do_copyover( CHAR_DATA * ch, char *argument )
{

#if defined(WIN32)
    send_to_char( "Sorry, copyover not yet available in the Win32 port.\n\r",
                  ch );
    return;
#else
    char strPath[MAX_STRING_LENGTH];
    FILE *fp;
    DESCRIPTOR_DATA *d, *d_next;
    char buf[100], buf2[100];
    int close args( ( int fd ) );
    if ( IS_NPC( ch ) )
    {
        send_to_char( "Mobs dont need to be hotbooting the mud.\n", ch );
        return;
    }

    sprintf( strPath, "%s/%s", sysconfig.area_dir, COPYOVER_FILE );
    fp = fopen( strPath, "w" );

    if ( !fp )
    {
        send_to_char( "Copyover file not writeable, aborted.\n\r", ch );
        perror( "do_copyover:fopen" );
        return;
    }

    /* Consider changing all saved areas here, if you use OLC */

    do_asave( ch, "changed" );
    do_force( ch, "all save" );
    sprintf( buf,
             "\n\r\n\rThe earth splits in two, as %s remolds the world as you know it.\n\r",
             ch->name );

    /* For each playing descriptor, save its state */
    for ( d = descriptor_list; d; d = d_next )
    {
        CHAR_DATA *och = CH( d );
        d_next = d->next;       /* We delete from the list , so need to save this */

        if ( !d->character || d->connected < 0 )    /* drop those logging on */
        {
            write_to_descriptor( d->descriptor,
                                 "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r",
                                 66 );
            close_socket( d );  /* throw'em out */
        }
        else
        {
            fprintf( fp, "%d %s %s\n", d->descriptor, och->name, d->host );
/*                      if (IS_SET(och->act,PLR_QUESTOR));
			{
				REMOVE_BIT(ch->act,PLR_QUESTOR);
				ch->countdown = 0;
				ch->nextquest = 0;
				ch->questmob = 0;
				ch->questobj = 0;
				ch->questgiver = NULL;
			}*//*Auto-Quest not implemented yet */
/*                      if (och->level == 1)
			{
				write_to_descriptor (d->descriptor, "Since you are level one, and level one characters do not save, you gain a free level!\n\r", 0);
				advance_level (och);
				och->level++;
			} */
            save_char_obj( och );
            write_to_descriptor( d->descriptor, buf, sizeof( buf ) );
        }
    }

    fprintf( fp, "-1\n" );
    fclose( fp );

    /* Close reserve and other always-open files and release other resources */

    fclose( fpReserve );

    /* exec - descriptors are inherited */

    sprintf( buf, "%d", port );
    sprintf( buf2, "%d", control );
    execl( EXE_FILE, " ", buf, "EmberMUD", buf2, ( char * ) NULL );

    /* Failed - sucessful exec will not return */

    perror( "do_copyover: execl" );
    send_to_char( "Copyover FAILED!\n\r", ch );

    /* Here you might want to reopen fpReserve */
#endif
}

void init_descriptor( DESCRIPTOR_DATA * dnew, int desc )
{
    static DESCRIPTOR_DATA d_zero;
    *dnew = d_zero;
    dnew->descriptor = desc;
    dnew->character = NULL;
    dnew->connected = CON_GET_NAME;
    dnew->showstr_head = alloc_mem( strlen( " " ) + 1 );
    strcpy( dnew->showstr_head, "" );
    dnew->showstr_point = 0;
    dnew->pEdit = NULL;         /* OLC */
    dnew->pString = NULL;       /* OLC */
    dnew->editor = 0;           /* OLC */
    dnew->outsize = 2000;
    dnew->outbuf = alloc_mem( dnew->outsize );

/*     {
	int i;
	for (i = 0;i < MAX_HISTORY;i++)
	dnew->hist_cmd[i][0] = '\0';
     } */
}

/* Recover from a copyover - load players */
void copyover_recover( void )
{
    DESCRIPTOR_DATA *dnew;
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    char name[100];
    char host[MSL];
    int desc;
    int close args( ( int fd ) );
    bool fOld;

    sprintf( buf, "%s/%s", sysconfig.area_dir, COPYOVER_FILE );
    fp = fopen( buf, "r" );

    if ( !fp )                  /* there are some descriptors open which will hang forever then ? */
    {
#if defined(cbuilder)
        logf_string( "Error: copyover_recover: fopen %s", COPYOVER_FILE );
        return -1;
#else
        perror( "copyover_recover:fopen" );
        exit( 1 );
#endif
    }

    unlink( COPYOVER_FILE );    /* In case something crashes - doesn't prevent reading  */

    for ( ;; )
    {
        fscanf( fp, "%d %s %s\n", &desc, name, host );
        if ( desc == -1 )
            break;

        /* Write something, and check if it goes error-free */
        if ( !write_to_descriptor
             ( desc, "\n\rThe earth reforms, new to you as you know it...\n\r",
               52 ) )
        {
            close( desc );      /* nope */
            continue;
        }

        dnew = alloc_perm( sizeof( DESCRIPTOR_DATA ) );
        init_descriptor( dnew, desc );  /* set up various stuff */

        dnew->host = str_dup( host );
        dnew->next = descriptor_list;
        descriptor_list = dnew;
        dnew->connected = CON_COPYOVER_RECOVER; /* -15, so close_socket frees the char */

        /* Now, find the pfile */

        fOld = load_char_obj( dnew, name );

        if ( !fOld )            /* Player file not found?! */
        {
            write_to_descriptor( desc,
                                 "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r",
                                 100 );
            close_socket( dnew );
        }
        else                    /* ok! */
        {
            /*              write_to_descriptor (desc, "\n\rCopyover recovery complete.\n\r",0); */

            /* Just In Case */
            if ( !dnew->character->in_room )
                dnew->character->in_room = get_room_index( ROOM_VNUM_TEMPLE );

            /* Insert in the char_list */
            dnew->character->next = char_list;
            char_list = dnew->character;

            /* Inset into the player_list */
            dnew->character->next_player = player_list;
            player_list = dnew->character;

            char_to_room( dnew->character, dnew->character->in_room );
            do_look( dnew->character, "" );
            act( "$n materializes!", dnew->character, NULL, NULL, TO_ROOM );
            dnew->connected = CON_PLAYING;

            /* Somehow this was left out, and it was causing a crash whenever
               a player had a pet, and the mud was hotbooted. It was trying
               to recall a pet that was never even loaded, thus resulting in
               the death of the mud.
               -Lancelight
             */
            if ( dnew->character->pet != NULL )
            {
                char_to_room( dnew->character->pet, dnew->character->in_room );
                act( "$n materializes!.", dnew->character->pet, NULL, NULL,
                     TO_ROOM );
            }

#if defined(cbuilder)
            AddUser( dnew->character );
#endif
        }

    }

    fCopyOver = FALSE;
}

#if defined(unix)
/* RT max open files fix */

void maxfilelimit(  )
{
    struct rlimit r;

    getrlimit( RLIMIT_NOFILE, &r );
    r.rlim_cur = r.rlim_max;
    setrlimit( RLIMIT_NOFILE, &r );
}
#endif

/*
 * Big mama top level function.
 */
int boot_db(  )
{
#if defined(unix)
    /* open file fix */
    maxfilelimit(  );
#endif

    /*
     * Load config variables from config file
     */
    if ( !load_config_file(  ) )
        return -1;

    /*
     * Init some data space stuff.
     */
    {
        init_string_space(  );
        fBootDb = TRUE;
    }

    /*
     * Init random number generator.
     */
    {
        init_mm(  );
    }
/* Set the auction channel to NULL and get it ready for use. -Lancelight */

    auction_info.item = NULL;
    auction_info.owner = NULL;
    auction_info.high_bidder = NULL;
    auction_info.current_bid = 0;
    auction_info.status = 0;
    auction_info.gold_held = 0;
/*    auction_info.silver_held    = 0; Uncoment this for dual monatary
systems. -Lancelight */

    /*
     * Set time and weather.
     */
    {
        long lhour, lday, lmonth;

        lhour =
            ( current_time - 650336715 ) / ( PULSE_TICK / PULSE_PER_SECOND );
        time_info.hour = lhour % 24;
        lday = lhour / 24;
        time_info.day = lday % 35;
        lmonth = lday / 35;
        time_info.month = lmonth % 17;
        time_info.year = lmonth / 17;

        if ( time_info.hour < 5 )
            weather_info.sunlight = SUN_DARK;
        else if ( time_info.hour < 6 )
            weather_info.sunlight = SUN_RISE;
        else if ( time_info.hour < 19 )
            weather_info.sunlight = SUN_LIGHT;
        else if ( time_info.hour < 20 )
            weather_info.sunlight = SUN_SET;
        else
            weather_info.sunlight = SUN_DARK;

        weather_info.change = 0;
        weather_info.mmhg = 960;
        if ( time_info.month >= 7 && time_info.month <= 12 )
            weather_info.mmhg += number_range( 1, 50 );
        else
            weather_info.mmhg += number_range( 1, 80 );

        if ( weather_info.mmhg <= 980 )
            weather_info.sky = SKY_LIGHTNING;
        else if ( weather_info.mmhg <= 1000 )
            weather_info.sky = SKY_RAINING;
        else if ( weather_info.mmhg <= 1020 )
            weather_info.sky = SKY_CLOUDY;
        else
            weather_info.sky = SKY_CLOUDLESS;
    }

    /*
     * Assign gsn's for skills which have them.
     */
    {
        int sn;

        for ( sn = 0; sn < MAX_SKILL; sn++ )
        {
            if ( skill_table[sn].pgsn != NULL )
                *skill_table[sn].pgsn = sn;
        }
    }

    /*
     * Read in all the area files.
     */
    {
        FILE *fpList;

        sprintf( strPath, "%s/%s", sysconfig.area_dir, sysconfig.area_list );

        if ( ( fpList = fopen( strPath, "r" ) ) == NULL )
        {
#if defined(cbuilder)
            logf_string( "Error: fopen %s", sysconfig.area_list );
            return -1;
#else
            perror( sysconfig.area_list );
            exit( 1 );
#endif
        }

        sprintf( strPath, "%s/%s", sysconfig.area_dir,
                 sysconfig.mudprogs_file );

        /* Load Mob/Obj/RoomProgs */
        if ( ( fpArea = fopen( strPath, "r" ) ) == NULL )
        {
#if defined(cbuilder)
            logf_string( "Error: fopen %s", sysconfig.mudprogs_file );
            return -1;
#else
            perror( strPath );
            exit( 1 );
#endif
        }
        load_mudprogs( fpArea );
        fclose( fpArea );

        sprintf( strPath, "%s/%s", sysconfig.area_dir,
                 sysconfig.factions_file );

        /* Load factions */
        if ( ( fpArea = fopen( strPath, "r" ) ) == NULL )
        {
#if defined(cbuilder)
            logf_string( "Error: fopen %s", sysconfig.factions_file );
            return -1;
#else
            perror( strPath );
            exit( 1 );
#endif
        }

        load_factions( fpArea );
        fclose( fpArea );

        sprintf( strPath, "%s/%s", sysconfig.area_dir, sysconfig.socials_file );

        /* Load Socials */
        if ( ( fpArea = fopen( strPath, "r" ) ) == NULL )
        {
#if defined(cbuilder)
            logf_string( "Error: fopen %s", sysconfig.socials_file );
            return -1;
#else
            perror( strPath );
            exit( 1 );
#endif
        }

        load_socials( fpArea );
        fclose( fpArea );

        for ( ;; )
        {
            strcpy( strArea, fread_word( fpList ) );
            if ( strArea[0] == '$' )
                break;

            if ( strArea[0] == '-' )
            {
                fpArea = stdin;
            }
            else
            {
                sprintf( strPath, "%s/%s", sysconfig.area_dir, strArea );
                if ( ( fpArea = fopen( strPath, "r" ) ) == NULL )
                {
#if defined(cbuilder)
                    logf_string( "Error: fopen %s", strPath );
                    return -1;
#else
                    perror( strPath );
                    exit( 1 );
#endif
                }
            }

            for ( ;; )
            {
                char *word;

                if ( fread_letter( fpArea ) != '#' )
                {
                    bug( "Boot_db: # not found.", 0 );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                word = fread_word( fpArea );

                if ( word[0] == '$' )
                    break;
                else if ( !str_cmp( word, "AREADATA" ) )
                    load_area( fpArea );
                else if ( !str_cmp( word, "CLANS" ) )
                    load_clans( fpArea );
                else if ( !str_cmp( word, "HELPS" ) )
                    if ( !str_cmp( sysconfig.help_file, strArea ) )
                        load_helps( fpArea );
                    else
                        bug( "%s contains a #HELPS section, only %s can contain #HELPS.", strArea, sysconfig.help_file );

                else if ( !str_cmp( word, "TODO" ) )
                    load_todo( fpArea );
                else if ( !str_cmp( word, "MOBILES" ) )
                    load_mobiles( fpArea );
                else if ( !str_cmp( word, "PROGS" ) )
                    load_progs( fpArea );
                else if ( !str_cmp( word, "OBJECTS" ) )
                    load_objects( fpArea );
                else if ( !str_cmp( word, "RESETS" ) )
                    skip_section( fpArea, word );
                else if ( !str_cmp( word, "ROOMS" ) )
                    load_rooms( fpArea );
                else if ( !str_cmp( word, "SHOPS" ) )
                    skip_section( fpArea, word );
/*				else if ( !str_cmp( word,
"SOCIALS"  ) ) load_socials  (fpArea);*/
                else if ( !str_cmp( word, "FACTIONAFFS" ) )
                    skip_section( fpArea, word );
                else
                {
                    bug( "Boot_db: bad section name.", 0 );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }
            }

            if ( fpArea != stdin )
                fclose( fpArea );
            fpArea = NULL;
        }
        fclose( fpList );

        sprintf( strPath, "%s/%s", sysconfig.area_dir, sysconfig.area_list );

        /* Load resets and shops only after everything else is loaded in */
        if ( ( fpList = fopen( strPath, "r" ) ) == NULL )
        {
#if defined(cbuilder)
            logf_string( "Error: fopen %s", strPath );
            return -1;
#else
            perror( strPath );
            exit( 1 );
#endif
        }

        for ( ;; )
        {
            strcpy( strArea, fread_word( fpList ) );
            if ( strArea[0] == '$' )
                break;

            if ( strArea[0] == '-' )
            {
                fpArea = stdin;
            }
            else
            {
                sprintf( strPath, "%s/%s", sysconfig.area_dir, strArea );
                if ( ( fpArea = fopen( strPath, "r" ) ) == NULL )
                {
#if defined(cbuilder)
                    logf_string( "Error: fopen %s", strPath );
                    return -1;
#else
                    perror( strPath );
                    exit( 1 );
#endif
                }
            }

            for ( ;; )
            {
                char *word;

                if ( fread_letter( fpArea ) != '#' )
                {
                    bug( "Boot_db: # not found.", 0 );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                word = fread_word( fpArea );

                if ( word[0] == '$' )
                    break;
                /* OLC */
                else if ( !str_cmp( word, "AREADATA" ) )
                    skip_section( fpArea, word );   /*load_area    (fpArea) */
                else if ( !str_cmp( word, "CLANS" ) )
                    skip_section( fpArea, word );   /*load_clans   (fparea) */
                else if ( !str_cmp( word, "HELPS" ) )
                    skip_section( fpArea, word );   /*load_helps   (fpArea) */
                else if ( !str_cmp( word, "TODO" ) )
                    skip_section( fpArea, word );   /*load_todo   (fpArea) */
                else if ( !str_cmp( word, "MOBILES" ) )
                    skip_section( fpArea, word );   /*load_mobiles (fpArea) */
                else if ( !str_cmp( word, "PROGS" ) )
                    skip_section( fpArea, word );   /*load_progs   (fpArea) */
                else if ( !str_cmp( word, "OBJECTS" ) )
                    skip_section( fpArea, word );   /*load_objects (fpArea) */
                else if ( !str_cmp( word, "RESETS" ) )
                    load_resets( fpArea );
                else if ( !str_cmp( word, "ROOMS" ) )
                    skip_section( fpArea, word );   /*load_rooms   (fpArea) */
                else if ( !str_cmp( word, "SHOPS" ) )
                    load_shops( fpArea );
/*				else if ( !str_cmp( word,
"SOCIALS"  ) ) skip_section (fpArea,word);*//*load_socials (fpArea) */
                else if ( !str_cmp( word, "FACTIONAFFS" ) )
                    load_factionaffs( fpArea );
                else
                {
                    bug( "Boot_db: bad section name.", 0 );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }
            }

            if ( fpArea != stdin )
                fclose( fpArea );
            fpArea = NULL;
        }
        fclose( fpList );
    }

    /*
     * Fix up exits.
     * Declare db booting over.
     * Reset all areas once.
     * Load up the notes file.
     */
    {
        fix_exits(  );
        fBootDb = FALSE;
        boot_done(  );
        init_supermob(  );
        area_update(  );
        load_boards(  );        /* Load all boards */
        load_bans(  );          /* Letz ban those SOB's. -Lancelight */
        load_classes(  );       /* Load classes for Askill usage -Lancelight */
        load_disabled(  );      /* Load disabled commands. -Lancelight */
        save_notes(  );
        MOBtrigger = TRUE;
    }

    if ( !help_greeting )       /* Hugin */
    {
        bug( "boot_db: No help_greeting read.", 0 );
        help_greeting = "By what name do you wish to be known ? ";
    }

    if ( fCopyOver )
        copyover_recover(  );

    return 0;
}

/***************************************************************************/

/*
 * Skip a section.
 */
void skip_section( FILE * fp, char *section )
{
    int number;
    char *word;
    char letter;

    if ( !str_cmp( section, "AREADATA" ) )
    {
        for ( ;; )
        {
            word = feof( fp ) ? "End" : fread_word( fp );
            if ( !str_cmp( word, "End" ) )
                return;
            fread_to_eol( fp );
        }
    }
    if ( !str_cmp( section, "HELPS" ) )
    {
        for ( ;; )
        {
            number = fread_number( fp );
            word = fread_string( fp );
            if ( word[0] == '$' )
                return;
            fread_string( fp );
        }
    }

    if ( !str_cmp( section, "TODO" ) )
    {
        for ( ;; )
        {
            number = fread_number( fp );
            word = fread_string( fp );
            if ( word[0] == '$' )
                return;
            fread_string( fp );
        }
    }
    if ( !str_cmp( section, "MOBILES" ) || !str_cmp( section, "OBJECTS" )
         || !str_cmp( section, "ROOMS" ) )
    {
        for ( ;; )
        {
            word = feof( fp ) ? "#0" : fread_string_eol( fp );
            if ( word[0] == '#' && word[1] == '0' )
                return;
        }
    }
    if ( !str_cmp( section, "PROGS" ) || !str_cmp( section, "RESETS" ) )
    {
        for ( ;; )
        {
            switch ( letter = fread_letter( fp ) )
            {
            case 'S':
                if ( !str_cmp( section, "RESETS" ) )
                    return;
            case 's':
                fread_to_eol( fp );
                return;
            }
            fread_to_eol( fp );
        }
    }
    if ( !str_cmp( section, "SHOPS" ) )
    {
        for ( ;; )
        {
            number = fread_number( fp );
            if ( number == 0 )
                return;
            fread_to_eol( fp );
        }
    }
    if ( !str_cmp( section, "FACTIONAFFS" ) )
    {
        for ( ;; )
        {
            number = fread_number( fp );
            if ( number == 0 )
                return;
            fread_to_eol( fp );
        }
    }
/*	if (!str_cmp( section, "SOCIALS" ))
	{
		for ( ; ; )
		{
			word = fread_word( fp );
			if ( !strcmp( word, "#0" ) )
				return;
			fread_to_eol( fp );
		}
	}*/

    if ( !str_cmp( section, "CLANS" ) )
    {
        for ( ;; )
        {
            word = fread_word( fp );
            if ( !strcmp( word, "#$" ) )
            {
                return;
            }
            fread_to_eol( fp );
        }
    }
    bug( "skip_section: Invalid section name.", 0 );
#if defined(cbuilder)
    return -1;
#else
    exit( 1 );
#endif
}

/*
 * OLC
 * Use these macros to load any new area formats that you choose to
 * support on your MUD.  See the load_area format below for
 * a short example.
 */
#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                \
		if ( !str_cmp( word, literal ) )    \
		{                                   \
		    field  = value;                 \
		    fMatch = TRUE;                  \
		    break;                          \
				}

#define SKEY( string, field )                       \
		if ( !str_cmp( word, string ) )     \
		{                                   \
		    free_string( &field );           \
		    field = fread_string( fp );     \
		    fMatch = TRUE;                  \
		    break;                          \
				}

/* OLC
 * Snarf an 'area' header line.   Check this format.  MUCH better.  Add fields
 * too.
 *
 * #AREAFILE
 * Name   { All } Locke    Newbie School~
 * Repop  A teacher pops in the room and says, 'Repop coming!'~
 * Recall 3001
 * End
 */
void load_area( FILE * fp )
{
    AREA_DATA *pArea;
    AREA_DATA *pTempArea;
    char *word = NULL;
    bool fMatch;

    pArea = alloc_perm( sizeof( *pArea ) );
    pArea->age = 15;
    pArea->nplayer = 0;
    pArea->filename = str_dup( strArea );
    pArea->vnum = top_area;
    pArea->name = str_dup( "New Area" );
    pArea->builders = str_dup( "" );
    pArea->security = 9;        /* 9 -- Hugin */
    pArea->lvnum = 0;
    pArea->uvnum = 0;
    pArea->area_flags = 0;
/*  pArea->recall       = ROOM_VNUM_TEMPLE;        ROM OLC */

    for ( ;; )
    {
        word = feof( fp ) ? "End" : fread_word( fp );
        fMatch = FALSE;

        switch ( UPPER( word[0] ) )
        {
        case 'N':
            SKEY( "Name", pArea->name );
            break;
        case 'S':
            KEY( "Security", pArea->security, fread_number( fp ) );
            break;
        case 'V':
            if ( !str_cmp( word, "VNUMs" ) )
            {
                pArea->lvnum = fread_number( fp );
                pArea->uvnum = fread_number( fp );

            }
            break;
        case 'E':
            if ( !str_cmp( word, "End" ) )
            {
                fMatch = TRUE;
                if ( area_first == NULL )
                    area_first = pArea;
                if ( area_last != NULL )
                    area_last->next = pArea;
                area_last = pArea;
                pArea->next = NULL;
                top_area++;

                /* Check to ensure we have unique vnums. */
                for ( pTempArea = area_first; pTempArea;
                      pTempArea = pTempArea->next )
                    if ( pTempArea->vnum != pArea->vnum
                         &&
                         ( ( pArea->lvnum >= pTempArea->lvnum
                             && pArea->lvnum <= pTempArea->uvnum )
                           || ( pArea->uvnum >= pTempArea->lvnum
                                && pArea->uvnum <= pTempArea->uvnum ) ) )
                    {
                        bug( "Overlapping vnum range!", pArea->name );
#if defined(cbuilder)
                        return -1;
#else
                        exit( 1 );
#endif
                    }
                return;
            }
            break;
        case 'B':
            SKEY( "Builders", pArea->builders );
            break;
/* removed for ROM OLC
	    case 'R':
			KEY( "Recall", pArea->recall, fread_number( fp ) );
			break;
*/
        }
    }
}

/*
 * Sets vnum range for area using OLC protection features.
 */
void assign_area_vnum( int vnum )
{
    if ( area_last->lvnum == 0 || area_last->uvnum == 0 )
        area_last->lvnum = area_last->uvnum = vnum;
    if ( vnum != URANGE( area_last->lvnum, vnum, area_last->uvnum ) )
    {
        if ( vnum < area_last->lvnum )
            area_last->lvnum = vnum;
        else
            area_last->uvnum = vnum;
    }
    return;
}

/*
 * Snarf a help section.
 */
void load_helps( FILE * fp )
{
    HELP_DATA *pHelp;

    for ( ;; )
    {
        pHelp = alloc_perm( sizeof( *pHelp ) );
        pHelp->level = fread_number( fp );
        pHelp->keyword = fread_string( fp );
        if ( pHelp->keyword[0] == '$' )
            break;
        pHelp->text = fread_string( fp );

        if ( !str_cmp( pHelp->keyword, "greeting" ) )
            help_greeting = pHelp->text;

        if ( !str_cmp( pHelp->keyword, "ansigreet" ) )
            ansi_greeting = pHelp->text;

        if ( help_first == NULL )
            help_first = pHelp;
        if ( help_last != NULL )
            help_last->next = pHelp;

        help_last = pHelp;
        pHelp->next = NULL;
        top_help++;
    }

    return;
}

void load_todo( FILE * fp )
{
    TODO_DATA *pTodo;

    for ( ;; )
    {
        pTodo = alloc_perm( sizeof( *pTodo ) );
        pTodo->level = fread_number( fp );
        pTodo->keyword = fread_string( fp );
        if ( pTodo->keyword[0] == '$' )
            break;
        pTodo->text = fread_string( fp );
/*
	if ( !str_cmp( pTodo->keyword, "greeting" ) )
	    help_greeting = pHelp->text;
       
       if (!str_cmp(pHelp->keyword, "ansigreet"))
	 ansi_greeting = pHelp->text;*/

        if ( todo_first == NULL )
            todo_first = pTodo;
        if ( todo_last != NULL )
            todo_last->next = pTodo;

        todo_last = pTodo;
        pTodo->next = NULL;
        top_todo++;
    }

    return;
}

/*
 * Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c
 */
void new_reset( ROOM_INDEX_DATA * pR, RESET_DATA * pReset )
{
    RESET_DATA *pr;

    if ( !pR )
        return;

    pr = pR->reset_last;

    if ( !pr )
    {
        pR->reset_first = pReset;
        pR->reset_last = pReset;
    }
    else
    {
        pR->reset_last->next = pReset;
        pR->reset_last = pReset;
        pR->reset_last->next = NULL;
    }

    top_reset++;
    return;
}

/*
 * Snarf a reset section.       Changed for OLC.
 */
void load_resets( FILE * fp )
{
    RESET_DATA *pReset = NULL;
    int iLastRoom = 0;
    int iLastObj = 0;

    if ( !area_last )
    {
        bug( "Load_resets: no #AREA seen yet.", 0 );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    for ( ;; )
    {
        EXIT_DATA *pexit;
        ROOM_INDEX_DATA *pRoomIndex;
        char letter = 0;

        if ( ( letter = fread_letter( fp ) ) == 'S' )
            break;

        if ( letter == '*' )
        {
            fread_to_eol( fp );
            continue;
        }

        pReset = alloc_perm( sizeof( *pReset ) );
        pReset->command = letter;
        /* if_flag */ fread_number( fp );
        pReset->vnum = fread_number( fp );
        pReset->arg2 = fread_number( fp );
        pReset->arg3 = ( letter == 'G' || letter == 'R' )
            ? 0 : fread_number( fp );
        fread_to_eol( fp );

        /*
         * Validate parameters.
         * We're calling the index functions for the side effect.
         */
        switch ( letter )
        {
        default:
            bug( "Load_resets: bad command '%c'.", letter );
#if defined(cbuilder)
            return -1;
#else
            exit( 1 );
#endif
            break;

        case 'M':
            get_mob_index( pReset->vnum );
            if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) )
            {
                new_reset( pRoomIndex, pReset );
                iLastRoom = pReset->arg3;
            }
            break;

        case 'O':
            get_obj_index( pReset->vnum );
            if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) )
            {
                new_reset( pRoomIndex, pReset );
                iLastObj = pReset->arg3;
            }
            break;

        case 'P':
            get_obj_index( pReset->vnum );
            if ( ( pRoomIndex = get_room_index( iLastObj ) ) )
            {
                new_reset( pRoomIndex, pReset );
            }
            break;

        case 'G':
        case 'E':
            get_obj_index( pReset->vnum );
            if ( ( pRoomIndex = get_room_index( iLastRoom ) ) )
            {
                new_reset( pRoomIndex, pReset );
                iLastObj = iLastRoom;
            }
            break;

        case 'D':
            pRoomIndex = get_room_index( pReset->vnum );

            if ( pReset->arg2 < 0
                 || pReset->arg2 > 5
                 || !pRoomIndex
                 || !( pexit = pRoomIndex->exit[pReset->arg2] )
                 || !IS_SET( pexit->rs_flags, EX_ISDOOR ) )
            {
                bug( "Load_resets: 'D': exit %d not door.", pReset->arg2 );
#if defined(cbuilder)
                return -1;
#else
                exit( 1 );
#endif
            }

            switch ( pReset->arg3 )
            {
            default:
                bug( "Load_resets: 'D': bad 'locks': %d.", pReset->arg3 );
            case 0:
                SET_BIT( pexit->rs_flags, EX_CLOSED );
                break;
            case 1:
                SET_BIT( pexit->rs_flags, EX_LOCKED );
                SET_BIT( pexit->rs_flags, EX_CLOSED );
                break;
            case 2:
                SET_BIT( pexit->rs_flags, EX_PICKPROOF );
                SET_BIT( pexit->rs_flags, EX_CLOSED );
                SET_BIT( pexit->rs_flags, EX_LOCKED );
                break;
            case 3:
                SET_BIT( pexit->rs_flags, EX_PASSPROOF );
                SET_BIT( pexit->rs_flags, EX_CLOSED );
                SET_BIT( pexit->rs_flags, EX_LOCKED );
                break;
            case 4:
                SET_BIT( pexit->rs_flags, EX_PICKPROOF );
                SET_BIT( pexit->rs_flags, EX_PASSPROOF );
                SET_BIT( pexit->rs_flags, EX_CLOSED );
                SET_BIT( pexit->rs_flags, EX_LOCKED );
                break;
            case 5:
                SET_BIT( pexit->rs_flags, EX_CLOSED );
                SET_BIT( pexit->rs_flags, EX_HIDDEN );
                break;
            case 6:
                SET_BIT( pexit->rs_flags, EX_HIDDEN );
                SET_BIT( pexit->rs_flags, EX_CLOSED );
                SET_BIT( pexit->rs_flags, EX_LOCKED );
                break;
            case 7:
                SET_BIT( pexit->rs_flags, EX_HIDDEN );
                SET_BIT( pexit->rs_flags, EX_PASSPROOF );
                SET_BIT( pexit->rs_flags, EX_CLOSED );
                SET_BIT( pexit->rs_flags, EX_LOCKED );
                break;
            case 8:
                SET_BIT( pexit->rs_flags, EX_PICKPROOF );
                SET_BIT( pexit->rs_flags, EX_PASSPROOF );
                SET_BIT( pexit->rs_flags, EX_CLOSED );
                SET_BIT( pexit->rs_flags, EX_LOCKED );
                SET_BIT( pexit->rs_flags, EX_HIDDEN );
                break;
            case 9:
                SET_BIT( pexit->rs_flags, EX_PICKPROOF );
                SET_BIT( pexit->rs_flags, EX_HIDDEN );
                SET_BIT( pexit->rs_flags, EX_CLOSED );
                SET_BIT( pexit->rs_flags, EX_LOCKED );
                break;

            }
            /*
             * By calling new_reset we are assigning reset data for doors.
             * This data is not used in updating the game any longer.  But
             * displaying resets in this manner may be to your liking.
             * I have left the code here so you may do so.  Uncomment data in
             * display_resets in olc.c if you wish to do this.
             *
             *
             new_reset( pRoomIndex, pReset );
             *
             * End Resets Comment.
             */
            break;

        case 'R':
            if ( pReset->arg2 < 0 || pReset->arg2 > 6 ) /* Last Door. */
            {
                bug( "Load_resets: 'R': bad exit %d.", pReset->arg2 );
#if defined(cbuilder)
                return -1;
#else
                exit( 1 );
#endif
            }

            if ( ( pRoomIndex = get_room_index( pReset->vnum ) ) )
                new_reset( pRoomIndex, pReset );

            break;
        }
    }

    return;
}

/*
 * Snarf a room section.
 */
void load_rooms( FILE * fp )
{
    ROOM_INDEX_DATA *pRoomIndex;

    if ( area_last == NULL )
    {
        bug( "load_rooms: no #AREA seen yet.", 0 );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    for ( ;; )
    {
        sh_int vnum;
        char letter;
        int door;
        int iHash;

        letter = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_rooms: # not found.", 0 );
#if defined(cbuilder)
            return -1;
#else
            exit( 1 );
#endif
        }

        vnum = fread_number( fp );
        if ( vnum == 0 )
            break;

        fBootDb = FALSE;
        if ( get_room_index( vnum ) != NULL )
        {
            bug( "Load_rooms: vnum %d duplicated.", vnum );
#if defined(cbuilder)
            return -1;
#else
            exit( 1 );
#endif
        }
        fBootDb = TRUE;

        pRoomIndex = alloc_perm( sizeof( *pRoomIndex ) );
        pRoomIndex->people = NULL;
        pRoomIndex->contents = NULL;
        pRoomIndex->extra_descr = NULL;
        pRoomIndex->area = area_last;
        pRoomIndex->vnum = vnum;
        pRoomIndex->name = fread_string( fp );
        pRoomIndex->description = fread_string( fp );
        /* Area number */ fread_number( fp );
        pRoomIndex->room_flags = fread_flag( fp );
        pRoomIndex->sector_type = fread_number( fp );
        pRoomIndex->light = 0;
        for ( door = 0; door <= 5; door++ )
            pRoomIndex->exit[door] = NULL;

        for ( ;; )
        {
            letter = fread_letter( fp );

            if ( letter == 'S' )
                break;

            if ( letter == 'D' )
            {
                EXIT_DATA *pexit;
                int locks;

                door = fread_number( fp );
                if ( door < 0 || door > 5 )
                {
                    bug( "Fread_rooms: vnum %d has bad door number.", vnum );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                pexit = alloc_perm( sizeof( *pexit ) );
                pexit->description = fread_string( fp );
                pexit->keyword = fread_string( fp );
                pexit->exit_info = 0;
                pexit->rs_flags = 0;    /* OLC */
                locks = fread_number( fp );
                pexit->key = fread_number( fp );
                pexit->u1.vnum = fread_number( fp );
                pexit->orig_door = door;    /* OLC */

                switch ( locks )    /* OLC exit_info to rs_flags. */
                {
                case 1:
                    pexit->rs_flags = EX_ISDOOR;
                    break;
                case 2:
                    pexit->rs_flags = EX_ISDOOR | EX_PICKPROOF;
                    break;
                case 3:
                    pexit->rs_flags = EX_ISDOOR | EX_PASSPROOF | EX_HIDDEN;
                    break;
                case 4:
                    pexit->rs_flags =
                        EX_ISDOOR | EX_PICKPROOF | EX_PASSPROOF | EX_HIDDEN;
                    break;
                case 5:
                    pexit->rs_flags = EX_ISDOOR | EX_PASSPROOF;
                    break;
                case 6:
                    pexit->rs_flags = EX_ISDOOR | EX_PICKPROOF | EX_PASSPROOF;
                    break;
                case 7:
                    pexit->rs_flags = EX_ISDOOR | EX_HIDDEN;
                    break;
                case 8:
                    pexit->rs_flags = EX_ISDOOR | EX_PICKPROOF | EX_HIDDEN;
                    break;
                }

                pRoomIndex->exit[door] = pexit;
                top_exit++;
            }
            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA *ed;

                ed = alloc_perm( sizeof( *ed ) );
                ed->keyword = fread_string( fp );
                ed->description = fread_string( fp );
                ed->next = pRoomIndex->extra_descr;
                pRoomIndex->extra_descr = ed;
                top_ed++;
            }
            else
            {
                bug( "Load_rooms: vnum %d has flag not 'DES'.", vnum );
#if defined(cbuilder)
                return -1;
#else
                exit( 1 );
#endif
            }
        }

        iHash = vnum % MAX_KEY_HASH;
        pRoomIndex->next = room_index_hash[iHash];
        room_index_hash[iHash] = pRoomIndex;
        top_room++;
        top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room;    /* OLC */
        assign_area_vnum( vnum );   /* OLC */
    }

    return;
}

/*
 * Snarf a shop section.
 */
void load_shops( FILE * fp )
{
    SHOP_DATA *pShop;

    for ( ;; )
    {
        MOB_INDEX_DATA *pMobIndex;
        int iTrade;

        pShop = alloc_perm( sizeof( *pShop ) );
        pShop->keeper = fread_number( fp );
        if ( pShop->keeper == 0 )
            break;
        for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
            pShop->buy_type[iTrade] = fread_number( fp );
        pShop->profit_buy = fread_number( fp );
        pShop->profit_sell = fread_number( fp );
        pShop->open_hour = fread_number( fp );
        pShop->close_hour = fread_number( fp );
        fread_to_eol( fp );
        pMobIndex = get_mob_index( pShop->keeper );
        pMobIndex->pShop = pShop;

        if ( shop_first == NULL )
            shop_first = pShop;
        if ( shop_last != NULL )
            shop_last->next = pShop;

        shop_last = pShop;
        pShop->next = NULL;
        top_shop++;
    }

    return;
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void )
{
    char buf[MAX_STRING_LENGTH];
    extern const sh_int rev_dir[];
    ROOM_INDEX_DATA *pRoomIndex;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;
    int iHash;
    int door;

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( pRoomIndex = room_index_hash[iHash];
              pRoomIndex != NULL; pRoomIndex = pRoomIndex->next )
        {
            bool fexit;

            fexit = FALSE;
            for ( door = 0; door <= 5; door++ )
            {
                if ( ( pexit = pRoomIndex->exit[door] ) != NULL )
                {
                    if ( pexit->u1.vnum <= 0
                         || get_room_index( pexit->u1.vnum ) == NULL )
                        pexit->u1.to_room = NULL;
                    else
                    {
                        fexit = TRUE;
                        pexit->u1.to_room = get_room_index( pexit->u1.vnum );
                    }
                }
            }
            if ( !fexit )
                SET_BIT( pRoomIndex->room_flags, ROOM_NO_MOB );
        }
    }

    for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    {
        for ( pRoomIndex = room_index_hash[iHash];
              pRoomIndex != NULL; pRoomIndex = pRoomIndex->next )
        {
            for ( door = 0; door <= 5; door++ )
            {
                if ( ( pexit = pRoomIndex->exit[door] ) != NULL
                     && ( to_room = pexit->u1.to_room ) != NULL
                     && ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
                     && pexit_rev->u1.to_room != pRoomIndex
                     && ( pRoomIndex->vnum < 1200 || pRoomIndex->vnum > 1299 ) )
                {
                    sprintf( buf, "Fix_exits: %d:%d -> %d:%d -> %d.",
                             pRoomIndex->vnum, door,
                             to_room->vnum, rev_dir[door],
                             ( pexit_rev->u1.to_room == NULL )
                             ? 0 : pexit_rev->u1.to_room->vnum );
                    bug( buf, 0 );
                }
            }
        }
    }

    return;
}

/*
 * Repopulate areas periodically.
 */
void area_update( void )
{
    AREA_DATA *pArea;

    for ( pArea = area_first; pArea != NULL; pArea = pArea->next )
    {

        if ( ++pArea->age < 3 )
            continue;

        /*
         * Check age and reset.
         * Note: Mud School resets every 3 minutes (not 15).
         */
        if ( ( !pArea->empty && ( pArea->nplayer == 0 || pArea->age >= 15 ) )
             || pArea->age >= 31 )
        {
            ROOM_INDEX_DATA *pRoomIndex;

            reset_area( pArea );
            pArea->age = number_range( 0, 3 );
            pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );
            if ( pRoomIndex != NULL && pArea == pRoomIndex->area )
                pArea->age = 15 - 2;
            else if ( pArea->nplayer == 0 )
                pArea->empty = TRUE;
        }
    }

    return;
}

/* OLC
 * Reset one room.  Called by reset_area and olc.
 */
void reset_room( ROOM_INDEX_DATA * pRoom )
{
    RESET_DATA *pReset = NULL;
    CHAR_DATA *pMob = NULL;
    OBJ_DATA *pObj = NULL;
    CHAR_DATA *LastMob = NULL;
    OBJ_DATA *LastObj = NULL;
    int iExit;
    int level = 0;
    int num_resets = 0;

    if ( !pRoom )
        return;

    for ( iExit = 0; iExit < MAX_DIR; iExit++ )
    {
        EXIT_DATA *pExit;
        if ( ( pExit = pRoom->exit[iExit] )
             /*  && !IS_SET( pExit->exit_info, EX_BASHED )   ROM OLC */  )
        {
            pExit->exit_info = pExit->rs_flags;
            if ( ( pExit->u1.to_room != NULL )
                 && ( ( pExit = pExit->u1.to_room->exit[rev_dir[iExit]] ) ) )
            {
                /* nail the other side */
                pExit->exit_info = pExit->rs_flags;
            }
        }
    }

    for ( pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next )
    {
        MOB_INDEX_DATA *pMobIndex;
        OBJ_INDEX_DATA *pObjIndex;
        OBJ_INDEX_DATA *pObjToIndex;
        ROOM_INDEX_DATA *pRoomIndex;

        switch ( pReset->command )
        {
        default:
            bug( "Reset_room: bad command %c.", pReset->command );
            break;

        case 'M':
            /* Reset the last mob to null so we can tell later if this mob
             * loaded or not.  -Zane */
            LastMob = NULL;

            /*
             * Find out how many mobs remain for this reset and what the max is.
             * If current < max then spawn a mob.
             */
            if ( pReset->arg2 >= 1 )
                num_resets = pReset->arg2 - pReset->active_count;
            else if ( pReset->arg2 < 0 )
                num_resets = 999 - pReset->active_count;    /*If it's -1 then allow virtually infinite spawns */
            else
                num_resets = 1 - pReset->active_count;

            if ( num_resets > 0 )
            {
                if ( !( pMobIndex = get_mob_index( pReset->vnum ) ) )
                {
                    bug( "Reset_room: 'M': bad vnum %d.", pReset->vnum );
                    continue;
                }

                pMob = create_mobile( pMobIndex );

                /*
                 * Keep track of the number of mobs loaded for this reset.
                 */
                pMob->reset_count = &pReset->active_count;
                pReset->active_count++;

                /*
                 * Some more hard coding.
                 * (if a mob is loading in a dark room, give them infravision)
                 */
                if ( room_is_dark( pRoom ) )
                {
                    SET_BIT( pMob->affected_by, AFF_INFRARED );
                }

                /*
                 * Pet shop mobiles get ACT_PET set.
                 */
                {
                    ROOM_INDEX_DATA *pRoomIndexPrev;

                    pRoomIndexPrev = get_room_index( pRoom->vnum - 1 );

                    if ( pRoomIndexPrev
                         && IS_SET( pRoomIndexPrev->room_flags,
                                    ROOM_PET_SHOP ) )
                    {
                        SET_BIT( pMob->act, ACT_PET );
                    }
                }

                char_to_room( pMob, pRoom );

                LastMob = pMob;
                level = URANGE( 0, pMob->level - 2, LEVEL_HERO - 1 );   /* -1 ROM */
            }
            break;

        case 'O':
            if ( !( pObjIndex = get_obj_index( pReset->vnum ) ) )
            {
                bug( "Reset_room: 'O': bad vnum %d.", pReset->vnum );
                continue;
            }

            if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) )
            {
                bug( "Reset_room: 'O': bad vnum %d.", pReset->arg3 );
                continue;
            }

            if ( pRoom->area->nplayer > 0
                 || count_obj_list( pObjIndex, pRoom->contents ) > 0 )
            {
                break;
            }

            pObj =
                create_object( pObjIndex,
                               UMIN( number_fuzzy( level ), LEVEL_HERO - 1 ) );
            pObj->cost = 0;
            obj_to_room( pObj, pRoom );
            break;

        case 'P':
            if ( !( pObjIndex = get_obj_index( pReset->vnum ) ) )
            {
                bug( "Reset_room: 'P': bad vnum %d.", pReset->vnum );
                continue;
            }

            if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) )
            {
                bug( "Reset_room: 'P': bad vnum %d.", pReset->arg3 );
                continue;
            }

            if ( pRoom->area->nplayer > 0
                 || !( LastObj = get_obj_type( pObjToIndex ) )
                 || count_obj_list( pObjIndex, LastObj->contains ) > 0 )
            {
                break;
            }

            /* lastObj->level  -  ROM */
            pObj = create_object( pObjIndex, number_fuzzy( LastObj->level ) );
            obj_to_obj( pObj, LastObj );
            break;

        case 'G':
        case 'E':
            if ( !LastMob )
            {
                break;
            }

            /*
             * Add as many items as we did mobs on the last row.
             */
            if ( !( pObjIndex = get_obj_index( pReset->vnum ) ) )
            {
                bug( "Reset_room: 'E' or 'G': bad vnum %d.", pReset->vnum );
                continue;
            }

            if ( LastMob->pIndexData->pShop )   /* Shop-keeper? */
            {
                int olevel;

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
                    /* ROM patch weapon, treasure */
                case ITEM_WEAPON:
                    olevel = number_range( 5, 15 );
                    break;
                case ITEM_TREASURE:
                    olevel = number_range( 10, 20 );
                    break;

                    break;
                }

                pObj = create_object( pObjIndex, olevel );
                SET_BIT( pObj->extra_flags, ITEM_INVENTORY );   /* ROM OLC */

            }
            else                /* ROM OLC else version */
            {
                /*
                 * Commented out limit code because even though the limit was
                 * being calculated it wasn't being used.  -Zane
                 */
/*               int limit;

			   if (pReset->arg2 > 50 *//* old format */
/*				   || pReset->arg2 == -1 *//* no limit *//*)  
   {
   limit = 9999;
   }
   else
   {
   limit = pReset->arg2;
   } */

                pObj =
                    create_object( pObjIndex,
                                   UMIN( number_fuzzy( level ),
                                         LEVEL_HERO - 1 ) );
            }

            obj_to_char( pObj, LastMob );

            if ( pReset->command == 'E' )
                equip_char( LastMob, pObj, pReset->arg3 );
            break;

        case 'D':
            break;

        case 'R':
            if ( !( pRoomIndex = get_room_index( pReset->vnum ) ) )
            {
                bug( "Reset_room: 'R': bad vnum %d.", pReset->vnum );
                continue;
            }

            {
                EXIT_DATA *pExit;
                int d0;
                int d1;

                for ( d0 = 0; d0 < pReset->arg2 - 1; d0++ )
                {
                    d1 = number_range( d0, pReset->arg2 - 1 );
                    pExit = pRoomIndex->exit[d0];
                    pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
                    pRoomIndex->exit[d1] = pExit;
                }
            }
            break;
        }
    }

    return;
}

/* OLC
 * Reset one area.
 */
void reset_area( AREA_DATA * pArea )
{
    ROOM_INDEX_DATA *pRoom;
    int vnum;

    for ( vnum = pArea->lvnum; vnum <= pArea->uvnum; vnum++ )
    {
        if ( ( pRoom = get_room_index( vnum ) ) )
        {
            reset_room( pRoom );
        }
    }

    return;
}

/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA * pMobIndex )
{
    CHAR_DATA *mob;
    int i;

    mobile_count++;

    if ( pMobIndex == NULL )
    {
        bug( "Create_mobile: NULL pMobIndex.", 0 );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    if ( char_free == NULL )
        mob = alloc_perm( sizeof( *mob ) );
    else
    {
        mob = char_free;
        char_free = char_free->next;
    }

    clear_char( mob );
    mob->pIndexData = pMobIndex;

    mob->name = str_dup( pMobIndex->player_name );  /* OLC */
    mob->short_descr = str_dup( pMobIndex->short_descr );   /* OLC */
    mob->long_descr = str_dup( pMobIndex->long_descr ); /* OLC */
    mob->description = str_dup( pMobIndex->description );   /* OLC */

    /* read from prototype */
    mob->act = pMobIndex->act;
    mob->comm = COMM_BRIEF;
    mob->affected_by = pMobIndex->affected_by;
    strncpy( mob->newaff, pMobIndex->newaff, sizeof( mob->newaff ) );
    mob->alignment = pMobIndex->alignment;
    mob->level = pMobIndex->level;
    mob->hitroll = pMobIndex->hitroll;
    mob->breath_percent = pMobIndex->breath_percent;
    mob->rnd_obj_percent = pMobIndex->rnd_obj_percent;
    mob->rnd_obj_num = pMobIndex->rnd_obj_num;
    mob->rnd_obj_types = pMobIndex->rnd_obj_types;
    mob->damroll = pMobIndex->damage[DICE_BONUS];
    mob->max_hit = dice( pMobIndex->hit[DICE_NUMBER],
                         pMobIndex->hit[DICE_TYPE] )
        + pMobIndex->hit[DICE_BONUS];
    mob->hit = mob->max_hit;
    mob->max_mana = dice( pMobIndex->mana[DICE_NUMBER],
                          pMobIndex->mana[DICE_TYPE] )
        + pMobIndex->mana[DICE_BONUS];
    mob->mana = mob->max_mana;
    mob->damage[DICE_NUMBER] = pMobIndex->damage[DICE_NUMBER];
    mob->damage[DICE_TYPE] = pMobIndex->damage[DICE_TYPE];
    mob->dam_type = pMobIndex->dam_type;
    for ( i = 0; i < 4; i++ )
        mob->armor[i] = pMobIndex->ac[i];
    mob->off_flags = pMobIndex->off_flags;
    mob->imm_flags = pMobIndex->imm_flags;
    mob->res_flags = pMobIndex->res_flags;
    mob->vuln_flags = pMobIndex->vuln_flags;
    mob->start_pos = pMobIndex->start_pos;
    mob->default_pos = pMobIndex->default_pos;
    mob->sex = pMobIndex->sex;
    if ( mob->sex == 3 )        /* random sex */
        mob->sex = number_range( 1, 2 );
    mob->race = pMobIndex->race;
    if ( pMobIndex->gold == 0 )
        mob->gold = 0;
    else
        mob->gold = number_range( pMobIndex->gold / 2,
                                  pMobIndex->gold * 3 / 2 );
    mob->form = pMobIndex->form;
    mob->parts = pMobIndex->parts;
    mob->size = pMobIndex->size;
    mob->material = pMobIndex->material;

    /* computed on the spot */

    for ( i = 0; i < MAX_STATS; i++ )
        mob->perm_stat[i] = UMIN( 25, 11 + mob->level / 4 );

    if ( IS_SET( mob->act, ACT_WARRIOR ) )
    {
        mob->perm_stat[STAT_STR] += 3;
        mob->perm_stat[STAT_INT] -= 1;
        mob->perm_stat[STAT_CON] += 2;
    }

    if ( IS_SET( mob->act, ACT_THIEF ) )
    {
        mob->perm_stat[STAT_DEX] += 3;
        mob->perm_stat[STAT_INT] += 1;
        mob->perm_stat[STAT_WIS] -= 1;
    }

    if ( IS_SET( mob->act, ACT_CLERIC ) )
    {
        mob->perm_stat[STAT_WIS] += 3;
        mob->perm_stat[STAT_DEX] -= 1;
        mob->perm_stat[STAT_STR] += 1;
    }

    if ( IS_SET( mob->act, ACT_MAGE ) )
    {
        mob->perm_stat[STAT_INT] += 3;
        mob->perm_stat[STAT_STR] -= 1;
        mob->perm_stat[STAT_DEX] += 1;
    }

    if ( IS_SET( mob->off_flags, OFF_FAST ) )
        mob->perm_stat[STAT_DEX] += 2;

    mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
    mob->perm_stat[STAT_CON] += ( mob->size - SIZE_MEDIUM ) / 2;

    mob->position = mob->start_pos;

    /* link the mob to the world list */
    mob->next = char_list;
    char_list = mob;
    pMobIndex->count++;
    mob->reset_count = NULL;

    /* rnd_obj_percent is the percent chance of a random object
       loading on a mob at all. */

    if ( number_range( 0, 99 ) < pMobIndex->rnd_obj_percent )
        load_random_objs( mob, pMobIndex );

    return mob;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mobile( CHAR_DATA * parent, CHAR_DATA * clone )
{
    int i;
    AFFECT_DATA *paf;
    NEWAFFECT_DATA *npaf;

    if ( parent == NULL || clone == NULL || !IS_NPC( parent ) )
        return;

    /* start fixing values */
    clone->name = str_dup( parent->name );
    clone->version = parent->version;
    clone->short_descr = str_dup( parent->short_descr );
    clone->long_descr = str_dup( parent->long_descr );
    clone->description = str_dup( parent->description );
    clone->sex = parent->sex;
    clone->Class = parent->Class;
    clone->race = parent->race;
    clone->level = parent->level;
    clone->trust = 0;
    clone->timer = parent->timer;
    clone->wait = parent->wait;
    clone->hit = parent->hit;
    clone->max_hit = parent->max_hit;
    clone->mana = parent->mana;
    clone->max_mana = parent->max_mana;
    clone->move = parent->move;
    clone->max_move = parent->max_move;
    clone->gold = parent->gold;
    clone->exp = parent->exp;
    clone->act = parent->act;
    clone->comm = parent->comm;
    clone->imm_flags = parent->imm_flags;
    clone->res_flags = parent->res_flags;
    clone->vuln_flags = parent->vuln_flags;
    clone->invis_level = parent->invis_level;
    clone->affected_by = parent->affected_by;
    strncpy( clone->newaff, parent->newaff, sizeof( clone->newaff ) );
    clone->position = parent->position;
    clone->practice = parent->practice;
    clone->train = parent->train;
    clone->saving_throw = parent->saving_throw;
    clone->alignment = parent->alignment;
    clone->hitroll = parent->hitroll;
    clone->breath_percent = parent->breath_percent;
    clone->rnd_obj_percent = parent->rnd_obj_percent;
    clone->rnd_obj_num = parent->rnd_obj_num;
    clone->rnd_obj_types = parent->rnd_obj_types;
    clone->damroll = parent->damroll;
    clone->wimpy = parent->wimpy;
    clone->form = parent->form;
    clone->parts = parent->parts;
    clone->size = parent->size;
    clone->material = parent->material;
    clone->off_flags = parent->off_flags;
    clone->dam_type = parent->dam_type;
    clone->start_pos = parent->start_pos;
    clone->default_pos = parent->default_pos;

    for ( i = 0; i < 4; i++ )
        clone->armor[i] = parent->armor[i];

    for ( i = 0; i < MAX_STATS; i++ )
    {
        clone->perm_stat[i] = parent->perm_stat[i];
        clone->mod_stat[i] = parent->mod_stat[i];
    }

    for ( i = 0; i < 3; i++ )
        clone->damage[i] = parent->damage[i];

    /* now add the affects */
    for ( paf = parent->affected; paf != NULL; paf = paf->next )
        affect_to_char( clone, paf );
    for ( npaf = parent->newaffected; npaf != NULL; npaf = npaf->next )
        newaffect_to_char( clone, npaf );
}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA * pObjIndex, int level )
{
    static OBJ_DATA obj_zero;
    OBJ_DATA *obj;

    if ( pObjIndex == NULL )
    {
        bug( "Create_object: NULL pObjIndex.", 0 );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    if ( obj_free == NULL )
        obj = alloc_perm( sizeof( *obj ) );
    else
    {
        obj = obj_free;
        if ( obj_free->next != obj_free )
            obj_free = obj_free->next;
        else
            bug( "Obj_free->next == Obj_free!", 0 );
    }

    *obj = obj_zero;
    obj->pIndexData = pObjIndex;
    obj->in_room = NULL;
    obj->enchanted = FALSE;

    obj->clan = pObjIndex->clan;

    obj->level = pObjIndex->level;
    obj->wear_loc = -1;

    obj->name = str_dup( pObjIndex->name ); /* OLC */
    obj->short_descr = str_dup( pObjIndex->short_descr );   /* OLC */
    obj->description = str_dup( pObjIndex->description );   /* OLC */
    obj->material = pObjIndex->material;
    obj->item_type = pObjIndex->item_type;
    obj->extra_flags = pObjIndex->extra_flags;
    obj->wear_flags = pObjIndex->wear_flags;
    obj->value[0] = pObjIndex->value[0];
    obj->value[1] = pObjIndex->value[1];
    obj->value[2] = pObjIndex->value[2];
    obj->value[3] = pObjIndex->value[3];
    obj->value[4] = pObjIndex->value[4];
    obj->weight = pObjIndex->weight;

    obj->cost = pObjIndex->cost;

    /*
     * Mess with object properties.
     */
    switch ( obj->item_type )
    {
    default:
        bug( "Read_object: vnum %d bad type.", pObjIndex->vnum );
        break;

    case ITEM_LIGHT:
        if ( obj->value[2] == 999 )
            obj->value[2] = -1;
        break;
    case ITEM_ARMOR:
    case ITEM_BOAT:
    case ITEM_CLOTHING:
    case ITEM_CONTAINER:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_DRINK_CON:
    case ITEM_FOOD:
    case ITEM_FOUNTAIN:
    case ITEM_FURNITURE:
    case ITEM_KEY:
    case ITEM_MAP:
    case ITEM_MONEY:
    case ITEM_PILL:
    case ITEM_POTION:
    case ITEM_PROTECT:
    case ITEM_SCROLL:
    case ITEM_STAFF:
    case ITEM_TRASH:
    case ITEM_TREASURE:
    case ITEM_WAND:
    case ITEM_WEAPON:
    case ITEM_PORTAL:
        break;
    }

    if ( obj->next != object_list )
        obj->next = object_list;
    else if ( object_list != NULL )
        bug( "Obj->next == object_list!", 0 );
    object_list = obj;
    pObjIndex->count++;

    return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object( OBJ_DATA * parent, OBJ_DATA * clone )
{
    int i;
    AFFECT_DATA *paf;
/*    EXTRA_DESCR_DATA *ed,*ed_new; */

    if ( parent == NULL || clone == NULL )
        return;

    /* start fixing the object */
    clone->name = str_dup( parent->name );
    clone->short_descr = str_dup( parent->short_descr );
    clone->description = str_dup( parent->description );
    clone->item_type = parent->item_type;
    clone->extra_flags = parent->extra_flags;
    clone->wear_flags = parent->wear_flags;
    clone->weight = parent->weight;
    clone->cost = parent->cost;
    clone->level = parent->level;
    clone->condition = parent->condition;
    clone->material = parent->material;
    clone->timer = parent->timer;

    for ( i = 0; i < 5; i++ )
        clone->value[i] = parent->value[i];

    /* affects */
    clone->enchanted = parent->enchanted;

    for ( paf = parent->affected; paf != NULL; paf = paf->next )
        affect_to_obj( clone, paf );

    /* extended desc */
/*
    for (ed = parent->extra_descr; ed != NULL; ed = ed->next);
    {
	ed_new              = alloc_perm( sizeof(*ed_new) );

	ed_new->keyword         = str_dup( ed->keyword);
	ed_new->description     = str_dup( ed->description );
	ed_new->next            = clone->extra_descr;
	clone->extra_descr      = ed_new;
    }
*/

}

/*
 * Clear a new character.
 */
void clear_char( CHAR_DATA * ch )
{
    static CHAR_DATA ch_zero;
    int i;

    *ch = ch_zero;
    ch->name = &str_empty[0];
    ch->short_descr = &str_empty[0];
    ch->long_descr = &str_empty[0];
    ch->description = &str_empty[0];
    ch->logon = current_time;
    ch->last_note = 0;
    ch->lines = PAGELEN;
    for ( i = 0; i < 4; i++ )
        ch->armor[i] = 100;
    ch->comm = 0;
    ch->position = POS_STANDING;
    ch->practice = 0;
    ch->hit = 20;
    ch->max_hit = 20;
    ch->mana = 100;
    ch->max_mana = 100;
    ch->move = 100;
    ch->max_move = 100;
    for ( i = 0; i < MAX_STATS; i++ )
    {
        ch->perm_stat[i] = 13;
        ch->mod_stat[i] = 0;
    }
    memset( ch->newaff, 0,
            ( MAX_NEWAFF_BIT / 8 ) + ( MAX_NEWAFF_BIT % 8 ? 1 : 0 ) );
    return;
}

/*
 * Free a character.
 */
void free_char( CHAR_DATA * ch )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf_next;
    NEWAFFECT_DATA *npaf;
    NEWAFFECT_DATA *npaf_next;
    QUEUE_DATA *mq;
    QUEUE_DATA *mq_next;
    sh_int i;
    IMMCMD_TYPE *cmd;

    if ( IS_NPC( ch ) )
        mobile_count--;

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
        obj_next = obj->next_content;
        extract_obj( obj );
    }

    for ( paf = ch->affected; paf != NULL; paf = paf_next )
    {
        paf_next = paf->next;
        affect_remove( ch, paf );
    }

    for ( npaf = ch->newaffected; npaf != NULL; npaf = npaf_next )
    {
        npaf_next = npaf->next;
        newaffect_remove( ch, npaf );
    }

    if ( ch->name )
        free_string( &ch->name );
    if ( ch->short_descr )
        free_string( &ch->short_descr );
    if ( ch->long_descr )
        free_string( &ch->long_descr );
    if ( ch->description )
        free_string( &ch->description );

    if ( ch->hate )
    {
        free_string( &ch->hate->name );
        free_mem( &ch->hate );
        ch->hate = NULL;
    }

    if ( ch->pcdata != NULL )
    {
        for ( mq = ch->pcdata->fmessage; mq != NULL; mq = mq_next )
        {
            mq_next = mq->next;
            free_string( &mq->sender );
            free_string( &mq->text );
            free_mem( &mq );
        }
        for ( mq = ch->pcdata->ftell_q; mq != NULL; mq = mq_next )
        {
            mq_next = mq->next;
            free_string( &mq->sender );
            free_string( &mq->text );
            free_mem( &mq );
        }

        for ( i = 0; i < MAX_ALIAS; i++ )
        {
            if ( ch->pcdata->alias[i] == NULL
                 || ch->pcdata->alias_sub[i] == NULL )
                break;

            free_string( &ch->pcdata->alias[i] );
            free_string( &ch->pcdata->alias_sub[i] );
        }
        free_string( &ch->pcdata->pwd );
        free_string( &ch->pcdata->bamfin );
        free_string( &ch->pcdata->bamfout );
        free_string( &ch->pcdata->title );
        free_string( &ch->pcdata->prompt );
        free_string( &ch->pcdata->spouse );
        free_string( &ch->pcdata->nemesis );
        free_string( &ch->pcdata->email );
        free_string( &ch->pcdata->comment );

        while ( ch->pcdata->immcmdlist != NULL )
        {
            cmd = ch->pcdata->immcmdlist->next;
            free_string( &ch->pcdata->immcmdlist->cmd );
            free( ch->pcdata->immcmdlist );
            ch->pcdata->immcmdlist = cmd;
        }

        free_faction_standings( ch->pcdata->faction_standings );
        ch->pcdata->faction_standings = NULL;

        ch->pcdata->next = pcdata_free;
        pcdata_free = ch->pcdata;
    }

    ch->next = char_free;
    char_free = ch;
    return;
}

/*
 * Get an extra description from a list.
 */
char *get_extra_descr( const char *name, EXTRA_DESCR_DATA * ed )
{
    for ( ; ed != NULL; ed = ed->next )
    {
        if ( is_name( ( char * ) name, ed->keyword ) )
            return ed->description;
    }
    return NULL;
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum )
{
    MOB_INDEX_DATA *pMobIndex;

    for ( pMobIndex = mob_index_hash[vnum % MAX_KEY_HASH];
          pMobIndex != NULL; pMobIndex = pMobIndex->next )
    {
        if ( pMobIndex->vnum == vnum )
            return pMobIndex;
    }

    if ( fBootDb )
    {
        bug( "Get_mob_index: bad vnum %d.", vnum );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    return NULL;
}

/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum )
{
    OBJ_INDEX_DATA *pObjIndex;

    for ( pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH];
          pObjIndex != NULL; pObjIndex = pObjIndex->next )
    {
        if ( pObjIndex->vnum == vnum )
            return pObjIndex;
    }

    if ( fBootDb )
    {
        bug( "Get_obj_index: bad vnum %d.", vnum );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    return NULL;
}

/*
 * Translates mudprog virtual number to its MPROG_DATA struct.
 * Hash table lookup.
 */
MPROG_DATA *get_mprog_by_vnum( int vnum )
{
    MPROG_DATA *pMudProg;

    for ( pMudProg = mudprog_first; pMudProg != NULL;
          pMudProg = pMudProg->next )
    {
        if ( pMudProg->vnum == vnum )
            return pMudProg;
    }

    if ( fBootDb )
    {
        bug( "Get_Mprog_By_Vnum: bad vnum %d.", vnum );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    return NULL;
}

/*
 * Translates mudprog group virtual number to its MPROG_GROUP struct.
 * Hash table lookup.
 */
MPROG_GROUP *get_mprog_group_by_vnum( int vnum )
{
    MPROG_GROUP *pMprogGroup;

    for ( pMprogGroup = mprog_group_first; pMprogGroup != NULL;
          pMprogGroup = pMprogGroup->next )
    {
        if ( pMprogGroup->vnum == vnum )
            return pMprogGroup;
    }

    if ( fBootDb )
    {
        bug( "Get_Mprog_Group_By_Vnum: bad vnum %d.", vnum );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    return NULL;
}

/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( int vnum )
{
    ROOM_INDEX_DATA *pRoomIndex;

    for ( pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH];
          pRoomIndex != NULL; pRoomIndex = pRoomIndex->next )
    {
        if ( pRoomIndex->vnum == vnum )
            return pRoomIndex;
    }

    if ( fBootDb )
    {
        bug( "Get_room_index: bad vnum %d.", vnum );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    return NULL;
}

/*
 * Read a letter from a file.
 */
char fread_letter( FILE * fp )
{
    signed char c;

    do
    {
        c = getc( fp );
    }
    while ( isspace( c ) );

    return c;
}

/*
 * Read a number from a file.
 */
int fread_number( FILE * fp )
{
    int number;
    bool sign;
    signed char c;

    do
    {
        c = getc( fp );
    }
    while ( isspace( c ) );

    number = 0;

    sign = FALSE;
    if ( c == '+' )
    {
        c = getc( fp );
    }
    else if ( c == '-' )
    {
        sign = TRUE;
        c = getc( fp );
    }

    if ( !isdigit( c ) )
    {
        bug( "Fread_number: bad format.", 0 );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    while ( isdigit( c ) )
    {
        number = number * 10 + c - '0';
        c = getc( fp );
    }

    if ( sign )
        number = 0 - number;

    if ( c == '|' )
        number += fread_number( fp );
    else if ( c != ' ' )
        ungetc( c, fp );

    return number;
}

long fread_flag( FILE * fp )
{
    int number;
    signed char c;

    do
    {
        c = getc( fp );
    }
    while ( isspace( c ) );

    number = 0;

    if ( !isdigit( c ) && c != '-' )    /* ROM OLC */
    {
        while ( ( 'A' <= c && c <= 'Z' ) || ( 'a' <= c && c <= 'z' ) )
        {
            number += flag_convert( c );
            c = getc( fp );
        }
    }

    if ( c == '-' )             /* ROM OLC */
    {
        number = fread_number( fp );
        return -number;
    }

    while ( isdigit( c ) )
    {
        number = number * 10 + c - '0';
        c = getc( fp );
    }

    if ( c == '|' )
        number += fread_flag( fp );

    else if ( c != ' ' )
        ungetc( c, fp );

    return number;
}

long flag_convert( char letter )
{
    long bitsum = 0;
    char i;

    if ( 'A' <= letter && letter <= 'Z' )
    {
        bitsum = 1;
        for ( i = letter; i > 'A'; i-- )
            bitsum *= 2;
    }
    else if ( 'a' <= letter && letter <= 'z' )
    {
        bitsum = 67108864;      /* 2^26 */
        for ( i = letter; i > 'a'; i-- )
            bitsum *= 2;
    }

    return bitsum;
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE * fp )
{
    signed char c;

    do
    {
        c = getc( fp );
    }
    while ( c != '\n' && c != '\r' );

    do
    {
        c = getc( fp );
    }
    while ( c == '\n' || c == '\r' );

    ungetc( c, fp );
    return;
}

/*
 * Allocate memory for a string, read one word from a file,
 *  and return a pointer to the word.  Please remember to
 *  free this pointer when you're done with it...
 */
char *get_word( FILE * fp )
{
    char *word, *top;
    char ch;
    unsigned int len = 0;

    word = malloc( MAX_INPUT_LENGTH );
    top = word;

    do                          /* Skip whitespace */
    {
        ch = getc( fp );
        if ( ch == EOF )
            return NULL;
    }
    while ( isspace( ch ) );

    *word = ch;
    word++;
    len++;
    for ( ;; )
    {
        ch = getc( fp );
        if ( ch == EOF )
            return NULL;
        if ( ( len > MAX_INPUT_LENGTH ) || ( isspace( ch ) ) )
        {
            *word = '\0';
            return top;
        }
        *word = ch;
        word++;
        len++;
    }
}

/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE * fp )
{
    static signed char word[MAX_INPUT_LENGTH];
    signed char *pword;
    signed char cEnd;

    do
        cEnd = getc( fp );
    while ( isspace( cEnd ) );

    if ( cEnd == '\'' || cEnd == '"' )
        pword = word;
    else
    {
        word[0] = cEnd;
        pword = word + 1;
        cEnd = ' ';
    }

    for ( ; pword < word + MAX_INPUT_LENGTH; pword++ )
    {
        *pword = getc( fp );
        if ( cEnd == ' ' ? isspace( *pword ) : *pword == cEnd )
        {
            if ( cEnd == ' ' )
                ungetc( *pword, fp );
            *pword = '\0';
            return word;
        }
    }

    bug( "Fread_word: word too long.", 0 );
#if defined(cbuilder)
    return -1;
#else
    exit( 1 );
#endif
    return NULL;
}

/*
 * Allocate some ordinary memory,
 *   with the expectation of freeing it someday.
 */
void *alloc_mem( int sMem )
{
    void *pMem;

    if ( !( pMem = calloc( sMem, sizeof( char ) ) ) )
    {
        perror( "malloc failure" );
        fprintf( stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    return pMem;
}

/*
 * Free some memory.
 *
 * You have to pass the address of the pointer pointing to the memory
 * being freed.  This way we can set that pointer to null.  Safer than
 * allowing the user to set it to null later. - Zane
 *
 * All calls to free_mem should look something like:
 *
 * free_mem(&ptr);
 *
 * Where ptr is a pointer to the area of memory to be freed.
 *
 */
void free_mem( void *pMemPtr )
{
    void **tmp = ( void ** ) pMemPtr;
    void *pMem = *tmp;

    free( pMem );

    *tmp = NULL;
    return;
}

/*
 * Allocate some permanent memory.
 * Permanent memory is never freed,
 *   pointers into it may be copied safely.
 */
void *alloc_perm( int sMem )
{
    static char *pMemPerm;
    static int iMemPerm;
    void *pMem;

    while ( sMem % sizeof( long ) != 0 )
        sMem++;
    if ( sMem > MAX_PERM_BLOCK )
    {
        bug( "Alloc_perm: %d too large.", sMem );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    if ( pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK )
    {
        iMemPerm = 0;
        if ( ( pMemPerm = calloc( 1, MAX_PERM_BLOCK ) ) == NULL )
        {
#if defined(cbuilder)
            log_string( "Error: Alloc_perm" );
            return -1;
#else
            perror( "Alloc_perm" );
            exit( 1 );
#endif
        }
    }

    pMem = pMemPerm + iMemPerm;
    iMemPerm += sMem;
    nAllocPerm += 1;
    sAllocPerm += sMem;
    return pMem;
}

/* strlen() that doesn't count any valid color codes -Zak */
unsigned int str_len( const char *str )
{
    int numb = 0;

    while ( *str != '\0' )
    {
        if ( *str != '`' )      /* If there's no `, add to the count  */
        {
            numb++;
            str++;
            continue;
        }
        str++;                  /* If there _IS_ a `, check next char */
        switch ( *str )
        {
/* If it's \0, count the first ` and get outta here */
        case '\0':
            numb++;
            return numb;
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
        case '0':
            str++;
            break;
/* If it's not a color code, count the ` and the following char, and advance */
        default:
            numb++;
            numb++;
            str++;
            break;
        }
        continue;
    }
    return numb;
}

#if defined(_MSC_VER)
/*
 * MSVC doesn't have a snprintf() function, it has _snprintf().  I'm wrapping that function
 * so we can use snprintf() to our heart's desire.
 */
void snprintf( char *buffer, size_t count, const char *format, ... )
{
    va_list args;

    va_start( args, format );
    _vsnprintf( buffer, count, format, args );
    va_end( args );
}
#endif

/*
 * Search for a string within a string, case insensitive.
 */
char *str_str( char *str1, char *str2 )
{
    char *strcopy1;
    char *strcopy2;
    char *location;
    char *result;

    if ( !str1 || !str2 || str1[0] == '\0' || str2[0] == '\0' )
        return NULL;

#if defined(_MSC_VER)
    strcopy1 = _strdup( str1 );
    strcopy2 = _strdup( str2 );
#else
    strcopy1 = strdup( str1 );
    strcopy2 = strdup( str2 );
#endif

    str_upr( strcopy1 );
    str_upr( strcopy2 );

    if ( ( location = strstr( strcopy1, strcopy2 ) ) == NULL )
    {
        free( strcopy1 );
        free( strcopy2 );
        return NULL;
    }
    else
    {
        result = str1 + ( location - strcopy1 );
        free( strcopy1 );
        free( strcopy2 );
        return result;
    }
}

/*
 * Uppercase a string.  Added because not all OS's have strupr().
 */
char *str_upr( char *str )
{
    char *buf;

    if ( !str || str[0] == '\0' )
        return str;

    buf = str;

    while ( *buf != '\0' )
    {
        if ( ( *buf >= 'a' ) && ( *buf <= 'z' ) )
        {
            *buf = *buf - 32;
            buf++;
        }
        else
            buf++;
    }

    return str;
}

void do_areas( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    AREA_DATA *pArea1;
    AREA_DATA *pArea2;
    int iArea;
    int iAreaHalf;

    if ( argument[0] != '\0' )
    {
        send_to_char( "No argument is used with this command.\n\r", ch );
        return;
    }

    iAreaHalf = ( top_area + 1 ) / 2;
    pArea1 = area_first;
    pArea2 = area_first;
    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
        pArea2 = pArea2->next;

    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
    {
        sprintf( buf, "%-39s%-39s\n\r",
                 pArea1->name, ( pArea2 != NULL ) ? pArea2->name : "" );
        send_to_char( buf, ch );
        pArea1 = pArea1->next;
        if ( pArea2 != NULL )
            pArea2 = pArea2->next;
    }

    return;
}

void do_memory( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    sprintf( buf, "Affects %5d\n\r", top_affect );
    send_to_char( buf, ch );
    sprintf( buf, "Areas   %5d\n\r", top_area );
    send_to_char( buf, ch );
    sprintf( buf, "ExDes   %5d\n\r", top_ed );
    send_to_char( buf, ch );
    sprintf( buf, "Exits   %5d\n\r", top_exit );
    send_to_char( buf, ch );
    sprintf( buf, "Helps   %5d\n\r", top_help );
    send_to_char( buf, ch );
/*    sprintf( buf, "Socials %5d\n\r", social_count  ); send_to_char( buf,
ch );*/
    sprintf( buf, "Mobs    %5d\n\r", top_mob_index );
    send_to_char( buf, ch );
    sprintf( buf, "(in use)%5d\n\r", mobile_count );
    send_to_char( buf, ch );
    sprintf( buf, "Objs    %5d\n\r", top_obj_index );
    send_to_char( buf, ch );
    sprintf( buf, "Resets  %5d\n\r", top_reset );
    send_to_char( buf, ch );
    sprintf( buf, "Rooms   %5d\n\r", top_room );
    send_to_char( buf, ch );
    sprintf( buf, "Shops   %5d\n\r", top_shop );
    send_to_char( buf, ch );
/*    sprintf( buf, "Clans   %5d\n\r", top_clan    ); send_to_char( buf, ch ); */

    sprintf( buf, "Strings %5d strings of %7d bytes (max %d).\n\r",
             nAllocString, sAllocString, MAX_STRING );
    send_to_char( buf, ch );

    sprintf( buf, "Perms   %5d blocks  of %7d bytes.\n\r",
             nAllocPerm, sAllocPerm );
    send_to_char( buf, ch );

    return;
}

void do_dump( CHAR_DATA * ch, char *argument )
{
    int count, count2, num_pcs, aff_count;
    CHAR_DATA *fch;
    MOB_INDEX_DATA *pMobIndex;
    PC_DATA *pc;
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    DESCRIPTOR_DATA *d;
    AFFECT_DATA *af;
    NEWAFFECT_DATA *naf;
    FILE *fp;
    int vnum, nMatch = 0;
    char buf[MAX_STRING_LENGTH];

    /* open file */
    fclose( fpReserve );
    sprintf( buf, "%s/mem.dmp", sysconfig.area_dir );
    fp = fopen( buf, "w" );

    /* report use of data structures */

    num_pcs = 0;
    aff_count = 0;

    /* mobile prototypes */
    fprintf( fp, "MobProt	%4d (%8d bytes)\n",
             top_mob_index, top_mob_index * ( sizeof( *pMobIndex ) ) );

    /* mobs */
    count = 0;
    count2 = 0;
    for ( fch = char_list; fch != NULL; fch = fch->next )
    {
        count++;
        if ( fch->pcdata != NULL )
            num_pcs++;
        for ( af = fch->affected; af != NULL; af = af->next )
            aff_count++;
        for ( naf = fch->newaffected; naf != NULL; naf = naf->next )
            aff_count++;
    }
    for ( fch = char_free; fch != NULL; fch = fch->next )
        count2++;

    fprintf( fp, "Mobs	%4d (%8d bytes), %2d free (%d bytes)\n",
             count, count * ( sizeof( *fch ) ), count2,
             count2 * ( sizeof( *fch ) ) );

    /* pcdata */
    count = 0;
    for ( pc = pcdata_free; pc != NULL; pc = pc->next )
        count++;

    fprintf( fp, "Pcdata	%4d (%8d bytes), %2d free (%d bytes)\n",
             num_pcs, num_pcs * ( sizeof( *pc ) ), count,
             count * ( sizeof( *pc ) ) );

    /* descriptors */
    count = 0;
    count2 = 0;
    for ( d = descriptor_list; d != NULL; d = d->next )
        count++;
    for ( d = descriptor_free; d != NULL; d = d->next )
        count2++;

    fprintf( fp, "Descs	%4d (%8d bytes), %2d free (%d bytes)\n",
             count, count * ( sizeof( *d ) ), count2,
             count2 * ( sizeof( *d ) ) );

    /* object prototypes */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
        if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
        {
            for ( af = pObjIndex->affected; af != NULL; af = af->next )
                aff_count++;
            nMatch++;
        }

    fprintf( fp, "ObjProt	%4d (%8d bytes)\n",
             top_obj_index, top_obj_index * ( sizeof( *pObjIndex ) ) );

    /* objects */
    count = 0;
    count2 = 0;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        count++;
        for ( af = obj->affected; af != NULL; af = af->next )
            aff_count++;
    }
    for ( obj = obj_free; obj != NULL; obj = obj->next )
        count2++;

    fprintf( fp, "Objs	%4d (%8d bytes), %2d free (%d bytes)\n",
             count, count * ( sizeof( *obj ) ), count2,
             count2 * ( sizeof( *obj ) ) );

    /* affects */
    count = 0;
    for ( af = affect_free; af != NULL; af = af->next )
        count++;

    fprintf( fp,
             "Affects	%4d (%8d bytes), %2d free (%d bytes) New Affects not yet included\n",
             aff_count, aff_count * ( sizeof( *af ) ), count,
             count * ( sizeof( *af ) ) );

    /* rooms */
    fprintf( fp, "Rooms	%4d (%8d bytes)\n",
             top_room, top_room * ( sizeof( ROOM_INDEX_DATA ) ) );

    /* exits */
    fprintf( fp, "Exits	%4d (%8d bytes)\n",
             top_exit, top_exit * ( sizeof( EXIT_DATA ) ) );

    fclose( fp );

    /* start printing out mobile data */
    sprintf( buf, "%s/mob.dmp", sysconfig.area_dir );
    fp = fopen( buf, "w" );

    fprintf( fp, "\nMobile Analysis\n" );
    fprintf( fp, "---------------\n" );
    nMatch = 0;
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
        if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
        {
            nMatch++;
            fprintf( fp, "#%-4d %3d active %3d killed     %s\n",
                     pMobIndex->vnum, pMobIndex->count,
                     pMobIndex->killed, pMobIndex->short_descr );
        }
    fclose( fp );
    fflush( fp );

    /* start printing out object data */
    sprintf( buf, "%s/obj.dmp", sysconfig.area_dir );
    fp = fopen( buf, "w" );

    fprintf( fp, "\nObject Analysis\n" );
    fprintf( fp, "---------------\n" );
    nMatch = 0;
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
        if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
        {
            nMatch++;
            fprintf( fp, "#%-4d %3d active %3d reset      %s\n",
                     pObjIndex->vnum, pObjIndex->count,
                     pObjIndex->reset_num, pObjIndex->short_descr );
        }

    /* close file */
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
}

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( int number )
{
    switch ( number_bits( 2 ) )
    {
    case 0:
        number -= 1;
        break;
    case 3:
        number += 1;
        break;
    }

    return UMAX( 1, number );
}

/*
 * Generate a random number.
 */
int number_range( int from, int to )
{
    int power;
    int number;

    if ( from == 0 && to == 0 )
        return 0;

    if ( ( to = to - from + 1 ) <= 1 )
        return from;

    for ( power = 2; power < to; power <<= 1 )
        ;

    while ( ( number = number_mm(  ) & ( power - 1 ) ) >= to )
        ;

    return from + number;
}

/*
 * Generate a percentile roll.
 */
int number_percent( void )
{
    int percent;

    while ( ( percent = number_mm(  ) & ( 128 - 1 ) ) > 99 )
        ;

    return 1 + percent;
}

/*
 * Generate a random door.
 */
int number_door( void )
{
    int door;

    while ( ( door = number_mm(  ) & ( 8 - 1 ) ) > 5 )
        ;

    return door;
}

int number_bits( int width )
{
    return number_mm(  ) & ( ( 1 << width ) - 1 );
}

/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static int rgiState[2 + 55];

void init_mm(  )
{
    int *piState;
    int iState;

    piState = &rgiState[2];

    piState[-2] = 55 - 55;
    piState[-1] = 55 - 24;

    piState[0] = ( ( int ) current_time ) & ( ( 1 << 30 ) - 1 );
    piState[1] = 1;
    for ( iState = 2; iState < 55; iState++ )
    {
        piState[iState] = ( piState[iState - 1] + piState[iState - 2] )
            & ( ( 1 << 30 ) - 1 );
    }
    return;
}

int number_mm( void )
{
    int *piState;
    int iState1;
    int iState2;
    int iRand;

    piState = &rgiState[2];
    iState1 = piState[-2];
    iState2 = piState[-1];
    iRand = ( piState[iState1] + piState[iState2] ) & ( ( 1 << 30 ) - 1 );
    piState[iState1] = iRand;
    if ( ++iState1 == 55 )
        iState1 = 0;
    if ( ++iState2 == 55 )
        iState2 = 0;
    piState[-2] = iState1;
    piState[-1] = iState2;
    return iRand >> 6;
}

/*
 * Roll some dice.
 */
int dice( int number, int size )
{
    int idice;
    int sum;

    switch ( size )
    {
    case 0:
        return 0;
    case 1:
        return number;
    }

    for ( idice = 0, sum = 0; idice < number; idice++ )
        sum += number_range( 1, size );

    return sum;
}

/*
 * Simple linear interpolation.
 */
int interpolate( int level, int value_00, int value_32 )
{
    return value_00 + level * ( value_32 - value_00 ) / 32;
}

/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde( char *str )
{
    for ( ; *str != '\0'; str++ )
        if ( *str == '~' )
            *str = '-';

    return;
}

/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr )
{
    if ( astr == NULL )
    {
        bug( "Str_cmp: null astr.", 0 );
        return TRUE;
    }

    if ( bstr == NULL )
    {
        bug( "Str_cmp: null bstr.", 0 );
        return TRUE;
    }

    for ( ; *astr || *bstr; astr++, bstr++ )
    {
        if ( LOWER( *astr ) != LOWER( *bstr ) )
            return TRUE;
    }

    return FALSE;
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr )
{
    if ( !astr || !bstr || !*astr || !*bstr )
    {
        bug( "Str_prefix: null astr or bstr.", 0 );
        return TRUE;
    }

    while ( LOWER( *astr ) == LOWER( *bstr ) )
        if ( !*astr++ || !*bstr++ )
        {
            astr--;
            break;
        }

    if ( !*astr )
        return FALSE;
    else
        return TRUE;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;
    int ichar;
    char c0;

    if ( ( c0 = LOWER( astr[0] ) ) == '\0' )
        return FALSE;

    sstr1 = strlen( astr );
    sstr2 = strlen( bstr );

    for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ )
    {
        if ( c0 == LOWER( bstr[ichar] ) && !str_prefix( astr, bstr + ichar ) )
            return FALSE;
    }

    return TRUE;
}

/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr )
{
    int sstr1;
    int sstr2;

    sstr1 = strlen( astr );
    sstr2 = strlen( bstr );
    if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
        return FALSE;
    else
        return TRUE;
}

/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str )
{
    static char strcap[MAX_STRING_LENGTH];
    int i;

    for ( i = 0; str[i] != '\0'; i++ )
        strcap[i] = LOWER( str[i] );
    strcap[i] = '\0';
    strcap[0] = UPPER( strcap[0] );
    return strcap;
}

/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA * ch, char *file, char *str )
{
    FILE *fp;

    if ( IS_NPC( ch ) || str[0] == '\0' )
        return;

    fclose( fpReserve );
    if ( ( fp = fopen( file, "a" ) ) == NULL )
    {
        perror( file );
        if ( ch->Class != 4 )
            send_to_char( "Could not open the file!\n\r", ch );
    }
    else
    {
        if ( ch->Class != 4 )
            fprintf( fp, "[%5d] %s: %s\n",
                     ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
        else
            fprintf( fp, "[Console] %s: %s\n", ch->name, str );
        fclose( fp );
    }

    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

/*
 * Reports a bug.
 */
void bug( const char *str, ... )
{
    char buf[MAX_STRING_LENGTH * 4];
    char buf2[MAX_STRING_LENGTH];

    FILE *fp;

    if ( fpArea )
    {
        int iLine;
        int iChar;

        if ( fpArea == stdin )
        {
            iLine = 0;
        }
        else
        {
            iChar = ftell( fpArea );
            fseek( fpArea, 0, 0 );
            for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
            {
                while ( getc( fpArea ) != '\n' )
                    ;
            }
            fseek( fpArea, iChar, 0 );
        }

        sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
        log_string( buf );
/*        info(NULL, INFOACT_CODER, NULL, buf); */

        sprintf( buf2, "%s/shutdown.txt", sysconfig.area_dir );
        if ( ( fp = fopen( buf2, "a" ) ) != NULL )
        {
            fprintf( fp, "[*****] %s\n", buf );
            fclose( fp );
        }
    }

    strcpy( buf, "[*****] BUG: " );
    {
        va_list param;

        va_start( param, str );
        vsprintf( buf, str, param );
        va_end( param );
    }
    log_string( buf );
/*    info(NULL, INFOACT_CODER, NULL, buf); */
/*
 *   fclose( fpReserve );
 *   if ( ( fp = fopen( BUG_FILE, "a" ) ) )
 *   {
 *	fprintf( fp, "%s\n", buf );
 *	fclose( fp );
 *   }
 *   fpReserve = fopen( NULL_FILE, "r" );
 */
    return;
}

/*
 * writes the text to a seperate file...used for tracing bugs
 *
 * -Thanatos
 */
void bug_trace( const char *str )
{
    static char buf[MAX_STRING_LENGTH];
    FILE *fp;

    fclose( fpReserve );

    sprintf( buf, "%s/%s", sysconfig.area_dir, sysconfig.bug_file );
    if ( ( fp = fopen( buf, "a" ) ) )
    {
        fprintf( fp, "%s ==> ", ctime( &current_time ) );
        fprintf( fp, "%s\n", str );
        fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );

    return;

}

/*
 * Writes a string to the log.
 */

#if !defined(cbuilder)
void log_string( const char *str )
{
    char *strtime;

    strtime = ctime( &current_time );
    strtime[strlen( strtime ) - 1] = '\0';
#if defined(WIN32)
    fprintf( stdout, "%s :: %s\n", strtime, str );
#else
    fprintf( stderr, "%s :: %s\n", strtime, str );
#endif
    return;
}
#endif

/*
 * Writes a string to the log accepting arguments
 */
#if !defined(cbuilder)
void logf_string( const char *str, ... )
{
    char *strtime;
    char list[MAX_STRING_LENGTH * 4];
    {
        va_list param;

        va_start( param, str );
        vsprintf( list, str, param );
        va_end( param );
    }
    strtime = ctime( &current_time );
    strtime[strlen( strtime ) - 1] = '\0';
#if defined(WIN32)
    fprintf( stdout, "%s :: %s\n", strtime, list );
#else
    fprintf( stderr, "%s :: %s\n", strtime, list );
#endif
    return;
}
#endif

void update_last( char *line1, char *line2, char *line3 )
{
#ifdef HEAVY_DEBUG
    extern char last_file[MAX_INPUT_LENGTH];
    int ilog;
    FILE *fp1;
    char buf[MAX_STRING_LENGTH];

    if ( last_file[0] == '\0' )
    {
        for ( ilog = 1000; ilog < 2000; ilog++ )
        {
            sprintf( buf, "%s/%d.log", sysconfig.log_dir, ilog );
            fp1 = fopen( buf, "r" );
            if ( fp1 == NULL )
            {
                sprintf( last_file, "../log/%d.last", ilog - 1 );
                fclose( fp1 );
                break;
            }
            fclose( fp1 );
        }

    }

    fp1 = fopen( last_file, "w" );
    fprintf( fp1, "%s\n%s\n%s\n", line1, line2, line3 );
    fclose( fp1 );
#endif
    return;
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain( void )
{
    return;
}

/* the functions */

/* the functions */

/* This routine transfers between alpha and numeric forms of the
 *  mud_prog bitvector types. This allows the use of the words in the
 *  MudProg files.
 */

int mprog_name_to_type( char *name )
{
    if ( !str_cmp( name, "in_file_prog" ) )
        return IN_FILE_PROG;
    if ( !str_cmp( name, "act_prog" ) )
        return ACT_PROG;
    if ( !str_cmp( name, "speech_prog" ) )
        return SPEECH_PROG;
    if ( !str_cmp( name, "rand_prog" ) )
        return RAND_PROG;
    if ( !str_cmp( name, "fight_prog" ) )
        return FIGHT_PROG;
    if ( !str_cmp( name, "fightgroup_prog" ) )
        return FIGHTGROUP_PROG;
    if ( !str_cmp( name, "hitprcnt_prog" ) )
        return HITPRCNT_PROG;
    if ( !str_cmp( name, "death_prog" ) )
        return DEATH_PROG;
    if ( !str_cmp( name, "entry_prog" ) )
        return ENTRY_PROG;
    if ( !str_cmp( name, "greet_prog" ) )
        return GREET_PROG;
    if ( !str_cmp( name, "all_greet_prog" ) )
        return ALL_GREET_PROG;
    if ( !str_cmp( name, "give_prog" ) )
        return GIVE_PROG;
    if ( !str_cmp( name, "bribe_prog" ) )
        return BRIBE_PROG;
    if ( !str_cmp( name, "enter_prog" ) )
        return ENTER_PROG;
    if ( !str_cmp( name, "leave_prog" ) )
        return LEAVE_PROG;
    if ( !str_cmp( name, "sleep_prog" ) )
        return SLEEP_PROG;
    if ( !str_cmp( name, "rest_prog" ) )
        return REST_PROG;
    if ( !str_cmp( name, "rfight_prog" ) )
        return RFIGHT_PROG;
    if ( !str_cmp( name, "rdeath_prog" ) )
        return RDEATH_PROG;
    if ( !str_cmp( name, "wear_prog" ) )
        return WEAR_PROG;
    if ( !str_cmp( name, "remove_prog" ) )
        return REMOVE_PROG;
    if ( !str_cmp( name, "sac_prog" ) )
        return SAC_PROG;
    if ( !str_cmp( name, "examine_prog" ) )
        return EXA_PROG;
    if ( !str_cmp( name, "look_prog" ) )
        return LOOK_PROG;
    if ( !str_cmp( name, "zap_prog" ) )
        return ZAP_PROG;
    if ( !str_cmp( name, "get_prog" ) )
        return GET_PROG;
    if ( !str_cmp( name, "drop_prog" ) )
        return DROP_PROG;
    if ( !str_cmp( name, "use_prog" ) )
        return USE_PROG;
    if ( !str_cmp( name, "hit_prog" ) )
        return HIT_PROG;
    if ( !str_cmp( name, "command_prog" ) )
        return COMMAND_PROG;

    return ( ERROR_PROG );
}

/* Load the progs from MUDPROGS_FILE */
void load_mudprogs( FILE * fp )
{
    MPROG_DATA *pMudProg;
    MPROG_GROUP *pMprogGroup;
    MPROG_LIST *pMprogList;
    char *word;
    char letter;
    int vnum;

    for ( ;; )
    {
        word = fread_word( fp );

        if ( !str_cmp( word, "#$" ) )
            break;
        else if ( !str_cmp( word, "#MOBPROGS" ) )
        {
            for ( ;; )
            {
                letter = fread_letter( fp );
                if ( letter != '#' )
                {
                    bug( "Load_MudProgs: # not found.", 0 );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                vnum = fread_number( fp );
                if ( vnum == 0 )
                    break;

                fBootDb = FALSE;
                if ( get_mprog_by_vnum( vnum ) != NULL )
                {
                    bug( "Load_MudProgs: vnum %d duplicated.", vnum );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }
                fBootDb = TRUE;

                pMudProg = new_mudprog(  );
                pMudProg->vnum = vnum;
                pMudProg->name = fread_string( fp );
                pMudProg->description = fread_string( fp );
                pMudProg->prog_type = MOB_PROG;
                pMudProg->trigger_type = mprog_name_to_type( fread_word( fp ) );

                if ( pMudProg->trigger_type == ERROR_PROG )
                {
                    bug( "Load_MudProgs: MudProg file type error" );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }
                else if ( pMudProg->trigger_type == IN_FILE_PROG )
                {
                    bug( "Load_MudProgs: MudProg file contains a call to file." );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                pMudProg->arglist = fread_string( fp );
                pMudProg->comlist = fread_string( fp );

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
                pMudProg = NULL;
            }
        }
        else if ( !str_cmp( word, "#ROOMPROGS" ) )
        {
            for ( ;; )
            {
                letter = fread_letter( fp );
                if ( letter != '#' )
                {
                    bug( "Load_MudProgs: # not found.", 0 );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                vnum = fread_number( fp );
                if ( vnum == 0 )
                    break;

                fBootDb = FALSE;
                if ( get_mprog_by_vnum( vnum ) != NULL )
                {
                    bug( "Load_MudProgs: vnum %d duplicated.", vnum );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }
                fBootDb = TRUE;

                pMudProg = new_mudprog(  );
                pMudProg->vnum = vnum;
                pMudProg->name = fread_string( fp );
                pMudProg->description = fread_string( fp );
                pMudProg->prog_type = ROOM_PROG;
                pMudProg->trigger_type = mprog_name_to_type( fread_word( fp ) );

                if ( pMudProg->trigger_type == ERROR_PROG )
                {
                    bug( "Load_MudProgs: MudProg file type error" );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }
                else if ( pMudProg->trigger_type == IN_FILE_PROG )
                {
                    bug( "Load_MudProgs: MudProg file contains a call to file." );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                pMudProg->arglist = fread_string( fp );
                pMudProg->comlist = fread_string( fp );

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
                pMudProg = NULL;
            }
        }
        else if ( !str_cmp( word, "#OBJPROGS" ) )
        {
            for ( ;; )
            {
                letter = fread_letter( fp );
                if ( letter != '#' )
                {
                    bug( "Load_MudProgs: # not found.", 0 );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                vnum = fread_number( fp );
                if ( vnum == 0 )
                    break;

                fBootDb = FALSE;
                if ( get_mprog_by_vnum( vnum ) != NULL )
                {
                    bug( "Load_MudProgs: vnum %d duplicated.", vnum );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }
                fBootDb = TRUE;

                pMudProg = new_mudprog(  );
                pMudProg->vnum = vnum;
                pMudProg->name = fread_string( fp );
                pMudProg->description = fread_string( fp );
                pMudProg->prog_type = OBJ_PROG;
                pMudProg->trigger_type = mprog_name_to_type( fread_word( fp ) );

                if ( pMudProg->trigger_type == ERROR_PROG )
                {
                    bug( "Load_MudProgs: MudProg file type error" );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }
                else if ( pMudProg->trigger_type == IN_FILE_PROG )
                {
                    bug( "Load_MudProgs: MudProg file contains a call to file." );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                pMudProg->arglist = fread_string( fp );
                pMudProg->comlist = fread_string( fp );

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
                pMudProg = NULL;
            }
        }
        else if ( !str_cmp( word, "#PROGGROUPS" ) )
        {
            for ( ;; )
            {
                letter = fread_letter( fp );
                if ( letter != '#' )
                {
                    bug( "Load_MudProgs: # not found.", 0 );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                vnum = fread_number( fp );
                if ( vnum == 0 )
                    break;

                fBootDb = FALSE;
                if ( get_mprog_group_by_vnum( vnum ) != NULL )
                {
                    bug( "Load_MudProgs: vnum %d duplicated.", vnum );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }
                fBootDb = TRUE;

                pMprogGroup = new_mudprog_group(  );
                pMprogGroup->vnum = vnum;
                pMprogGroup->name = fread_string( fp );
                pMprogGroup->description = fread_string( fp );
                pMprogGroup->prog_type = 0;

                while ( fread_letter( fp ) != '~' )
                {
                    if ( ( pMudProg =
                           get_mprog_by_vnum( fread_number( fp ) ) ) == NULL )
                    {
                        bug( "Load_MudProgs: Prog group references invalid prog vnum." );
#if defined(cbuilder)
                        return -1;
#else
                        exit( 1 );
#endif
                    }

                    if ( pMprogGroup->prog_type == 0 )
                        pMprogGroup->prog_type = pMudProg->prog_type;
                    else if ( pMprogGroup->prog_type != pMudProg->prog_type )
                    {
                        bug( "Load_MudProgs: Prog group contains mixed prog types." );
#if defined(cbuilder)
                        return -1;
#else
                        exit( 1 );
#endif
                    }

                    pMprogList =
                        ( MPROG_LIST * ) alloc_mem( sizeof( *pMprogList ) );
                    pMprogList->next = pMprogGroup->mudprogs;
                    pMprogGroup->mudprogs = pMprogList;

                    pMprogGroup->mudprogs->mudprog = pMudProg;

                    pMprogList = NULL;
                    pMudProg = NULL;
                }

                if ( mprog_group_last )
                {
                    mprog_group_last->next = pMprogGroup;
                    mprog_group_last = pMprogGroup;
                }
                else
                {
                    mprog_group_first = pMprogGroup;
                    mprog_group_last = pMprogGroup;
                }
                pMprogGroup->next = NULL;
                pMprogGroup = NULL;
            }
        }
        else
        {
            bug( "Load_MudProgs: bad section name.", 0 );
#if defined(cbuilder)
            return -1;
#else
            exit( 1 );
#endif
        }
    }
}

/* Load the config file */
int load_config_file( void )
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    char word[MAX_STRING_LENGTH];
    char *ptr = buf;
    bool bEOF = FALSE;
    bool bNewSection = FALSE;

    if ( ( fp = fopen( CONFIG_FILE, "r" ) ) == NULL )
    {
#if defined(cbuilder)
        logf_string( "Error: fopen %s", CONFIG_FILE );
        return FALSE;
#else
        perror( CONFIG_FILE );
        exit( 1 );
#endif
    }

    while ( !bEOF )
    {
        if ( !bNewSection )
            bEOF = temp_fread_string_eol( fp, buf );
        else
            bNewSection = FALSE;

        /* Skip blank lines and commented lines */
        if ( *buf == ';' || *buf == '\0' )
            continue;

        /* Try to match a section header */
        if ( !str_cmp( buf, "[Paths]" ) )
        {
            /*
             * Read in each line of the section.  If a new section header is 
             * encountered then end this section.
             */
            while ( !bEOF )
            {
                bEOF = temp_fread_string_eol( fp, buf );

                /* Skip blank lines and commented lines */
                if ( *buf == ';' || *buf == '\0' )
                    continue;

                /* Check to see if this is a new section. */
                if ( *buf == '[' )
                {
                    bNewSection = TRUE;
                    break;
                }

                ptr = one_argument2( buf, word );

                /* Try to match a variable name */
                if ( !str_cmp( word, "AreaDir" ) )
                    sysconfig.area_dir =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "PlayerDir" ) )
                    sysconfig.player_dir =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "PlayerTemp" ) )
                    sysconfig.player_temp =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "GodDir" ) )
                    sysconfig.god_dir = strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "ClanDir" ) )
                    sysconfig.clan_dir =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "NoteDir" ) )
                    sysconfig.note_dir =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "LogDir" ) )
                    sysconfig.log_dir = strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "ClassDir" ) )
                    sysconfig.class_dir =
                        strdup( get_config_value( ptr, word ) );
                else
                {
                    /* Unknown line or section header */
#if defined(cbuilder)
                    logf_string
                        ( "Error: load_config_file: Invalid variable name." );
                    return FALSE;
#else
                    bug( "Load_Config_File: Invalid variable name - '%s'.",
                         word );
                    exit( 1 );
#endif
                }
            }
        }
        else if ( !str_cmp( buf, "[Files]" ) )
        {
            /*
             * Read in each line of the section.  If a new section header is 
             * encountered then end this section.
             */
            while ( !bEOF )
            {
                bEOF = temp_fread_string_eol( fp, buf );

                /* Skip blank lines and commented lines */
                if ( *buf == ';' || *buf == '\0' )
                    continue;

                /* Check to see if this is a new section. */
                if ( *buf == '[' )
                {
                    bNewSection = TRUE;
                    break;
                }

                ptr = one_argument2( buf, word );

                /* Try to match a variable name */
                if ( !str_cmp( word, "AreaList" ) )
                    sysconfig.area_list =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "HelpFile" ) )
                    sysconfig.help_file =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "TodoFile" ) )
                    sysconfig.todo_file =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "MudProgs" ) )
                    sysconfig.mudprogs_file =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "BugFile" ) )
                    sysconfig.bug_file =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "IdeaFile" ) )
                    sysconfig.idea_file =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "TypoFile" ) )
                    sysconfig.typo_file =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "ShutdownFile" ) )
                    sysconfig.shutdown_file =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "ChaosFile" ) )
                    sysconfig.chaos_file =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "ClansFile" ) )
                    sysconfig.clans_file =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "FactionsFile" ) )
                    sysconfig.factions_file =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "SocialsFile" ) )
                    sysconfig.socials_file =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "HelpLogFile" ) )
                    sysconfig.help_log_file =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "BanFile" ) )
                    sysconfig.ban_file =
                        strdup( get_config_value( ptr, word ) );
                else if ( !str_cmp( word, "DisableFile" ) )
                    sysconfig.disable_file =
                        strdup( get_config_value( ptr, word ) );
                else
                {
                    /* Unknown line or section header */
#if defined(cbuilder)
                    logf_string
                        ( "Error: load_config_file: Invalid variable name." );
                    return FALSE;
#else
                    bug( "Load_Config_File: Invalid variable name - '%s'.",
                         word );
                    exit( 1 );
#endif
                }
            }
        }
        else
        {
            /* Unknown line or section header */
#if defined(cbuilder)
            logf_string( "Error: load_config_file: Invalid line encountered." );
            return FALSE;
#else
            bug( "Load_Config_File: Invalid line." );
            exit( 1 );
#endif
        }
    }

    if ( fp )
        fclose( fp );

    return TRUE;
}

/*
 * Accept a string and buffer as an arg.  Filter the string for a value after
 * an = sign and return a pointer to the buffer.
 */
char *get_config_value( char *inbuf, char *outbuf )
{
    inbuf = one_argument2( inbuf, outbuf );

    if ( str_cmp( outbuf, "=" ) )
    {
#if defined(cbuilder)
        logf_string( "Error: get_config_value: Variable format erorr." );
        return -1;
#else
        perror( "get_config_value: Variable format error." );
        exit( 1 );
#endif
    }

    inbuf = one_argument2( inbuf, outbuf );

    if ( *outbuf == '\0' )
    {
#if defined(cbuilder)
        logf_string( "Error: get_config_value: Variable format erorr." );
        return -1;
#else
        perror( "get_config_value: Variable format error." );
        exit( 1 );
#endif
    }

    return outbuf;
}

/* 
 * Assign a prog to a mob/obj/room.
 */
void load_progs( FILE * fp )
{
    MPROG_DATA *pMudProg;
    MPROG_LIST *pMprogList;
    MPROG_GROUP *pMprogGroup;
    MPROG_GROUP_LIST *pMprogGroupList;
    MOB_INDEX_DATA *pMobIndex;
    OBJ_INDEX_DATA *pObjIndex;
    ROOM_INDEX_DATA *pRoomIndex;
    int iFound;
    int progvnum;
    int targetvnum;

    if ( !area_last )
    {
        bug( "Load_Progs: no #AREA seen yet.", 0 );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    for ( ;; )
    {
        char letter;

        if ( ( letter = fread_letter( fp ) ) == 'S' )
            break;

        if ( letter == '*' )
        {
            fread_to_eol( fp );
            continue;
        }

        targetvnum = fread_number( fp );
        progvnum = fread_number( fp );

        pMudProg = NULL;
        pMprogGroup = NULL;
        pMprogList = NULL;
        pMprogGroupList = NULL;

        fBootDb = FALSE;
        pMudProg = get_mprog_by_vnum( progvnum );
        fBootDb = TRUE;

        if ( !pMudProg )
            pMprogGroup = get_mprog_group_by_vnum( progvnum );

        switch ( letter )
        {
        default:
            bug( "Load_Progs: bad prog type '%c'.", letter );
#if defined(cbuilder)
            return -1;
#else
            exit( 1 );
#endif
            break;

        case 'M':
            iFound = 0;
            pMobIndex = get_mob_index( targetvnum );

            if ( pMudProg )
            {
                if ( pMudProg->prog_type != MOB_PROG )
                {
                    bug( "Load_Progs: invalid prog type." );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                assign_mobprog( pMobIndex, pMudProg, NULL );
            }
            else
            {
                if ( pMprogGroup->prog_type != MOB_PROG )
                {
                    bug( "Load_Progs: invalid prog type." );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                assign_mobprog( pMobIndex, NULL, pMprogGroup );
            }

            break;
        case 'O':
            iFound = 0;
            pObjIndex = get_obj_index( targetvnum );

            if ( pMudProg )
            {
                if ( pMudProg->prog_type != OBJ_PROG )
                {
                    bug( "Load_Progs: invalid prog type." );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                assign_objprog( pObjIndex, pMudProg, NULL );
            }
            else
            {
                if ( pMprogGroup->prog_type != OBJ_PROG )
                {
                    bug( "Load_Progs: invalid prog type." );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                assign_objprog( pObjIndex, NULL, pMprogGroup );
            }

            break;
        case 'R':
            iFound = 0;
            pRoomIndex = get_room_index( targetvnum );

            if ( pMudProg )
            {
                if ( pMudProg->prog_type != ROOM_PROG )
                {
                    bug( "Load_Progs: invalid prog type." );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                assign_roomprog( pRoomIndex, pMudProg, NULL );
            }
            else
            {
                if ( pMprogGroup->prog_type != ROOM_PROG )
                {
                    bug( "Load_Progs: invalid prog type." );
#if defined(cbuilder)
                    return -1;
#else
                    exit( 1 );
#endif
                }

                assign_roomprog( pRoomIndex, NULL, pMprogGroup );
            }

            break;
        }

        fread_to_eol( fp );
    }

    return;
}

/*
 * Snarf a mob section.
 */
void load_mobiles( FILE * fp )
{
    MOB_INDEX_DATA *pMobIndex;

    if ( !area_last )           /* OLC */
    {
        bug( "Load_mobiles: no #AREA seen yet.", 0 );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }

    for ( ;; )
    {
        sh_int vnum;
        char letter, temp;
        int iHash;

        letter = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_mobiles: # not found.", 0 );
#if defined(cbuilder)
            return -1;
#else
            exit( 1 );
#endif
        }

        vnum = fread_number( fp );
        if ( vnum == 0 )
            break;

        fBootDb = FALSE;
        if ( get_mob_index( vnum ) != NULL )
        {
            bug( "Load_mobiles: vnum %d duplicated.", vnum );
#if defined(cbuilder)
            return -1;
#else
            exit( 1 );
#endif
        }
        fBootDb = TRUE;

        pMobIndex = alloc_perm( sizeof( *pMobIndex ) );
        pMobIndex->vnum = vnum;
        pMobIndex->area = area_last;    /* OLC */
        pMobIndex->player_name = fread_string( fp );
        pMobIndex->short_descr = fread_string( fp );
        pMobIndex->long_descr = fread_string( fp );
        pMobIndex->description = fread_string( fp );
        pMobIndex->race = race_lookup( fread_string( fp ) );

        pMobIndex->long_descr[0] = UPPER( pMobIndex->long_descr[0] );
        pMobIndex->description[0] = UPPER( pMobIndex->description[0] );

        pMobIndex->act = fread_flag( fp ) | ACT_IS_NPC
            | race_table[pMobIndex->race].act;
        pMobIndex->affected_by = fread_flag( fp )
            | race_table[pMobIndex->race].aff;
        pMobIndex->pShop = NULL;
        pMobIndex->alignment = fread_number( fp );
        letter = fread_letter( fp );

        pMobIndex->level = fread_number( fp );
        pMobIndex->hitroll = fread_number( fp );
        pMobIndex->breath_percent = fread_number( fp );
        pMobIndex->rnd_obj_percent = fread_number( fp );
        pMobIndex->rnd_obj_num = fread_number( fp );
        pMobIndex->rnd_obj_types = fread_flag( fp );

        /* read hit dice */
        pMobIndex->hit[DICE_NUMBER] = fread_number( fp );
        /* 'd'          */ fread_letter( fp );
        pMobIndex->hit[DICE_TYPE] = fread_number( fp );
        /* '+'          */ fread_letter( fp );
        pMobIndex->hit[DICE_BONUS] = fread_number( fp );

        /* read mana dice */
        pMobIndex->mana[DICE_NUMBER] = fread_number( fp );
        fread_letter( fp );
        pMobIndex->mana[DICE_TYPE] = fread_number( fp );
        fread_letter( fp );
        pMobIndex->mana[DICE_BONUS] = fread_number( fp );

        /* read damage dice */
        pMobIndex->damage[DICE_NUMBER] = fread_number( fp );
        fread_letter( fp );
        pMobIndex->damage[DICE_TYPE] = fread_number( fp );
        fread_letter( fp );
        pMobIndex->damage[DICE_BONUS] = fread_number( fp );
        pMobIndex->dam_type = fread_number( fp );

        /* read armor class */
        pMobIndex->ac[AC_PIERCE] = fread_number( fp ) * 10;
        pMobIndex->ac[AC_BASH] = fread_number( fp ) * 10;
        pMobIndex->ac[AC_SLASH] = fread_number( fp ) * 10;
        pMobIndex->ac[AC_EXOTIC] = fread_number( fp ) * 10;

        /* read flags and add in data from the race table */
        pMobIndex->off_flags = fread_flag( fp )
            | race_table[pMobIndex->race].off;
        pMobIndex->imm_flags = fread_flag( fp )
            | race_table[pMobIndex->race].imm;
        pMobIndex->res_flags = fread_flag( fp )
            | race_table[pMobIndex->race].res;
        pMobIndex->vuln_flags = fread_flag( fp )
            | race_table[pMobIndex->race].vuln;
        /* vital statistics */
        pMobIndex->start_pos = fread_number( fp );
        pMobIndex->default_pos = fread_number( fp );
        pMobIndex->sex = fread_number( fp );
        pMobIndex->gold = fread_number( fp );

        pMobIndex->form = fread_flag( fp ) | race_table[pMobIndex->race].form;
        pMobIndex->parts = fread_flag( fp ) | race_table[pMobIndex->race].parts;
        /* size */
        temp = fread_letter( fp );
        switch ( temp )
        {
        case ( 'T' ):
            pMobIndex->size = SIZE_TINY;
            break;
        case ( 'S' ):
            pMobIndex->size = SIZE_SMALL;
            break;
        case ( 'M' ):
            pMobIndex->size = SIZE_MEDIUM;
            break;
        case ( 'L' ):
            pMobIndex->size = SIZE_LARGE;
            break;
        case ( 'H' ):
            pMobIndex->size = SIZE_HUGE;
            break;
        case ( 'G' ):
            pMobIndex->size = SIZE_GIANT;
            break;
        default:
            pMobIndex->size = SIZE_MEDIUM;
            break;
        }

        pMobIndex->material = material_lookup( fread_word( fp ) );

/*       pMobIndex->clan=letter-64; */
/*       if (letter == 'S') pMobIndex->clan=0; */
        letter = fread_letter( fp );

        /* R indicates beginning of random object info. */
        if ( letter == 'R' )
        {
            pMobIndex->rnd_obj_percent = fread_number( fp );
            pMobIndex->rnd_obj_num = fread_number( fp );
            pMobIndex->rnd_obj_types = fread_flag( fp );
        }
        else
        {
            ungetc( letter, fp );
        }

        iHash = vnum % MAX_KEY_HASH;
        pMobIndex->next = mob_index_hash[iHash];
        mob_index_hash[iHash] = pMobIndex;
        top_mob_index++;
        top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;   /* OLC */
        assign_area_vnum( vnum );   /* OLC */
        kill_table[URANGE( 0, pMobIndex->level, MAX_LEVEL - 1 )].number++;
    }

    return;
}

/*
 * Snarf an obj section.
 */
void load_objects( FILE * fp )
{
    OBJ_INDEX_DATA *pObjIndex;

    if ( !area_last )           /* OLC */
    {
        bug( "Load_objects: no #AREA seen yet.", 0 );
#if defined(cbuilder)
        return -1;
#else
        exit( 1 );
#endif
    }
    for ( ;; )
    {
        sh_int vnum;
        char letter;
        int iHash;

        letter = fread_letter( fp );
        if ( letter != '#' )
        {
            bug( "Load_objects: # not found.", 0 );
#if defined(cbuilder)
            return -1;
#else
            exit( 1 );
#endif
        }

        vnum = fread_number( fp );
        if ( vnum == 0 )
            break;

        fBootDb = FALSE;
        if ( get_obj_index( vnum ) != NULL )
        {
            bug( "Load_objects: vnum %d duplicated.", vnum );
#if defined(cbuilder)
            return -1;
#else
            exit( 1 );
#endif
        }
        fBootDb = TRUE;

        pObjIndex = alloc_perm( sizeof( *pObjIndex ) );
        pObjIndex->vnum = vnum;
        pObjIndex->area = area_last;    /* OLC */
        pObjIndex->reset_num = 0;
        pObjIndex->name = fread_string( fp );
        pObjIndex->short_descr = fread_string( fp );
        pObjIndex->description = fread_string( fp );
        pObjIndex->material = material_lookup( fread_string( fp ) );

        pObjIndex->item_type = fread_number( fp );
        pObjIndex->extra_flags = fread_flag( fp );
        pObjIndex->wear_flags = ( short int ) fread_flag( fp );
        pObjIndex->value[0] = fread_flag( fp );
        pObjIndex->value[1] = fread_flag( fp );
        pObjIndex->value[2] = fread_flag( fp );
        pObjIndex->value[3] = fread_flag( fp );
        pObjIndex->value[4] = fread_flag( fp );
        pObjIndex->level = fread_number( fp );
        pObjIndex->weight = fread_number( fp );
        pObjIndex->cost = fread_number( fp );

        /* condition */
        letter = fread_letter( fp );
        switch ( letter )
        {
        case ( 'P' ):
            pObjIndex->condition = 100;
            break;
        case ( 'G' ):
            pObjIndex->condition = 90;
            break;
        case ( 'A' ):
            pObjIndex->condition = 75;
            break;
        case ( 'W' ):
            pObjIndex->condition = 50;
            break;
        case ( 'D' ):
            pObjIndex->condition = 25;
            break;
        case ( 'B' ):
            pObjIndex->condition = 10;
            break;
        case ( 'R' ):
            pObjIndex->condition = 0;
            break;
        default:
            pObjIndex->condition = 100;
            break;
        }

        for ( ;; )
        {
            char letter;

            letter = fread_letter( fp );

            if ( letter == 'A' )
            {
                AFFECT_DATA *paf;

                paf = alloc_perm( sizeof( *paf ) );
                paf->type = -1;
                paf->level = pObjIndex->level;
                paf->duration = -1;
                paf->location = fread_number( fp );
                paf->modifier = fread_number( fp );
                paf->bitvector = 0;
                paf->next = pObjIndex->affected;
                pObjIndex->affected = paf;
                top_affect++;
            }

            else if ( letter == 'E' )
            {
                EXTRA_DESCR_DATA *ed;

                ed = alloc_perm( sizeof( *ed ) );
                ed->keyword = fread_string( fp );
                ed->description = fread_string( fp );
                ed->next = pObjIndex->extra_descr;
                pObjIndex->extra_descr = ed;
                top_ed++;
            }

            else if ( letter == 'C' )
            {
                pObjIndex->clan = fread_number( fp );
            }

            else
            {
                ungetc( letter, fp );
                break;
            }
        }

        /*
         * Translate spell "slot numbers" to internal "skill numbers."
         */
        switch ( pObjIndex->item_type )
        {
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
            pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
            pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
            pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
            break;

        case ITEM_STAFF:
        case ITEM_WAND:
            pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
            break;
        }

        iHash = vnum % MAX_KEY_HASH;
        pObjIndex->next = obj_index_hash[iHash];
        obj_index_hash[iHash] = pObjIndex;
        top_obj_index++;
        top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;   /* OLC */
        assign_area_vnum( vnum );   /* OLC */
    }

    return;
}

bool assign_mobprog( MOB_INDEX_DATA * pMob, MPROG_DATA * pMudProg,
                     MPROG_GROUP * pMprogGroup )
{
    MPROG_LIST *pTempList;
    MPROG_LIST *pInnerList;
    MPROG_GROUP_LIST *pGroupList;
    int iFound = 0;

    if ( pMudProg )
    {
        for ( pTempList = pMob->mudprogs; pTempList;
              pTempList = pTempList->next )
            if ( pTempList->mudprog->vnum == pMudProg->vnum )
                iFound = 1;

        if ( !iFound )
        {
            pTempList = malloc( sizeof( *pTempList ) );
            pTempList->next = pMob->mudprogs;
            pMob->mudprogs = pTempList;
            pTempList->mudprog = pMudProg;
            SET_BIT( pMob->progtypes, pMudProg->trigger_type );
        }
    }
    else if ( pMprogGroup )
    {
        for ( pGroupList = pMob->mprog_groups; pGroupList;
              pGroupList = pGroupList->next )
            if ( pGroupList->mprog_group->vnum == pMprogGroup->vnum )
                iFound = 1;

        if ( !iFound )
        {
            pGroupList = malloc( sizeof( *pGroupList ) );
            pGroupList->next = pMob->mprog_groups;
            pMob->mprog_groups = pGroupList;
            pGroupList->mprog_group = pMprogGroup;

            for ( pTempList = pMprogGroup->mudprogs; pTempList;
                  pTempList = pTempList->next )
            {
                iFound = 0;

                for ( pInnerList = pMob->mudprogs; pInnerList;
                      pInnerList = pInnerList->next )
                    if ( pInnerList->mudprog->vnum == pTempList->mudprog->vnum )
                        iFound = 1;

                if ( !iFound )
                {
                    pInnerList = malloc( sizeof( *pInnerList ) );
                    pInnerList->next = pMob->mudprogs;
                    pMob->mudprogs = pInnerList;
                    pInnerList->mudprog = pTempList->mudprog;
                    SET_BIT( pMob->progtypes,
                             pInnerList->mudprog->trigger_type );
                }
            }
        }
    }
    else
        return FALSE;

    return TRUE;
}

bool assign_objprog( OBJ_INDEX_DATA * pObj, MPROG_DATA * pMudProg,
                     MPROG_GROUP * pMprogGroup )
{
    MPROG_LIST *pTempList;
    MPROG_LIST *pInnerList;
    MPROG_GROUP_LIST *pGroupList;
    int iFound = 0;

    if ( pMudProg )
    {
        for ( pTempList = pObj->mudprogs; pTempList;
              pTempList = pTempList->next )
            if ( pTempList->mudprog->vnum == pMudProg->vnum )
                iFound = 1;

        if ( !iFound )
        {
            pTempList = malloc( sizeof( *pTempList ) );
            pTempList->next = pObj->mudprogs;
            pObj->mudprogs = pTempList;
            pTempList->mudprog = pMudProg;
            SET_BIT( pObj->progtypes, pMudProg->trigger_type );
        }
    }
    else if ( pMprogGroup )
    {
        for ( pGroupList = pObj->mprog_groups; pGroupList;
              pGroupList = pGroupList->next )
            if ( pGroupList->mprog_group->vnum == pMprogGroup->vnum )
                iFound = 1;

        if ( !iFound )
        {
            pGroupList = malloc( sizeof( *pGroupList ) );
            pGroupList->next = pObj->mprog_groups;
            pObj->mprog_groups = pGroupList;
            pGroupList->mprog_group = pMprogGroup;

            for ( pTempList = pMprogGroup->mudprogs; pTempList;
                  pTempList = pTempList->next )
            {
                iFound = 0;

                for ( pInnerList = pObj->mudprogs; pInnerList;
                      pInnerList = pInnerList->next )
                    if ( pInnerList->mudprog->vnum == pTempList->mudprog->vnum )
                        iFound = 1;

                if ( !iFound )
                {
                    pInnerList = malloc( sizeof( *pInnerList ) );
                    pInnerList->next = pObj->mudprogs;
                    pObj->mudprogs = pInnerList;
                    pInnerList->mudprog = pTempList->mudprog;
                    SET_BIT( pObj->progtypes,
                             pInnerList->mudprog->trigger_type );
                }
            }
        }
    }
    else
        return FALSE;

    return TRUE;
}

bool assign_roomprog( ROOM_INDEX_DATA * pRoom, MPROG_DATA * pMudProg,
                      MPROG_GROUP * pMprogGroup )
{
    MPROG_LIST *pTempList;
    MPROG_LIST *pInnerList;
    MPROG_GROUP_LIST *pGroupList;
    int iFound = 0;

    if ( pMudProg )
    {
        for ( pTempList = pRoom->mudprogs; pTempList;
              pTempList = pTempList->next )
            if ( pTempList->mudprog->vnum == pMudProg->vnum )
                iFound = 1;

        if ( !iFound )
        {
            pTempList = malloc( sizeof( *pTempList ) );
            pTempList->next = pRoom->mudprogs;
            pRoom->mudprogs = pTempList;
            pTempList->mudprog = pMudProg;
            SET_BIT( pRoom->progtypes, pMudProg->trigger_type );
        }
    }
    else if ( pMprogGroup )
    {
        for ( pGroupList = pRoom->mprog_groups; pGroupList;
              pGroupList = pGroupList->next )
            if ( pGroupList->mprog_group->vnum == pMprogGroup->vnum )
                iFound = 1;

        if ( !iFound )
        {
            pGroupList = malloc( sizeof( *pGroupList ) );
            pGroupList->next = pRoom->mprog_groups;
            pRoom->mprog_groups = pGroupList;
            pGroupList->mprog_group = pMprogGroup;

            for ( pTempList = pMprogGroup->mudprogs; pTempList;
                  pTempList = pTempList->next )
            {
                iFound = 0;

                for ( pInnerList = pRoom->mudprogs; pInnerList;
                      pInnerList = pInnerList->next )
                    if ( pInnerList->mudprog->vnum == pTempList->mudprog->vnum )
                        iFound = 1;

                if ( !iFound )
                {
                    pInnerList = malloc( sizeof( *pInnerList ) );
                    pInnerList->next = pRoom->mudprogs;
                    pRoom->mudprogs = pInnerList;
                    pInnerList->mudprog = pTempList->mudprog;
                    SET_BIT( pRoom->progtypes,
                             pInnerList->mudprog->trigger_type );
                }
            }
        }
    }
    else
        return FALSE;

    return TRUE;
}

bool unassign_mobprog( MOB_INDEX_DATA * pMob, MPROG_DATA * pMudProg,
                       MPROG_GROUP * pMprogGroup )
{
    MPROG_LIST *pTempList;
    MPROG_LIST *pInnerList;
    MPROG_GROUP_LIST *pGroupList;
    MPROG_GROUP_LIST *pGInnerList;
    int iFound = 0;

    if ( pMudProg )
    {
        for ( pTempList = pMob->mudprogs;
              pTempList && pTempList->mudprog->vnum != pMudProg->vnum;
              pTempList = pTempList->next )
            ;

        if ( pTempList )
        {
            iFound = 0;

            for ( pGroupList = pMob->mprog_groups; pGroupList;
                  pGroupList = pGroupList->next )
                for ( pInnerList = pGroupList->mprog_group->mudprogs;
                      pInnerList; pInnerList = pInnerList->next )
                    if ( pInnerList->mudprog->vnum == pMudProg->vnum )
                        iFound = 1;

            if ( iFound )
                return FALSE;
            else
            {
                if ( pTempList == pMob->mudprogs )
                    pMob->mudprogs = pTempList->next;
                else
                {
                    for ( pInnerList = pMob->mudprogs;
                          pInnerList->next != pTempList;
                          pInnerList = pInnerList->next )
                        ;

                    pInnerList->next = pTempList->next;
                }
                free_mem( &pTempList );
            }
        }
    }
    else if ( pMprogGroup )
    {
        for ( pGroupList = pMob->mprog_groups; pGroupList
              && pGroupList->mprog_group->vnum != pMprogGroup->vnum;
              pGroupList = pGroupList->next )
            ;

        if ( pGroupList )
        {
            if ( pGroupList == pMob->mprog_groups )
                pMob->mprog_groups = pGroupList->next;
            else
            {
                for ( pGInnerList = pMob->mprog_groups;
                      pGInnerList->next != pGroupList;
                      pGInnerList = pGInnerList->next )
                    ;

                pGInnerList->next = pGroupList->next;
            }

            for ( pTempList = pGroupList->mprog_group->mudprogs; pTempList;
                  pTempList = pTempList->next )
                unassign_mobprog( pMob, pTempList->mudprog, NULL );

            free_mem( &pGroupList );
        }
        else
            return FALSE;
    }
    else
        return FALSE;

    return TRUE;
}

bool unassign_objprog( OBJ_INDEX_DATA * pObj, MPROG_DATA * pMudProg,
                       MPROG_GROUP * pMprogGroup )
{
    MPROG_LIST *pTempList;
    MPROG_LIST *pInnerList;
    MPROG_GROUP_LIST *pGroupList;
    MPROG_GROUP_LIST *pGInnerList;
    int iFound = 0;

    if ( pMudProg )
    {
        for ( pTempList = pObj->mudprogs;
              pTempList && pTempList->mudprog->vnum != pMudProg->vnum;
              pTempList = pTempList->next )
            ;

        if ( pTempList )
        {
            iFound = 0;

            for ( pGroupList = pObj->mprog_groups; pGroupList;
                  pGroupList = pGroupList->next )
                for ( pInnerList = pGroupList->mprog_group->mudprogs;
                      pInnerList; pInnerList = pInnerList->next )
                    if ( pInnerList->mudprog->vnum == pMudProg->vnum )
                        iFound = 1;

            if ( iFound )
                return FALSE;
            else
            {
                if ( pTempList == pObj->mudprogs )
                    pObj->mudprogs = pTempList->next;
                else
                {
                    for ( pInnerList = pObj->mudprogs;
                          pInnerList->next != pTempList;
                          pInnerList = pInnerList->next )
                        ;

                    pInnerList->next = pTempList->next;
                }
                free_mem( &pTempList );
            }
        }
    }
    else if ( pMprogGroup )
    {
        for ( pGroupList = pObj->mprog_groups; pGroupList
              && pGroupList->mprog_group->vnum != pMprogGroup->vnum;
              pGroupList = pGroupList->next )
            ;

        if ( pGroupList )
        {
            if ( pGroupList == pObj->mprog_groups )
                pObj->mprog_groups = pGroupList->next;
            else
            {
                for ( pGInnerList = pObj->mprog_groups;
                      pGInnerList->next != pGroupList;
                      pGInnerList = pGInnerList->next )
                    ;

                pGInnerList->next = pGroupList->next;
            }

            for ( pTempList = pGroupList->mprog_group->mudprogs; pTempList;
                  pTempList = pTempList->next )
                unassign_objprog( pObj, pTempList->mudprog, NULL );

            free_mem( &pGroupList );
        }
        else
            return FALSE;
    }
    else
        return FALSE;

    return TRUE;
}

bool unassign_roomprog( ROOM_INDEX_DATA * pRoom, MPROG_DATA * pMudProg,
                        MPROG_GROUP * pMprogGroup )
{
    MPROG_LIST *pTempList;
    MPROG_LIST *pInnerList;
    MPROG_GROUP_LIST *pGroupList;
    MPROG_GROUP_LIST *pGInnerList;
    int iFound = 0;

    if ( pMudProg )
    {
        for ( pTempList = pRoom->mudprogs;
              pTempList && pTempList->mudprog->vnum != pMudProg->vnum;
              pTempList = pTempList->next )
            ;

        if ( pTempList )
        {
            iFound = 0;

            for ( pGroupList = pRoom->mprog_groups; pGroupList;
                  pGroupList = pGroupList->next )
                for ( pInnerList = pGroupList->mprog_group->mudprogs;
                      pInnerList; pInnerList = pInnerList->next )
                    if ( pInnerList->mudprog->vnum == pMudProg->vnum )
                        iFound = 1;

            if ( iFound )
                return FALSE;
            else
            {
                if ( pTempList == pRoom->mudprogs )
                    pRoom->mudprogs = pTempList->next;
                else
                {
                    for ( pInnerList = pRoom->mudprogs;
                          pInnerList->next != pTempList;
                          pInnerList = pInnerList->next )
                        ;

                    pInnerList->next = pTempList->next;
                }
                free_mem( &pTempList );
            }
        }
    }
    else if ( pMprogGroup )
    {
        for ( pGroupList = pRoom->mprog_groups; pGroupList
              && pGroupList->mprog_group->vnum != pMprogGroup->vnum;
              pGroupList = pGroupList->next )
            ;

        if ( pGroupList )
        {
            if ( pGroupList == pRoom->mprog_groups )
                pRoom->mprog_groups = pGroupList->next;
            else
            {
                for ( pGInnerList = pRoom->mprog_groups;
                      pGInnerList->next != pGroupList;
                      pGInnerList = pGInnerList->next )
                    ;

                pGInnerList->next = pGroupList->next;
            }

            for ( pTempList = pGroupList->mprog_group->mudprogs; pTempList;
                  pTempList = pTempList->next )
                unassign_roomprog( pRoom, pTempList->mudprog, NULL );

            free_mem( &pGroupList );
        }
        else
            return FALSE;
    }
    else
        return FALSE;

    return TRUE;
}
