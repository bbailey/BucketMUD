/*
This code was written by Kyle Boyd, boyd1@proaxis.com, and
permission is given for it to be distributed ONLY in official
distributions of EmberMUD code or into existing EmberMUDs.
Usage of this code anywhere else will result in the offending
mud administrators being called names, as well as anything else
I can think of to do to them.
*/

#if defined(WIN32)

#include <windows.h>

#else

#include <unistd.h>

#include <sys/time.h>

#include <sys/resource.h>

#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
/* I just copied the header files list from db.c, don't know how many of these I don't need. */
#include "merc.h"
#include "db.h"
#include "interp.h"

#define 	TABLESIZE(a)	(sizeof(a)/sizeof(a)[0])
#define 	RAND_VNUM_ARMOR 	30
#define 	RAND_VNUM_RING	31
#define 	RAND_VNUM_LIGHT	32
#define 	RAND_VNUM_WEAPON	33
#define 	RAND_VNUM_BAG	34
#define 	RAND_VNUM_SCROLL	35
#define 	RAND_VNUM_WAND	36
#define 	RAND_VNUM_POTION	37
/* prefix types for armor table */
/* <a/an/some/(nothing)> <material name> <armor type> */
#define	PREF_A	1
#define	PREF_AN	2
#define	PREF_SOME	3
#define	PREF_NONE	4
#define	MSL	(MAX_STRING_LENGTH)

void wear_rand_obj args( ( CHAR_DATA * ch, OBJ_DATA * obj ) );
OBJ_DATA *make_rand_armor args( ( sh_int level, bool ismagic ) );
OBJ_DATA *make_random_obj args( ( sh_int level, long posible_types ) );
OBJ_DATA *make_rand_ring args( ( sh_int level ) );
OBJ_DATA *make_rand_weapon args( ( sh_int level, bool ismagic ) );
OBJ_DATA *make_rand_bag args( ( sh_int level ) );
/*
OBJ_DATA * make_rand_magic	args( ( sh_int level ) );
OBJ_DATA * make_rand_light	args( ( sh_int level ) );
*/
int get_random_material args( ( sh_int level ) );

void load_random_objs( CHAR_DATA * mob, MOB_INDEX_DATA * mobIndex )
{
    OBJ_DATA *obj;
    int actual_num;
    int i;

    if ( mobIndex->rnd_obj_types == 0 )
        return;
    actual_num = number_range( 1, mobIndex->rnd_obj_num );

    for ( i = 0; i < actual_num; i++ )
    {
        obj = make_random_obj( mob->level, mobIndex->rnd_obj_types );
        if ( obj != NULL )
        {
            obj_to_char( obj, mob );
            wear_rand_obj( mob, obj );
        }
        else
        {
            return;
        }
    }

}

OBJ_DATA *make_random_obj( sh_int level, long possible_type )
{
    int type = 0;
    if ( IS_SET( possible_type, ITEM_RAND_ANY ) )
    {
        switch ( number_range( 1, 8 ) )
        {
        case 1:
            type = ITEM_MAGIC_WEAPON;
            break;
        case 2:
        case 3:
            type = ITEM_PLAIN_ARMOR;
            break;
        case 4:
        case 5:
            type = ITEM_PLAIN_WEAPON;
            break;
        case 6:
            type = ITEM_MAGIC_ARMOR;
            break;
        case 7:
            type = ITEM_RAND_RING;
            break;
        case 8:
            type = ITEM_RAND_CONTAINER;
            break;
            /*  case 9: type = ITEM_MAGIC_ITEM; break;
               case 10: type = ITEM_RAND_LIGHT; break; */
        }
    }
    else
    {
        while ( type == 0 )
        {
            switch ( number_range( 1, 8 ) )
            {
            case 1:
                type = ITEM_MAGIC_WEAPON;
                break;
            case 2:
            case 3:
                type = ITEM_PLAIN_ARMOR;
                break;
            case 4:
            case 5:
                type = ITEM_PLAIN_WEAPON;
                break;
            case 6:
                type = ITEM_MAGIC_ARMOR;
                break;
            case 7:
                type = ITEM_RAND_RING;
                break;
            case 8:
                type = ITEM_RAND_CONTAINER;
                break;
                /*  case 9: type = ITEM_MAGIC_ITEM; break;
                   case 10: type = ITEM_RAND_LIGHT; break; */
            }
            if ( !IS_SET( possible_type, type ) )
                type = 0;
        }
    }
    switch ( type )
    {
    case ITEM_PLAIN_ARMOR:
        return make_rand_armor( level, FALSE );
    case ITEM_MAGIC_ARMOR:
        return make_rand_armor( level, TRUE );
    case ITEM_RAND_RING:
        return make_rand_ring( level );
/*	case ITEM_MAGIC_ITEM: return make_rand_armor( level, FALSE ); */
    case ITEM_PLAIN_WEAPON:
        return make_rand_weapon( level, FALSE );
    case ITEM_MAGIC_WEAPON:
        return make_rand_weapon( level, TRUE );
    case ITEM_RAND_CONTAINER:
        return make_rand_bag( level );
/*	case ITEM_RAND_LIGHT: return make_rand_ring( level ); */
    }
    log_string( "Returning NULL from make_random_obj." );
    return NULL;
}

