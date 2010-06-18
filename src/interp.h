/* this is a listing of all the commands and command related data */
#ifndef _INTERP_H_
#define _INTERP_H_
#include "olc.h"
/* for command types */
#define ML      MAX_LEVEL       /* implementor */
#define L1      MAX_LEVEL - 1   /* creator */
#define L2      MAX_LEVEL - 2   /* supreme being */
#define L3      MAX_LEVEL - 3   /* deity */
#define L4      MAX_LEVEL - 4   /* god */
#define L5      MAX_LEVEL - 5   /* immortal */
#define L6      MAX_LEVEL - 6   /* demigod */
#define L7      MAX_LEVEL - 7   /* angel */
#define L8      MAX_LEVEL - 8   /* avatar */
#define IM      LEVEL_IMMORTAL  /* angel */
#define HE      LEVEL_HERO      /* hero */

/*
 * Structure for a command in the command lookup table.
 */
struct cmd_type {
    char *const name;
    DO_FUN *do_fun;
    sh_int position;
    bool imm;
    sh_int log;
    bool show;
};

struct immcmd_type {
    char *cmd;
    IMMCMD_TYPE *next;
};

/* the command table itself */
extern const struct cmd_type cmd_table[];

extern bool can_do_immcmd( CHAR_DATA * ch, char *cmd );
extern bool is_immcmd( char *command );

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN( do_advance );
DECLARE_DO_FUN( do_aexits );
DECLARE_DO_FUN( do_aentrances );
DECLARE_DO_FUN( do_afk );
DECLARE_DO_FUN( do_anonymous );
DECLARE_DO_FUN( do_alias );
DECLARE_DO_FUN( do_allow );
DECLARE_DO_FUN( do_answer );
DECLARE_DO_FUN( do_areas );
DECLARE_DO_FUN( do_at );
DECLARE_DO_FUN( do_auction );
DECLARE_DO_FUN( do_autoassist );
DECLARE_DO_FUN( do_autoexit );
DECLARE_DO_FUN( do_autogold );
DECLARE_DO_FUN( do_autolist );
DECLARE_DO_FUN( do_autoloot );
DECLARE_DO_FUN( do_autosac );
DECLARE_DO_FUN( do_autosplit );
DECLARE_DO_FUN( do_award );
DECLARE_DO_FUN( do_backstab );
DECLARE_DO_FUN( do_bamfin );
DECLARE_DO_FUN( do_bamfout );
DECLARE_DO_FUN( do_ban );       /* Does not use the old ban functions. -Lancelight */
DECLARE_DO_FUN( do_bash );
DECLARE_DO_FUN( do_berserk );
DECLARE_DO_FUN( do_board );
DECLARE_DO_FUN( do_brandish );
DECLARE_DO_FUN( do_brief );
DECLARE_DO_FUN( do_bug );
DECLARE_DO_FUN( do_buy );
DECLARE_DO_FUN( do_cast );
DECLARE_DO_FUN( do_changes );
DECLARE_DO_FUN( do_channels );
DECLARE_DO_FUN( do_circle );
DECLARE_DO_FUN( do_clone );
DECLARE_DO_FUN( do_close );
DECLARE_DO_FUN( do_commands );
DECLARE_DO_FUN( do_comment );
DECLARE_DO_FUN( do_combine );
DECLARE_DO_FUN( do_compact );
DECLARE_DO_FUN( do_compare );
DECLARE_DO_FUN( do_consider );
DECLARE_DO_FUN( do_hotboo );
DECLARE_DO_FUN( do_copyover );
DECLARE_DO_FUN( do_credits );
DECLARE_DO_FUN( do_bank );
DECLARE_DO_FUN( do_deposit );
DECLARE_DO_FUN( do_withdraw );
DECLARE_DO_FUN( do_account );
DECLARE_DO_FUN( do_tick );
DECLARE_DO_FUN( do_deaf );
DECLARE_DO_FUN( do_delet );
DECLARE_DO_FUN( do_delete );
DECLARE_DO_FUN( do_deny );
DECLARE_DO_FUN( do_description );
DECLARE_DO_FUN( do_dirt );
DECLARE_DO_FUN( do_disarm );
DECLARE_DO_FUN( do_disconnect );
DECLARE_DO_FUN( do_donate );
DECLARE_DO_FUN( do_down );
DECLARE_DO_FUN( do_drink );
DECLARE_DO_FUN( do_drop );
DECLARE_DO_FUN( do_dump );
DECLARE_DO_FUN( do_east );
DECLARE_DO_FUN( do_eat );
DECLARE_DO_FUN( do_echo );
DECLARE_DO_FUN( do_effects );
DECLARE_DO_FUN( do_email );
DECLARE_DO_FUN( do_emote );
DECLARE_DO_FUN( do_enter );     /*Enter a portal */
DECLARE_DO_FUN( do_equipment );
DECLARE_DO_FUN( do_examine );
DECLARE_DO_FUN( do_exits );
DECLARE_DO_FUN( do_fill );
DECLARE_DO_FUN( do_flee );
DECLARE_DO_FUN( do_follow );
DECLARE_DO_FUN( do_force );
DECLARE_DO_FUN( do_freeze );
DECLARE_DO_FUN( do_gain );
DECLARE_DO_FUN( do_get );
DECLARE_DO_FUN( do_give );
DECLARE_DO_FUN( do_gossip );
#ifdef USE_GOCIAL
DECLARE_DO_FUN( do_gocial );
#endif
#ifdef USE_MUSIC
DECLARE_DO_FUN( do_music );
#endif
DECLARE_DO_FUN( do_goto );
DECLARE_DO_FUN( do_group );
DECLARE_DO_FUN( do_groups );
DECLARE_DO_FUN( do_gtell );
DECLARE_DO_FUN( do_heal );
DECLARE_DO_FUN( do_help );
DECLARE_DO_FUN( do_todo );
DECLARE_DO_FUN( do_hide );
DECLARE_DO_FUN( do_holylight );
DECLARE_DO_FUN( do_idea );
DECLARE_DO_FUN( do_immtalk );
#ifdef USE_ADMINTALK
DECLARE_DO_FUN( do_admintalk );
#endif
#ifdef USE_HEROTALK
DECLARE_DO_FUN( do_herotalk );
#endif
DECLARE_DO_FUN( do_imotd );
DECLARE_DO_FUN( do_inventory );
DECLARE_DO_FUN( do_invis );
DECLARE_DO_FUN( do_jail );
DECLARE_DO_FUN( do_junk );
DECLARE_DO_FUN( do_kick );
DECLARE_DO_FUN( do_shield );
DECLARE_DO_FUN( do_weapon );
DECLARE_DO_FUN( do_blackjack );
DECLARE_DO_FUN( do_kill );
DECLARE_DO_FUN( do_last );
DECLARE_DO_FUN( do_lastimm );
#ifdef USE_ADMINTALK
DECLARE_DO_FUN( do_lastadmin );
#endif
#ifdef USE_HEROTALK
DECLARE_DO_FUN( do_lasthero );
#endif
DECLARE_DO_FUN( do_levelgain );
DECLARE_DO_FUN( do_levels );
DECLARE_DO_FUN( do_list );
DECLARE_DO_FUN( do_load );
DECLARE_DO_FUN( do_lock );
DECLARE_DO_FUN( do_log );
DECLARE_DO_FUN( do_look );
DECLARE_DO_FUN( do_lore );
DECLARE_DO_FUN( do_memory );
DECLARE_DO_FUN( do_messages );
DECLARE_DO_FUN( do_tq );
DECLARE_DO_FUN( do_spousetalk );
DECLARE_DO_FUN( do_marry );
DECLARE_DO_FUN( do_mobchange );
DECLARE_DO_FUN( do_divorce );
DECLARE_DO_FUN( do_consent );
DECLARE_DO_FUN( do_mfind );
DECLARE_DO_FUN( do_mload );
DECLARE_DO_FUN( do_mset );
DECLARE_DO_FUN( do_mstat );
DECLARE_DO_FUN( do_mwhere );
DECLARE_DO_FUN( do_motd );
DECLARE_DO_FUN( do_murde );
DECLARE_DO_FUN( do_murder );
DECLARE_DO_FUN( do_newlock );
DECLARE_DO_FUN( do_news );
DECLARE_DO_FUN( do_nochannels );
DECLARE_DO_FUN( do_noemote );
DECLARE_DO_FUN( do_nofollow );
DECLARE_DO_FUN( do_noloot );
DECLARE_DO_FUN( do_north );
DECLARE_DO_FUN( do_noshout );
DECLARE_DO_FUN( do_nosummon );
DECLARE_DO_FUN( do_nocolor );
DECLARE_DO_FUN( do_note );
DECLARE_DO_FUN( do_notell );
DECLARE_DO_FUN( do_ofind );
DECLARE_DO_FUN( do_oload );
DECLARE_DO_FUN( do_ooc );
DECLARE_DO_FUN( do_open );
DECLARE_DO_FUN( do_order );
DECLARE_DO_FUN( do_oset );
DECLARE_DO_FUN( do_ostat );
DECLARE_DO_FUN( do_outfit );
DECLARE_DO_FUN( do_owhere );
DECLARE_DO_FUN( do_pardon );
DECLARE_DO_FUN( do_password );
DECLARE_DO_FUN( do_peace );
DECLARE_DO_FUN( do_pecho );
DECLARE_DO_FUN( do_permban );   /* Added by Lancelight */
DECLARE_DO_FUN( do_permit );    /* Permban and Permit. */
DECLARE_DO_FUN( do_pick );
DECLARE_DO_FUN( do_pk );
DECLARE_DO_FUN( do_pour );
DECLARE_DO_FUN( do_pose );
DECLARE_DO_FUN( do_practice );
DECLARE_DO_FUN( do_prompt );
DECLARE_DO_FUN( do_pload );
DECLARE_DO_FUN( do_punload );
DECLARE_DO_FUN( do_purge );
DECLARE_DO_FUN( do_put );
DECLARE_DO_FUN( do_quaff );
DECLARE_DO_FUN( do_question );
DECLARE_DO_FUN( do_qui );
DECLARE_DO_FUN( do_quiet );
DECLARE_DO_FUN( do_quit );
DECLARE_DO_FUN( do_read );
DECLARE_DO_FUN( do_reboo );
DECLARE_DO_FUN( do_reboot );
DECLARE_DO_FUN( do_recall );
DECLARE_DO_FUN( do_recho );
DECLARE_DO_FUN( do_recite );
DECLARE_DO_FUN( do_remove );
DECLARE_DO_FUN( do_repeat );
DECLARE_DO_FUN( do_reply );
#ifdef USE_REBIRTH
DECLARE_DO_FUN( do_rebirt );
DECLARE_DO_FUN( do_rebirth );
#endif
#ifdef USE_REMORT
DECLARE_DO_FUN( do_remor );
DECLARE_DO_FUN( do_remort );
#endif
DECLARE_DO_FUN( do_repop );
DECLARE_DO_FUN( do_report );
DECLARE_DO_FUN( do_rescue );
DECLARE_DO_FUN( do_rest );
DECLARE_DO_FUN( do_restore );
DECLARE_DO_FUN( do_return );
DECLARE_DO_FUN( do_rset );
DECLARE_DO_FUN( do_rstat );
DECLARE_DO_FUN( do_rules );
DECLARE_DO_FUN( do_sacrifice );
DECLARE_DO_FUN( do_save );
DECLARE_DO_FUN( do_say );
DECLARE_DO_FUN( do_scan );
DECLARE_DO_FUN( do_score );
DECLARE_DO_FUN( do_scroll );
DECLARE_DO_FUN( do_sell );
DECLARE_DO_FUN( do_sendinfo );
DECLARE_DO_FUN( do_set );
DECLARE_DO_FUN( do_shell );
DECLARE_DO_FUN( do_shout );
DECLARE_DO_FUN( do_shutdow );
DECLARE_DO_FUN( do_shutdown );
DECLARE_DO_FUN( do_sit );
DECLARE_DO_FUN( do_skill );
DECLARE_DO_FUN( do_skills );
DECLARE_DO_FUN( do_sla );
DECLARE_DO_FUN( do_slay );
DECLARE_DO_FUN( do_spellup );
DECLARE_DO_FUN( do_mortslay );
DECLARE_DO_FUN( do_sleep );
DECLARE_DO_FUN( do_slookup );
DECLARE_DO_FUN( do_sneak );
DECLARE_DO_FUN( do_snoop );
DECLARE_DO_FUN( do_socialfind );
DECLARE_DO_FUN( do_south );
DECLARE_DO_FUN( do_sockets );
DECLARE_DO_FUN( do_spells );
DECLARE_DO_FUN( do_sklist );
DECLARE_DO_FUN( do_splist );
DECLARE_DO_FUN( do_split );
DECLARE_DO_FUN( do_sset );
DECLARE_DO_FUN( do_stand );
DECLARE_DO_FUN( do_stat );
DECLARE_DO_FUN( do_steal );
DECLARE_DO_FUN( do_story );
DECLARE_DO_FUN( do_string );
DECLARE_DO_FUN( do_switch );
DECLARE_DO_FUN( do_tell );
DECLARE_DO_FUN( do_telloff );
DECLARE_DO_FUN( do_beep );
DECLARE_DO_FUN( do_time );
DECLARE_DO_FUN( do_title );
DECLARE_DO_FUN( do_train );
DECLARE_DO_FUN( do_transfer );
DECLARE_DO_FUN( do_trip );
DECLARE_DO_FUN( do_trust );
DECLARE_DO_FUN( do_typo );
DECLARE_DO_FUN( do_unlock );
DECLARE_DO_FUN( do_up );
DECLARE_DO_FUN( do_value );
DECLARE_DO_FUN( do_version );
DECLARE_DO_FUN( do_visible );
DECLARE_DO_FUN( do_vnum );
DECLARE_DO_FUN( do_wake );
DECLARE_DO_FUN( do_wear );
DECLARE_DO_FUN( do_weather );
DECLARE_DO_FUN( do_west );
DECLARE_DO_FUN( do_where );
DECLARE_DO_FUN( do_who );
DECLARE_DO_FUN( do_whoname );
DECLARE_DO_FUN( do_wimpy );
DECLARE_DO_FUN( do_wizhelp );
DECLARE_DO_FUN( do_wizgrant );
DECLARE_DO_FUN( do_wizrevoke );
DECLARE_DO_FUN( do_wizlock );
DECLARE_DO_FUN( do_wizlist );
DECLARE_DO_FUN( do_worth );
DECLARE_DO_FUN( do_yell );
DECLARE_DO_FUN( do_zap );
DECLARE_DO_FUN( do_info );
DECLARE_DO_FUN( do_sinfo );
DECLARE_DO_FUN( do_search );
DECLARE_DO_FUN( do_beacon );
DECLARE_DO_FUN( do_beaconreset );
DECLARE_DO_FUN( do_mpasound );
DECLARE_DO_FUN( do_mpat );
DECLARE_DO_FUN( do_mpclean );
DECLARE_DO_FUN( do_mpeatcorpse );
DECLARE_DO_FUN( do_mpecho );
DECLARE_DO_FUN( do_mpinvis );
DECLARE_DO_FUN( do_mpechoaround );
DECLARE_DO_FUN( do_mpechoat );
DECLARE_DO_FUN( do_mpfollowpath );
DECLARE_DO_FUN( do_mpforce );
DECLARE_DO_FUN( do_mpgoto );
DECLARE_DO_FUN( do_mpjunk );
DECLARE_DO_FUN( do_mpkill );
DECLARE_DO_FUN( do_mpmload );
DECLARE_DO_FUN( do_mpoload );
DECLARE_DO_FUN( do_mppurge );
DECLARE_DO_FUN( do_mpstat );
DECLARE_DO_FUN( do_mptransfer );
DECLARE_DO_FUN( do_mpremember );
DECLARE_DO_FUN( do_mpforget );
DECLARE_DO_FUN( do_mprandomsocial );
DECLARE_DO_FUN( do_mpsilentforce );
DECLARE_DO_FUN( do_mpsilentcast );
DECLARE_DO_FUN( do_mpdosilent );
DECLARE_DO_FUN( do_mpdefault );
DECLARE_DO_FUN( do_chaos );
DECLARE_DO_FUN( do_cwho );
DECLARE_DO_FUN( do_alias );
DECLARE_DO_FUN( do_unalias );
DECLARE_DO_FUN( do_brew );
DECLARE_DO_FUN( do_scribe );
DECLARE_DO_FUN( do_finger );
DECLARE_DO_FUN( do_rlist );
DECLARE_DO_FUN( do_new_discon );
DECLARE_DO_FUN( do_track );
/* All those clan commands! */
DECLARE_DO_FUN( do_promote );
DECLARE_DO_FUN( do_show );
DECLARE_DO_FUN( do_clan );
DECLARE_DO_FUN( do_join );
DECLARE_DO_FUN( do_petition );
DECLARE_DO_FUN( do_decline );
DECLARE_DO_FUN( do_offer );
DECLARE_DO_FUN( do_accept );
DECLARE_DO_FUN( do_clantalk );
DECLARE_DO_FUN( do_resign );
DECLARE_DO_FUN( do_crecall );
DECLARE_DO_FUN( do_demote );

DECLARE_DO_FUN( do_objcheck );
DECLARE_DO_FUN( do_cdeposit );
DECLARE_DO_FUN( do_roster );
DECLARE_DO_FUN( do_olevel );
DECLARE_DO_FUN( do_mlevel );

#endif
