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

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#ifndef WIN32

#include <unistd.h>             /* unlink() */

#endif

bool check_disabled( const struct cmd_type * command );

DISABLED_DATA *disabled_first;

#define END_MARKER    "END"     /* for load_disabled() and save_disabled() */

bool check_social args( ( CHAR_DATA * ch, char *command, char *argument ) );

/*
 * Command logging types.
 */
#define LOG_NORMAL      0
#define LOG_ALWAYS      1
#define LOG_NEVER       2

/*
 * Log-all switch.
 */
bool fLogAll = FALSE;

/*
 * Command table.
 */
const struct cmd_type cmd_table[] = {
    /*
     * Common movement commands.
     */
    {"north", do_north, POS_STANDING, 0, LOG_NEVER, 0},
    {"east", do_east, POS_STANDING, 0, LOG_NEVER, 0},
    {"south", do_south, POS_STANDING, 0, LOG_NEVER, 0},
    {"west", do_west, POS_STANDING, 0, LOG_NEVER, 0},
    {"up", do_up, POS_STANDING, 0, LOG_NEVER, 0},
    {"down", do_down, POS_STANDING, 0, LOG_NEVER, 0},
/* Enter a portal */
    {"enter", do_enter, POS_STANDING, 0, LOG_NORMAL, 1},
    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    {"disable", do_disable, POS_DEAD, 1, LOG_NORMAL, 1},
    {"pload", do_pload, POS_DEAD, 1, LOG_NORMAL, 1},
    {"punload", do_punload, POS_DEAD, 1, LOG_NORMAL, 1},
    {"at", do_at, POS_DEAD, 1, LOG_NORMAL, 1},
    {"auction", do_auction, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"buy", do_buy, POS_RESTING, 0, LOG_NORMAL, 1},
    {"cast", do_cast, POS_FIGHTING, 0, LOG_NORMAL, 1},
    {"channels", do_channels, POS_DEAD, 0, LOG_NORMAL, 1},
    {"exits", do_exits, POS_RESTING, 0, LOG_NORMAL, 1},
    {"get", do_get, POS_RESTING, 0, LOG_NORMAL, 1},
    {"goto", do_goto, POS_DEAD, 1, LOG_NORMAL, 1},
    {"heal", do_heal, POS_RESTING, 0, LOG_NORMAL, 1},
    {"hit", do_kill, POS_FIGHTING, 0, LOG_NORMAL, 0},
    {"inventory", do_inventory, POS_DEAD, 0, LOG_NORMAL, 1},
    {"kill", do_kill, POS_FIGHTING, 0, LOG_NORMAL, 1},
    {"look", do_look, POS_RESTING, 0, LOG_NORMAL, 1},
    {"last", do_last, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"lastimm", do_lastimm, POS_SLEEPING, 1, LOG_NORMAL, 1},
#ifdef USE_ADMINTALK
    {"lastadmin", do_lastadmin, POS_SLEEPING, 1, LOG_NORMAL, 1},
#endif
#ifdef USE_HEROTALK
    {"lasthero", do_lasthero, POS_SLEEPING, 1, LOG_NORMAL, 1},
#endif
    {"messages", do_messages, POS_RESTING, 0, LOG_NORMAL, 1},
    {"order", do_order, POS_RESTING, 0, LOG_ALWAYS, 1},
    {CFG_OOC_NAME, do_ooc, MIN_POS_OOC, 0, LOG_NORMAL, 1},
    {"practice", do_practice, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"rest", do_rest, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"sit", do_sit, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"sockets", do_sockets, POS_DEAD, 1, LOG_NORMAL, 1},
    {"stand", do_stand, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"tell", do_tell, MIN_POS_TELL, 0, LOG_NORMAL, 1},
    {"telloff", do_telloff, POS_RESTING, 1, LOG_NORMAL, 1},
    {"tick", do_tick, POS_DEAD, 0, LOG_NORMAL, 1},
    {"track", do_track, POS_RESTING, 0, LOG_NORMAL, 1},
    {"tq", do_tq, MIN_POS_TELL, 0, LOG_NORMAL, 1},
    {"beep", do_beep, POS_RESTING, 0, LOG_NORMAL, 1},
    {"wield", do_wear, POS_RESTING, 0, LOG_NORMAL, 1},
    {"wizhelp", do_wizhelp, POS_DEAD, 0, LOG_NORMAL, 1},
    {"wizgrant", do_wizgrant, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"wizrevoke", do_wizrevoke, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"withdraw", do_withdraw, POS_STANDING, 0, LOG_NORMAL, 1},
    {"deposit", do_deposit, POS_STANDING, 0, LOG_NORMAL, 1},
    {"account", do_account, POS_STANDING, 0, LOG_NORMAL, 1},
    {"askill", do_skill, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"factions", show_faction_standings, POS_DEAD, 0, LOG_NORMAL, 1},

    /*
     * Informational commands.
     */
    {"affects", do_effects, POS_DEAD, 0, LOG_NORMAL, 1},
    {"areas", do_areas, POS_DEAD, 0, LOG_NORMAL, 1},
    {"bug", do_bug, POS_DEAD, 0, LOG_NORMAL, 1},
    {"changes", do_changes, POS_DEAD, 0, LOG_NORMAL, 1},
    {"commands", do_commands, POS_DEAD, 0, LOG_NORMAL, 1},
    {"compare", do_compare, POS_RESTING, 0, LOG_NORMAL, 1},
    {"consider", do_consider, POS_RESTING, 0, LOG_NORMAL, 1},
    {"credits", do_credits, POS_DEAD, 0, LOG_NORMAL, 1},
    {"cwho", do_cwho, POS_DEAD, 0, LOG_NORMAL, 1},
    {"equipment", do_equipment, POS_DEAD, 0, LOG_NORMAL, 1},
    {"examine", do_examine, POS_RESTING, 0, LOG_NORMAL, 1},
    {"effects", do_effects, POS_DEAD, 0, LOG_NORMAL, 1},
    {"help", do_help, POS_DEAD, 0, LOG_NORMAL, 1},
    {"todo", do_todo, POS_DEAD, 0, LOG_NORMAL, 1},
    {"idea", do_idea, POS_DEAD, 0, LOG_NORMAL, 1},
    {"info", do_info, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"levels", do_levels, POS_DEAD, 0, LOG_NORMAL, 1},
    {"motd", do_motd, POS_DEAD, 0, LOG_NORMAL, 1},
    {"news", do_news, POS_DEAD, 0, LOG_NORMAL, 1},
    {"read", do_read, POS_RESTING, 0, LOG_NORMAL, 1},
    {"report", do_report, POS_RESTING, 0, LOG_NORMAL, 1},
    {"rules", do_rules, POS_DEAD, 0, LOG_NORMAL, 1},
    {"score", do_score, POS_DEAD, 0, LOG_NORMAL, 1},
    {"scan", do_scan, POS_RESTING, 0, LOG_NORMAL, 1},
    {"search", do_search, POS_STANDING, 0, LOG_NORMAL, 1},
/*    { "show",         do_show,        POS_DEAD,        0,  LOG_NORMAL, 1 }, */
    {"skills", do_skills, POS_DEAD, 0, LOG_NORMAL, 1},
    {"socials", do_socialfind, POS_DEAD, 0, LOG_NORMAL, 1},
    {"spells", do_spells, POS_DEAD, 0, LOG_NORMAL, 1},
    {"story", do_story, POS_DEAD, 0, LOG_NORMAL, 1},
    {"time", do_time, POS_DEAD, 0, LOG_NORMAL, 1},
    {"typo", do_typo, POS_DEAD, 0, LOG_NORMAL, 1},
    {"weather", do_weather, POS_RESTING, 0, LOG_NORMAL, 1},
    {"who", do_who, POS_DEAD, 0, LOG_NORMAL, 1},
    {"whois", do_finger, POS_DEAD, 0, LOG_NORMAL, 1},
    {"wizlist", do_wizlist, POS_DEAD, 0, LOG_NORMAL, 1},
    {"worth", do_worth, POS_SLEEPING, 0, LOG_NORMAL, 1},

    /*
     * Configuration commands.
     */
    {"afk", do_afk, POS_DEAD, 0, LOG_NORMAL, 1},
#ifdef ANONYMOUS
    {"anonymous", do_anonymous, POS_DEAD, 0, LOG_NORMAL, 1},
#endif
    {"alias", do_alias, POS_DEAD, 0, LOG_NORMAL, 1},
    {"autolist", do_autolist, POS_DEAD, 0, LOG_NORMAL, 1},
    {"autoassist", do_autoassist, POS_DEAD, 0, LOG_NORMAL, 1},
    {"autoexit", do_autoexit, POS_DEAD, 0, LOG_NORMAL, 1},
    {"autogold", do_autogold, POS_DEAD, 0, LOG_NORMAL, 1},
    {"autoloot", do_autoloot, POS_DEAD, 0, LOG_NORMAL, 1},
    {"autosac", do_autosac, POS_DEAD, 0, LOG_NORMAL, 1},
    {"autosplit", do_autosplit, POS_DEAD, 0, LOG_NORMAL, 1},
    {"brief", do_brief, POS_DEAD, 0, LOG_NORMAL, 1},
    {"channels", do_channels, POS_DEAD, 0, LOG_NORMAL, 1},
    {"combine", do_combine, POS_DEAD, 0, LOG_NORMAL, 1},
    {"compact", do_compact, POS_DEAD, 0, LOG_NORMAL, 1},
    {"config", do_autolist, POS_DEAD, 0, LOG_NORMAL, 1},
    {"description", do_description, POS_DEAD, 0, LOG_NORMAL, 1},
    {"delet", do_delet, POS_DEAD, 0, LOG_ALWAYS, 0},
    {"delete", do_delete, POS_DEAD, 0, LOG_ALWAYS, 1},
    {"nofollow", do_nofollow, POS_DEAD, 0, LOG_NORMAL, 1},
    {"noloot", do_noloot, POS_DEAD, 0, LOG_NORMAL, 1},
    {"nosummon", do_nosummon, POS_DEAD, 0, LOG_NORMAL, 1},
    {"color", do_nocolor, POS_DEAD, 0, LOG_NORMAL, 1},
    {"outfit", do_outfit, POS_RESTING, 0, LOG_ALWAYS, 1},
    {"password", do_password, POS_DEAD, 0, LOG_NEVER, 1},
    {"pk", do_pk, POS_DEAD, 0, LOG_ALWAYS, 1},
    {"prompt", do_prompt, POS_DEAD, 0, LOG_NORMAL, 1},
    {"scroll", do_scroll, POS_DEAD, 0, LOG_NORMAL, 1},
    {"title", do_title, POS_DEAD, 0, LOG_NORMAL, 1},
    {"unalias", do_unalias, POS_DEAD, 0, LOG_NORMAL, 1},
    {"wimpy", do_wimpy, POS_DEAD, 0, LOG_NORMAL, 1},
    {"spousetalk", do_spousetalk, POS_DEAD, 0, LOG_NORMAL, 1},
    {"divorce", do_divorce, POS_DEAD, 1, LOG_NORMAL, 1},
    {"consent", do_consent, POS_DEAD, 0, LOG_NORMAL, 1},
    {"marry", do_marry, POS_DEAD, 1, LOG_NORMAL, 1},

    /*
     * Communication commands.
     */
    {"answer", do_answer, MIN_POS_QA, 0, LOG_NORMAL, 1},
    {"auction", do_auction, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"deaf", do_deaf, POS_DEAD, 0, LOG_NORMAL, 1},
    {"emote", do_emote, POS_RESTING, 0, LOG_NORMAL, 1},
    {".", do_gossip, MIN_POS_GOS, 0, LOG_NORMAL, 0},
    {CFG_GOS_NAME, do_gossip, MIN_POS_GOS, 0, LOG_NORMAL, 1},
#ifdef USE_MUSIC
    {CFG_MUS_NAME, do_music, MIN_POS_MUS, 0, LOG_NORMAL, 1},
#endif
#ifdef USE_GOCIAL
    {"gocial", do_gocial, MIN_POS_GOS, 0, LOG_NORMAL, 1},
#endif
    {":", do_emote, POS_RESTING, 0, LOG_NORMAL, 0},
    {"gtell", do_gtell, POS_DEAD, 0, LOG_NORMAL, 1},
    {";", do_gtell, POS_DEAD, 0, LOG_NORMAL, 0},
    {CFG_OOC_NAME, do_ooc, MIN_POS_OOC, 0, LOG_NORMAL, 1},
    {"note", do_note, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"board", do_board, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"pose", do_pose, POS_RESTING, 0, LOG_NORMAL, 1},
    {"question", do_question, MIN_POS_QA, 0, LOG_NORMAL, 1},
    {"quiet", do_quiet, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"reply", do_reply, MIN_POS_TELL, 0, LOG_NORMAL, 1},
    {"say", do_say, POS_RESTING, 0, LOG_NORMAL, 1},
    {"'", do_say, POS_RESTING, 0, LOG_NORMAL, 0},
    {"shout", do_shout, MIN_POS_SHOUT, 0, LOG_NORMAL, 1},
    {"yell", do_yell, MIN_POS_YELL, 0, LOG_NORMAL, 1},

    /*
     * Object manipulation commands.
     */
    {"brandish", do_brandish, POS_RESTING, 0, LOG_NORMAL, 1},
    {"close", do_close, POS_RESTING, 0, LOG_NORMAL, 1},
    {"donate", do_donate, POS_RESTING, 0, LOG_NORMAL, 1},
    {"drink", do_drink, POS_RESTING, 0, LOG_NORMAL, 1},
    {"drop", do_drop, POS_RESTING, 0, LOG_NORMAL, 1},
    {"eat", do_eat, POS_RESTING, 0, LOG_NORMAL, 1},
    {"fill", do_fill, POS_RESTING, 0, LOG_NORMAL, 1},
    {"give", do_give, POS_RESTING, 0, LOG_NORMAL, 1},
    {"hold", do_wear, POS_RESTING, 0, LOG_NORMAL, 1},
    {"identify", do_lore, POS_RESTING, 0, LOG_NORMAL, 1},
    {"list", do_list, POS_RESTING, 0, LOG_NORMAL, 1},
    {"lock", do_lock, POS_RESTING, 0, LOG_NORMAL, 1},
    {"lore", do_lore, POS_RESTING, 0, LOG_NORMAL, 1},
    {"open", do_open, POS_RESTING, 0, LOG_NORMAL, 1},
    {"pick", do_pick, POS_RESTING, 0, LOG_NORMAL, 1},
    {"pour", do_pour, POS_RESTING, 0, LOG_NORMAL, 1},
    {"put", do_put, POS_RESTING, 0, LOG_NORMAL, 1},
    {"quaff", do_quaff, POS_RESTING, 0, LOG_NORMAL, 1},
    {"recite", do_recite, POS_RESTING, 0, LOG_NORMAL, 1},
    {"remove", do_remove, POS_RESTING, 0, LOG_NORMAL, 1},
#ifdef USE_REBIRTH
    {"rebirt", do_rebirt, POS_RESTING, 0, LOG_ALWAYS, 1},
    {"rebirth", do_rebirth, POS_RESTING, 0, LOG_ALWAYS, 1},
#endif
#ifdef USE_REMORT
    {"remor", do_remor, POS_DEAD, 0, LOG_ALWAYS, 0},
    {"remort", do_remort, POS_STANDING, 0, LOG_ALWAYS, 1},
#endif
    {"sell", do_sell, POS_RESTING, 0, LOG_NORMAL, 1},
    {"take", do_get, POS_RESTING, 0, LOG_NORMAL, 1},
    {"sacrifice", do_sacrifice, POS_RESTING, 0, LOG_NORMAL, 1},
    {"junk", do_junk, POS_RESTING, 0, LOG_NORMAL, 0},
    {"tap", do_sacrifice, POS_RESTING, 0, LOG_NORMAL, 0},
    {"unlock", do_unlock, POS_RESTING, 0, LOG_NORMAL, 1},
    {"value", do_value, POS_RESTING, 0, LOG_NORMAL, 1},
    {"wear", do_wear, POS_RESTING, 0, LOG_NORMAL, 1},
    {"zap", do_zap, POS_RESTING, 0, LOG_NORMAL, 1},

    /*
     * Combat commands.
     */
    {"backstab", do_backstab, POS_STANDING, 0, LOG_NORMAL, 1},
    {"circle", do_circle, POS_FIGHTING, 0, LOG_NORMAL, 1},
    {"bash", do_bash, POS_FIGHTING, 0, LOG_NORMAL, 1},
    {"bs", do_backstab, POS_STANDING, 0, LOG_NORMAL, 0},
    {"berserk", do_berserk, POS_FIGHTING, 0, LOG_NORMAL, 1},
    {"dirt", do_dirt, POS_FIGHTING, 0, LOG_NORMAL, 1},
    {"disarm", do_disarm, POS_FIGHTING, 0, LOG_NORMAL, 1},
    {"flee", do_flee, POS_FIGHTING, 0, LOG_NORMAL, 1},
    {"kick", do_kick, POS_FIGHTING, 0, LOG_NORMAL, 1},
    {"shield", do_shield, POS_FIGHTING, 0, LOG_NORMAL, 1},
    {"weapon", do_weapon, POS_FIGHTING, 0, LOG_NORMAL, 1},
    {"blackjack", do_blackjack, POS_STANDING, 0, LOG_NORMAL, 1},
    {"murde", do_murde, POS_FIGHTING, 1, LOG_NORMAL, 0},
    {"murder", do_murder, POS_FIGHTING, 1, LOG_ALWAYS, 1},
    {"rescue", do_rescue, POS_FIGHTING, 0, LOG_NORMAL, 0},
    {"trip", do_trip, POS_FIGHTING, 0, LOG_NORMAL, 1},

    /*
     * Miscellaneous commands.
     */
    {"follow", do_follow, POS_RESTING, 0, LOG_NORMAL, 1},
    {"gain", do_gain, POS_STANDING, 0, LOG_NORMAL, 1},
    {"group", do_group, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"sgroups", do_groups, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"hide", do_hide, POS_RESTING, 0, LOG_NORMAL, 1},
    {"qui", do_qui, POS_DEAD, 0, LOG_NORMAL, 0},
    {"quit", do_quit, POS_DEAD, 0, LOG_NORMAL, 1},
    {"recall", do_recall, POS_FIGHTING, 0, LOG_NORMAL, 1},
    {"/", do_recall, POS_FIGHTING, 0, LOG_NORMAL, 0},
    {"save", do_save, POS_DEAD, 0, LOG_NORMAL, 1},
    {"sleep", do_sleep, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"sneak", do_sneak, POS_STANDING, 0, LOG_NORMAL, 1},
    {"split", do_split, POS_RESTING, 0, LOG_NORMAL, 1},
    {"steal", do_steal, POS_STANDING, 0, LOG_NORMAL, 1},
    {"train", do_train, POS_RESTING, 0, LOG_NORMAL, 1},
    {"visible", do_visible, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"wake", do_wake, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"where", do_where, POS_RESTING, 0, LOG_NORMAL, 1},
    {"brew", do_brew, POS_RESTING, 0, LOG_NORMAL, 1},
    {"scribe", do_scribe, POS_RESTING, 0, LOG_NORMAL, 1},
    {"comment", do_comment, POS_DEAD, 0, LOG_NORMAL, 1},
    {"email", do_email, POS_DEAD, 0, LOG_NORMAL, 1},
    {"mversion", do_version, POS_DEAD, 0, LOG_NORMAL, 1},

/*
     * Clan commands!
     */
    {"promote", do_promote, POS_RESTING, 0, LOG_NORMAL, 1},
    {"clan", do_clan, POS_DEAD, 1, LOG_NORMAL, 1},
    {"show", do_show, POS_RESTING, 0, LOG_NORMAL, 1},
    {"join", do_join, POS_RESTING, 0, LOG_NORMAL, 1},
    {"petition", do_petition, POS_RESTING, 0, LOG_NORMAL, 1},
    {"accept", do_accept, POS_RESTING, 0, LOG_NORMAL, 0},
    {"decline", do_decline, POS_RESTING, 0, LOG_NORMAL, 0},
    {"offer", do_offer, POS_RESTING, 0, LOG_NORMAL, 1},
    {"clantalk", do_clantalk, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"resign", do_resign, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"demote", do_demote, POS_SLEEPING, 0, LOG_NORMAL, 1},
    {"crecall", do_crecall, POS_FIGHTING, 0, LOG_NORMAL, 1},

    /*
     * Immortal commands.
     */
    {"mpchangefaction", do_mpchangefaction, POS_DEAD, 0, LOG_NORMAL, 1},
    {"mpsilentchangefaction", do_mpsilentchangefaction, POS_DEAD, 0, LOG_NORMAL,
     1},
    {"advance", do_advance, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"advance", do_levelgain, POS_DEAD, 0, LOG_NORMAL, 1},
    {"award", do_award, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"hotboo", do_hotboo, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"hotboot", do_copyover, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"dump", do_dump, POS_DEAD, 1, LOG_ALWAYS, 0},
    {"trust", do_trust, POS_DEAD, 1, LOG_ALWAYS, 1},

    {"allow", do_allow, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"ban", do_ban, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"permban", do_permban, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"cut", do_new_discon, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"deny", do_deny, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"disconnect", do_disconnect, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"freeze", do_freeze, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"reboo", do_reboo, POS_DEAD, 1, LOG_NORMAL, 0},
    {"reboot", do_reboot, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"repop", do_repop, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"splist", do_splist, POS_DEAD, 1, LOG_NORMAL, 1},
    {"sklist", do_sklist, POS_DEAD, 1, LOG_NORMAL, 1},
    {"sinfo", do_sendinfo, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"set", do_set, POS_DEAD, 1, LOG_ALWAYS, 1},
#ifdef REQUIRE_EDIT_PERMISSION
    {"setedit", do_setedit, POS_DEAD, 1, LOG_ALWAYS, 1},
#endif
#ifdef REQUIRE_MUDPROG_PERMISSION
    {"setprog", do_setprog, POS_DEAD, 1, LOG_ALWAYS, 1},
#endif
    {"shutdow", do_shutdow, POS_DEAD, 1, LOG_NORMAL, 0},
    {"shutdown", do_shutdown, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"wizlock", do_wizlock, POS_DEAD, 1, LOG_ALWAYS, 1},

    {"force", do_force, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"load", do_load, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"newlock", do_newlock, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"nochannels", do_nochannels, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"noemote", do_noemote, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"noshout", do_noshout, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"notell", do_notell, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"pecho", do_pecho, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"pardon", do_pardon, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"purge", do_purge, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"restore", do_restore, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"sla", do_sla, POS_DEAD, 1, LOG_NORMAL, 0},
    {"slay", do_slay, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"spellup", do_spellup, POS_DEAD, 1, LOG_NORMAL, 1},
    {"mortslay", do_mortslay, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"teleport", do_transfer, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"transfer", do_transfer, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"objcheck", do_objcheck, POS_DEAD, 1, LOG_NORMAL, 1},
    {"poofin", do_bamfin, POS_DEAD, 1, LOG_NORMAL, 1},
    {"poofout", do_bamfout, POS_DEAD, 1, LOG_NORMAL, 1},
    {"gecho", do_echo, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"holylight", do_holylight, POS_DEAD, 1, LOG_NORMAL, 1},
    {"log", do_log, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"memory", do_memory, POS_DEAD, 1, LOG_NORMAL, 1},
    {"mwhere", do_mwhere, POS_DEAD, 1, LOG_NORMAL, 1},
    {"owhere", do_owhere, POS_DEAD, 1, LOG_NORMAL, 1},
    {"peace", do_peace, POS_DEAD, 1, LOG_NORMAL, 1},
    {"echo", do_recho, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"repeat", do_repeat, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"return", do_return, POS_DEAD, 1, LOG_NORMAL, 1},
    {"snoop", do_snoop, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"stat", do_stat, POS_DEAD, 1, LOG_NORMAL, 1},
    {"shell", do_shell, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"string", do_string, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"switch", do_switch, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"wizinvis", do_invis, POS_DEAD, 1, LOG_NORMAL, 1},
    {"jail", do_jail, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"vnum", do_vnum, POS_DEAD, 1, LOG_NORMAL, 1},
    {"clone", do_clone, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"immtalk", do_immtalk, POS_DEAD, 1, LOG_NORMAL, 1},
#ifdef USE_ADMINTALK
    {"admintalk", do_admintalk, POS_DEAD, 1, LOG_NORMAL, 1},
#endif
#ifdef USE_HEROTALK
    {"herotalk", do_herotalk, POS_DEAD, 1, LOG_NORMAL, 1},
#endif
    {"imotd", do_imotd, POS_DEAD, 1, LOG_NORMAL, 1},
    {",", do_immtalk, POS_DEAD, 1, LOG_NORMAL, 0},
    {"beacon", do_beacon, POS_DEAD, BEACON_WIZ, LOG_NORMAL, 1},
    {"beaconreset", do_beaconreset, POS_DEAD, BEACON_WIZ, LOG_NORMAL, 1},
    {"chaos", do_chaos, POS_DEAD, 1, LOG_ALWAYS, 1},
    {"rlist", do_rlist, POS_DEAD, 1, LOG_NORMAL, 0},
    {"aexits", do_aexits, POS_DEAD, 1, LOG_NORMAL, 1},
    {"aentrances", do_aentrances, POS_DEAD, 1, LOG_NORMAL, 1},
    {"cdeposit", do_cdeposit, POS_RESTING, 0, LOG_NORMAL, 1},
    {"roster", do_roster, POS_RESTING, 0, LOG_NORMAL, 1},
    {"olevel", do_olevel, POS_DEAD, 1, LOG_NORMAL, 1},
    {"mlevel", do_mlevel, POS_DEAD, 1, LOG_NORMAL, 1},

    /*
     * OLC
     */
    {"edit", do_olc, POS_DEAD, 1, LOG_NORMAL, 1},
    {"asave", do_asave, POS_DEAD, 1, LOG_NORMAL, 1},
    {"alist", do_alist, POS_DEAD, 1, LOG_NORMAL, 1},
    {"resets", do_resets, POS_DEAD, 1, LOG_NORMAL, 1},

    /*
     * MudProg commands.
     */
    {"mpstat", do_mpstat, POS_DEAD, 1, LOG_NORMAL, 1},
    {"mpasound", do_mpasound, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpclean", do_mpclean, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpinvis", do_mpinvis, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpjunk", do_mpjunk, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpeatcorpse", do_mpeatcorpse, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpecho", do_mpecho, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpechoat", do_mpechoat, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpechoaround", do_mpechoaround, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpfollowpath", do_mpfollowpath, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpkill", do_mpkill, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpmload", do_mpmload, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpoload", do_mpoload, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mppurge", do_mppurge, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpgoto", do_mpgoto, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpat", do_mpat, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mptransfer", do_mptransfer, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpforce", do_mpforce, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpremember", do_mpremember, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpforget", do_mpforget, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpdosilent", do_mpdosilent, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mprandomsocial", do_mprandomsocial, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpsilentforce", do_mpsilentforce, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpsilentcast", do_mpsilentcast, POS_DEAD, 0, LOG_NORMAL, 0},
    {"mpdefault", do_mpdefault, POS_DEAD, 0, LOG_NORMAL, 0},

    /*
     * End of list.
     */
    {"", 0, POS_DEAD, 0, LOG_NORMAL, 0}
};

