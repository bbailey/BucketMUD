/* this is a listing of all the commands and command related data */
#ifndef _INTERP_H_
#define _INTERP_H_
#include "olc.h"
/* for command types */
#define ML      MAX_LEVEL	/* implementor */
#define L1      MAX_LEVEL - 1	/* creator */
#define L2      MAX_LEVEL - 2	/* supreme being */
#define L3      MAX_LEVEL - 3	/* deity */
#define L4      MAX_LEVEL - 4	/* god */
#define L5      MAX_LEVEL - 5	/* immortal */
#define L6      MAX_LEVEL - 6	/* demigod */
#define L7      MAX_LEVEL - 7	/* angel */
#define L8      MAX_LEVEL - 8	/* avatar */
#define IM      LEVEL_IMMORTAL	/* angel */
#define HE      LEVEL_HERO	/* hero */

/*
 * Structure for a command in the command lookup table.
 */
struct cmd_type
{
    char *const name;
    DO_FUN *do_fun;
    sh_int position;
    bool imm;
    sh_int log;
    bool show;
};

struct immcmd_type
{
    char *cmd;
    IMMCMD_TYPE *next;
};

/* the command table itself */
extern const struct cmd_type cmd_table[];

extern bool can_do_immcmd(CHAR_DATA * ch, char *cmd);
extern bool is_immcmd(char *command);

