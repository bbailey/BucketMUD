#ifndef _CONFIG_H_
#define _CONFIG_H_

#define CONFIG_FILE "ember.cfg"

/* These strings probably should NOT contain color codes */
#define CFG_QUIT "Alas, all good things must come to an end.\n\r"
#define CFG_CONNECT_MSG "Welcome to a MUD based on EmberMUD.\n\r"
#define CFG_ASK_ANSI "Use ANSI Color? [Y/n]: "

/* Do you want a music channel?  Can be used as any other
 * channel name by changing the info in the channel layout and
 * names.
 */
#define USE_MUSIC

/* Do you want a high level imm only channel calling admintalk
 * if so, remove the comments from the section below.  In
 * addition there is also gocials and herotalk.
 */

#define USE_ADMINTALK
#define USE_HEROTALK
#define USE_GOCIAL

/*
 * Channel config
 * $n = The speaker's name
 * $t = The text
 * Both variables MUST be present!
 */
#define CFG_OOC        "`K[`WOOC`K]`w $n`K: `c'`C$t`c'`0"
#define CFG_GOS        "`K[`WGOSSIP`K]`w $n`K: `b'`B$t`b'`0"
#define CFG_QUESTION   "`K[`WQUESTION`K]`w $n`K: `r'`R$t`r'`0"
#define CFG_ANSWER     "`K[`WANSWER`K]`w $n`K: `r'`R$t`r'`0"
#define CFG_IMM        "`K[`WIMM`K]`w $n`K: `w'`W$t`w'`0"
#define CFG_MUS        "`K[`WMUSIC`K]`w $n`K sings:  `m'`M$t`m'`0"
#define CFG_ADMIN      "`K[`WADMIN`K]`w $n`K: `y'`Y$t`y'`0"
#define CFG_HERO       "`K[`WHERO`K]`w $n`K: `y'`Y$t`y'`0"

/* Not really channels, but config'd the same way */
#define CFG_SAY        "`G$n says `g'`G$t`g'`0"
#define CFG_SAY_SELF   "`GYou say `g'`G$t`g'`0"
#define CFG_YELL       "`Y$n yells `y'`Y$t`y'`0"
#define CFG_YELL_SELF  "`YYou yell `y'`Y$t`y'`0"
#define CFG_SHOUT      "`W$n shouts `w'`W$t`w'`0"
#define CFG_SHOUT_SELF "`WYou shout `w'`W$t`w'`0"

/* Can't use $n for the info channel! */
#define CFG_INFO       "`WINFO: `R$t`0"

#define COLOR_AUC   DARK_MAGENTA
#define COLOR_TELL  RED

/*
 * Additionally, the names of four of the channels 
 * can be changed (be sure to change above too).
 */
#define CFG_OOC_NAME "OOC"
#define CFG_GOS_NAME "GOSSIP"
#define CFG_MUS_NAME "MUSIC"
#define CFG_GOC_NAME "GOCIAL"
/* 
 * Drunk.  Here is where you can define whether a channel is 
 * going to check for drunk status.  If it is defined here, the 
 * channel will check.  The only one off by default is tells.
 * Also you can turn on or off drunk movement.
 */

#define GOS_DRUNK
#define QA_DRUNK
#define OOC_DRUNK
#define SAY_DRUNK
#define SHOUT_DRUNK
#define YELL_DRUNK
#define EMOTE_DRUNK
#define GTELL_DRUNK
#define SPOUSE_DRUNK
/* #define TELL_DRUNK */
#define CLANTALK_DRUNK
#define DRUNK_MOVE

/* These values describe how the "Auction Info" command works. You can 
comment/un-comment just about anything and everything as far as 
showing/hiding stats goes for an item. They are pretty self explanatory. 
By default the auction info will list the "Short descr, Level, Type, Extra 
Keywords, and Weapon Types."
-Lancelight
*/

#define ALLOW_SHORT_DESCR
#define ALLOW_LEVEL
#define ALLOW_TYPE
/* #define ALLOW_WEAR_BITS */
/* #define ALLOW_EXTRA_BITS */
/* #define ALLOW_WEIGHT */
/* #define ALLOW_COST */
/* #define ALLOW_CONDITION */
/* #define ALLOW_TIMER */
/* #define ALLOW_AFFECTS */
/* #define ALLOW_ARMOR_VALUES */
#define ALLOW_EXTRA_DESCR_KEYWORDS

#define ALLOW_ITEM_TYPE

/* The following allows will only take affect if you have defined the
allow item type above!!!! */

/* #define ALLOW_SCROLL_POTION_PILL *//* This shows the level and spells 
   of a scroll/pill/potion */
/* #define ALLOW_WAND_STAFF *//* This shows all the stats of a wand/staff */
#define ALLOW_WEAPON            /*This must be on for the weapon allows to work */
#define ALLOW_WEAPON_TYPE
/* #define ALLOW_WEAPON_DAMAGE */
/* #define ALLOW_WEAPON_FLAGS */

/*
 * DISABLE_AUC_INFO
 * If this is defined, the "free" identify of any item that
 * is being auctioned is disabled.  This is a temp-fix for
 * the issue mentioned in SourceForge bug #478557 until I
 * can think of a better solution.
 * This has been replaced by the expanded auction options
 * above.
 */
/*
#define DISABLE_AUC_INFO
*/
/* $n  = Bidder's name (optional)
 * %ld = Bid amount (REQUIRED)
 * $p  = Item (optional)
 */
#define CFG_AUC_BID      "`K[`WAUCTION`K] `w$n has offered %ld gold for $p."
#define CFG_AUC_SELL     "`K[`WAUCTION`K] `w$p SOLD to $n for %ld gold."
#define CFG_AUC_ONCE     "`K[`WAUCTION`K] `w$p - going once to $n for %ld gold."
#define CFG_AUC_TWICE    "`K[`WAUCTION`K] `w$p - going twice to $n for %ld gold."

/* $n = Seller's name (optional)
 * $p = Item (optional)
 */
#define CFG_AUC_BEGIN    "`K[`WAUCTION`K] `w$n is offering $p for auction."
#define CFG_AUC_REMOVE   "`K[`WAUCTION`K] `wNo bids on $p - item removed."
#define CFG_AUC_TOWINNER "`K[`WAUCTION`K] `w$p appears in your hands!"

/* $p = Item (optional)
 */
#define CFG_AUC_RETURNED "$p has been returned to you."

/*
 * These settings determine the minimum position to use certain
 * channels, and to use tells.  Some people like to dampen all 
 * RP related communication while sleeping, other prefer to let
 * players talk while sleeping, these allow you to decide for
 * your individual MUD.  These are in the process of being
 * implemented.  Currently they are only defined, and not 
 * used.
 */

#define MIN_POS_OOC             POS_SLEEPING
#define MIN_POS_GOS             POS_SLEEPING
#define MIN_POS_MUS             POS_SLEEPING
#define MIN_POS_TELL            POS_SLEEPING
#define MIN_POS_QA              POS_SLEEPING
#define MIN_POS_YELL            POS_RESTING
#define MIN_POS_SHOUT           POS_RESTING
#define MIN_POS_AUCTION         POS_SLEEPING

/* The following settings have to do with how low cp can go.  I recently
had a player come on a create a 0 cp human warrior, and level like there
was no tomorrow.  Changing the exp tables would hurt everybody so I added
a line for if their cp is less than a certain number, their cp gets
another number added to it.  This defines those to numbers.  The next has
to do with how low can a "normal" character use trains to lower their
cp. */
#define CP_MIN_PENALTY  10
#define CP_PENALTY      35
#define CP_TRAIN_MIN    30

/* The next item has to do with auto_hatred.   Auto_hatred makes it so 
   that if you attack a mob, it will remember you forever (or until imm
   peace command is used in the same room with them).  If you don't want
   to use it, uncomment #undef AUTO_HATE, and comment out #def AUTO_HATE
*/
#define AUTO_HATE
/* #undef AUTO_HATE */

/* Recreation options:  This update to cvs (4/2002) brings in two possible
 * options for recreation after reaching hero.  It is recomended only one at
 * a time be used.  They are rebirth - you stay the same class, but get hp
 * bonuses, and keep your skills.  Remort - you recreate with additional
 * race/class options.  Uncomment the ones you want to use, if any.
 */

#define USE_REBIRTH

/*
#define USE_REMORT
*/

/* The next has to do with selling multiples of the same item, or when
   a shopkeeper already has at least 1 of something.  Set to 0 if you
   only want your shopkeepers buying 1 of an item. 
*/

#define MORE_COST_MULTIPLIER  .75

/* The following affect how successful mobs are at rescue, particularly
   when a pet tries to rescue its master.  Higher the number, the greater
   chance they will fail.  Don't make it to high, or it will do funny
   things.  10 is probably as high as you would want to go.  */

#define RESCUE_PENALTY 5