bool is_immcmd( char *command )
{
    struct cmd_type *cmd;

    for ( cmd = ( struct cmd_type * ) cmd_table; *cmd->name; cmd++ )
    {
        if ( *command == *cmd->name && !str_prefix( command, cmd->name ) )
        {
            return cmd->imm;
        }
    }

    return FALSE;
}

bool can_do_immcmd( CHAR_DATA * ch, char *cmd )
{
    IMMCMD_TYPE *tmp;

    if ( IS_NPC( ch ) )
    {
        /* If npc is switched, use PC's perms */
        if ( ch->desc != NULL )
        {
            ch = ch->desc->original;
        }
        /* Otherwise, mobs can't do imm commands */
        else
        {
            return FALSE;
        }
    }

    for ( tmp = ch->pcdata->immcmdlist; tmp != NULL; tmp = tmp->next )
    {
        if ( !str_cmp( tmp->cmd, cmd ) )
        {
            return TRUE;
        }
    }

    return FALSE;
}

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA * ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char *logline;
    struct cmd_type *cmd;
    int trust;
    bool found;
    bool can_do;

    /*
     * Strip leading spaces.
     */
    while ( isspace( *argument ) )
        argument++;

    if ( !*argument )
        return;

    /*
     * No hiding.
     */
    REMOVE_BIT( ch->affected_by, AFF_HIDE );

    /*
     * Implement freeze command.
     */
    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_FREEZE ) )
    {
        send_to_char( "You're totally frozen!\n\r", ch );
        return;
    }

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    logline = argument;
    if ( !isalpha( *argument ) && !isdigit( *argument ) )
    {
        *command = *argument++;
        command[1] = '\0';
        while ( isspace( *argument ) )
            argument++;
    }
    else
        argument = one_argument( argument, command );

    /*
     * MUDProg command triggers.
     */
    if ( !IS_NPC( ch ) )
    {
        can_do = mprog_command_trigger( command, ch, argument );
        if ( !can_do )
            return;
    }

    /*
     * Look for command in command table.
     */
    found = FALSE;
    trust = get_trust( ch );
    for ( cmd = ( struct cmd_type * ) cmd_table; *cmd->name; cmd++ )
    {
        if ( *command == LOWER( *cmd->name )
             && !str_prefix( command, cmd->name ) )
        {
            /* if the command is an imm command but the char can't
             * execute it, keep searching */
            if ( ( cmd->imm ) && ( !can_do_immcmd( ch, cmd->name ) ) )
            {
                continue;
            }
            else
            {
                found = TRUE;
                break;
            }
        }
    }

    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_AFK ) )
    {
        char buff[MAX_STRING_LENGTH];

        if ( str_prefix( command, "afk" ) )
        {
            sprintf( buff, "`RYou're still AFK!`w - %d message%s waiting.\n\r",
                     ch->pcdata->messages,
                     ( ( ch->pcdata->messages > 1 )
                       || ( ch->pcdata->messages < 1 ) ) ? "s" : "" );
            send_to_char( buff, ch );
        }
    }

    /*
     * Log and snoop.
     */
    if ( ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_LOG ) )
         || fLogAll || cmd->log == LOG_ALWAYS )
    {
        sprintf( log_buf, "Log %s: %s", ch->name, logline );
        log_string( log_buf );
    }

    if ( ch->desc && ch->desc->snoop_by )
    {
        write_to_buffer( ch->desc->snoop_by, "% ", 2 );
        write_to_buffer( ch->desc->snoop_by, logline, 0 );
        write_to_buffer( ch->desc->snoop_by, "\n\r", 2 );
    }

    if ( !found )
    {
        /*
         * Look for command in socials table.
         */
        if ( !check_social( ch, command, argument ) )
            send_to_char( "Huh?\n\r", ch );
        return;
    }
    else /* a normal valid command.. check if it is disabled */
    if ( check_disabled( cmd ) )
    {
        send_to_char( "This command has been temporarily disabled.\n\r", ch );
        return;
    }

    /*
     * Character not in position for command?
     */
    if ( ch->position < cmd->position )
    {
        switch ( ch->position )
        {
        case POS_DEAD:
            send_to_char( "Lie still; you are DEAD.\n\r", ch );
            break;

        case POS_MORTAL:
        case POS_INCAP:
            send_to_char( "You are hurt far too bad for that.\n\r", ch );
            break;

        case POS_STUNNED:
            send_to_char( "You are too stunned to do that.\n\r", ch );
            break;

        case POS_SLEEPING:
            send_to_char( "In your dreams, or what?\n\r", ch );
            break;

        case POS_RESTING:
            send_to_char( "Nah... You feel too relaxed...\n\r", ch );
            break;

        case POS_SITTING:
            send_to_char( "Better stand up first.\n\r", ch );
            break;

        case POS_FIGHTING:
            send_to_char( "No way!  You are still fighting!\n\r", ch );
            break;
        }
        return;
    }

    /*
     * Dispatch the command.
     */
    ( cmd->do_fun ) ( ch, argument );

    tail_chain(  );
    return;
}