/*
struct rand_bag_material
{
	char *	name;
	bool	isprefix;		
	sh_int	size;		
};
*/
const struct rand_bag_material rand_bagmaterial_table[] = {
    {"large", TRUE, 125},
    {"small", TRUE, 75},
    {"of holding", FALSE, 180},
    {"tiny", TRUE, 50},
    {"leather", TRUE, 100},
    {"felt", TRUE, 80},
    {"rough leather", TRUE, 110},
    {"cloth", TRUE, 90},
    {"of flexibility", FALSE, 160},
    {"with a large hole", FALSE, 25}
};
/*
struct rand_bag_type
{
	char * 	name;
	sh_int	capacity;
	long		wear_loc;	
};
*/
const struct rand_bag_type rand_bag_table[] = {
    {"bag", 15, ITEM_HOLD},
    {"bag", 20, ITEM_HOLD},
    {"bag", 20, ITEM_HOLD},
    {"bag", 10, ITEM_HOLD},
    {"satchel", 25, ITEM_HOLD | ITEM_WEAR_ABOUT},
    {"belt", 20, ITEM_HOLD | ITEM_WEAR_WAIST},
    {"backpack", 30, ITEM_WEAR_ABOUT},
    {"bag", 15, ITEM_HOLD},
    {"bag", 20, ITEM_HOLD},
    {"bag", 20, ITEM_HOLD},
    {"bag", 10, ITEM_HOLD},
    {"satchel", 25, ITEM_HOLD | ITEM_WEAR_ABOUT},
    {"belt", 20, ITEM_HOLD | ITEM_WEAR_WAIST},
    {"backpack", 30, ITEM_WEAR_ABOUT},
    {"fanny pack", 5, ITEM_WEAR_WAIST}  /* just for fun! */
};
/*
struct rand_light_type
{
	char *	name;
	sh_int	duration;
};
*/
/* I couldn't think of many of these... maybe someone else can add some? */
/*
const	struct rand_light_type rand_light_table[] =
{	
	{ "torch", 20 },
	{ "torch", 25 },
	{ "lantern", 35 },
	{ "lantern", 40 },
	{ "sceptre", -1 }
};
*/
/*
struct rand_weapon_type
{
	char *	name;
	sh_int	magic_mod;
	sh_int	damage_mod;
	sh_int	weapon_type;
};
*/
/*
WEAPON_EXOTIC
WEAPON_SWORD
WEAPON_DAGGER
WEAPON_SPEAR
WEAPON_MACE
WEAPON_AXE
WEAPON_FLAIL
WEAPON_WHIP
WEAPON_POLEARM
*/
const struct rand_weapon_type rand_weapon_table[] = {
    {"sword", 1, 1, WEAPON_SWORD},
    {"sword", 2, 1, WEAPON_SWORD},
    {"sword", 1, 2, WEAPON_SWORD},
    {"sword", 3, 1, WEAPON_SWORD},
    {"sword", 1, 3, WEAPON_SWORD},
    {"sword", 4, 1, WEAPON_SWORD},
    {"sword", 1, 4, WEAPON_SWORD},
    {"sword", 2, 3, WEAPON_SWORD},
    {"sword", 0, 5, WEAPON_SWORD},
    {"sword", 3, 3, WEAPON_SWORD},
    {"sword", 4, 2, WEAPON_SWORD},
    {"longsword", 2, 2, WEAPON_SWORD},
    {"longsword", 3, 3, WEAPON_SWORD},
    {"longsword", 3, 4, WEAPON_SWORD},
    {"longsword", 3, 5, WEAPON_SWORD},
    {"broadsword", 2, 6, WEAPON_SWORD},
    {"dagger", 1, 2, WEAPON_DAGGER},
    {"dagger", 1, 3, WEAPON_DAGGER},
    {"dagger", 1, 4, WEAPON_DAGGER},
    {"dagger", 2, 3, WEAPON_DAGGER},
    {"dagger", 3, 4, WEAPON_DAGGER},
    {"mace", 1, 1, WEAPON_MACE},
    {"mace", 2, 2, WEAPON_MACE},
    {"mace", 3, 2, WEAPON_MACE},
    {"mace", 5, 1, WEAPON_MACE},
    {"morningstar", 3, 2, WEAPON_MACE},
    {"spear", 2, 3, WEAPON_SPEAR},
    {"axe", 1, 4, WEAPON_AXE},
    {"flail", 2, 3, WEAPON_FLAIL},
    {"whip", 1, 4, WEAPON_WHIP},
    {"halberd", 0, 5, WEAPON_POLEARM},
    {"spear", 3, 3, WEAPON_SPEAR},
    {"trident", 4, 2, WEAPON_SPEAR},
    {"axe", 0, 5, WEAPON_AXE},
    {"battleaxe", 0, 6, WEAPON_AXE},
    {"flail", 3, 3, WEAPON_FLAIL},
    {"whip", 2, 4, WEAPON_WHIP},
    {"halberd", 0, 6, WEAPON_POLEARM}
};

/*
struct rand_item_material
{
	char *	name;
	sh_int	coolness;
	sh_int	magic_mod;
	sh_int	level;
};
*/