/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
extern void do_advance(CHAR_DATA * ch, char *argument);
extern void do_aexits(CHAR_DATA * ch, char *argument);
extern void do_aentrances(CHAR_DATA * ch, char *argument);
extern void do_afk(CHAR_DATA * ch, char *argument);
extern void do_alias(CHAR_DATA * ch, char *argument);
extern void do_allow(CHAR_DATA * ch, char *argument);
extern void do_answer(CHAR_DATA * ch, char *argument);
extern void do_areas(CHAR_DATA * ch, char *argument);
extern void do_at(CHAR_DATA * ch, char *argument);
extern void do_auction(CHAR_DATA * ch, char *argument);
extern void do_autoassist(CHAR_DATA * ch, char *argument);
extern void do_autoexit(CHAR_DATA * ch, char *argument);
extern void do_autogold(CHAR_DATA * ch, char *argument);
extern void do_autolist(CHAR_DATA * ch, char *argument);
extern void do_autoloot(CHAR_DATA * ch, char *argument);
extern void do_autosac(CHAR_DATA * ch, char *argument);
extern void do_autosplit(CHAR_DATA * ch, char *argument);
extern void do_award(CHAR_DATA * ch, char *argument);
extern void do_backstab(CHAR_DATA * ch, char *argument);
extern void do_bamfin(CHAR_DATA * ch, char *argument);
extern void do_bamfout(CHAR_DATA * ch, char *argument);
extern void do_ban(CHAR_DATA * ch, char *argument);	/* Does not use the old ban functions. -Lancelight */
extern void do_bash(CHAR_DATA * ch, char *argument);
extern void do_berserk(CHAR_DATA * ch, char *argument);
extern void do_brandish(CHAR_DATA * ch, char *argument);
extern void do_brief(CHAR_DATA * ch, char *argument);
extern void do_bug(CHAR_DATA * ch, char *argument);
extern void do_buy(CHAR_DATA * ch, char *argument);
extern void do_cast(CHAR_DATA * ch, char *argument);
extern void do_changes(CHAR_DATA * ch, char *argument);
extern void do_channels(CHAR_DATA * ch, char *argument);
extern void do_circle(CHAR_DATA * ch, char *argument);
extern void do_clone(CHAR_DATA * ch, char *argument);
extern void do_close(CHAR_DATA * ch, char *argument);
extern void do_comment(CHAR_DATA * ch, char *argument);
extern void do_combine(CHAR_DATA * ch, char *argument);
extern void do_compact(CHAR_DATA * ch, char *argument);
extern void do_compare(CHAR_DATA * ch, char *argument);
extern void do_consider(CHAR_DATA * ch, char *argument);
extern void do_hotboo(CHAR_DATA * ch, char *argument);
extern void do_copyover(CHAR_DATA * ch, char *argument);
extern void do_credits(CHAR_DATA * ch, char *argument);
extern void do_bank(CHAR_DATA * ch, char *argument);
extern void do_deposit(CHAR_DATA * ch, char *argument);
extern void do_withdraw(CHAR_DATA * ch, char *argument);
extern void do_account(CHAR_DATA * ch, char *argument);
extern void do_tick(CHAR_DATA * ch, char *argument);
extern void do_deaf(CHAR_DATA * ch, char *argument);
extern void do_delet(CHAR_DATA * ch, char *argument);
extern void do_delete(CHAR_DATA * ch, char *argument);
extern void do_deny(CHAR_DATA * ch, char *argument);
extern void do_description(CHAR_DATA * ch, char *argument);
extern void do_dirt(CHAR_DATA * ch, char *argument);
extern void do_disarm(CHAR_DATA * ch, char *argument);
extern void do_disconnect(CHAR_DATA * ch, char *argument);
extern void do_donate(CHAR_DATA * ch, char *argument);
extern void do_down(CHAR_DATA * ch, char *argument);
extern void do_drink(CHAR_DATA * ch, char *argument);
extern void do_drop(CHAR_DATA * ch, char *argument);
extern void do_dump(CHAR_DATA * ch, char *argument);
extern void do_east(CHAR_DATA * ch, char *argument);
extern void do_eat(CHAR_DATA * ch, char *argument);
extern void do_echo(CHAR_DATA * ch, char *argument);
extern void do_effects(CHAR_DATA * ch, char *argument);
extern void do_email(CHAR_DATA * ch, char *argument);
extern void do_emote(CHAR_DATA * ch, char *argument);
extern void do_enter(CHAR_DATA * ch, char *argument);	/*Enter a portal */
extern void do_equipment(CHAR_DATA * ch, char *argument);
extern void do_examine(CHAR_DATA * ch, char *argument);
extern void do_exits(CHAR_DATA * ch, char *argument);
extern void do_fill(CHAR_DATA * ch, char *argument);
extern void do_flee(CHAR_DATA * ch, char *argument);
extern void do_follow(CHAR_DATA * ch, char *argument);
extern void do_force(CHAR_DATA * ch, char *argument);
extern void do_freeze(CHAR_DATA * ch, char *argument);
extern void do_gain(CHAR_DATA * ch, char *argument);
extern void do_get(CHAR_DATA * ch, char *argument);
extern void do_give(CHAR_DATA * ch, char *argument);
extern void do_gossip(CHAR_DATA * ch, char *argument);
#ifdef USE_GOCIAL
extern void do_gocial(CHAR_DATA * ch, char *argument);
#endif
#ifdef USE_MUSIC
extern void do_music(CHAR_DATA * ch, char *argument);
#endif
extern void do_goto(CHAR_DATA * ch, char *argument);
extern void do_group(CHAR_DATA * ch, char *argument);
extern void do_groups(CHAR_DATA * ch, char *argument);
extern void do_gtell(CHAR_DATA * ch, char *argument);
extern void do_heal(CHAR_DATA * ch, char *argument);
extern void do_help(CHAR_DATA * ch, char *argument);
extern void do_hide(CHAR_DATA * ch, char *argument);
extern void do_holylight(CHAR_DATA * ch, char *argument);
extern void do_idea(CHAR_DATA * ch, char *argument);
extern void do_immtalk(CHAR_DATA * ch, char *argument);
#ifdef USE_ADMINTALK
extern void do_admintalk(CHAR_DATA * ch, char *argument);
#endif
#ifdef USE_HEROTALK
extern void do_herotalk(CHAR_DATA * ch, char *argument);
#endif
extern void do_imotd(CHAR_DATA * ch, char *argument);
extern void do_inventory(CHAR_DATA * ch, char *argument);
extern void do_invis(CHAR_DATA * ch, char *argument);
extern void do_jail(CHAR_DATA * ch, char *argument);
extern void do_junk(CHAR_DATA * ch, char *argument);
extern void do_kick(CHAR_DATA * ch, char *argument);
extern void do_shield(CHAR_DATA * ch, char *argument);
extern void do_weapon(CHAR_DATA * ch, char *argument);
extern void do_blackjack(CHAR_DATA * ch, char *argument);
extern void do_kill(CHAR_DATA * ch, char *argument);
extern void do_last(CHAR_DATA * ch, char *argument);
extern void do_lastimm(CHAR_DATA * ch, char *argument);
#ifdef USE_ADMINTALK
extern void do_lastadmin(CHAR_DATA * ch, char *argument);
#endif
#ifdef USE_HEROTALK
extern void do_lasthero(CHAR_DATA * ch, char *argument);
#endif
extern void do_levelgain(CHAR_DATA * ch, char *argument);
extern void do_levels(CHAR_DATA * ch, char *argument);
extern void do_list(CHAR_DATA * ch, char *argument);
extern void do_load(CHAR_DATA * ch, char *argument);
extern void do_lock(CHAR_DATA * ch, char *argument);
extern void do_log(CHAR_DATA * ch, char *argument);
extern void do_look(CHAR_DATA * ch, char *argument);
extern void do_lore(CHAR_DATA * ch, char *argument);
extern void do_memory(CHAR_DATA * ch, char *argument);
extern void do_messages(CHAR_DATA * ch, char *argument);
extern void do_tq(CHAR_DATA * ch, char *argument);
extern void do_spousetalk(CHAR_DATA * ch, char *argument);
extern void do_marry(CHAR_DATA * ch, char *argument);
extern void do_mobchange(CHAR_DATA * ch, char *argument);
extern void do_divorce(CHAR_DATA * ch, char *argument);
extern void do_consent(CHAR_DATA * ch, char *argument);
extern void do_mfind(CHAR_DATA * ch, char *argument);
extern void do_mload(CHAR_DATA * ch, char *argument);
extern void do_mset(CHAR_DATA * ch, char *argument);
extern void do_mstat(CHAR_DATA * ch, char *argument);
extern void do_mwhere(CHAR_DATA * ch, char *argument);
extern void do_motd(CHAR_DATA * ch, char *argument);
extern void do_murde(CHAR_DATA * ch, char *argument);
extern void do_murder(CHAR_DATA * ch, char *argument);
extern void do_newlock(CHAR_DATA * ch, char *argument);
extern void do_nochannels(CHAR_DATA * ch, char *argument);
extern void do_noemote(CHAR_DATA * ch, char *argument);
extern void do_nofollow(CHAR_DATA * ch, char *argument);
extern void do_noloot(CHAR_DATA * ch, char *argument);
extern void do_north(CHAR_DATA * ch, char *argument);
extern void do_noshout(CHAR_DATA * ch, char *argument);
extern void do_nosummon(CHAR_DATA * ch, char *argument);
extern void do_nocolor(CHAR_DATA * ch, char *argument);
extern void do_notell(CHAR_DATA * ch, char *argument);
extern void do_ofind(CHAR_DATA * ch, char *argument);
extern void do_oload(CHAR_DATA * ch, char *argument);
extern void do_ooc(CHAR_DATA * ch, char *argument);
extern void do_open(CHAR_DATA * ch, char *argument);
extern void do_order(CHAR_DATA * ch, char *argument);
extern void do_oset(CHAR_DATA * ch, char *argument);
extern void do_ostat(CHAR_DATA * ch, char *argument);
extern void do_outfit(CHAR_DATA * ch, char *argument);
extern void do_owhere(CHAR_DATA * ch, char *argument);
extern void do_pardon(CHAR_DATA * ch, char *argument);
extern void do_password(CHAR_DATA * ch, char *argument);
extern void do_peace(CHAR_DATA * ch, char *argument);
extern void do_pecho(CHAR_DATA * ch, char *argument);
extern void do_permban(CHAR_DATA * ch, char *argument);	/* Added by Lancelight */
extern void do_permit(CHAR_DATA * ch, char *argument);	/* Permban and Permit. */
extern void do_pick(CHAR_DATA * ch, char *argument);
extern void do_pk(CHAR_DATA * ch, char *argument);
extern void do_pour(CHAR_DATA * ch, char *argument);
extern void do_pose(CHAR_DATA * ch, char *argument);
extern void do_practice(CHAR_DATA * ch, char *argument);
extern void do_prompt(CHAR_DATA * ch, char *argument);
extern void do_pload(CHAR_DATA * ch, char *argument);
extern void do_punload(CHAR_DATA * ch, char *argument);
extern void do_purge(CHAR_DATA * ch, char *argument);
extern void do_put(CHAR_DATA * ch, char *argument);
extern void do_quaff(CHAR_DATA * ch, char *argument);
extern void do_question(CHAR_DATA * ch, char *argument);
extern void do_qui(CHAR_DATA * ch, char *argument);
extern void do_quiet(CHAR_DATA * ch, char *argument);
extern void do_quit(CHAR_DATA * ch, char *argument);
extern void do_read(CHAR_DATA * ch, char *argument);
extern void do_reboo(CHAR_DATA * ch, char *argument);
extern void do_reboot(CHAR_DATA * ch, char *argument);
extern void do_recall(CHAR_DATA * ch, char *argument);
extern void do_recho(CHAR_DATA * ch, char *argument);
extern void do_recite(CHAR_DATA * ch, char *argument);
extern void do_remove(CHAR_DATA * ch, char *argument);
extern void do_repeat(CHAR_DATA * ch, char *argument);
extern void do_reply(CHAR_DATA * ch, char *argument);
#ifdef USE_REBIRTH
extern void do_rebirt(CHAR_DATA * ch, char *argument);
extern void do_rebirth(CHAR_DATA * ch, char *argument);
#endif
#ifdef USE_REMORT
extern void do_remor(CHAR_DATA * ch, char *argument);
extern void do_remort(CHAR_DATA * ch, char *argument);
#endif
extern void do_repop(CHAR_DATA * ch, char *argument);
extern void do_report(CHAR_DATA * ch, char *argument);
extern void do_rescue(CHAR_DATA * ch, char *argument);
extern void do_rest(CHAR_DATA * ch, char *argument);
extern void do_restore(CHAR_DATA * ch, char *argument);
extern void do_return(CHAR_DATA * ch, char *argument);
extern void do_rset(CHAR_DATA * ch, char *argument);
extern void do_rstat(CHAR_DATA * ch, char *argument);
extern void do_rules(CHAR_DATA * ch, char *argument);
extern void do_sacrifice(CHAR_DATA * ch, char *argument);
extern void do_save(CHAR_DATA * ch, char *argument);
extern void do_say(CHAR_DATA * ch, char *argument);
extern void do_scan(CHAR_DATA * ch, char *argument);
extern void do_score(CHAR_DATA * ch, char *argument);
extern void do_scroll(CHAR_DATA * ch, char *argument);
extern void do_sell(CHAR_DATA * ch, char *argument);
extern void do_sendinfo(CHAR_DATA * ch, char *argument);
extern void do_set(CHAR_DATA * ch, char *argument);
extern void do_shout(CHAR_DATA * ch, char *argument);
extern void do_shutdow(CHAR_DATA * ch, char *argument);
extern void do_shutdown(CHAR_DATA * ch, char *argument);
extern void do_sit(CHAR_DATA * ch, char *argument);
extern void do_skill(CHAR_DATA * ch, char *argument);
extern void do_skills(CHAR_DATA * ch, char *argument);
extern void do_sla(CHAR_DATA * ch, char *argument);
extern void do_slay(CHAR_DATA * ch, char *argument);
extern void do_spellup(CHAR_DATA * ch, char *argument);
extern void do_mortslay(CHAR_DATA * ch, char *argument);
extern void do_sleep(CHAR_DATA * ch, char *argument);
extern void do_slookup(CHAR_DATA * ch, char *argument);
extern void do_sneak(CHAR_DATA * ch, char *argument);
extern void do_snoop(CHAR_DATA * ch, char *argument);
extern void do_south(CHAR_DATA * ch, char *argument);
extern void do_sockets(CHAR_DATA * ch, char *argument);
extern void do_spells(CHAR_DATA * ch, char *argument);
extern void do_sklist(CHAR_DATA * ch, char *argument);
extern void do_splist(CHAR_DATA * ch, char *argument);
extern void do_split(CHAR_DATA * ch, char *argument);
extern void do_sset(CHAR_DATA * ch, char *argument);
extern void do_stand(CHAR_DATA * ch, char *argument);
extern void do_stat(CHAR_DATA * ch, char *argument);
extern void do_steal(CHAR_DATA * ch, char *argument);
extern void do_story(CHAR_DATA * ch, char *argument);
extern void do_string(CHAR_DATA * ch, char *argument);
extern void do_switch(CHAR_DATA * ch, char *argument);
extern void do_tell(CHAR_DATA * ch, char *argument);
extern void do_telloff(CHAR_DATA * ch, char *argument);
extern void do_beep(CHAR_DATA * ch, char *argument);
extern void do_time(CHAR_DATA * ch, char *argument);
extern void do_title(CHAR_DATA * ch, char *argument);
extern void do_train(CHAR_DATA * ch, char *argument);
extern void do_transfer(CHAR_DATA * ch, char *argument);
extern void do_trip(CHAR_DATA * ch, char *argument);
extern void do_trust(CHAR_DATA * ch, char *argument);
extern void do_typo(CHAR_DATA * ch, char *argument);
extern void do_unlock(CHAR_DATA * ch, char *argument);
extern void do_up(CHAR_DATA * ch, char *argument);
extern void do_value(CHAR_DATA * ch, char *argument);
extern void do_version(CHAR_DATA * ch, char *argument);
extern void do_visible(CHAR_DATA * ch, char *argument);
extern void do_vnum(CHAR_DATA * ch, char *argument);
extern void do_wake(CHAR_DATA * ch, char *argument);
extern void do_wear(CHAR_DATA * ch, char *argument);
extern void do_weather(CHAR_DATA * ch, char *argument);
extern void do_west(CHAR_DATA * ch, char *argument);
extern void do_where(CHAR_DATA * ch, char *argument);
extern void do_who(CHAR_DATA * ch, char *argument);
extern void do_whoname(CHAR_DATA * ch, char *argument);
extern void do_wimpy(CHAR_DATA * ch, char *argument);
extern void do_wizgrant(CHAR_DATA * ch, char *argument);
extern void do_wizrevoke(CHAR_DATA * ch, char *argument);
extern void do_wizlock(CHAR_DATA * ch, char *argument);
extern void do_wizlist(CHAR_DATA * ch, char *argument);
extern void do_worth(CHAR_DATA * ch, char *argument);
extern void do_yell(CHAR_DATA * ch, char *argument);
extern void do_zap(CHAR_DATA * ch, char *argument);
extern void do_info(CHAR_DATA * ch, char *argument);
extern void do_sinfo(CHAR_DATA * ch, char *argument);
extern void do_search(CHAR_DATA * ch, char *argument);
extern void do_beacon(CHAR_DATA * ch, char *argument);
extern void do_beaconreset(CHAR_DATA * ch, char *argument);
extern void do_mpasound(CHAR_DATA * ch, char *argument);
extern void do_mpat(CHAR_DATA * ch, char *argument);
extern void do_mpclean(CHAR_DATA * ch, char *argument);
extern void do_mpeatcorpse(CHAR_DATA * ch, char *argument);
extern void do_mpecho(CHAR_DATA * ch, char *argument);
extern void do_mpinvis(CHAR_DATA * ch, char *argument);
extern void do_mpechoaround(CHAR_DATA * ch, char *argument);
extern void do_mpechoat(CHAR_DATA * ch, char *argument);
extern void do_mpfind(CHAR_DATA * ch, char *argument);
extern void do_mpfollowpath(CHAR_DATA * ch, char *argument);
extern void do_mpforce(CHAR_DATA * ch, char *argument);
extern void do_mpgoto(CHAR_DATA * ch, char *argument);
extern void do_mpjunk(CHAR_DATA * ch, char *argument);
extern void do_mpkill(CHAR_DATA * ch, char *argument);
extern void do_mpmload(CHAR_DATA * ch, char *argument);
extern void do_mpoload(CHAR_DATA * ch, char *argument);
extern void do_mppurge(CHAR_DATA * ch, char *argument);
extern void do_mpstat(CHAR_DATA * ch, char *argument);
extern void do_mptransfer(CHAR_DATA * ch, char *argument);
extern void do_mpremember(CHAR_DATA * ch, char *argument);
extern void do_mpforget(CHAR_DATA * ch, char *argument);
extern void do_mprandomsocial(CHAR_DATA * ch, char *argument);
extern void do_mpsilentforce(CHAR_DATA * ch, char *argument);
extern void do_mpsilentcast(CHAR_DATA * ch, char *argument);
extern void do_mpdosilent(CHAR_DATA * ch, char *argument);
extern void do_mpdefault(CHAR_DATA * ch, char *argument);
extern void do_chaos(CHAR_DATA * ch, char *argument);
extern void do_cwho(CHAR_DATA * ch, char *argument);
extern void do_unalias(CHAR_DATA * ch, char *argument);
extern void do_brew(CHAR_DATA * ch, char *argument);
extern void do_scribe(CHAR_DATA * ch, char *argument);
extern void do_finger(CHAR_DATA * ch, char *argument);
extern void do_rlist(CHAR_DATA * ch, char *argument);
extern void do_new_discon(CHAR_DATA * ch, char *argument);
extern void do_track(CHAR_DATA * ch, char *argument);
/* All those clan commands! */
extern void do_promote(CHAR_DATA * ch, char *argument);
extern void do_show(CHAR_DATA * ch, char *argument);
extern void do_clan(CHAR_DATA * ch, char *argument);
extern void do_join(CHAR_DATA * ch, char *argument);
extern void do_petition(CHAR_DATA * ch, char *argument);
extern void do_decline(CHAR_DATA * ch, char *argument);
extern void do_offer(CHAR_DATA * ch, char *argument);
extern void do_accept(CHAR_DATA * ch, char *argument);
extern void do_clantalk(CHAR_DATA * ch, char *argument);
extern void do_resign(CHAR_DATA * ch, char *argument);
extern void do_crecall(CHAR_DATA * ch, char *argument);
extern void do_demote(CHAR_DATA * ch, char *argument);

extern void do_objcheck(CHAR_DATA * ch, char *argument);
extern void do_cdeposit(CHAR_DATA * ch, char *argument);
extern void do_roster(CHAR_DATA * ch, char *argument);
extern void do_olevel(CHAR_DATA * ch, char *argument);
extern void do_mlevel(CHAR_DATA * ch, char *argument);

#endif