bool check_social( CHAR_DATA * ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
/*    struct social_type *cmd;*/
    SOCIALLIST_DATA *cmd;
    bool found;

    found = FALSE;
    for ( cmd = social_first; cmd != NULL; cmd = cmd->next )
    {
        if ( ( is_name( command, cmd->name ) ) )
        {
            found = TRUE;
            break;
        }
    }

    if ( !found )
        return FALSE;

    if ( !IS_NPC( ch ) && IS_SET( ch->comm, COMM_NOEMOTE ) )
    {
        send_to_char( "You are anti-social!\n\r", ch );
        return TRUE;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
        send_to_char( "Lie still; you are DEAD.\n\r", ch );
        return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
        send_to_char( "You are hurt far too bad for that.\n\r", ch );
        return TRUE;

    case POS_STUNNED:
        send_to_char( "You are too stunned to do that.\n\r", ch );
        return TRUE;

    case POS_SLEEPING:
        /*
         * I just know this is the path to a 12" 'if' statement.  :(
         * But two players asked for it already!  -- Furey
         */
        if ( !str_cmp( cmd->name, "snore" ) )
            break;
        send_to_char( "In your dreams, or what?\n\r", ch );

        return TRUE;
    }

    one_argument( argument, arg );
    victim = NULL;
    if ( !*arg )
    {
        if ( str_cmp( cmd->others_no_arg, "none" ) )
        {
            act( cmd->others_no_arg, ch, NULL, victim, TO_ROOM );
        }
        if ( str_cmp( cmd->char_no_arg, "none" ) )
        {
            act( cmd->char_no_arg, ch, NULL, victim, TO_CHAR );
        }
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
        if ( !str_cmp( cmd->char_not_found, "none" ) )
        {

            send_to_char( "There is nothing here that matches that name.", ch );
        }
        else
            act( cmd->char_not_found, ch, NULL, victim, TO_CHAR );
    }
    else if ( victim == ch )
    {
        if ( str_cmp( cmd->others_auto, "none" ) )
        {
            act( cmd->others_auto, ch, NULL, victim, TO_ROOM );
        }
        if ( str_cmp( cmd->char_auto, "none" ) )
        {
            act( cmd->char_auto, ch, NULL, victim, TO_CHAR );
        }
    }
    else
    {
        if ( str_cmp( cmd->others_found, "none" ) )
        {
            act( cmd->others_found, ch, NULL, victim, TO_NOTVICT );
        }
        if ( str_cmp( cmd->char_found, "none" ) )
        {
            act( cmd->char_found, ch, NULL, victim, TO_CHAR );
        }
        if ( str_cmp( cmd->vict_found, "none" ) )
        {
            act( cmd->vict_found, ch, NULL, victim, TO_VICT );
        }

        if ( !IS_NPC( ch ) && IS_NPC( victim )
             && !IS_AFFECTED( victim, AFF_CHARM )
             && IS_AWAKE( victim ) && victim->desc == NULL )
        {
            switch ( number_bits( 4 ) )
            {
            case 0:

            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                if ( str_cmp( cmd->others_found, "none" ) )
                {
                    act( cmd->others_found, victim, NULL, ch, TO_NOTVICT );
                }
                if ( str_cmp( cmd->char_found, "none" ) )
                {
                    act( cmd->char_found, victim, NULL, ch, TO_CHAR );
                }
                if ( str_cmp( cmd->vict_found, "none" ) )
                {
                    act( cmd->vict_found, victim, NULL, ch, TO_VICT );
                }
                break;

            case 9:
            case 10:
            case 11:
            case 12:
                act( "$n slaps $N.", victim, NULL, ch, TO_NOTVICT );
                act( "You slap $N.", victim, NULL, ch, TO_CHAR );
                act( "$n slaps you.", victim, NULL, ch, TO_VICT );
                break;
            }
        }
    }

    return TRUE;
}