const struct rand_item_material rand_material_table[] = {
    /* ***IMPORTANT*** Never assign 2 materials to the same level. */
    /* don't put any more than 4 levels apart, either. */
    /* also put them in order by levels, worst materials at top, best at bottom */
    /* forgetting this shouldn't crash the mud, but it will get pretty messed up. */
    {"rusty", 0, 0, 0 * MAX_LEVEL / 100},
    {"wooden", 0, 1, 1 * MAX_LEVEL / 100},
    {"iron", 1, 0, 2 * MAX_LEVEL / 100},
    {"steel", 2, 1, 4 * MAX_LEVEL / 100},
    {"copper", 2, 2, 6 * MAX_LEVEL / 100},
    {"brass", 2, 3, 8 * MAX_LEVEL / 100},
    {"bronze", 3, 3, 10 * MAX_LEVEL / 100},
    {"silver", 4, 5, 14 * MAX_LEVEL / 100},
    {"gold", 5, 6, 18 * MAX_LEVEL / 100},
    {"bone", 3, 11, 22 * MAX_LEVEL / 100},
    {"platinum", 7, 8, 26 * MAX_LEVEL / 100},
    {"ebony", 9, 6, 30 * MAX_LEVEL / 100},
    {"chitonous", 9, 10, 34 * MAX_LEVEL / 100},
    {"mithril", 11, 11, 38 * MAX_LEVEL / 100},
    {"adamantium", 12, 12, 42 * MAX_LEVEL / 100},
    {"crystalline", 5, 17, 46 * MAX_LEVEL / 100},
    {"titanium", 13, 14, 50 * MAX_LEVEL / 100},
    {"emerald", 15, 15, 54 * MAX_LEVEL / 100},
    {"sapphire", 16, 16, 58 * MAX_LEVEL / 100},
    {"ivory", 17, 17, 62 * MAX_LEVEL / 100},
    {"ruby", 18, 18, 66 * MAX_LEVEL / 100},
    {"jade", 19, 19, 70 * MAX_LEVEL / 100},
    {"diamond", 20, 20, 74 * MAX_LEVEL / 100},
    {"dilithium", 21, 21, 78 * MAX_LEVEL / 100},
    {"awesome", 25, 25, 82 * MAX_LEVEL / 100},
    /* random objs really are not supposed to be this high... */
    /* change these to be more normal if you want to have them */
    /* I just ran out of material types. */
    {"supreme", 26, 26, 86 * MAX_LEVEL / 100},
    {"ultimate", 27, 27, 90 * MAX_LEVEL / 100},
    {"kick-ass", 30, 30, 94 * MAX_LEVEL / 100},
    {"supercalifragilistdexpealidosiousistic", 30, 30, 98}  /* he he he... */
};

/*
struct rand_ring_type
{
	char * name;
	sh_int coolness;	
	long   extra_flags;	
};
*/
/*
ITEM_GLOW	
ITEM_HUM	
ITEM_DARK	
ITEM_LOCK	
ITEM_EVIL	
ITEM_INVIS		
ITEM_MAGIC		
ITEM_NODROP		
ITEM_BLESS	
ITEM_ANTI_GOOD	
ITEM_ANTI_EVIL
ITEM_ANTI_NEUTRAL	
*/

const struct rand_ring_type rand_ring_table[] = {
    {"dirty", 0, 0},
    {"small", 1, 0},
    {"sticky", 1, ITEM_NODROP},
    {"glowing", 3, ITEM_GLOW},
    {"dark", 3, ITEM_DARK | ITEM_ANTI_GOOD},
    {"holy", 4, ITEM_GLOW | ITEM_BLESS | ITEM_ANTI_EVIL},
    {"transparent", 3, ITEM_INVIS},
    {"vibrating", 4, ITEM_HUM},
    {"large", 3, 0},
    {"smoky", 4, ITEM_DARK},
    {"flaming", 5, ITEM_GLOW},
    {"ancient", 5, ITEM_MAGIC},
    {"ornate", 6, ITEM_MAGIC},
    {"exquisite", 6, 0},
    {"dirty", 0, 0},
    {"scratched", 1, 0},
    {"broken", 1, 0},
    {"glowing", 2, ITEM_GLOW},
    {"square", 2, 0,},
    {"six sided", 4, ITEM_EVIL},
    {"cursed", 0, ITEM_NODROP},
    {"evil", 2, ITEM_EVIL},
    {"dusty", 1, 0},
    {"tarnished", 1, 0},
    {"heavy", 2, 0}
};

/*
struct rand_armor_type 
{
	char *	name;
	sh_int	wearloc;	
	sh_int	protect_mod;	
	sh_int	magic_mod;
	sh_int	p_type;
};
#define	PREF_A	1
#define	PREF_AN	2
#define	PREF_SOME	3
#define	PREF_NONE	4
*/