/* These settings determine how
your mud will re-act to the nosummon command when invoked by a player.

NOSUMMONLEVEL is the level that is used to determine if nosummon applies
to a specific player or not. Let's say that Player A is level 20 and
Player B is level 22 and has "nosummon" turned on. BUT Player B is a
player killer and you have a NOSUMMONLEVEL = 5. Since Player B is within 5
levels of the summoner, he will be summoned. Same goes for if the Level 22
tried to summon the level 20. 5 Levels above or below the summoner can be
summoned so long as the victim is a player killer and is in an area that
allows summoning.

NOSUMMONFULL determines whether or not to use the restriction rules
defined below, or to never allow summoning when someone has the nosummon
flag. 0 will use the restrictions, 1 will never allow summoning.

NOSUMMONPK works differently then the other 2 restrictions.

For example,
 If player A is PK or NON-PK, and player B is non-PK, the rule that is
used would be the NOSUMMONLEVELNONPK rule because player A could not kill
player B anyways. So, if player B had nosummon on, player A could not
gate/summon/portal to player B whatsoever. If player B did NOT have
nosummon on, then the NOSUMMONLEVELNONPK restriction will kick in. So if
player A is 5 levels below player B, they could not gate. If they were 4
levels under, it would be allowed.  This is an example of a NON-PK victim,
mobs work the same way as non-pk victims.

The difference kicks in, when the VICTIM is flagged as PK.

If player A is PK or NON-PK, and player B is PK, the rule that is used
would be the NOSUMMONLEVELPK rule because player B is pk. So if player B
has nosummon turned on, and player A's level - NOSUMMONLEVELPK is BELOW
player A's level, the summon/gate/portal would WORK. The reason it is just
the opposite of the Mobs or Non-Pk players is so that a level 100 PK
player does not gate/summon/portal to another PK player that is like 50
levels under him.

If you would like PK players summoning rules to be the same as mobs/nonpk
(based on victim->level >= NOSUMMONLEVELPK instead of victim->level <=
NOSUMMONLEVELPK) then define NOSUMMONPKSAME to 1)

 -Lancelight*/

#define NOSUMMONFULL  0
/* If nosummonfull = 0, then use these level restrictions. */

#define NOSUMMONLEVELMOB 5
#define NOSUMMONLEVELPK 5
#define NOSUMMONPKSAME 0
#define NOSUMMONLEVELNONPK 5    /* This is not the same as the above 2!!
                                   Read the above examples for reasons why */

/*  This has to do with a way to buy for free.  You buy a bunch of stuff
 *  from the shopkeeper then kill him.  You get your gold back and then
 *  some.  
 */

#define SHOPKEEPER_NOSTEAL

/* MAX_ATTAINABLE_STATS: This value allows you to tweak your PC/MOB Max
stats that they can have (Str, Con, Int, Dex, Wis). The default is set to
25, but the tables are also setup for a value of 30. If you wish to
have values HIGHER then 30, make sure you edit the const.c Stat tables to
reflect the higher values. Also remember to edit races that will be using
the higher values. Remeber only use 25 or 30 unless you edit
the tables in const.c or you will get errors all over const.c 
-Lancelight*/

#define MAX_ATTAINABLE_STATS 25

/* Another stat item.  This defines the penalty for re-rolling your stats
 * if you don't like the first roll.  This defines how close it is possible to 
 * role stats to the max stat in the pc race table.   When rolling stats it 
 * was being penalized by 5 which meant if your race could get a 22 in DEX, 
 * the highest you could roll during creation was 18.   With this you can 
 * adjust that number.  This was suggested by Zak on the mailing list.  This 
 * number should be positive because it is subtracted from the value that is rolled.
 *
 * Default is 0 or the penalty is OFF.
 
   - Dorzak 7/19/2001 */

#define REROLL_PENALTY   0

/* MAX_MORTAL_WEAPON_DAMAGE
 *
 * If a player does more than this amount of damage in a single hit
 * with a weapon then the damage is logged, reset to 0 and the
 * weapon is destroyed.  This is really to prevent builders and IMMs
 * from cheating by giving god weapons to players.
 */

#define MAX_MORTAL_WEAPON_DAMAGE    1000

/* ONLY_DAGGER_BS
 *
 * This is an option to allow only daggers to backstab
 * define if you want only daggers to backstab.
 */

#define ONLY_DAGGER_BS

/* MAX_COUNTER_PERCENTAGE
 * 
 * If the skill check on a counter-attack succeeds then we roll a 1d100
 * die and check against MAX_COUNTER_PERCENTAGE to see if we proceed with
 * the counter attack.  The default is 20 (or 20%), this means that if a
 * player has 100% counter then at MOST they'll counter-attack 20% of the
 * time.
 *
 * NEVER SET THIS TO 100!  For your own safety anything over 100 will be
 * set to 99 since 100 would cause the mud to go into an infinite loop if
 * two players with 100% counter skill were to fight each other.
 *
 * The max I'd recommend here is 50%,
 */

#define MAX_COUNTER_PERCENTAGE      20

   /* This is if you want to allow players to convert trains into practices
      the opposite of gain convert. */
/* #define CAN_GAIN_REVERT */
#undef CAN_GAIN_REVERT
#define CAN_GAIN_CONVERT
/* #undef CAN_GAIN_CONVERT */

/*
 * If using rebirth or remort the following when defined gives
 * a 10% bonus to the max possible hp/mana/move gains for each
 * incarnation
 */

#define BONUS_INCARNATIONS

/* If NO_RDNS is defined the mud won't try and look up the host name
from the IP address.  This prevents the reverse DNS lag that sometimes
happens when players connect. (Can lag the whole mud)  The downside
is you won't have hostnames in your log file and you won't be able
to ban players by hostname.  NO_RDNS is commented out by default. */

#define NO_RDNS

/* These 2 values control what level spells/items that brew and scribe
will produce. The default value is .40 (40% of chars level) the 1st one
sets the level of the obj, the second sets the level of the spells.*/

#define BREW_SCRIBE_USE_LEVEL 1
#define BREW_SCRIBE_LEVEL .75

/* Random dodge messages. - Nevarlos */
#define DDG_MSG1    "$N easily avoids your attack."
#define DDG_MSGS1   "You easily avoid $n's attack."
#define DDG_MSG2    "$N chuckles as $E evades your attack."
#define DDG_MSGS2   "You chuckle as you evade $n's attack."
#define DDG_MSG3    "$N nimbly jumps away from your attack."
#define DDG_MSGS3   "You nimbly jump away from $n's attack."
#define DDG_MSG4    "$N yawns and sidesteps your attack."
#define DDG_MSGS4   "You yawn and sidestep $n's attack."
#define DDG_MSG5    "$N smoothly ducks under your attack."
#define DDG_MSGS5   "You smoothly duck under $n's attack."

/* Random parry messages. - Nevarlos */

#define PRY_MSG1    "$N slaps away your attack."
#define PRY_MSGS1   "You slap away $n's attack."
#define PRY_MSG2    "$N easily redirects your attack away from $M."
#define PRY_MSGS2   "You easily redirect $n's attack away from you."
#define PRY_MSG3    "$N laughs as $E knocks away your attack."
#define PRY_MSGS3   "You laugh as you knock away $n's attack."
#define PRY_MSG4    "$N beats back your attack."
#define PRY_MSGS4   "You beat back $n's attack."
#define PRY_MSG5    "$N deftly parries your attack with $S weapon."
#define PRY_MSGS5   "You deftly parry $n's attack with your weapon."

/* Random shield block messages.  -Nevarlos */

#define BLK_MSG1    "$N easily stops your attack with $S shield."
#define BLK_MSGS1   "You easily stop $n's attack with your shield."
#define BLK_MSG2    "$N's shield interrupts your attack."
#define BLK_MSGS2   "Your shield interrupts $n's attack."
#define BLK_MSG3    "$N redirects your attack with $S shield."
#define BLK_MSGS3   "You redirect $n's attack with your shield."
#define BLK_MSG4    "$N slams $S shield in front of your attack."
#define BLK_MSGS4   "You slam your shield in front of $n's attack."
#define BLK_MSG5    "Your attack clashes against $N's shield."
#define BLK_MSGS5   "$n's attack clashes against your shield."

/*
 * Wear Locations - You can define a different look for your wear
 *                  locations here.
 */
#define WORN_LIGHT    "`K<`wused as light`K>"
#define WORN_FINGER   "`K<`wworn on finger`K>"
#define WORN_FINGER2  "`K<`wworn on finger`K>"
#define WORN_NECK     "`K<`wworn around neck`K>"
#define WORN_NECK2    "`K<`wworn around neck`K>"
#define WORN_BODY     "`K<`wworn on body`K>"
#define WORN_HEAD     "`K<`wworn on head`K>"
#define WORN_LEGS     "`K<`wworn on legs`K>"
#define WORN_FEET     "`K<`wworn on feet`K>"
#define WORN_HANDS    "`K<`wworn on hands`K>"
#define WORN_ARMS     "`K<`wworn on arms`K>"
#define WORN_SHIELD   "`K<`wworn as shield`K>"
#define WORN_BODY2    "`K<`wworn about body`K>"
#define WORN_WAIST    "`K<`wworn about waist`K>"
#define WORN_WRIST    "`K<`wworn around wrist`K>"
#define WORN_WRIST2   "`K<`wworn around wrist`K>"
#define WORN_WIELDED  "`K<`wwielded`K>"
#define WORN_HELD     "`K<`wheld`K>"
#define WORN_WIELDED2 "`K<`wsecondary weapon`K>"

/* Comment this if you do not want to have the anonymous command
available*/

#define ANONYMOUS

/* This determines at what level the recall command no longer works for
mortals. Set this to MAX_LEVEL if you dont want to use it. */
#define RECALL_LEVEL 20

/* this determines if beacon is a mortal or immortal command.  Some muds
have it one way, other another.  Beacon sets the recall room of the
person who types beacon.  It can not be used to set anothers beacon.  One
caveat is there is no reset portion to it, so if you set it to a mortal
command, realize that.  However word of recall spell and recall scrolls
are not affected by this. 1 for imm, 0 for mortal */

#define BEACON_WIZ  1

/* IMPORTANT!!!!!!!!!!!
   Do NOT Turn on OLC_AUTOSAVE unless you and your builders KNOW what your
doing. It is possible, however unlikly, that you could corrupt an area
file using this. Its extremly convient for your builders, but can be
disastrous if a new builder starts poking around where he shouldnt be and
types done while someone else is editing the area. Now that thats out of
the way here is what it does.

  Basically whenever you type done, and asave changed is run and thats
that. pretty simple eh? By default, this option is OFF. Change it to 1 to
turn it on.
     -Lancelight
*/