/*
 * Return true if an argument is completely numeric.
 */
bool is_number( char *arg )
{

    if ( *arg == '\0' )
        return FALSE;

    if ( *arg == '+' || *arg == '-' )
        arg++;

    for ( ; *arg != '\0'; arg++ )
    {
        if ( !isdigit( *arg ) )
            return FALSE;
    }

    return TRUE;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( char *argument, char *arg )
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
        if ( *pdot == '.' )
        {
            *pdot = '\0';
            number = atoi( argument );
            *pdot = '.';
            strcpy( arg, pdot + 1 );
            return number;
        }
    }

    strcpy( arg, argument );
    return 1;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 * Converts string to lower case.
 */
char *one_argument( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace( *argument ) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *arg_first = LOWER( *argument );
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    while ( isspace( *argument ) )
        argument++;

    return argument;
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 * Doesn't change case.
 */
char *one_argument2( char *argument, char *arg_first )
{
    char cEnd;

    while ( isspace( *argument ) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *arg_first = *argument;
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    while ( isspace( *argument ) )
        argument++;

    return argument;
}

/* chk_command function coded by Laurie Zenner.  */
void chk_command( CHAR_DATA * ch, char *argument )
{
    char workstr[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int pos;
    int var;
    int cmd;
    int trust;
    bool found;

    strcpy( workstr, argument );
    argument[0] = '\0';

    /*
     * Strip leading spaces.
     */
    pos = 0;
    while ( isspace( workstr[pos] ) )
        pos++;

    if ( workstr[pos] == '\0' )
        return;

    /*
     * Implement freeze command.
     */
    if ( !IS_NPC( ch ) && IS_SET( ch->act, PLR_FREEZE ) )
        return;

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    if ( !isalpha( workstr[pos] ) && !isdigit( workstr[pos] ) )
    {
        command[0] = workstr[pos];
        command[1] = '\0';
        while ( isspace( workstr[pos] ) )
            pos++;
    }
    else
    {
        var = 0;
        while ( !isspace( workstr[pos] ) )
        {
            if ( workstr[pos] == '\0' )
                break;

            command[var] = workstr[pos];
            var++;
            pos++;
        }
        command[pos] = '\0';
        while ( isspace( workstr[pos] ) )
            pos++;
    }

    /*
     * Look for command in command table.
     */
    found = FALSE;
    trust = get_trust( ch );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( command[0] == cmd_table[cmd].name[0]
             && !str_prefix( command, cmd_table[cmd].name ) )
        {
            /* if the command is an imm command but the char can't
               execute it, keep searching. */

            if ( ( cmd_table[cmd].imm )
                 && ( !can_do_immcmd( ch, cmd_table[cmd].name ) ) )
            {
                continue;
            }
            else
            {
                found = TRUE;
                sprintf( argument, "%s", cmd_table[cmd].name );
                break;
            }
        }
    }

    if ( found )
    {
        if ( ch->position < cmd_table[cmd].position )
            found = FALSE;      /* Character not in position for command */
    }
    else
    {

        /* Look for command in social table */
        SOCIALLIST_DATA *cmd;
        for ( cmd = social_first; cmd != NULL; cmd = cmd->next )
        {
            if ( ( is_name( command, cmd->name ) ) )
            {
                found = TRUE;
                sprintf( argument, "%s", cmd->name );
                break;
            }
        }

        if ( found )
        {
            if ( !IS_NPC( ch ) && IS_SET( ch->comm, COMM_NOEMOTE ) )
                found = FALSE;
            else
            {
                switch ( ch->position )
                {
                case POS_DEAD:
                case POS_INCAP:
                case POS_MORTAL:
                case POS_STUNNED:
                    found = FALSE;
                    break;

                case POS_SLEEPING:
                    if ( str_cmp( cmd->name, "snore" ) )
                        found = FALSE;
                    break;
                }
            }
        }
        if ( found )
        {
            var = 0;
            while ( !isspace( workstr[pos] ) )
            {
                if ( workstr[pos] == '\0' )
                    return;

                arg[var] = workstr[pos];
                pos++;
                var++;
            }
            arg[var] = '\0';

            victim = NULL;
            if ( arg[0] != '\0' )
            {
                victim = get_char_room( ch, arg );
                if ( victim == NULL )
                    found = FALSE;
            }
        }
    }

    if ( !found )
        argument[0] = '\0';

    return;
}

/*
 * Contributed by Alander.
 */
void do_commands( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;

    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( !cmd_table[cmd].imm && cmd_table[cmd].show )
        {
            sprintf( buf, "%-12s", cmd_table[cmd].name );
            send_to_char( buf, ch );
            if ( ++col % 6 == 0 )
                send_to_char( "\n\r", ch );
        }
    }

    if ( col % 6 != 0 )
        send_to_char( "\n\r", ch );
    return;
}

void do_wizhelp( CHAR_DATA * ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;

    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( cmd_table[cmd].imm && can_do_immcmd( ch, cmd_table[cmd].name )
             && cmd_table[cmd].show )
        {
            sprintf( buf, "%s%-12s`w",
                     check_disabled( &cmd_table[cmd] ) ? "`K-" : "`w ",
                     cmd_table[cmd].name );
            send_to_char( buf, ch );
            if ( ++col % 6 == 0 )
                send_to_char( "\n\r", ch );
        }
    }

    if ( col % 6 != 0 )
        send_to_char( "\n\r", ch );

    if ( !IS_NPC( ch ) && ch->pcdata->immcmdlist == NULL )
        send_to_char( "Huh?\n\r", ch );

    return;
}