const struct rand_armor_type rand_armor_table[] = {
    {"helmet", ITEM_WEAR_HEAD, 5, 0, PREF_A},
    {"helmet", ITEM_WEAR_HEAD, 4, 0, PREF_A},
    {"helmet", ITEM_WEAR_HEAD, 5, 1, PREF_A},
    {"cap", ITEM_WEAR_HEAD, 2, 2, PREF_A},
    {"helm", ITEM_WEAR_HEAD, 4, 1, PREF_A},
    {"visor", ITEM_WEAR_HEAD, 4, 1, PREF_A},
    {"helm", ITEM_WEAR_HEAD, 4, 2, PREF_A},
    {"war helm", ITEM_WEAR_HEAD, 7, -1, PREF_A},
    {"hat", ITEM_WEAR_HEAD, 1, 2, PREF_A},
    {"crown", ITEM_WEAR_HEAD, 0, 3, PREF_A},
    {"cloak", ITEM_WEAR_ABOUT, 3, 1, PREF_A},
    {"cape", ITEM_WEAR_ABOUT, 3, 0, PREF_A},
    {"wrap", ITEM_WEAR_ABOUT, 2, 2, PREF_A},
    {"cloak", ITEM_WEAR_ABOUT, 4, 1, PREF_A},
    {"belt", ITEM_WEAR_WAIST, 2, 1, PREF_A},
    {"girth", ITEM_WEAR_WAIST, 2, 2, PREF_A},
    {"belt", ITEM_WEAR_WAIST, 3, 0, PREF_A},
    {"chain", ITEM_WEAR_WAIST, 3, -1, PREF_A},
    {"belt", ITEM_WEAR_WAIST, 3, 0, PREF_A},
    {"girth", ITEM_WEAR_WAIST, 2, 0, PREF_A},
    {"plate", ITEM_WEAR_BODY, 7, 0, PREF_A},
    {"shirt", ITEM_WEAR_BODY, 2, 3, PREF_A},
    {"breastplate", ITEM_WEAR_BODY, 8, 1, PREF_A},
    {"vest", ITEM_WEAR_BODY, 4, 2, PREF_A},
    {"jerkin", ITEM_WEAR_BODY, 4, 0, PREF_A},
    {"platemail", ITEM_WEAR_BODY, 8, 0, PREF_NONE},
    {"scalemail", ITEM_WEAR_BODY, 6, 0, PREF_NONE},
    {"chainmail", ITEM_WEAR_BODY, 7, 0, PREF_NONE},
    {"battlesuit", ITEM_WEAR_BODY, 9, -1, PREF_A},
    {"suit armor", ITEM_WEAR_BODY, 5, 1, PREF_NONE},
    {"robe", ITEM_WEAR_BODY, 2, 4, PREF_A},
    {"breastplate", ITEM_WEAR_BODY, 7, 1, PREF_A},
    {"plate", ITEM_WEAR_BODY, 6, 1, PREF_A},
    {"pants", ITEM_WEAR_LEGS, 3, 0, PREF_SOME},
    {"greaves", ITEM_WEAR_LEGS, 6, 0, PREF_SOME},
    {"leggings", ITEM_WEAR_LEGS, 3, 1, PREF_SOME},
    {"skirt", ITEM_WEAR_LEGS, 4, 0, PREF_A},
    {"boots", ITEM_WEAR_FEET, 4, 1, PREF_SOME},
    {"boots", ITEM_WEAR_FEET, 4, 0, PREF_SOME},
    {"boots", ITEM_WEAR_FEET, 3, 0, PREF_SOME},
    {"sandals", ITEM_WEAR_FEET, 1, 1, PREF_SOME},
    {"shoes", ITEM_WEAR_FEET, 2, 0, PREF_SOME},
    {"gloves", ITEM_WEAR_HANDS, 3, 1, PREF_SOME},
    {"gauntlets", ITEM_WEAR_HANDS, 4, 0, PREF_SOME},
    {"gloves", ITEM_WEAR_HANDS, 2, 2, PREF_SOME},
    {"gloves", ITEM_WEAR_HANDS, 3, 0, PREF_SOME},
    {"gauntlets", ITEM_WEAR_HANDS, 4, -1, PREF_SOME},
    {"sleeves", ITEM_WEAR_ARMS, 3, 0, PREF_SOME},
    {"vambrances", ITEM_WEAR_ARMS, 4, 0, PREF_SOME},
    {"sleeves", ITEM_WEAR_ARMS, 3, 2, PREF_SOME},
    {"sleeves", ITEM_WEAR_ARMS, 2, 1, PREF_SOME},
    {"shield", ITEM_WEAR_SHIELD, 5, 0, PREF_A},
    {"shield", ITEM_WEAR_SHIELD, 4, 1, PREF_A},
    {"shield", ITEM_WEAR_SHIELD, 4, 0, PREF_A},
    {"tower shield", ITEM_WEAR_SHIELD, 6, -1, PREF_A},
    {"buckler", ITEM_WEAR_WRIST, 4, 0, PREF_A},
    {"bracer", ITEM_WEAR_WRIST, 3, 0, PREF_A},
    {"bracelet", ITEM_WEAR_WRIST, 1, 3, PREF_A},
    {"armband", ITEM_WEAR_WRIST, 2, 2, PREF_AN},
    {"bracelet", ITEM_WEAR_WRIST, 1, 2, PREF_A},
    {"amulet", ITEM_WEAR_NECK, 1, 3, PREF_AN},
    {"necklace", ITEM_WEAR_NECK, 1, 1, PREF_A},
    {"necklace", ITEM_WEAR_NECK, 1, 2, PREF_A}
};

/*
 * Get the material appropriate for the level... 
 * Watch out if you modify this... I used a pretty funky algorithm 
 * Just trust me when I say it works, ok? 
 */
int get_random_material( sh_int level )
{
    int poss[37];
    int upper = 0, lower = -1, x, material;

    for ( x = 0; x <= 36; x++ )
    {
        poss[x] = -1;
    }
    for ( x = 0; x < TABLESIZE( rand_material_table ); x++ )
    {
        if ( rand_material_table[x].level <= level + 5
             && rand_material_table[x].level >= level - 5 )
        {
            if ( lower == -1 )
                lower = x;
            upper = x;
        }
    }
    x = ( rand_material_table[lower].level +
          rand_material_table[upper].level ) / 2;
    if ( x == rand_material_table[lower].level
         || x == rand_material_table[upper].level )
    {
        material = upper;
    }
    else
    {
        int y, z, p = 0, a;
        for ( y = lower; y < upper; y++ )
        {
            a = 0;
            if ( rand_material_table[y].level < x
                 && rand_material_table[y].level > x - 6 )
                for ( z = rand_material_table[y].level - x + 6; p < 36 && a < z;
                      p++ )
                {
                    poss[p] = y;
                    a++;
                }
            else if ( rand_material_table[y].level > x
                      && rand_material_table[y].level < x + 6 )
                for ( z = x + 6 - rand_material_table[y].level; p < 36 && a < z;
                      p++ )
                {
                    poss[p] = y;
                    a++;
                }
            else
                for ( z = 0; z < 6 && p < 36; z++ )
                {
                    poss[p] = y;
                    p++;
                }
        }
        for ( a = 0; poss[a] != -1; a++ );
        material = poss[number_range( 0, a - 1 )];
    }
    return URANGE( 0, material, TABLESIZE( rand_material_table ) - 1 );
}

