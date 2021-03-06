#ifndef BUCKETMUD_BV_TABLES_H
#define BUCKETMUD_BV_TABLES_H

#include "BitVectorStr.h"

typedef enum {
    BV_COMM_QUIET = 0,
    BV_COMM_DEAF = 1,
    BV_COMM_NO_WIZ = 2,
    BV_COMM_NO_AUCTION = 3,
    BV_COMM_NO_GOSSIP = 4,
    BV_COMM_NO_QUESTION = 5,
    BV_COMM_NO_OOC = 6,
    BV_COMM_NO_INFO = 7,
    BV_COMM_NO_CLAN = 8,
    BV_COMM_SNOOP_CLAN = 9,
    BV_COMM_NO_MUSIC = 10,
    BV_COMM_COMPACT = 11,
    BV_COMM_BRIEF = 12,
    BV_COMM_PROMPT = 13,
    BV_COMM_COMBINE = 14,
    BV_COMM_TELNET_GA = 15,
    BV_COMM_NO_SPOUSETALK = 16,
    BV_COMM_NO_ADMIN = 17,
    BV_COMM_NO_HERO = 18,
    BV_COMM_NO_EMOTE = 19,
    BV_COMM_NO_SHOUT = 20,
    BV_COMM_NO_TELL = 21,
    BV_COMM_NO_CHANNELS = 22,
    BV_COMM_VIS_CONSOLE = 23,
    BV_COMM_NO_GOCIAL = 24,
    BV_COMM_TELLOFF = 25,
    BV_COMM_MAX
} BV_COMM_FLAGS;

extern const BitVectorStringList bv_str_list_comm[];

typedef enum {
    BV_OFF_AREA_ATTACK = 0,
    BV_OFF_BACKSTAB = 1,
    BV_OFF_BASH = 2,
    BV_OFF_BERSERK = 3,
    BV_OFF_DISARM = 4,
    BV_OFF_DODGE = 5,
    BV_OFF_FADE = 6,
    BV_OFF_FAST = 7,
    BV_OFF_KICK = 8,
    BV_OFF_KICK_DIRT = 9,
    BV_OFF_PARRY = 10,
    BV_OFF_RESCUE = 11,
    BV_OFF_TAIL = 12,
    BV_OFF_TRIP = 13,
    BV_OFF_CRUSH = 14,
    BV_OFF_ASSIST_ALL = 15,
    BV_OFF_ASSIST_ALIGN = 16,
    BV_OFF_ASSIST_RACE = 17,
    BV_OFF_ASSIST_PLAYERS = 18,
    BV_OFF_ASSIST_GUARD = 19,
    BV_OFF_ASSIST_VNUM = 20,
    BV_OFF_MAX
} BV_OFF_FLAGS;

extern const BitVectorStringList bv_str_list_off[];

typedef enum {
    BV_VULN_MAGIC = 2,
    BV_VULN_WEAPON = 3,
    BV_VULN_BASH = 4,
    BV_VULN_PIERCE = 5,
    BV_VULN_SLASH = 6,
    BV_VULN_FIRE = 7,
    BV_VULN_COLD = 8,
    BV_VULN_LIGHTNING = 9,
    BV_VULN_ACID = 10,
    BV_VULN_POISON = 11,
    BV_VULN_NEGATIVE = 12,
    BV_VULN_HOLY = 13,
    BV_VULN_ENERGY = 14,
    BV_VULN_MENTAL = 15,
    BV_VULN_DISEASE = 16,
    BV_VULN_DROWNING = 17,
    BV_VULN_LIGHT = 18,
    BV_VULN_WOOD = 23,
    BV_VULN_SILVER = 24,
    BV_VULN_IRON = 25,
    BV_VULN_MAX
} BV_VULN_FLAGS;

extern const BitVectorStringList bv_str_list_vuln[];

#endif // BUCKETMUD_BV_TABLES_H