/* interpret without dumb stuff... used for mob programs... forcing, etc */
void mpinterpret( CHAR_DATA * ch, char *argument )
{
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    int cmd;
    int trust;
    bool found;

    /*
     * Strip leading spaces.
     */
    while ( isspace( *argument ) )
        argument++;
    if ( argument[0] == '\0' )
        return;

    /*
     * Grab the command word.
     * Special parsing so ' can be a command,
     *   also no spaces needed after punctuation.
     */
    strcpy( logline, argument );
    if ( !isalpha( argument[0] ) && !isdigit( argument[0] ) )
    {
        command[0] = argument[0];
        command[1] = '\0';
        argument++;
        while ( isspace( *argument ) )
            argument++;
    }
    else
    {
        argument = one_argument( argument, command );
    }

    /*
     * Look for command in command table.
     */
    found = FALSE;
    trust = get_trust( ch );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( command[0] == cmd_table[cmd].name[0]
             && !str_prefix( command, cmd_table[cmd].name )
             && !cmd_table[cmd].imm )
        {
            found = TRUE;
            break;
        }
    }

    if ( !found )
    {
        /*
         * Look for command in socials table.
         */
        if ( !check_social( ch, command, argument ) )
            send_to_char( "Huh?\n\r", ch );
        return;
    }

    /*
     * Character not in position for command?
     */
    if ( ch->position < cmd_table[cmd].position )
    {
        return;
    }

    /*
     * Dispatch the command.
     */
    update_last( "Interpret: ", command, argument );
    ( *cmd_table[cmd].do_fun ) ( ch, argument );

    tail_chain(  );
    return;
}