#define OLC_AUTOSAVE 0

/* The following defines will determine how the PK system for looting corpses
runs. -Lancelight*/

#define LOOTING_CHANCE 10       /* gives a 1 to 10 chance of getting an item from
                                   whoever the player killed. The higher the number,
                                   the less of a chance the killer has of getting an
                                   item from the poor victim. -Lancelight */
#define LOOTING_ALLOWED 1       /* Change this to 0 if you do not want to allow player
                                   looting at all.
                                   1 = Use random Looting
                                   2 = Allow full looting
                                   FULL corpse Looting. I do NOT recommend setting this
                                   unless you know that your players can handle it. */

/* The following defines will determine how the jail systems acts. The 1st
2 determine how the jail releases people. It can either release them to a
certain vnum, or to a players recall room. Currently it defaults to the
players recall room. But if you want it to go to a certian room, then
set JAIL_RELEASE_RECALL to 0, and give JAIL_RELEASE_VNUM the vnum u want
it to release to.

The next couple defines, determines what the jail command will do to the player,
like if they can wear items, whether it removes them, what channels they
can talk on and what commands they can do while in jail. To turn something
on, give it a 1. To turn it off, give it a 0.
 -Lancelight */

/* IMPORTANT NOTE!!!:
Remember, if you change ANYTHING here, you must do a complete re-compile.
dont compile just the affected object file or else you will find that the
changes will not work.
-Lancelight
*/
#define JAIL_RELEASE_VNUM 3001
#define JAIL_RELEASE_RECALL 1

#define JAIL_CELL_VNUM 3143     /* The actual jail room vnum */
#define JAIL_REMOVES_EQ 1       /*Removes eq from the char to their inv */
#define JAIL_CAN_WEAR 0         /*Can they wear things while in jail */
#define JAIL_NOCHANNEL 1
#define JAIL_NOSHOUT 1
#define JAIL_NOYELL 1
#define JAIL_NOTELL 1
#define JAIL_NOEMOTE 1
#define JAIL_CAN_DELETE 0       /* Can they delete while jailed */
#define JAIL_CAN_WRITE_NOTE 0   /* Can they write notes while in jail */
#define JAIL_CAN_SAC 0          /* Can they junk/sac items in jail */
#define JAIL_CAN_DONATE 0       /* Can they donate EQ while in jail */

/* The below email define should be used for sending email to the mud
admin to request that they be permitted to play on the mud from their
site. They will only get this msg if you have set something like ban
foo.com permit. When a char logs in who does not have the permit flag,
they will recieve a msg where to send email to. This includes new players
aswell. Feel free to use this anyplace else in the mud aswell.
-Lancelight
*/

#define MUD_ADMIN_EMAIL	"mudadmin@whatever.com"

/* This vnums are for use with the banking system. The 1st one is for
players that are not thieves. The second is the vnum of the thieves guild.
-Lancelight
*/

#define ROOM_VNUM_BANK        3008
#define ROOM_VNUM_BANK_THIEF  3029

/* Color defines for use in config.h primarily */
#define GREY          "`w"
#define WHITE         "`W"
#define GREEN         "`G"
#define BLUE          "`B"
#define RED           "`R"
#define CYAN          "`C"
#define YELLOW        "`Y"
#define MAGENTA       "`M"
#define DARK_GREEN    "`g"
#define DARK_BLUE     "`b"
#define DARK_RED      "`r"
#define DARK_CYAN     "`c"
#define DARK_YELLOW   "`y"
#define DARK_MAGENTA  "`m"
#define DARK_GREY     "`K"
#define BLACK         "`k"

/* RT ASCII conversions -- used so we can have letters in this file */

#define A                       1
#define B                       2
#define C                       4
#define D                       8
#define E                       16
#define F                       32
#define G                       64
#define H                       128

#define I                       256
#define J                       512
#define K                       1024
#define L                       2048
#define M                       4096
#define N                       8192
#define O                       16384
#define P                       32768

#define Q                       65536
#define R                       131072
#define S                       262144
#define T                       524288
#define U                       1048576
#define V                       2097152
#define W                       4194304
#define X                       8388608

#define Y                       16777216
#define Z                       33554432
#define aa                      67108864    /* doubled due to conflicts */
#define bb                      134217728
#define cc                      268435456
#define dd                      536870912
#define ee                      1073741824

/*
 * String and memory management parameters.
 */

/*
 * The Max_Key_Hash should always be a prime number that's at least 20%
 * or more larger than the number of unique strings you're loading in.
 * Use the mem command in the mud to determine how many non-unique
 * strings are being loaded and guess at the number of duplicates. :)
 * -Zane
 */
#define MAX_KEY_HASH            65413
#define MAX_STRING_LENGTH        4096
#define MAX_INPUT_LENGTH          256
#define PAGELEN                    22

/*
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 * so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#if defined(WIN32)
#define NULL_FILE               "nul"   // To reserve one stream
#endif

#if defined(unix)
#define NULL_FILE       "/dev/null" /* To reserve one stream        */
#endif

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_CREATION_POINTS        10   /* Not enforced, but used in some calculations */
                                       /* Namely the max CP cost for a skill/spell */
#define MAX_SOCIALS               256
#define MAX_SKILL                 200
#define MAX_GROUP                  36
#define MAX_IN_GROUP               20
#define MAX_ALIAS                  20
#define MAX_CLASS                   4
#define MAX_PC_RACE                12
#define MAX_LEVEL                  60
#define MAX_EXP            2147483647
#define MAX_CHUNKS                 80   /* Used in ssm.c */
#define MAX_OUTPUT_BUFFER       32000
/* Allows you to tune how easy/hard it is to level.  For instance 0.75 would give you only
 * 75% of the exp you'd normally get from any task basically making it 25% hard to level - Zane */
#define EXP_MULTIPLIER				1
#define MAX_ATTACK_TYPE            32
#define MAX_LAST_LENGTH            20   /*How long can the last list be? */
#define LEVEL_HERO                 (MAX_LEVEL - 9)
#define LEVEL_IMMORTAL             (MAX_LEVEL - 7)
#define LEVEL_ADMIN                (MAX_LEVEL - 3)

#define PULSE_PER_SECOND            4
#define PULSE_VIOLENCE            ( 3 * PULSE_PER_SECOND )
#define PULSE_MOBILE              ( 4 * PULSE_PER_SECOND)
#define PULSE_AUCTION             (20 * PULSE_PER_SECOND)   /* Tweak this to make auctions slower or faster. -Lancelight */
#define PULSE_TICK                (30 * PULSE_PER_SECOND)
#define PULSE_AREA                (60 * PULSE_PER_SECOND)
#define AUCTION_LENGTH                    5
#define MINIMUM_BID                     10  /*set this to whatever. -Lancelight */
#define IMPLEMENTOR             MAX_LEVEL
#define CREATOR                 (MAX_LEVEL - 1)
#define SUPREME                 (MAX_LEVEL - 2)
#define DEITY                   (MAX_LEVEL - 3)
#define GOD                     (MAX_LEVEL - 4)
#define IMMORTAL                (MAX_LEVEL - 5)
#define DEMI                    (MAX_LEVEL - 6)
#define ANGEL                   (MAX_LEVEL - 7)
#define AVATAR                  (MAX_LEVEL - 8)
#define HERO                    LEVEL_HERO

#define CLASS_MAGE              0
#define CLASS_CLERIC            1
#define CLASS_THIEF             2
#define CLASS_WARRIOR           3
#define CLASS_CONSOLE           4

/*
 * Per-class stuff.
 */

#define MAX_GUILD       2
#define MAX_STATS       5
#define STAT_STR        0
#define STAT_INT        1
#define STAT_WIS        2
#define STAT_DEX        3
#define STAT_CON        4

/********************************************************
 *                                                      *
 * Clan stuff  -- By Kyle Boyd                          *
 *                                                      *
 ********************************************************/

#define MAX_CLAN                   10
#define MAX_CLAN_MEMBERS           30
#define MAX_RANK                    5
#define MIN_CLAN_LEVEL             15
#define RANK_CAN_ACCEPT             3

/* Clan anti-flags */
#define CLAN_ANTI_MAGE          (A)
#define CLAN_ANTI_CLERIC        (B)
#define CLAN_ANTI_THIEF         (C)
#define CLAN_ANTI_WARRIOR       (D)
#define CLAN_ANTI_GOOD          (H)
#define CLAN_ANTI_EVIL          (I)
#define CLAN_REQ_PK             (J)
#define CLAN_NO_PK              (K)

/* Clan flags */
#define CLAN_CREATING           (A) /* just made... not available to chars */
#define CLAN_ACTIVE             (B) /* Actively recruiting */
#define CLAN_INACTIVE           (C) /* Not recruiting */
#define CLAN_DISBANDING         (D) /* Disbanding... chars must resign soon. */
#define CLAN_DEAD               (E) /* Chars forced to resign. */
#define CLAN_AUTO_ACCEPT        (F) /* autoaccept new members */
#define CLAN_PRIVATE            (G) /* Members not displayed on the who list except to each other. */
#define CLAN_SECRET             (H) /* Only members and IMPs even know about this clan. */

/* Flags for joining a clan */
#define JOIN_START              0   /* Not trying to join a clan */
#define JOIN_SEE_LIST           1   /* Starts trying to join, so show char the list */
#define JOIN_CONFIRM            2   /* make sure char is joining the right clan */
#define JOIN_PETITIONING        3   /* char is petitioning to join a clan that is not autoaccept */
#define JOIN_CONSIDERING        4   /* char is considering to accept a petitioner */
#define JOIN_OFFERING           5   /* char is offering membership to someone */
#define JOIN_DECIDING           6   /* char is deciding to accept or decline an offer to join */
#define JOIN_RESIGNING          7   /* char needs to confirm resignation */

