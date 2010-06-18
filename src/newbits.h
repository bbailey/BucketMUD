#ifndef __NEWBITS_H_INCLUDED__
#define __NEWBITS_H_INCLUDED__

#if     !defined(FALSE)
#define FALSE    0
#endif

#if     !defined(TRUE)
#define TRUE     1
#endif

#if     defined(_AIX)
#if     !defined(const)
#define const
#endif
#define unix
#else
#if !defined(CPP)
#endif
#endif

/* ---- New AFF Bits ---- */
#define MAX_NEWAFF_BIT              6
/* ---- */
#define NEWAFF_GHOST_FLY            1
#define NEWAFF_GHOST_NOCHAN         2
#define NEWAFF_GHOST_PASSDOOR       3
#define NEWAFF_GHOST_DETECTINVIS    4
#define NEWAFF_GHOST                5
#define NEWAFF_BLACKJACK            6

int IS_NEWAFF_SET( char *b, int bit );
void SET_NEWAFF( char *b, int bit );
void REMOVE_NEWAFF( char *b, int bit );

struct newaff_table_struct {
    int bit;
    char *name;
    int settable;
};

extern const struct newaff_table_struct newaff_table[MAX_NEWAFF_BIT];

#endif