void do_disable( CHAR_DATA * ch, char *argument )
{
    int i;
    DISABLED_DATA *p, *q;
    char buf[100];

    if ( IS_NPC( ch ) )
    {
        send_to_char( "RETURN first.\n\r", ch );
        return;
    }

    if ( !argument[0] )         /* Nothing specified. Show disabled commands. */
    {
        if ( !disabled_first )  /* Any disabled at all ? */
        {
            send_to_char( "There are no commands disabled.\n\r", ch );
            return;
        }

        send_to_char( "Disabled commands:\n\r"
                      "Command      Level   Disabled by\n\r", ch );

        for ( p = disabled_first; p; p = p->next )
        {
            sprintf( buf, "%-12s %5d %-12s\n\r", p->command->name, p->level,
                     p->disabled_by );
            send_to_char( buf, ch );
        }
        return;
    }

    /* command given */

    /* First check if it is one of the disabled commands */
    for ( p = disabled_first; p; p = p->next )
        if ( !str_cmp( argument, p->command->name ) )
            break;

    if ( p )                    /* this command is disabled */
    {
        /* Optional: The level of the imm to enable the command must equal or exceed level
           of the one that disabled it */

        if ( get_trust( ch ) < p->level )
        {
            send_to_char( "This command was disabled by a higher power.\n\r",
                          ch );
            return;
        }

        /* Remove */

        if ( disabled_first == p )  /* node to be removed == head ? */
            disabled_first = p->next;
        else                    /* Find the node before this one */
        {
            for ( q = disabled_first; q->next != p; q = q->next );  /* empty for */
            q->next = p->next;
        }

        free_string( &p->disabled_by ); /* free name of disabler */
        free_mem( &p );         /* free node */
        save_disabled(  );      /* save to disk */
        send_to_char( "Command enabled.\n\r", ch );
    }
    else                        /* not a disabled command, check if that command exists */
    {
        /* IQ test */
        if ( !str_cmp( argument, "disable" ) )
        {
            send_to_char( "You cannot disable the disable command.\n\r", ch );
            return;
        }

        /* Search for the command */
        for ( i = 0; cmd_table[i].name[0] != '\0'; i++ )
            if ( !str_cmp( cmd_table[i].name, argument ) )
                break;

        /* Found? */
        if ( cmd_table[i].name[0] == '\0' )
        {
            send_to_char( "No such command.\n\r", ch );
            return;
        }

        /* Can the imm use this command at all ? */
        if ( cmd_table[i].imm && ( !can_do_immcmd( ch, cmd_table[i].name ) ) )
        {
            send_to_char
                ( "You do not have access to that command; you cannot disable it.\n\r",
                  ch );
            return;
        }

        /* Disable the command */

        p = alloc_mem( sizeof( DISABLED_DATA ) );

        p->command = &cmd_table[i];
        p->disabled_by = str_dup( ch->name );   /* save name of disabler */
        p->level = get_trust( ch ); /* save trust */
        p->next = disabled_first;
        disabled_first = p;     /* add before the current first element */

        send_to_char( "Command disabled.\n\r", ch );
        save_disabled(  );      /* save to disk */
    }
}

