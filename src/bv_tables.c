#include <glib.h>

#include "BitVectorStr.h"
#include "bv_tables.h"

const BitVectorStringList bv_str_list_comm[] = {
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