/*********** End Clan Configs ************/

/********************************************************
 *                                                      *
 * Mob/Room/Obj Progs                                   *
 *                                                      *
 ********************************************************/

#define ERROR_PROG             -1
#define IN_FILE_PROG            0
#define ACT_PROG                A   /* mprogs, rprogs, oprogs       */
#define SPEECH_PROG             B   /* mprogs, rprogs, oprogs       */
#define RAND_PROG               C   /* mprogs       rprogs  oprogs  */
#define FIGHT_PROG              D   /* mprogs                       */
#define RFIGHT_PROG             D   /*              rprogs          */
#define DEATH_PROG              E   /* mprogs                       */
#define RDEATH_PROG             E   /*              rprogs          */
#define HITPRCNT_PROG           F   /* mprogs                       */
#define ENTRY_PROG              G   /* mprogs                       */
#define ENTER_PROG              G   /*              rprogs          */
#define GREET_PROG              H   /* mprogs                       */
#define ALL_GREET_PROG          I   /* mprogs                       */
#define GIVE_PROG               J   /* mprogs                       */
#define BRIBE_PROG              K   /* mprogs                       */
#define LEAVE_PROG              L   /*              rprogs          */
#define SLEEP_PROG              M   /*              rprogs          */
#define REST_PROG               N   /*              rprogs          */
#define WEAR_PROG               O   /*                      oprogs  */
#define REMOVE_PROG             P   /*                      oprogs  */
#define SAC_PROG                Q   /*                      oprogs  */
#define EXA_PROG                R   /*                      oprogs  */
#define LOOK_PROG               S   /*                      oprogs  */
#define ZAP_PROG                T   /*                      oprogs  */
#define GET_PROG                U   /*                      oprogs  */
#define DROP_PROG               V   /*                      oprogs  */
#define DAMAGE_PROG             W   /*                      oprogs  */
#define REPAIR_PROG             X   /*                      oprogs  */
#define USE_PROG                Y   /*                      oprogs  */
#define COMMAND_PROG            Z   /* mprogs       rprogs  oprogs  */
#define HIT_PROG				aa  /*                      oprogs  */
#define FIGHTGROUP_PROG         bb  /* mprogs                       */

/* Prog Types */
#define MOB_PROG					A
#define OBJ_PROG					B
#define ROOM_PROG					C

/********* End Mob/Room/Obj Progs **********/

/*
 * Time and weather stuff.
 */
#define SUN_DARK                    0
#define SUN_RISE                    1
#define SUN_LIGHT                   2
#define SUN_SET                     3

#define SKY_CLOUDLESS               0
#define SKY_CLOUDY                  1
#define SKY_RAINING                 2
#define SKY_LIGHTNING               3

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_FIDO              3090
#define MOB_VNUM_CITYGUARD         3060
#define MOB_VNUM_VAMPIRE           3404

/* for newly created characters - Kyle */
#define STARTING_PRACTICES 5
#define STARTING_TRAINS 1
#define STARTING_TITLE "the newbie"

/*Spicey "consider" comments*/

#define CON_MSG1 "$N can kiss his ass goodbye!"
#define CON_MSG2 "You can smell $N's fear as he knows your strength."
#define CON_MSG3 "$N can't walk the walk."
#define CON_MSG4 "See if you can overcome an equal adversary."
#define CON_MSG5 "You're in for quite a challenge."
#define CON_MSG6 "$N will chew you up and spit you out."
#define CON_MSG7 "$N will open up a whole case of whoop-ass on you."

/* Define this if you want a some 'Different' and a few more damage
messages. 
THIS IS ONLY FOR DAMAGE_BY_NUMBER !!!!!!!!!!!! Do NOT turn it on unless
you have DAMAGE_BY_NUMBER on aswell or freaky things may
happen. DAMAGE_BY_NUMBER is on by default so you shouldnt need to worry
about that unless you have changed it.
 -Lancelight*/

/*
#define EXTRA_DAMAGE_MSGS
*/

/* These are some example extra damage msg's. It gives you more options
then the default
messages, aswell as some messages that are unique to Ember and not ROM
style. They aren't
the prettiest things in the world, but its something to enhance the look
of combat. */

#define CFG_DAM0   "miss"
#define CFG_DAM0S  "misses"
#define CFG_DAM2   "`Mf`m`mondl`Me`w"
#define CFG_DAM2S  "`Mf`mondle`Ms`w"
#define CFG_DAM5   "`Ct`cickl`Ce`w"
#define CFG_DAM5S  "`Ct`cickle`Cs`w"
#define CFG_DAM10   "`Rs`rcratc`Rh`w"
#define CFG_DAM10S  "`Rs`rcratche`Rs`w"
#define CFG_DAM15   "`Bb`bruis`Be`w"
#define CFG_DAM15S  "`Bb`bruise`Bs`w"
#define CFG_DAM25  "`Gs`gcuf`Gf`w"
#define CFG_DAM25S "`Gs`gcuff`Gs`w"
#define CFG_DAM30  "`Ys`yki`Ym`w"
#define CFG_DAM30S "`Ys`ykim`Ys`w"
#define CFG_DAM35  "`Kg`Wraz`Ke`w"
#define CFG_DAM35S "`Kg`Wraze`Ks`w"
#define CFG_DAM45  "`Ml`macerat`Me`w"
#define CFG_DAM45S "`Ml`macerate`Ms`w"
#define CFG_DAM50  "`Ch`ci`Ct`w"
#define CFG_DAM50S "`Ch`cit`Cs`w"
#define CFG_DAM55  "`Rs`rtrik`Re`w"
#define CFG_DAM55S "`Rs`rtrike`Rs`w"
#define CFG_DAM65  "`Bf`blo`Bg`w"
#define CFG_DAM65S "`Bf`blog`Bs`w"
#define CFG_DAM70  "`Gp`goun`Gd`w"
#define CFG_DAM70S "`Gp`gound`Gs`w"
#define CFG_DAM75  "`Yi`ynjur`Ye`w"
#define CFG_DAM75S "`Yi`ynjure`Ys`w"
#define CFG_DAM85  "`Kt`Whras`Kh`w"
#define CFG_DAM85S "`Kt`Whrashe`Ks`w"
#define CFG_DAM90  "`Mp`mumme`Ml`w"
#define CFG_DAM90S "`Mp`mummel`Ms`w"
#define CFG_DAM95  "`Cw`coun`Cd`w"
#define CFG_DAM95S "`Cw`cound`Cs`w"
#define CFG_DAM105  "`Ri`rmpai`Rr`w"
#define CFG_DAM105S "`Ri`rmpair`Rs`w"
#define CFG_DAM110  "`Bm`bau`Bl`w"
#define CFG_DAM110S "`Bm`baul`Bs`w"
#define CFG_DAM115  "`Gi`gmpal`Ge`w"
#define CFG_DAM115S "`Gi`gmpale`Gs`w"
#define CFG_DAM125  "`Yb`yatte`Yr`w"
#define CFG_DAM125S "`Yb`yatter`Ys`w"
#define CFG_DAM130  "`KD`Wec`wim`Wat`Ke`w"
#define CFG_DAM130S "`KD`Wec`wima`Wte`Ks`w"
#define CFG_DAM135 "`MR`ma`Wva`mg`Me`w"
#define CFG_DAM135S "`MR`ma`Wvag`me`Ms`w"
#define CFG_DAM145 "`CD`cev`Wast`cat`Ce`w"
#define CFG_DAM145S "`CDe`cva`Wst`cat`Ces`w"
#define CFG_DAM150 "`RM`Wai`Rm`w"
#define CFG_DAM150S "`RM`Waim`Rs`w"
#define CFG_DAM155 "`BH`Wac`Bk`w"
#define CFG_DAM155S "`BH`Wack`Bs`w"
#define CFG_DAM165 "`GM`gut`Wil`gat`Ge`w"
#define CFG_DAM165S "`GM`gut`Wila`gte`Gs`w"
#define CFG_DAM170 "`YM`yo`Wle`ys`Yt`w"
#define CFG_DAM170S "`YM`yo`Wles`yt`Ys`w"
#define CFG_DAM175 "`KR`Wap`Ke`w"
#define CFG_DAM175S "`KR`Wape`Ks`w"
#define CFG_DAM185 "`MP`ml`Wund`me`Mr`w"
#define CFG_DAM185S "`MP`mlu`Wnd`mer`Ms`w"
#define CFG_DAM190 "`CD`cis`Wmem`cbe`Cr`w"
#define CFG_DAM190S "`CDi`csm`Wem`cbe`Crs`w"
#define CFG_DAM195 "`RM`ras`Wsa`rcr`Re`w"
#define CFG_DAM195S "`RM`ras`Wsac`rre`Rs`w"
#define CFG_DAM200 "`BD`bis`Wfig`bur`Be`w"
#define CFG_DAM200S "`BD`bis`Wfigu`bre`Bs`w"
#define CFG_DAM205 "`GM`ga`Wng`gl`Ge`w"
#define CFG_DAM205S "`GM`ga`Wngl`ge`Gs`w"
#define CFG_DAM215 "`Y/`y/`Y/ D`ye`Wmoli`ys`Yh \\`y\\`Y\\`w"
#define CFG_DAM215S "`Y/`y/`Y/ De`ymo`Wli`ysh`Yes \\`y\\`Y\\`w"
#define CFG_DAM220 "`K<{{ `WT`woo`Wl `K}}>`w"
#define CFG_DAM220S "`K<{{ T`Wo`wo`Wl`Ks }}>`w"
#define CFG_DAM225 "`m*** `MD`mis`Wembo`mwe`Ml `m***`w"
#define CFG_DAM225S "`m*** `MDi`mse`Wmbo`mwe`Mls `m***`w"
#define CFG_DAM230 "`C<><> B`cu`Wtch`ce`Cr <><>`w"
#define CFG_DAM230S "`C<><> B`cut`Wch`cer`Cs <><>`w"
#define CFG_DAM235 "`R(=\\ D`re`Wstr`ro`Ry /=)`w"
#define CFG_DAM235S "`R(=\\ D`res`Wtr`roy`Rs /=)`w"
#define CFG_DAM245 "`B-=[ S`bla`Wugh`bte`Br ]=-`w"
#define CFG_DAM245S "`B-=[ Sl`bau`Wgh`bte`Brs ]=-`w"
#define CFG_DAM250 "`G#`g#`G# T`ger`Wmin`gat`Ge #`g#`G#`w"
#define CFG_DAM250S "`G#`g#`G# Te`grm`Win`gat`Ges #`g#`G#`w"
#define CFG_DAM255 "`Y/\\/ S`yh`Watt`ye`Yr \\/\\`w"
#define CFG_DAM255S "`Y/\\/ S`yha`Wtt`yer`Ys \\/\\`w"
#define CFG_DAM265 "`K++ O`Wbl`witer`Wat`Ke ++`w"
#define CFG_DAM265S "`K++ Ob`Wli`wter`Wat`Kes ++`w"
#define CFG_DAM270 "`M{{} A`mnn`Wihil`mat`Me {})`w"
#define CFG_DAM270S "`M{{} An`mni`Whil`mat`Mes {})`w"
#define CFG_DAM275 "`C-- E`cra`Wdic`cat`Ce --`w"
#define CFG_DAM275S "`C-- Er`cad`Wic`cat`Ces --`w"
#define CFG_DAM285 "`R(`r(`R( P`rul`Wver`riz`Re )`r)`R)`w"
#define CFG_DAM285S "`R(`r(`R( Pu`rlv`Wer`riz`Res )`r)`R)`w"
#define CFG_DAM290 "`B[] D`bom`Win`bat`Be []`w"
#define CFG_DAM290S "`B[] D`bom`Wina`bte`Bs []`w"
#define CFG_DAM295 "`G:: E`gxt`Wermin`gat`Ge ::`w"
#define CFG_DAM295S "`G:: Ex`gte`Wrmin`gat`Ges ::`w"
#define CFG_DAM300 "`Y!! L`yiq`Wuid`yat`Ye !!`w"
#define CFG_DAM300S "`Y!! Li`yqu`Wid`yat`Yes !!`w"
#define CFG_DAM_HUGE  "`K>>>~~> `WO`wW`WN `K<~~<<<`w"
#define CFG_DAM_HUGES "`K>>>~~> `WO`wWN`WZ `K<~~<<<`w"