/* Check if that command is disabled
   Note that we check for equivalence of the do_fun pointers; this means
   that disabling 'chat' will also disable the '.' command
*/
bool check_disabled( const struct cmd_type *command )
{
    DISABLED_DATA *p;

    for ( p = disabled_first; p; p = p->next )
        if ( p->command->do_fun == command->do_fun )
            return TRUE;

    return FALSE;
}

/* Load disabled commands */
void load_disabled(  )
{
    FILE *fp;
    DISABLED_DATA *p;
    char *name;
    int i;

    disabled_first = NULL;

    fp = fopen( sysconfig.disable_file, "r" );

    if ( !fp )                  /* No disabled file.. no disabled commands : */
        return;

    name = fread_word( fp );

    while ( str_cmp( name, END_MARKER ) )   /* as long as name is NOT END_MARKER :) */
    {
        /* Find the command in the table */
        for ( i = 0; cmd_table[i].name[0]; i++ )
            if ( !str_cmp( cmd_table[i].name, name ) )
                break;

        if ( !cmd_table[i].name[0] )    /* command does not exist? */
        {
            bug( "Skipping uknown command in disable_file file.", 0 );
            fread_number( fp ); /* level */
            fread_word( fp );   /* disabled_by */
        }
        else                    /* add new disabled command */
        {
            p = alloc_mem( sizeof( DISABLED_DATA ) );
            p->command = &cmd_table[i];
            p->level = fread_number( fp );
            p->disabled_by = str_dup( fread_word( fp ) );
            p->next = disabled_first;

            disabled_first = p;

        }

        name = fread_word( fp );
    }

    fclose( fp );
}

/* Save disabled commands */
void save_disabled(  )
{
    FILE *fp;
    DISABLED_DATA *p;

    if ( !disabled_first )      /* delete file if no commands are disabled */
    {
        unlink( sysconfig.disable_file );
        return;
    }

    fp = fopen( sysconfig.disable_file, "w" );

    if ( !fp )
    {
        bug( "Could not open disable_file  for writing", 0 );
        return;
    }

    for ( p = disabled_first; p; p = p->next )
        fprintf( fp, "%s %d %s\n", p->command->name, p->level, p->disabled_by );

    fprintf( fp, "%s\n", END_MARKER );

    fclose( fp );
}