OBJ_DATA *make_rand_bag( sh_int level )
{
    int type, style, x;
    char buf[MSL], buf2[MSL], buf3[MSL];
    OBJ_DATA *obj;
    type = number_range( 0, TABLESIZE( rand_bag_table ) - 1 );
    style = number_range( 0, TABLESIZE( rand_bagmaterial_table ) - 1 );
    obj = create_object( get_obj_index( RAND_VNUM_BAG ), level );

    if ( rand_bagmaterial_table[style].isprefix == TRUE )
    {
        sprintf( buf, "%s %s", rand_bagmaterial_table[style].name,
                 rand_bag_table[type].name );
        free_string( &obj->name );
        obj->name = str_dup( buf );
        if ( rand_bagmaterial_table[style].name[0] == 'a' ||
             rand_bagmaterial_table[style].name[0] == 'e' ||
             rand_bagmaterial_table[style].name[0] == 'i' ||
             rand_bagmaterial_table[style].name[0] == 'o' ||
             rand_bagmaterial_table[style].name[0] == 'u' )
            sprintf( buf2, "an %s %s", rand_bagmaterial_table[style].name,
                     rand_bag_table[type].name );
        else
            sprintf( buf2, "a %s %s", rand_bagmaterial_table[style].name,
                     rand_bag_table[type].name );
        free_string( &obj->short_descr );
        obj->short_descr = str_dup( buf2 );
        if ( rand_bagmaterial_table[style].name[0] == 'a' ||
             rand_bagmaterial_table[style].name[0] == 'e' ||
             rand_bagmaterial_table[style].name[0] == 'i' ||
             rand_bagmaterial_table[style].name[0] == 'o' ||
             rand_bagmaterial_table[style].name[0] == 'u' )
            sprintf( buf3, "An %s %s lies here.",
                     rand_bagmaterial_table[style].name,
                     rand_bag_table[type].name );
        else
            sprintf( buf3, "A %s %s lies here.",
                     rand_bagmaterial_table[style].name,
                     rand_bag_table[type].name );
        free_string( &obj->description );
        obj->description = str_dup( buf3 );
    }
    else
    {
        sprintf( buf, "%s %s", rand_bag_table[type].name,
                 rand_bagmaterial_table[style].name );
        free_string( &obj->name );
        obj->name = str_dup( buf );
        if ( rand_bag_table[type].name[0] == 'a' ||
             rand_bag_table[type].name[0] == 'e' ||
             rand_bag_table[type].name[0] == 'i' ||
             rand_bag_table[type].name[0] == 'o' ||
             rand_bag_table[type].name[0] == 'u' )
            sprintf( buf2, "an %s %s", rand_bag_table[type].name,
                     rand_bagmaterial_table[style].name );
        else
            sprintf( buf2, "a %s %s", rand_bag_table[type].name,
                     rand_bagmaterial_table[style].name );
        free_string( &obj->short_descr );
        obj->short_descr = str_dup( buf2 );
        if ( rand_bag_table[type].name[0] == 'a' ||
             rand_bag_table[type].name[0] == 'e' ||
             rand_bag_table[type].name[0] == 'i' ||
             rand_bag_table[type].name[0] == 'o' ||
             rand_bag_table[type].name[0] == 'u' )
            sprintf( buf3, "An %s %s lies here.", rand_bag_table[type].name,
                     rand_bagmaterial_table[style].name );
        else
            sprintf( buf3, "A %s %s lies here.", rand_bag_table[type].name,
                     rand_bagmaterial_table[style].name );
        free_string( &obj->description );
        obj->description = str_dup( buf3 );
    }
    obj->level = number_range( 0, level );
    obj->wear_flags = ( short ) ( ITEM_TAKE | rand_bag_table[type].wear_loc );
    x = rand_bag_table[type].capacity;
    x *= 100;
    x /= rand_bagmaterial_table[style].size;
    obj->value[0] = x;          /* hopefully this is the right obj value :) */
    return obj;
}

OBJ_DATA *make_rand_weapon( sh_int level, bool ismagic )
{
    int material, type;
    char buf[MSL], buf2[MSL], buf3[MSL];
    OBJ_DATA *obj;
    material = get_random_material( level );
    type = number_range( 0, TABLESIZE( rand_weapon_table ) - 1 );
    obj = create_object( get_obj_index( RAND_VNUM_WEAPON ), level );
    sprintf( buf, "%s %s", rand_material_table[material].name,
             rand_weapon_table[type].name );
    free_string( &obj->name );
    obj->name = str_dup( buf );

    if ( rand_material_table[material].name[0] == 'a' ||
         rand_material_table[material].name[0] == 'e' ||
         rand_material_table[material].name[0] == 'i' ||
         rand_material_table[material].name[0] == 'o' ||
         rand_material_table[material].name[0] == 'u' )
        sprintf( buf2, "an %s %s", rand_material_table[material].name,
                 rand_weapon_table[type].name );
    else
        sprintf( buf2, "a %s %s", rand_material_table[material].name,
                 rand_weapon_table[type].name );
    free_string( &obj->short_descr );
    obj->short_descr = str_dup( buf2 );

    if ( rand_material_table[material].name[0] == 'a' ||
         rand_material_table[material].name[0] == 'e' ||
         rand_material_table[material].name[0] == 'i' ||
         rand_material_table[material].name[0] == 'o' ||
         rand_material_table[material].name[0] == 'u' )
        sprintf( buf3, "An %s %s lies here.",
                 rand_material_table[material].name,
                 rand_weapon_table[type].name );
    else
        sprintf( buf3, "A %s %s lies here.", rand_material_table[material].name,
                 rand_weapon_table[type].name );
    free_string( &obj->description );
    obj->description = str_dup( buf3 );
    obj->level = level;
    obj->value[0] = rand_weapon_table[type].weapon_type;
    obj->value[1] = ( level / 10 ) + 2;
/*	obj->value[2] = number_range( 3, 3 +
rand_weapon_table[type].damage_mod );*/
    obj->value[2] = number_range( 3, 3 + level / 3 );
    obj->value[3] = 0;          /* every random weapon will just "hit" for now... bah */
    obj->value[4] = 0;          /* maybe later give the weapon a chance to be flaming, whatever */
    if ( ismagic == TRUE && rand_weapon_table[type].magic_mod > 0 )
    {
        /* I snatched this code from the freely distributed ROM random obj code(which sucks :),
         * but I couldn't find the name of who wrote it.  Anyone know? 
         */
        static int attrib_types[] =
            { APPLY_STR, APPLY_STR, APPLY_DEX, APPLY_INT, APPLY_STR, APPLY_WIS,
            APPLY_DEX, APPLY_DEX, APPLY_CON
        };
        static int power_types[] =
            { APPLY_MANA, APPLY_HIT, APPLY_HIT, APPLY_AC, APPLY_HIT };
        static int combat_types[] =
            { APPLY_HITROLL, APPLY_HITROLL, APPLY_DAMROLL, APPLY_DAMROLL };

        AFFECT_DATA *paf = alloc_perm( sizeof( *paf ) );
        paf->type = -999;
        paf->duration = -1;
        paf->bitvector = 0;
        paf->next = obj->affected;
        obj->affected = paf;
        switch ( number_bits( 2 ) )
        {
        case 0:
            paf->location =
                attrib_types[number_range( 0, TABLESIZE( attrib_types ) - 1 )];
            paf->modifier =
                number_range( 1, rand_weapon_table[type].magic_mod );
            break;
        case 1:
            paf->location =
                power_types[number_range( 0, TABLESIZE( power_types ) - 1 )];
            paf->modifier =
                UMAX( 1,
                      number_range( level / 2,
                                    level +
                                    rand_material_table[material].magic_mod ) );
            break;
        case 2:
        case 3:
            paf->location =
                combat_types[number_range( 0, TABLESIZE( combat_types ) - 1 )];
            paf->modifier = number_range( 1, level / 8 + 1 );
            break;
        }
        if ( paf->location == APPLY_AC )
            paf->modifier *= -1;
    }
    return obj;
}