#define CFG_DAM0   "miss"
#define CFG1_DAM0S  "misses"
#define CFG1_DAM2   "scratch"
#define CFG1_DAM2S  "scratches"
#define CFG1_DAM4   "graze"
#define CFG1_DAM4S  "grazes"
#define CFG1_DAM6   "hit"
#define CFG1_DAM6S  "hits"
#define CFG1_DAM8   "injure"
#define CFG1_DAM8S  "injures"
#define CFG1_DAM10  "wound"
#define CFG1_DAM10S "wounds"
#define CFG1_DAM12  "maul"
#define CFG1_DAM12S "mauls"
#define CFG1_DAM14  "decimate"
#define CFG1_DAM14S "decimates"
#define CFG1_DAM16  "devastate"
#define CFG1_DAM16S "devastates"
#define CFG1_DAM18  "maim"
#define CFG1_DAM18S "maims"
#define CFG1_DAM20  "MUTILATE"
#define CFG1_DAM20S "MUTILATES"
#define CFG1_DAM22  "PULVERISE"
#define CFG1_DAM22S "PULVERISES"
#define CFG1_DAM24  "DISMEMBER"
#define CFG1_DAM24S "DISMEMBERS"
#define CFG1_DAM26  "MASSACRE"
#define CFG1_DAM26S "MASSACRES"
#define CFG1_DAM28  "MANGLE"
#define CFG1_DAM28S "MANGLES"
#define CFG1_DAM30  "*** DEMOLISH ***"
#define CFG1_DAM30S "*** DEMOLISHES ***"
#define CFG1_DAM37  "*** DEVASTATE ***"
#define CFG1_DAM37S "*** DEVASTATES ***"
#define CFG1_DAM50  "=== OBLITERATE ==="
#define CFG1_DAM50S "=== OBLITERATES ==="
#define CFG1_DAM63  ">>> ANNIHILATE <<<"
#define CFG1_DAM63S ">>> ANNIHILATES <<<"
#define CFG1_DAM75  "<<< ERADICATE >>>"
#define CFG1_DAM75S "<<< ERADICATES >>>"
#define CFG1_DAM83  "<><><> BUTCHER <><><>"
#define CFG1_DAM83S "<><><> BUTCHERS <><><>"
#define CFG1_DAM93  "<><><> DISEMBOWEL <><><>"
#define CFG1_DAM93S "<><><> DISEMBOWELS <><><>"
#define CFG1_DAM_HUGE  "do UNSPEAKABLE things to"
#define CFG1_DAM_HUGES "does UNSPEAKABLE things to"

/*
 * TO types for act.
 */
#define TO_ROOM             0
#define TO_NOTVICT          1
#define TO_VICT             2
#define TO_CHAR             3

/*
 * Shop types.
 */
#define MAX_TRADE        5

/*
* supermob vnums...used for room progs and
* eventually object progs
*/
#define MOB_VNUM_SUPERMOB       4
#define ROOM_VNUM_SUPERMOB      4

/*
 * Connected state for a channel.
 */
#define CON_PLAYING                      0
#define CON_GET_NAME                     1
#define CON_GET_OLD_PASSWORD             2
#define CON_CONFIRM_NEW_NAME             3
#define CON_GET_NEW_PASSWORD             4
#define CON_CONFIRM_NEW_PASSWORD         5
#define CON_GET_NEW_RACE                 6
#define CON_GET_NEW_SEX                  7
#define CON_GET_NEW_CLASS                8
#define CON_GET_ALIGNMENT                9
#define CON_DEFAULT_CHOICE              10
#define CON_GEN_GROUPS                  11
#define CON_PICK_WEAPON                 12
#define CON_READ_IMOTD                  13
#define CON_READ_MOTD                   14
#define CON_BREAK_CONNECT               15
#define CON_GET_STATS                   16
#define CON_GET_ANSI                    17
#define CON_COPYOVER_RECOVER            18
#define CON_NOTE_TO                     19
#define CON_NOTE_SUBJECT                20
#define CON_NOTE_EXPIRE                 21
#define CON_NOTE_TEXT                   22
#define CON_NOTE_FINISH                 23
#define CON_SHELL                       24
#define CON_BEGIN_REMORT                25

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC              (A) /* Auto set for mobs    */
#define ACT_SENTINEL            (B) /* Stays in one room    */
#define ACT_SCAVENGER           (C) /* Picks up objects     */
#define ACT_AGGRESSIVE          (F) /* Attacks PC's         */
#define ACT_STAY_AREA           (G) /* Won't leave area     */
#define ACT_WIMPY               (H)
#define ACT_PET                 (I) /* Auto set for pets    */
#define ACT_TRAIN               (J) /* Can train PC's       */
#define ACT_PRACTICE            (K) /* Can practice PC's    */
#define ACT_UNDEAD              (O)
#define ACT_CLERIC              (Q)
#define ACT_MAGE                (R)
#define ACT_THIEF               (S)
#define ACT_WARRIOR             (T)
#define ACT_NOALIGN             (U)
#define ACT_NOPURGE             (V)
#define ACT_MOBINVIS            (W)
#define ACT_NO_STEAL            (X)
#define ACT_IS_HEALER           (aa)
#define ACT_GAIN                (bb)
#define ACT_UPDATE_ALWAYS       (cc)
#define ACT_NO_KILL             (dd)

/* damage classes */
#define DAM_NONE                0
#define DAM_BASH                1
#define DAM_PIERCE              2
#define DAM_SLASH               3
#define DAM_FIRE                4
#define DAM_COLD                5
#define DAM_LIGHTNING           6
#define DAM_ACID                7
#define DAM_POISON              8
#define DAM_NEGATIVE            9
#define DAM_HOLY                10
#define DAM_ENERGY              11
#define DAM_MENTAL              12
#define DAM_DISEASE             13
#define DAM_DROWNING            14
#define DAM_LIGHT               15
#define DAM_OTHER               16
#define DAM_HARM                17

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK         (A)
#define OFF_BACKSTAB            (B)
#define OFF_BASH                (C)
#define OFF_BERSERK             (D)
#define OFF_DISARM              (E)
#define OFF_DODGE               (F)
#define OFF_FADE                (G)
#define OFF_FAST                (H)
#define OFF_KICK                (I)
#define OFF_KICK_DIRT           (J)
#define OFF_PARRY               (K)
#define OFF_RESCUE              (L)
#define OFF_TAIL                (M)
#define OFF_TRIP                (N)
#define OFF_CRUSH               (O)
#define ASSIST_ALL              (P)
#define ASSIST_ALIGN            (Q)
#define ASSIST_RACE             (R)
#define ASSIST_PLAYERS          (S)
#define ASSIST_GUARD            (T)
#define ASSIST_VNUM             (U)

/* return values for check_imm */
#define IS_NORMAL               0
#define IS_IMMUNE               1
#define IS_RESISTANT            2
#define IS_VULNERABLE           3

