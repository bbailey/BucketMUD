#include <glib.h>

#include "BitVectorStr.h"
#include "bv_tables.h"

const BitVectorStringList bv_str_list_comm[] =
{
    { "quiet", BV_COMM_QUIET },
    { "deaf", BV_COMM_DEAF },
    { "no_wiz", BV_COMM_NO_WIZ },
    { "no_auction", BV_COMM_NO_AUCTION },
    { "no_gossip", BV_COMM_NO_GOSSIP },
    { "no_question", BV_COMM_NO_QUESTION },
    { "no_ooc", BV_COMM_NO_OOC },
    { "no_info", BV_COMM_NO_INFO },
    { "no_clan", BV_COMM_NO_CLAN },
    { "snoop_clan", BV_COMM_SNOOP_CLAN },
    { "no_music", BV_COMM_NO_MUSIC },
    { "compact", BV_COMM_COMPACT },
    { "brief", BV_COMM_BRIEF },
    { "prompt", BV_COMM_PROMPT },
    { "combine", BV_COMM_COMBINE },
    { "telnet_ga", BV_COMM_TELNET_GA },
    { "no_spousetalk", BV_COMM_NO_SPOUSETALK },
    { "no_admin", BV_COMM_NO_ADMIN },
    { "no_hero", BV_COMM_NO_HERO },
    { "no_emote", BV_COMM_NO_EMOTE },
    { "no_shout", BV_COMM_NO_SHOUT },
    { "no_tell", BV_COMM_NO_TELL },
    { "no_channels", BV_COMM_NO_CHANNELS },
    { "vis_console", BV_COMM_VIS_CONSOLE },
    { "no_gocial", BV_COMM_NO_GOCIAL },
    { "telloff", BV_COMM_TELLOFF },
    { NULL, BV_COMM_MAX }
};

const BitVectorStringList bv_str_list_off[] =
{
    { "area_attack", BV_OFF_AREA_ATTACK },
    { "backstab", BV_OFF_BACKSTAB },
    { "bash", BV_OFF_BASH },
    { "berserk", BV_OFF_BERSERK },
    { "disarm", BV_OFF_DISARM },
    { "dodge", BV_OFF_DODGE },
    { "fade", BV_OFF_FADE },
    { "fast", BV_OFF_FAST },
    { "kick", BV_OFF_KICK },
    { "kick_dirt", BV_OFF_KICK },
    { "parry", BV_OFF_PARRY },
    { "rescue", BV_OFF_RESCUE },
    { "tail", BV_OFF_TAIL },
    { "trip", BV_OFF_TRIP },
    { "crush", BV_OFF_CRUSH },
    { "assist_all", BV_OFF_ASSIST_ALL },
    { "assist_align", BV_OFF_ASSIST_ALIGN },
    { "assist_race", BV_OFF_ASSIST_RACE },
    { "assist_players", BV_OFF_ASSIST_PLAYERS },
    { "assist_guard", BV_OFF_ASSIST_GUARD },
    { "assist_vnum", BV_OFF_ASSIST_VNUM},
    { NULL, BV_OFF_MAX }
};

const BitVectorStringList bv_str_list_vuln[] = {
    { "magic", BV_VULN_MAGIC },
    { "weapon", BV_VULN_WEAPON },
    { "bash", BV_VULN_BASH },
    { "pierce", BV_VULN_PIERCE },
    { "slash", BV_VULN_SLASH },
    { "fire", BV_VULN_FIRE },
    { "cold", BV_VULN_COLD },
    { "lightning", BV_VULN_LIGHTNING },
    { "acid", BV_VULN_ACID },
    { "poison", BV_VULN_POISON },
    { "negative", BV_VULN_NEGATIVE },
    { "holy", BV_VULN_HOLY },
    { "energy", BV_VULN_ENERGY },
    { "mental", BV_VULN_MENTAL },
    { "disease", BV_VULN_DISEASE },
    { "drowning", BV_VULN_DROWNING },
    { "light", BV_VULN_LIGHT },
    { "wood", BV_VULN_WOOD },
    { "silver", BV_VULN_SILVER },
    { "iron", BV_VULN_SILVER },
    { NULL, BV_VULN_MAX }
};
