#include "newbits.h"

const struct newaff_table_struct newaff_table[MAX_NEWAFF_BIT] = {   /* This table MUST be kept in the correct order */
    {NEWAFF_GHOST_FLY, "ghostfly", FALSE},
    {NEWAFF_GHOST_NOCHAN, "ghostnochan", FALSE},
    {NEWAFF_GHOST_PASSDOOR, "ghostpassdoor", FALSE},
    {NEWAFF_GHOST_DETECTINVIS, "ghostdetectinvis", FALSE},
    {NEWAFF_GHOST, "ghost", FALSE},
    {NEWAFF_BLACKJACK, "blackjack", FALSE}
};

/* ************************************************************************ */

/* -------- New Bit Functions -------- */
int IS_NEWAFF_SET( char *b, int bit )
{
    if ( bit > MAX_NEWAFF_BIT )
        return FALSE;
    else
        return ( *( b + ( bit >> 3 ) ) >> ( 7 - ( bit & 7 ) ) & 1 );
}

void SET_NEWAFF( char *b, int bit )
{
    if ( bit <= MAX_NEWAFF_BIT )
        ( *( b + ( bit >> 3 ) ) |= 1 << ( 7 - ( bit & 7 ) ) );
}

void REMOVE_NEWAFF( char *b, int bit )
{
    if ( bit <= MAX_NEWAFF_BIT )
        ( *( b + ( bit >> 3 ) ) &= ~( 1 << ( 7 - ( bit & 7 ) ) ) );
}