/* IMM bits for mobs */
#define IMM_SUMMON              (A)
#define IMM_CHARM               (B)
#define IMM_MAGIC               (C)
#define IMM_WEAPON              (D)
#define IMM_BASH                (E)
#define IMM_PIERCE              (F)
#define IMM_SLASH               (G)
#define IMM_FIRE                (H)
#define IMM_COLD                (I)
#define IMM_LIGHTNING           (J)
#define IMM_ACID                (K)
#define IMM_POISON              (L)
#define IMM_NEGATIVE            (M)
#define IMM_HOLY                (N)
#define IMM_ENERGY              (O)
#define IMM_MENTAL              (P)
#define IMM_DISEASE             (Q)
#define IMM_DROWNING            (R)
#define IMM_LIGHT               (S)

/* RES bits for mobs */
#define RES_CHARM               (B)
#define RES_MAGIC               (C)
#define RES_WEAPON              (D)
#define RES_BASH                (E)
#define RES_PIERCE              (F)
#define RES_SLASH               (G)
#define RES_FIRE                (H)
#define RES_COLD                (I)
#define RES_LIGHTNING           (J)
#define RES_ACID                (K)
#define RES_POISON              (L)
#define RES_NEGATIVE            (M)
#define RES_HOLY                (N)
#define RES_ENERGY              (O)
#define RES_MENTAL              (P)
#define RES_DISEASE             (Q)
#define RES_DROWNING            (R)
#define RES_LIGHT               (S)

/* VULN bits for mobs */
#define VULN_MAGIC              (C)
#define VULN_WEAPON             (D)
#define VULN_BASH               (E)
#define VULN_PIERCE             (F)
#define VULN_SLASH              (G)
#define VULN_FIRE               (H)
#define VULN_COLD               (I)
#define VULN_LIGHTNING          (J)
#define VULN_ACID               (K)
#define VULN_POISON             (L)
#define VULN_NEGATIVE           (M)
#define VULN_HOLY               (N)
#define VULN_ENERGY             (O)
#define VULN_MENTAL             (P)
#define VULN_DISEASE            (Q)
#define VULN_DROWNING           (R)
#define VULN_LIGHT              (S)
#define VULN_WOOD               (X)
#define VULN_SILVER             (Y)
#define VULN_IRON               (Z)

/* body form */
#define FORM_EDIBLE             (A)
#define FORM_POISON             (B)
#define FORM_MAGICAL            (C)
#define FORM_INSTANT_DECAY      (D)
#define FORM_OTHER              (E) /* defined by material bit */

/* actual form */
#define FORM_ANIMAL             (G)
#define FORM_SENTIENT           (H)
#define FORM_UNDEAD             (I)
#define FORM_CONSTRUCT          (J)
#define FORM_MIST               (K)
#define FORM_INTANGIBLE         (L)

#define FORM_BIPED              (M)
#define FORM_CENTAUR            (N)
#define FORM_INSECT             (O)
#define FORM_SPIDER             (P)
#define FORM_CRUSTACEAN         (Q)
#define FORM_WORM               (R)
#define FORM_BLOB               (S)

#define FORM_MAMMAL             (V)
#define FORM_BIRD               (W)
#define FORM_REPTILE            (X)
#define FORM_SNAKE              (Y)
#define FORM_DRAGON             (Z)
#define FORM_AMPHIBIAN          (aa)
#define FORM_FISH               (bb)
#define FORM_COLD_BLOOD         (cc)

/* body parts */
#define PART_HEAD               (A)
#define PART_ARMS               (B)
#define PART_LEGS               (C)
#define PART_HEART              (D)
#define PART_BRAINS             (E)
#define PART_GUTS               (F)
#define PART_HANDS              (G)
#define PART_FEET               (H)
#define PART_FINGERS            (I)
#define PART_EAR                (J)
#define PART_EYE                (K)
#define PART_LONG_TONGUE        (L)
#define PART_EYESTALKS          (M)
#define PART_TENTACLES          (N)
#define PART_FINS               (O)
#define PART_WINGS              (P)
#define PART_TAIL               (Q)
/* for combat */
#define PART_CLAWS              (U)
#define PART_FANGS              (V)
#define PART_HORNS              (W)
#define PART_SCALES             (X)
#define PART_TUSKS              (Y)

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND               (A)
#define AFF_INVISIBLE           (B)
#define AFF_DETECT_EVIL         (C)
#define AFF_DETECT_INVIS        (D)
#define AFF_DETECT_MAGIC        (E)
#define AFF_DETECT_HIDDEN       (F)
#define AFF_HOLD                (G) /* Unused       */
#define AFF_SANCTUARY           (H)

#define AFF_FAERIE_FIRE         (I)
#define AFF_INFRARED            (J)
#define AFF_CURSE               (K)
#define AFF_FLAMING             (L) /* Unused       */
#define AFF_POISON              (M)
#define AFF_PROTECT             (N)
#define AFF_PARALYSIS           (O) /* Unused       */
#define AFF_SNEAK               (P)

#define AFF_HIDE                (Q)
#define AFF_SLEEP               (R)
#define AFF_CHARM               (S)
#define AFF_FLYING              (T)
#define AFF_PASS_DOOR           (U)
#define AFF_HASTE               (V)
#define AFF_CALM                (W)
#define AFF_PLAGUE              (X)
#define AFF_WEAKEN              (Y)
#define AFF_DARK_VISION         (Z)
#define AFF_BERSERK             (aa)
#define AFF_SWIM                (bb)
#define AFF_REGENERATION        (cc)
#define AFF_WEB                 (dd)

/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL                   0
#define SEX_MALE                      1
#define SEX_FEMALE                    2

/* AC types */
#define AC_PIERCE                       0
#define AC_BASH                         1
#define AC_SLASH                        2
#define AC_EXOTIC                       3

/* dice */
#define DICE_NUMBER                     0
#define DICE_TYPE                       1
#define DICE_BONUS                      2

/* size */
#define SIZE_TINY                       0
#define SIZE_SMALL                      1
#define SIZE_MEDIUM                     2
#define SIZE_LARGE                      3
#define SIZE_HUGE                       4
#define SIZE_GIANT                      5

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE            2
#define OBJ_VNUM_MONEY_SOME           3

#define OBJ_VNUM_CORPSE_NPC          10
#define OBJ_VNUM_CORPSE_PC           11
#define OBJ_VNUM_SEVERED_HEAD        12
#define OBJ_VNUM_TORN_HEART          13
#define OBJ_VNUM_SLICED_ARM          14
#define OBJ_VNUM_SLICED_LEG          15
#define OBJ_VNUM_GUTS                16
#define OBJ_VNUM_BRAINS              17

#define OBJ_VNUM_MUSHROOM            20
#define OBJ_VNUM_LIGHT_BALL          21
#define OBJ_VNUM_SPRING              22

#define OBJ_VNUM_PIT               3010

#define OBJ_VNUM_SCHOOL_MACE       3700
#define OBJ_VNUM_SCHOOL_DAGGER     3701
#define OBJ_VNUM_SCHOOL_SWORD      3702
#define OBJ_VNUM_SCHOOL_VEST       3703
#define OBJ_VNUM_SCHOOL_SHIELD     3704
#define OBJ_VNUM_SCHOOL_BANNER     3716
#define OBJ_VNUM_MAP               3162
#define OBJ_VNUM_BLANK_SCROLL      3398
#define OBJ_VNUM_EMPTY_VIAL        3399

#define OBJ_VNUM_PORTAL       3392  /* This is the vnum of the portal
                                       created using the Nexus Spell */
/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT                    1
#define ITEM_SCROLL                   2
#define ITEM_WAND                     3
#define ITEM_STAFF                    4
#define ITEM_WEAPON                   5
#define ITEM_TREASURE                 8
#define ITEM_ARMOR                    9
#define ITEM_POTION                  10
#define ITEM_CLOTHING                11
#define ITEM_FURNITURE               12
#define ITEM_TRASH                   13
#define ITEM_CONTAINER               15
#define ITEM_DRINK_CON               17
#define ITEM_KEY                     18
#define ITEM_FOOD                    19
#define ITEM_MONEY                   20
#define ITEM_BOAT                    22
#define ITEM_CORPSE_NPC              23
#define ITEM_CORPSE_PC               24
#define ITEM_FOUNTAIN                25
#define ITEM_PILL                    26
#define ITEM_PROTECT                 27
#define ITEM_MAP                     28
#define ITEM_PORTAL                  29

/* furniture flags */
#define STAND_AT		(A)
#define STAND_ON		(B)
#define STAND_IN		(C)
#define SIT_AT			(D)
#define SIT_ON			(E)
#define SIT_IN			(F)
#define REST_AT			(G)
#define REST_ON			(H)
#define REST_IN			(I)
#define SLEEP_AT		(J)
#define SLEEP_ON		(K)
#define SLEEP_IN		(L)
#define PUT_AT			(M)
#define PUT_ON			(N)
#define PUT_IN			(O)
#define PUT_INSIDE		(P)
/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW               (A)
#define ITEM_HUM                (B)
#define ITEM_DARK               (C)
#define ITEM_LOCK               (D)
#define ITEM_EVIL               (E)
#define ITEM_INVIS              (F)
#define ITEM_MAGIC              (G)
#define ITEM_NODROP             (H)
#define ITEM_BLESS              (I)
#define ITEM_ANTI_GOOD          (J)
#define ITEM_ANTI_EVIL          (K)
#define ITEM_ANTI_NEUTRAL       (L)
#define ITEM_NOREMOVE           (M)
#define ITEM_INVENTORY          (N)
#define ITEM_NOPURGE            (O)
#define ITEM_ROT_DEATH          (P)
#define ITEM_VIS_DEATH          (Q)
#define ITEM_ANTI_CLERIC          (R)
#define ITEM_ANTI_THIEF           (S)
#define ITEM_ANTI_WARRIOR         (T)
#define ITEM_ANTI_MAGE            (X)
#define ITEM_ANTI_MALE            (Y)
#define ITEM_ANTI_NEUTER          (Z)
#define ITEM_ANTI_FEMALE          (aa)
#define ITEM_ROT_PLAYER_DEATH     (bb)
/*
 * Added for Random Object code
 */
