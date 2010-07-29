
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <glib.h>

#include "merc.h"
#include "olc.h"


GList *help_entries = NULL;

const struct olc_cmd_type helpsedit_table[] =
{
    {"show", helpsedit_show},
    {"create", helpsedit_create},
    {"delete", helpsedit_delete},
    {"keyword", helpsedit_keyword},
    {"level", helpsedit_level},
    {"text", helpsedit_text},
    {"?", show_help},
    {"version", show_version},
    {"commands", show_commands},
    {"", 0,}
};

HELP_DATA *new_help(void)
{
    HELP_DATA *pHelp;

    pHelp = (HELP_DATA *) malloc(sizeof(HELP_DATA));

    if (!pHelp)
    {
        bug("new help: Call to alloc_perm failed!", 0);
        exit(EXIT_FAILURE);
    }

    pHelp->keyword = str_dup("CHANGE_ME!");
    pHelp->level = 0;
    pHelp->text =
        str_dup
        ("Someone didnt change the text for this help, the bonheads.");

    return pHelp;
}

void free_help(HELP_DATA *pHelp)
{
    free_string(&pHelp->keyword);
    free_string(&pHelp->text);
    pHelp->level = 0;
    free(pHelp);
}

HELP_DATA *get_help(char *argument)
{
    GList *help_iter;
    HELP_DATA *pHelp = NULL;
    bool found = false;
    char arg[MAX_INPUT_LENGTH];
    argument = one_argument(argument, arg);

    for (help_iter = g_list_first(help_entries); help_iter != NULL; help_iter = g_list_next(help_iter))
    {
        pHelp = (HELP_DATA *) help_iter->data;
        if (str_cmp(arg,pHelp->keyword))
        {
            found = true;
            break;
        }
    }

    if (found)
        return pHelp;
    else
        return NULL;
}

void do_helpedit(CHAR_DATA * ch, char *argument)
{
    GList *help_iter;
    HELP_DATA *pHelp;
    char argall[MAX_INPUT_LENGTH], argone[MAX_INPUT_LENGTH];

    /* this parts handles help a b so that it returns help 'a b' */
    argall[0] = '\0';
    while (argument[0] != '\0')
    {
        argument = one_argument(argument, argone);
        if (argall[0] != '\0')
            strcat(argall, " ");
        strcat(argall, argone);
    }

    if (IS_NPC(ch))
    {
        send_to_char("Why does an NPC need to write helps?!\n\r", ch);
        return;
    }

    if (get_trust(ch) < HELP_EDIT_LEVEL)
    {
        send_to_char("Your not powerful enough to do this.\n\r", ch);
        return;
    }

    if (argall[0] == '\0')
    {
        send_to_char
        ("Syntax: edit help createhelp\nSyntax: edit help <keyword>\n\r",
         ch);
        return;
    }

    if (is_name(argall, "createhelp"))
    {
        if (helpsedit_create(ch, argument))
        {
            ch->desc->editor = ED_HELPOLC;
            SET_BIT(ch->act, PLR_BUILDING);
            act("$n has entered the Help Editor.", ch, NULL, NULL,
                TO_ROOM);
        }

        return;
    }

    for (help_iter = g_list_first(help_entries); help_iter != NULL; help_iter = g_list_next(help_iter))
    {
        pHelp = (HELP_DATA *) help_iter->data;

        if (is_name(argall, pHelp->keyword))
        {
            ch->desc->pEdit = (void *) pHelp;
            ch->desc->editor = ED_HELPOLC;
            SET_BIT(ch->act, PLR_BUILDING);
            act("$n has entered the Help Editor.", ch, NULL, NULL,
                TO_ROOM);
            return;
        }
    }

    send_to_char("There is no default help to edit.\n\r", ch);
    return;
}

