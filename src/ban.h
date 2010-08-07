#include "BitVector.h"
#include "BitVectorStr.h"

typedef struct ban_data
{
    struct ban_data *next;
    BitVector *bv_flags;
    int level;
    char *name;
} BAN_DATA;

typedef enum
{
    BV_BAN_SUFFIX = 0,
    BV_BAN_PREFIX = 1,
    BV_BAN_NEWBIES = 2,
    BV_BAN_ALL = 3,
    BV_BAN_PERMIT = 4,
    BV_BAN_PERMANENT = 5,
    BV_BAN_MAX
} BV_BAN_FLAG;

extern const BitVectorStringList bv_str_list_ban[];
extern BAN_DATA *ban_list;
extern BAN_DATA *new_ban(void);
extern void free_ban(BAN_DATA *ban);