OBJ_DATA *make_rand_ring( sh_int level )
{
    int material, type, x;
    char buf[MSL];
    OBJ_DATA *obj;
    static int attrib_types[] =
        { APPLY_STR, APPLY_STR, APPLY_DEX, APPLY_INT, APPLY_INT, APPLY_WIS,
        APPLY_DEX, APPLY_CON, APPLY_CON
    };
    static int power_types[] =
        { APPLY_MANA, APPLY_HIT, APPLY_MOVE, APPLY_AC, APPLY_HIT };
    static int combat_types[] =
        { APPLY_HITROLL, APPLY_HITROLL, APPLY_DAMROLL, APPLY_SAVING_SPELL,
        APPLY_DAMROLL
    };
    AFFECT_DATA *paf = alloc_perm( sizeof( *paf ) );
    material = get_random_material( level );
    type = number_range( 0, TABLESIZE( rand_ring_table ) - 1 );
    obj = create_object( get_obj_index( RAND_VNUM_RING ), level );
    sprintf( buf, "%s %s ring", rand_material_table[material].name,
             rand_ring_table[type].name );
    free_string( &obj->name );
    obj->name = str_dup( buf );

    if ( rand_ring_table[type].name[0] == 'a' ||
         rand_ring_table[type].name[0] == 'e' ||
         rand_ring_table[type].name[0] == 'i' ||
         rand_ring_table[type].name[0] == 'o' ||
         rand_ring_table[type].name[0] == 'u' )
        sprintf( buf, "an %s %s ring", rand_ring_table[type].name,
                 rand_material_table[material].name );
    else
        sprintf( buf, "a %s %s ring", rand_ring_table[type].name,
                 rand_material_table[material].name );
    free_string( &obj->short_descr );
    obj->short_descr = str_dup( buf );
    if ( rand_ring_table[type].name[0] == 'a' ||
         rand_ring_table[type].name[0] == 'e' ||
         rand_ring_table[type].name[0] == 'i' ||
         rand_ring_table[type].name[0] == 'o' ||
         rand_ring_table[type].name[0] == 'u' )
        sprintf( buf, "An %s %s ring lies on the ground here.",
                 rand_ring_table[type].name,
                 rand_material_table[material].name );
    else
        sprintf( buf, "A %s %s ring lies on the ground here.",
                 rand_ring_table[type].name,
                 rand_material_table[material].name );

    free_string( &obj->description );
    obj->description = str_dup( buf );
    for ( x = 0; x < 3; x++ )
        obj->value[x] =
            ( rand_material_table[material].coolness + ( level / 5 ) +
              rand_ring_table[type].coolness ) / 10;
    obj->value[3] =
        ( rand_material_table[material].coolness + ( level / 5 ) +
          rand_ring_table[type].coolness ) / 4;
    obj->extra_flags = rand_ring_table[type].extra_flags;
    obj->level = level;
    paf->type = -999;
    paf->duration = -1;
    paf->bitvector = 0;
    paf->next = obj->affected;
    obj->affected = paf;
    switch ( number_bits( 2 ) )
    {
    case 0:
        paf->location =
            attrib_types[number_range( 0, TABLESIZE( attrib_types ) - 1 )];
        paf->modifier =
            number_range( 1, 1 + ( rand_ring_table[type].coolness / 2 ) );
        break;
    case 1:
        paf->location =
            power_types[number_range( 0, TABLESIZE( power_types ) - 1 )];
        if ( paf->location == APPLY_AC )
            paf->modifier *= -1;
        paf->modifier =
            UMAX( 1,
                  number_range( level / 2,
                                level +
                                rand_material_table[material].magic_mod ) );
        break;
    case 2:
    case 3:
        paf->location =
            combat_types[number_range( 0, TABLESIZE( combat_types ) - 1 )];
        paf->modifier =
            number_range( 1,
                          ( level / 8 ) +
                          ( rand_ring_table[type].coolness / 3 ) + 1 );
        break;
    }
    return obj;
}