#define ITEM_PLAIN_ARMOR      (A)
#define ITEM_MAGIC_ARMOR      (B)
#define ITEM_RAND_RING        (C)
/* #define ITEM_MAGIC_ITEM    (D) */
#define ITEM_PLAIN_WEAPON     (E)
#define ITEM_MAGIC_WEAPON     (F)
#define ITEM_RAND_CONTAINER   (G)
/* #define ITEM_RAND_LIGHT    (H) */
#define ITEM_RAND_ANY (I)

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE               (A)
#define ITEM_WEAR_FINGER        (B)
#define ITEM_WEAR_NECK          (C)
#define ITEM_WEAR_BODY          (D)
#define ITEM_WEAR_HEAD          (E)
#define ITEM_WEAR_LEGS          (F)
#define ITEM_WEAR_FEET          (G)
#define ITEM_WEAR_HANDS         (H)
#define ITEM_WEAR_ARMS          (I)
#define ITEM_WEAR_SHIELD        (J)
#define ITEM_WEAR_ABOUT         (K)
#define ITEM_WEAR_WAIST         (L)
#define ITEM_WEAR_WRIST         (M)
#define ITEM_WIELD              (N)
#define ITEM_HOLD               (O)
#define ITEM_TWO_HANDS          (P)

/* weapon class */
#define WEAPON_EXOTIC           0
#define WEAPON_SWORD            1
#define WEAPON_DAGGER           2
#define WEAPON_SPEAR            3
#define WEAPON_MACE             4
#define WEAPON_AXE              5
#define WEAPON_FLAIL            6
#define WEAPON_WHIP             7
#define WEAPON_POLEARM          8

/* weapon types */
#define WEAPON_FLAMING          (A)
#define WEAPON_FROST            (B)
#define WEAPON_VAMPIRIC         (C)
#define WEAPON_SHARP            (D)
#define WEAPON_VORPAL           (E)
#define WEAPON_TWO_HANDS        (F)

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define APPLY_NONE                    0
#define APPLY_STR                     1
#define APPLY_DEX                     2
#define APPLY_INT                     3
#define APPLY_WIS                     4
#define APPLY_CON                     5
#define APPLY_SEX                     6
#define APPLY_CLASS                   7
#define APPLY_LEVEL                   8
#define APPLY_AGE                     9
#define APPLY_HEIGHT                 10
#define APPLY_WEIGHT                 11
#define APPLY_MANA                   12
#define APPLY_HIT                    13
#define APPLY_MOVE                   14
#define APPLY_GOLD                   15
#define APPLY_EXP                    16
#define APPLY_AC                     17
#define APPLY_HITROLL                18
#define APPLY_DAMROLL                19
#define APPLY_SAVING_PARA            20
#define APPLY_SAVING_ROD             21
#define APPLY_SAVING_PETRI           22
#define APPLY_SAVING_BREATH          23
#define APPLY_SAVING_SPELL           24
#define APPLY_ALIGN                  25

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE                1
#define CONT_PICKPROOF                2
#define CONT_CLOSED                   4
#define CONT_LOCKED                   8

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO               2
#define ROOM_VNUM_CHAT             1200
#define ROOM_VNUM_TEMPLE           3001
#define ROOM_VNUM_ALTAR            3054
#define ROOM_VNUM_SCHOOL           3700
#define ROOM_VNUM_DONATE           "3300"

/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK               (A)
#define ROOM_NO_MOB             (C)
#define ROOM_INDOORS            (D)
#define ROOM_NO_FLEE_TO         (E)
#define ROOM_ARENA              (F)
#define ROOM_PRIVATE            (J)
#define ROOM_SAFE               (K)
#define ROOM_SOLITARY           (L)
#define ROOM_PET_SHOP           (M)
#define ROOM_NO_RECALL          (N)
#define ROOM_IMP_ONLY           (O)
#define ROOM_GODS_ONLY          (P)
#define ROOM_HEROES_ONLY        (Q)
#define ROOM_NEWBIES_ONLY       (R)
#define ROOM_LAW                (S)
#define ROOM_DONATION           (T)
#define ROOM_NOTELEPORT         (U)
#define ROOM_MARK               (V)
#define ROOM_NOMAGIC            (W)
#define ROOM_ADMIN_ONLY         (X)
/*
 * Directions.
 * Used in #ROOMS.
 */
#define DIR_NORTH                     0
#define DIR_EAST                      1
#define DIR_SOUTH                     2
#define DIR_WEST                      3
#define DIR_UP                        4
#define DIR_DOWN                      5

/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR                     1
#define EX_CLOSED                     2
#define EX_LOCKED                     4
#define EX_PICKPROOF                  8
#define EX_PASSPROOF                 16
#define EX_HIDDEN                    32

/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE                   0
#define SECT_CITY                     1
#define SECT_FIELD                    2
#define SECT_FOREST                   3
#define SECT_HILLS                    4
#define SECT_MOUNTAIN                 5
#define SECT_WATER_SWIM               6
#define SECT_WATER_NOSWIM             7
#define SECT_UNUSED                   8
#define SECT_AIR                      9
#define SECT_DESERT                  10
#define SECT_MAX                     11

/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE                    -1
#define WEAR_LIGHT                    0
#define WEAR_FINGER_L                 1
#define WEAR_FINGER_R                 2
#define WEAR_NECK_1                   3
#define WEAR_NECK_2                   4
#define WEAR_BODY                     5
#define WEAR_HEAD                     6
#define WEAR_LEGS                     7
#define WEAR_FEET                     8
#define WEAR_HANDS                    9
#define WEAR_ARMS                    10
#define WEAR_SHIELD                  11
#define WEAR_ABOUT                   12
#define WEAR_WAIST                   13
#define WEAR_WRIST_L                 14
#define WEAR_WRIST_R                 15
#define WEAR_WIELD                   16
#define WEAR_HOLD                    17
#define WEAR_SECOND_WIELD            18
#define MAX_WEAR                     19

/*********** End Area Builder Values ***********/

/*
 * Conditions.
 */
#define COND_DRUNK                    0
#define COND_FULL                     1
#define COND_THIRST                   2

/*
 * Positions.
 */
#define POS_DEAD                      0
#define POS_MORTAL                    1
#define POS_INCAP                     2
#define POS_STUNNED                   3
#define POS_SLEEPING                  4
#define POS_RESTING                   5
#define POS_SITTING                   6
#define POS_FIGHTING                  7
#define POS_STANDING                  8

/*
 * ACT bits for players.
 */
#define PLR_IS_NPC              (A) /* Don't EVER set.      */
#define PLR_BOUGHT_PET          (B)

/* RT auto flags */
#define PLR_AUTOASSIST          (C)
#define PLR_AUTOEXIT            (D)
#define PLR_AUTOLOOT            (E)
#define PLR_AUTOSAC             (F)
#define PLR_AUTOGOLD            (G)
#define PLR_AUTOSPLIT           (H)
#define PLR_CONSENT              (I)
#define PLR_JAILED               (J)
#define PLR_BUILDING		 (K)
#define PLR_REMORT               (L)
#define PLR_REMORTING            (M)
/* RT personal flags */
#define PLR_HOLYLIGHT           (N)
#define PLR_WIZINVIS            (O)
#define PLR_CANLOOT             (P)
#define PLR_NOSUMMON            (Q)
#define PLR_NOFOLLOW            (R)
#define PLR_COLOR               (S)
/* Flag for ConsoleChar - Win32 - Zane */
#define PLR_CONSOLE             (aa)

#define PLR_AFK               (bb)
#define PLR_CAN_PROG            (cc)
#define PLR_CAN_EDIT            (dd)
#define PLR_NO_ANNOUNCE         (ee)

/* 2 bits reserved, U-V */
/* Permit from Banned sites flag */

#define PLR_PERMIT		(T)

/* penalty flags */
#define PLR_LOG                 (W)
#define PLR_DENY                (X)
#define PLR_FREEZE              (Y)
#define PLR_THIEF               (Z)
#define PLR_KILLER              (aa)

/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET              (A)
#define COMM_DEAF               (B)
#define COMM_NOWIZ              (C)
#define COMM_NOAUCTION          (D)
#define COMM_NOGOSSIP           (E)
#define COMM_NOQUESTION         (F)
#define COMM_NO_OOC             (G)
#define COMM_NOINFO             (H)
#define COMM_NOCLAN             (I)
#define COMM_SNOOP_CLAN         (J)
#define COMM_NOMUSIC            (K)
/* display flags */
#define COMM_COMPACT            (L)
#define COMM_BRIEF              (M)
#define COMM_PROMPT             (N)
#define COMM_COMBINE            (O)
#define COMM_TELNET_GA          (P)
#define COMM_NOSPOUSETALK       (Q)
#define COMM_NOADMIN            (R)
#define COMM_NOHERO             (S)
/* penalties */
#define COMM_NOEMOTE            (T)
#define COMM_NOSHOUT            (U)
#define COMM_NOTELL             (V)
#define COMM_NOCHANNELS         (W)
/* Console Visible flag for Win32 - Zane */
#define VIS_CONSOLE                             (X)
#define COMM_NOGOC              (Y)
#define COMM_TELLOFF            (Z)

/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX         16

/*
 * Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     1000

/*
 *  Target types.
 */
#define TAR_IGNORE                  0
#define TAR_CHAR_OFFENSIVE          1
#define TAR_CHAR_DEFENSIVE          2
#define TAR_CHAR_SELF               3
#define TAR_OBJ_INV                 4