void helpsedit(CHAR_DATA * ch, char *argument)
{
    HELP_DATA *pHelp;
    char command[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int cmd;

    smash_tilde(argument);
    strncpy(arg, argument, sizeof(arg) - 1);
    arg[sizeof(arg)] = '\0';
    argument = one_argument(argument, command);

    pHelp = (HELP_DATA *) ch->desc->pEdit;

    if (!str_cmp(command, "done"))
    {
        edit_done(ch);
        do_asave(ch, "helps");
        return;
    }

    if (get_trust(ch) < HELP_EDIT_LEVEL)
    {
        send_to_char("Insufficient security to modify helps.\n\r", ch);
        interpret(ch, arg);
        return;
    }

    if (command[0] == '\0')
    {
        helpsedit_show(ch, argument);
        return;
    }

    if (!pHelp)
    {
        send_to_char("You are not currently editing a help.\n\r", ch);
        interpret(ch, arg);
        return;
    }

    /* Search table and dispatch command */
    for (cmd = 0; helpsedit_table[cmd].name[0] != '\0'; cmd++)
    {
        if (!str_prefix(command, helpsedit_table[cmd].name))
        {
            if ((*helpsedit_table[cmd].olc_fun) (ch, argument))
            {
                return;
            }
            else
            {
                return;
            }
        }
    }

    /* If command not found, default to the standard interpreter. */
    interpret(ch, arg);
    return;
}

bool helpsedit_show(CHAR_DATA * ch, char *argument)
{
    HELP_DATA *pHelp;

    pHelp = (HELP_DATA *) ch->desc->pEdit;

    if (!pHelp)
    {
        send_to_char("You are not currently editing a help.\n\r", ch);
    }
    else
    {
        printf_to_char(ch, "Keywords:             %s\n\r", pHelp->keyword);
        printf_to_char(ch, "Level: %d\n\r", pHelp->level);
        printf_to_char(ch,
                       "Text: Type text to see the text (Some are VERY long, so they are not shown here.)");
    }

    return FALSE;
}

bool helpsedit_create(CHAR_DATA * ch, char *argument)
{
    HELP_DATA *pHelp = new_help();

    if (!pHelp)
    {
        send_to_char("ERROR!  Could not create a new help!\n\r", ch);
        return FALSE;
    }

    send_to_char("Help created.\n\r", ch);

    ch->desc->pEdit = (void *) pHelp;

    help_entries = g_list_append(help_entries, pHelp);

    return TRUE;
}

bool helpsedit_keyword(CHAR_DATA * ch, char *argument)
{
    HELP_DATA *pHelp;

    pHelp = (HELP_DATA *) ch->desc->pEdit;

    if (!pHelp)
    {
        send_to_char("You are not currently editing a help.\n\r", ch);
        return FALSE;
    }

    if (argument[0] == '\0')
    {
        send_to_char("Syntax:  keyword <keywords for this help>\n\r", ch);
        return FALSE;
    }

    if (strlen(argument) != str_len(argument))
    {
        send_to_char
        ("ERROR!  Color codes are not allowed in keywords.\n\r", ch);
        return FALSE;
    }

    if (strlen(argument) > 60)
    {
        send_to_char("ERROR! Keywords must be 60 characters or less.\n\r",
                     ch);
        return FALSE;
    }

    free_string(&pHelp->keyword);
    pHelp->keyword = str_dup(argument);

    send_to_char("Help keywords set.\n\r", ch);

    return TRUE;
}

bool helpsedit_level(CHAR_DATA * ch, char *argument)
{
    HELP_DATA *pHelp;

    pHelp = (HELP_DATA *) ch->desc->pEdit;

    if (!pHelp)
    {
        send_to_char("You are not currently editing a help.\n\r", ch);
        return FALSE;
    }

    if (argument[0] == '\0' || !is_number(argument))
    {
        send_to_char("Syntax:  level [number]\n\r", ch);
        return FALSE;
    }

    pHelp->level = atoi(argument);

    send_to_char("Level set.\n\r", ch);
    return TRUE;

}

bool helpsedit_text(CHAR_DATA * ch, char *argument)
{
    HELP_DATA *pHelp;

    pHelp = (HELP_DATA *) ch->desc->pEdit;

    if (!pHelp)
    {
        send_to_char("You are not currently editing a help.\n\r", ch);
        return FALSE;
    }
    if (argument[0] == '\0')
    {
        string_append(ch, &pHelp->text);
        return TRUE;
    }

    send_to_char("Syntax:  text    (No arguments allowed)\n\r", ch);
    return FALSE;
}

bool helpsedit_delete(CHAR_DATA * ch, char *argument)
{
    HELP_DATA *pHelp;

    pHelp = (HELP_DATA *) ch->desc->pEdit;

    if (!pHelp)
    {
        send_to_char("You are not currently editing a help.\r\n", ch);
        return FALSE;
    }

    if (argument[0] == '\0')
    {
        help_entries = g_list_remove_all(help_entries, pHelp);
        free_help(pHelp);
        pHelp = NULL;
        send_to_char("You have deleted the current helpfile.\r\n", ch);
        edit_done(ch);
        return TRUE;
    }

    send_to_char("Syntax:  delete  (No arguments allowed)\r\n", ch);
    return FALSE;
}