OBJ_DATA *make_rand_armor( sh_int level, bool ismagic )
{
    OBJ_DATA *obj;
    char buf[MSL];
    char logbuf[MSL];
    int x, material, type = 0;
    material = get_random_material( level );
    type = number_range( 0, TABLESIZE( rand_armor_table ) - 1 );
    obj = create_object( get_obj_index( RAND_VNUM_ARMOR ), level );
    sprintf( buf, "%s %s", rand_material_table[material].name,
             rand_armor_table[type].name );
    free_string( &obj->name );
    obj->name = str_dup( buf );

    if ( ( rand_material_table[material].name[0] == 'a' ||
           rand_material_table[material].name[0] == 'e' ||
           rand_material_table[material].name[0] == 'i' ||
           rand_material_table[material].name[0] == 'o' ||
           rand_material_table[material].name[0] == 'u' ) &&
         rand_armor_table[type].p_type == PREF_A )
        sprintf( buf, "an %s %s", rand_material_table[material].name,
                 rand_armor_table[type].name );
    else
        switch ( rand_armor_table[type].p_type )
        {
        case PREF_A:
            sprintf( buf, "a %s %s", rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        case PREF_NONE:
            sprintf( buf, "%s %s", rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        case PREF_SOME:
            sprintf( buf, "some %s %s", rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        }
    free_string( &obj->short_descr );
    obj->short_descr = str_dup( buf );

    if ( ( rand_material_table[material].name[0] == 'a' ||
           rand_material_table[material].name[0] == 'e' ||
           rand_material_table[material].name[0] == 'i' ||
           rand_material_table[material].name[0] == 'o' ||
           rand_material_table[material].name[0] == 'u' ) &&
         rand_armor_table[type].p_type == PREF_A )
        switch ( number_range( 1, 3 ) )
        {
        case 1:
            sprintf( buf, "An %s %s lies on the ground here.",
                     rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        case 2:
            sprintf( buf, "An %s %s has been dropped here.",
                     rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        case 3:
            sprintf( buf, "An %s %s is resting here.",
                     rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        }
    else if ( rand_armor_table[type].p_type == PREF_A )
    {
        switch ( number_range( 1, 3 ) )
        {
        case 1:
            sprintf( buf, "A %s %s lies on the ground here.",
                     rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        case 2:
            sprintf( buf, "A %s %s has been dropped here.",
                     rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        case 3:
            sprintf( buf, "A %s %s is resting here.",
                     rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        }
    }
    else if ( rand_armor_table[type].p_type == PREF_SOME )
    {
        switch ( number_range( 1, 4 ) )
        {
        case 1:
            sprintf( buf, "Some %s %s lie on the ground here.",
                     rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        case 2:
            sprintf( buf, "Some %s made out of %s lie here.",
                     rand_armor_table[type].name,
                     rand_material_table[material].name );
            break;
        case 3:
            sprintf( buf, "Some %s %s are resting here.",
                     rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        case 4:
            sprintf( buf, "Some %s %s have been dropped here.",
                     rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        }
    }
    else if ( rand_armor_table[type].p_type == PREF_NONE )
    {
        switch ( number_range( 1, 3 ) )
        {
        case 1:
            sprintf( buf, "%s %s lies on the ground here.",
                     rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        case 2:
            sprintf( buf, "%s %s is resting here.",
                     rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        case 3:
            sprintf( buf, "%s %s has been dropped here.",
                     rand_material_table[material].name,
                     rand_armor_table[type].name );
            break;
        }
    }
    else if ( rand_armor_table[type].p_type == PREF_AN )
        sprintf( buf, "An %s made of %s lies here.",
                 rand_armor_table[type].name,
                 rand_material_table[material].name );
    else
    {
        sprintf( buf,
                 "Buggy long description.  Tell an IMP if you see this description." );
        sprintf( logbuf, "make_rand_obj Long Desc bug, material=%s, type=%s. ",
                 rand_material_table[material].name,
                 rand_armor_table[type].name );
        log_string( logbuf );
    }
    free_string( &obj->description );
    buf[0] = UPPER( buf[0] );
    obj->description = str_dup( buf );
    for ( x = 0; x < 3; x++ )
        obj->value[x] =
            ( rand_material_table[material].coolness +
              rand_armor_table[type].protect_mod + ( level / 2 ) ) / 4;
    obj->value[3] =
        ( rand_material_table[material].coolness +
          rand_armor_table[type].protect_mod + ( level / 2 ) ) / 6;
    obj->level =
        UMIN( level, number_fuzzy( rand_material_table[material].level ) );
    obj->wear_flags = rand_armor_table[type].wearloc | ITEM_TAKE;

    if ( ismagic == TRUE && rand_armor_table[type].magic_mod > 0 )
    {
        /* I snatched this code from the freely distributed ROM random obj code(which sucks :),
         * but I couldn't find the name of who wrote it.  Anyone know? 
         */
        static int attrib_types[] =
            { APPLY_STR, APPLY_STR, APPLY_DEX, APPLY_INT,
            APPLY_INT, APPLY_WIS, APPLY_DEX, APPLY_CON, APPLY_CON
        };
        static int power_types[] =
            { APPLY_MANA, APPLY_HIT, APPLY_MOVE, APPLY_AC, APPLY_HIT };
        static int combat_types[] =
            { APPLY_HITROLL, APPLY_HITROLL, APPLY_DAMROLL,
            APPLY_SAVING_SPELL, APPLY_DAMROLL
        };

        AFFECT_DATA *paf = alloc_perm( sizeof( *paf ) );
        paf->type = -999;
        paf->duration = -1;
        paf->bitvector = 0;
        paf->next = obj->affected;
        obj->affected = paf;
        switch ( number_bits( 2 ) )
        {
        case 0:
            paf->location =
                attrib_types[number_range( 0, TABLESIZE( attrib_types ) - 1 )];
            paf->modifier = 1;
            break;
        case 1:
            paf->location =
                power_types[number_range( 0, TABLESIZE( power_types ) - 1 )];
            paf->modifier =
                UMAX( 1,
                      number_range( level / 2,
                                    level +
                                    rand_material_table[material].magic_mod ) );
            break;
        case 2:
        case 3:
            paf->location =
                combat_types[number_range( 0, TABLESIZE( combat_types ) - 1 )];
            paf->modifier = number_range( 1, level / 8 + 1 );
            break;
            if ( paf->location == APPLY_AC )
                paf->modifier *= -1;
        }
    }
    return obj;

}

/* Essentially the "wear" command with all the act(whatever) taken out */
void wear_rand_obj( CHAR_DATA * ch, OBJ_DATA * obj )
{
    if ( ch->level < obj->level )
        return;
    if ( obj->item_type == ITEM_LIGHT )
    {
        if ( get_eq_char( ch, WEAR_LIGHT ) != NULL )
            return;
        equip_char( ch, obj, WEAR_LIGHT );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) )
    {
        if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
             && get_eq_char( ch, WEAR_FINGER_R ) != NULL )
            return;

        if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL )
        {
            equip_char( ch, obj, WEAR_FINGER_L );
            return;
        }

        if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )
        {
            equip_char( ch, obj, WEAR_FINGER_R );
            return;
        }

        bug( "Wear_obj_random: no free finger.", 0 );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) )
    {
        if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
             && get_eq_char( ch, WEAR_NECK_2 ) != NULL )
            return;

        if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL )
        {
            equip_char( ch, obj, WEAR_NECK_1 );
            return;
        }

        if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL )
        {
            equip_char( ch, obj, WEAR_NECK_2 );
            return;
        }

        bug( "Wear_obj: no free neck.", 0 );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) )
    {
        if ( get_eq_char( ch, WEAR_BODY ) != NULL )
            return;
        equip_char( ch, obj, WEAR_BODY );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) )
    {
        if ( get_eq_char( ch, WEAR_HEAD ) != NULL )
            return;
        equip_char( ch, obj, WEAR_HEAD );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) )
    {
        if ( get_eq_char( ch, WEAR_LEGS ) != NULL )
            return;
        equip_char( ch, obj, WEAR_LEGS );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) )
    {
        if ( get_eq_char( ch, WEAR_FEET ) != NULL )
            return;
        equip_char( ch, obj, WEAR_FEET );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) )
    {
        if ( get_eq_char( ch, WEAR_HANDS ) != NULL )
            return;
        equip_char( ch, obj, WEAR_HANDS );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) )
    {
        if ( get_eq_char( ch, WEAR_ARMS ) != NULL )
            return;
        equip_char( ch, obj, WEAR_ARMS );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) )
    {
        if ( get_eq_char( ch, WEAR_ABOUT ) != NULL )
            return;
        equip_char( ch, obj, WEAR_ABOUT );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) )
    {
        if ( get_eq_char( ch, WEAR_WAIST ) != NULL )
            return;
        equip_char( ch, obj, WEAR_WAIST );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) )
    {
        if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
             && get_eq_char( ch, WEAR_WRIST_R ) != NULL )
            return;

        if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL )
        {
            equip_char( ch, obj, WEAR_WRIST_L );
            return;
        }

        if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL )
        {
            equip_char( ch, obj, WEAR_WRIST_R );
            return;
        }

        bug( "Wear_obj: no free wrist.", 0 );
        return;
    }

    if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) )
    {
        OBJ_DATA *weapon;
        OBJ_DATA *second_weapon;

        if ( get_eq_char( ch, WEAR_SHIELD ) != NULL )
            return;

        weapon = get_eq_char( ch, WEAR_WIELD );
        second_weapon = get_eq_char( ch, WEAR_SECOND_WIELD );
        if ( ( ( weapon != NULL ) && ( ch->size < SIZE_LARGE )
               && IS_WEAPON_STAT( weapon, WEAPON_TWO_HANDS ) )
             || ( weapon != NULL && ( second_weapon != NULL
                                      || get_eq_char( ch,
                                                      WEAR_HOLD ) != NULL ) ) )
            return;

        equip_char( ch, obj, WEAR_SHIELD );
        return;
    }