/* These are needed for the ban.c file. -Lancelight */

#define BAN_SUFFIX              A
#define BAN_PREFIX              B
#define BAN_NEWBIES             C
#define BAN_ALL                 D
#define BAN_PERMIT              E
#define BAN_PERMANENT           F

/*
 * Object defined in limbo.are
 * Used in save.c to load objects that don't exist.
 */
#define OBJ_VNUM_DUMMY  1

#define MAX_DIR 6
#define NO_FLAG -99             /* Must not be used in flags or stats. */

/* You can define or not define TRACK_THOUGH_DOORS, above, depending on
   whether or not you want track to find paths which lead through closed
   or hidden doors.
*/

#define TRACK_THROUGH_DOORS
#define TRACK_IS_SKILL
/* #undef TRACK_IS_SKILL */

#define TRACK_NO_PATH          -1
#define TRACK_ALREADY_THERE    -2
#define TRACK_ERROR            -3

/*
 * Area flags.
 */
#define         AREA_NONE       0
#define         AREA_CHANGED    1   /* Area has been modified. */
#define         AREA_ADDED      2   /* Area has been added to. */
#define         AREA_LOADING    4   /* Used for counting in db.c */

/* Begin other config options.
   A large amount of thes options are turned off to start with.
   Next to each one is a brief description of what changes when
   the option is enabled or disabled.  When one of these options
   is changed, it is usually best to recompile all the object files
   and not just the ones affected.   - Kyle 
 */

/*
#define DISABLE_MESSAGE_QUEUEING
 */
/* This disables the add2queue and add2tell functions.
   If you enounter crashes after a certain amount of tells have been
   told, enable this option.
 */

/*
#define REQUIRE_EDIT_PERMISSION
 */
/* This actives a command called "setedit" and makes it so all players
   not at MAX_LEVEL cannot use the edit command unless an IMP uses setedit
   on them first.  Use this option if you don't want unauthorized building going on.
 */

/*
#define REQUIRE_MUDPROG_PERMISSION
 */
/* Same as REQUIRE_EDIT_PERMISSION, but only for MudProgs.  This way, only
   approved players can create mprogs.  This is useful to prevent crashes and 
   abuse of mprogs by some uneducated or beginning imms.  The IMP command is
   "setprog".
 */

/*
#define HEAVY_DEBUG
 */
/* This option uses a HUGE amount of disk access, so it will probably lag your mud
   unless you have extremely fast disk access time.  Enabling this causes the mud to
   record what it does before it does it in certain critical functions, namely interpret()
   and the XXXX_update functions.  If you do not have a debugger or your debugger is not
   producing usable results then try enabling this.  It will create a ####.last file in the
   log directory, where #### is the number of the current log file.  It is a small file
   and will be only 3 lines long at all times.  However, it will be updated over 30 times
   per second.  When the mud crashes, check the most recent ####.last file and it will tell
   you what the mud was doing at the time of the crash.  This alternative to log all will
   not spam the log files like log all, remains set over crashes, and will tell you if the
   mud crashes doing something besides interpreting a command.  -Kyle
 */

/*
#define SKILL_DEBUG
*/

/* A while back a line to send something to a character whenever their skills are
checked was inserted.   If you need that on for some reason, uncomment the above line
*/

#define ANNOUNCE_CONNECTIONS
/* Defining this sets the option to announce the entry into the mud of players via the
   info channel.  Wizinvis imms are not announced. */

#ifdef ANNOUNCE_CONNECTIONS
#define IMMS_CAN_HIDE
/* With this set, immediately after entering their password and while viweing the imotd,
   any immortal can type "hidden" to prevent themselves from being announced.
 */
#endif

/*
#define NO_MOB_NAMES
 */
/* With this defined, characters may not name themselves after mobs.
   note: this does not prevent a builder from naming a mob after a player
   and making that character unusable.
 */

/*
#define USE_MORGUE
#define ROOM_VNUM_MORGUE 3001
 */
/* Set this to send all player corpses to the morgue when a player dies.  Room
   3001 is the temple, so you should probably create your own morgue and set 
   ROOM_VNUM_MORGUE to its vnum.
 */

#define DAMAGE_BY_AMOUNT

/* The mud defaults to showing damage messages based on the amount of damage done.
   If you with to have the mud show damage messages based on the percentage of total HP taken off the mob/player rather than
   the amount done, then comment out #define DAMAGE_BY_AMOUNT.
 */

/*
#define SHOW_DAMAGE_TO_CHARS
*/

/* With this defined, characters see exactly how much damage they do to a mob.
   From experience, most players really like this feature.  However, many mud
   administrators do not like it.  It's your choice.
 */

#define NO_OLC_WEATHER
/* This option disables weather while a character is in string_append mode.
 */

/* Set FACTION_EDIT_LEVEL to whatever level you want imms to have access
 * to faction OLC */
#define FACTION_EDIT_LEVEL MAX_LEVEL

/* Set SOCIAL_EDIT_LEVEL to whatever level you want imms to have access
 * to social OLC */
#define SOCIAL_EDIT_LEVEL MAX_LEVEL

/* Set HELP_EDIT_LEVEL to whatever level you want imms to have access
 * to help OLC */
#define HELP_EDIT_LEVEL MAX_LEVEL

/* Set TODO_EDIT_LEVEL to whatever level you want imms to have access
 * to help OLC */
#define TODO_EDIT_LEVEL MAX_LEVEL

/* With this defined, the amount of faction change will be shown.  Comment
 * it out if you don't like it. */
#define FACTION_SHOW_CHANGE_AMOUNT

/* Set CFG_FACTION_INITIAL_VALUE to whatever you want players' factions to
 * default to if a default isn't found for the player's race. */
#define CFG_FACTION_INITIAL_VALUE 0

/* Set CFG_FACTION_MAX_VALUE and CFG_FACTION_MIN_VALUE to whatever you want.
 * (keep in mind that 32000 and -32000 are the absolute max and min due
 *  to use of short int in the code) */
#define CFG_FACTION_MAX_VALUE 1000
#define CFG_FACTION_MIN_VALUE -1000

/* Set this to 0 to use only text descriptions for factions
 * Set this to 1 to use both text and numeric descriptions for factions
 * Set this to 2 to use only numeric descriptions for factions */
#define CFG_FACTION_MESSAGES 1

/* Consider message for 0% faction */
#define CFG_FACTION_CON_0 "feels indifferent towards you"

/* Consider messages for factions (within percent of maximum) */
#define CFG_FACTION_CON_15MAX "feels relatively indifferent towards you"
#define CFG_FACTION_CON_30MAX "gives you a friendly look"
#define CFG_FACTION_CON_45MAX "looks your way amiably"
#define CFG_FACTION_CON_60MAX "appears to be delighted by your presence"
#define CFG_FACTION_CON_75MAX "smiles affectionately at you"
#define CFG_FACTION_CON_90MAX "feels a great kinship towards you"
#define CFG_FACTION_CON_MAX   "worships the ground you walk on"

/* Consider messages for factions (within percent of minimum) */
#define CFG_FACTION_CON_15MIN "feels relatively indifferent towards you"
#define CFG_FACTION_CON_30MIN "has a slight distaste for you"
#define CFG_FACTION_CON_45MIN "looks your way apprehesively"
#define CFG_FACTION_CON_60MIN "appears to be disgusted by your presence"
#define CFG_FACTION_CON_75MIN "sneers threateningly at you"
#define CFG_FACTION_CON_90MIN "has a great hatred for you"
#define CFG_FACTION_CON_MIN   "loathes you"

/* Set this to the faction percentage at which you want shopkeepers 
 * to refuse to deal with a person */
#define CFG_FACTION_SHOPKEEPER_ABSOLUTE_MIN -75

/* These multipliers affect the cost of an item based upon your faction
 * standing with the shopkeeper.  Note: If a shopkeeper gives someone
 * a discount when they buy from him, he'll also offer the same percent 
 * more when that person sells to him. */
#define CFG_FACTION_SHOPKEEPER_MIN   3  /* 200% mark-up  */
#define CFG_FACTION_SHOPKEEPER_90MIN 2.5    /* 150% mark-up  */
#define CFG_FACTION_SHOPKEEPER_75MIN 2  /* 100% mark-up  */
#define CFG_FACTION_SHOPKEEPER_60MIN 1.5    /*  50% mark-up  */
#define CFG_FACTION_SHOPKEEPER_45MIN 1.25   /*  25% mark-up  */
#define CFG_FACTION_SHOPKEEPER_30MIN 1.10   /*  10% mark-up  */
#define CFG_FACTION_SHOPKEEPER_15MIN 1  /*   no mark-up  */
#define CFG_FACTION_SHOPKEEPER_15MAX 1  /*   no discount */
#define CFG_FACTION_SHOPKEEPER_30MAX 0.90   /*  10% discount */
#define CFG_FACTION_SHOPKEEPER_45MAX 0.85   /*  15% discount */
#define CFG_FACTION_SHOPKEEPER_60MAX 0.80   /*  20% discount */
#define CFG_FACTION_SHOPKEEPER_75MAX 0.75   /*  25% discount */
#define CFG_FACTION_SHOPKEEPER_90MAX 0.65   /*  35% discount */
#define CFG_FACTION_SHOPKEEPER_MAX   0.65   /*  35% discount */

/* Define this to enable the shell wiz-command. */
#define CFG_SHELL_ENABLED
/* Set this if you want to restrict access to the shell wiz-command
 * trust >= CFG_SHELL_LEVEL will have access to the command. */
#define CFG_SHELL_LEVEL MAX_LEVEL
/* Set this to whatever you want do_shell to run. */
#define SHELL_PATH "/bin/bash"

/* This goes at the end :) */
#endif
