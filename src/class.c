/* Online setting of skill/spell levels, 
 * (c) 1996 Erwin S. Andreasen <erwin@pip.dknet.dk>
 *
 */

#if defined(WIN32)
#include <windows.h>
#include <time.h>
#else
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "interp.h"

DECLARE_DO_FUN( do_skill );

/* 

  Class table levels loading/saving
  
*/

/* Save this class */
void save_class( int num )
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    int lev, cp, i;

    sprintf( buf, "%s%s", sysconfig.class_dir, class_table[num].name );

    if ( !( fp = fopen( buf, "w" ) ) )
    {
        bug( "Could not open file %s in order to save class %s.", buf,
             class_table[num].name );
        return;
    }

    for ( lev = 0; lev < LEVEL_IMMORTAL; lev++ )
        for ( i = 0; i < MAX_SKILL; i++ )
        {
            if ( !skill_table[i].name || !skill_table[i].name[0] )
                continue;

            if ( skill_table[i].skill_level[num] == lev )
            {
                cp = skill_table[i].rating[num];
                fprintf( fp, "%d %d %s\n", lev, cp, skill_table[i].name );
            }
        }

    fprintf( fp, "-1" );        /* EOF -1 */
    fclose( fp );
}

void save_classes(  )
{
    int i;

    for ( i = 0; i < MAX_CLASS; i++ )
        save_class( i );
}

/* Load a class */
void load_class( int num )
{
    char buf[MAX_STRING_LENGTH];
    int level, cp, n;
    FILE *fp;

    sprintf( buf, "%s%s", sysconfig.class_dir, class_table[num].name );

    if ( !( fp = fopen( buf, "r" ) ) )
    {
        bug( "Could not open file %s in order to load class %s.", buf,
             class_table[num].name );
        return;
    }

    fscanf( fp, "%d %d", &level, &cp );

    while ( level != -1 )
    {
        fscanf( fp, " %[^\n]\n", buf ); /* read name of skill into buf */

        n = skill_lookup( buf );    /* find index */

        if ( n == -1 )
        {
            char buf2[200];
            sprintf( buf2, "Class %s: unknown spell %s", class_table[num].name,
                     buf );
            bug( buf2, 0 );
        }
        else
        {
            skill_table[n].skill_level[num] = level;
            skill_table[n].rating[num] = cp;
        }

        fscanf( fp, "%d %d", &level, &cp );
    }

    fclose( fp );
}

void load_classes(  )
{
    int i, j;

    for ( i = 0; i < MAX_CLASS; i++ )
    {
        for ( j = 0; j < MAX_SKILL; j++ )
            skill_table[j].skill_level[i] = LEVEL_IMMORTAL;

        load_class( i );
    }
}

void do_skill( CHAR_DATA * ch, char *argument )
{
    char class_type[4], skill_type[MAX_SKILL], lvl[MAX_LEVEL];
    int sn, level, cp, class_no;

    argument = one_argument( argument, class_type );
    argument = one_argument( argument, skill_type );

    if ( !argument[0] )
    {
        send_to_char( "Syntax is: ASKILL <class> <skill> <level> <cp>.\n\r",
                      ch );
        return;
    }

    argument = one_argument( argument, lvl );
    if ( !is_number( lvl ) )
    {
        printf_to_char( ch, "Level range is from 0 to %d.\n\r",
                        LEVEL_IMMORTAL );
        return;
    }

    level = atoi( lvl );
    cp = atoi( argument );

    if ( !is_number( argument ) || level <= 0 )
    {
        cp = 1;
    }

    if ( ( sn = skill_lookup( skill_type ) ) == -1 )
    {
        printf_to_char( ch, "There is no such spell/skill as '%s'.\n\r",
                        skill_type );
        return;
    }

    for ( class_no = 0; class_no < MAX_CLASS; class_no++ )
        if ( !str_cmp( class_type, class_table[class_no].who_name ) )
            break;

    if ( class_no >= MAX_CLASS )
    {
        printf_to_char( ch,
                        "No class named '%s' exists. Use the 3-letter WHO names (Thi, Mag etc.)\n\r",
                        class_type );
        return;
    }

    skill_table[sn].skill_level[class_no] = level;
    skill_table[sn].rating[class_no] = cp;

    printf_to_char( ch, "OK, %ss will now gain %s at level %d%s for %d cp.\n\r",
                    class_table[class_no].name, skill_table[sn].name, level,
                    level == LEVEL_IMMORTAL ? " (i.e. never)" : "", cp );

    save_classes(  );
}