/* this part is buggy, have not taken the time to figure out why yet */
/* so for now mobs will not wield their random weapons. */
/*
    if ( CAN_WEAR( obj, ITEM_WIELD ) )
    {
	
	if ( !remove_obj(ch, WEAR_SECOND_WIELD, TRUE) )
	    return;

	
	if ((!IS_NPC(ch) && ch->size < SIZE_LARGE 
	&&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
 	&&  (( get_eq_char(ch,WEAR_SECOND_WIELD) !=NULL
 	|| get_eq_char(ch,WEAR_SHIELD) != NULL) 
 	|| get_eq_char(ch,WEAR_WIELD) != NULL))
 	|| (get_eq_char(ch,WEAR_WIELD) != NULL
 	&& (get_eq_char(ch,WEAR_SHIELD) != NULL
 	|| get_eq_char(ch,WEAR_SECOND_WIELD) !=NULL
 	|| get_eq_char(ch,WEAR_HOLD) != NULL))
 	|| (get_eq_char(ch,WEAR_SHIELD) != NULL
 	&& get_eq_char(ch,WEAR_HOLD) != NULL))
	{
	    if (get_eq_char(ch,WEAR_WIELD) != NULL)
	       if ( get_eq_char( ch, WEAR_SECOND_WIELD )  != NULL )
		return;
	    return;
	}

	equip_char( ch, obj, WEAR_WIELD );
            }
	else if (get_eq_char(ch,WEAR_SECOND_WIELD) == NULL) {
	    equip_char( ch, obj, WEAR_SECOND_WIELD);
            }

        	return;
    }
*/
    if ( CAN_WEAR( obj, ITEM_HOLD ) )
    {
        if ( get_eq_char( ch, WEAR_HOLD ) != NULL )
            return;
        if ( get_eq_char( ch, WEAR_WIELD ) != NULL
             && ( get_eq_char( ch, WEAR_SECOND_WIELD ) != NULL
                  || get_eq_char( ch, WEAR_SHIELD ) != NULL ) )
        {
            return;
        }
        equip_char( ch, obj, WEAR_HOLD );
        return;
    }
    return;
}
