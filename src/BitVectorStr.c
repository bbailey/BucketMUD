/*
 * BitVectorStr.c
 *
 * Copyright (c) 2010 Bobby Bailey
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdbool.h>
#include <glib.h>
#include "BitVector.h"
#include "BitVectorStr.h"

gchar *bv_to_string(const BitVector *bv, const BitVectorStringList *bv_str_list)
{
    gchar *output = NULL;
    GString *tmp_output = g_string_new("");
    int index = 0;

    for (index = 0; bv_str_list[index].name_ != NULL; index++)
    {
        if (bv_is_set(bv, bv_str_list[index].bit_))
            g_string_append_printf(tmp_output, "%s ", bv_str_list[index].name_);
    }

    if (!g_strcasecmp(tmp_output->str, ""))
    {
        output = g_strdup("none");
    }
    else
    {
        output = g_strdup(tmp_output->str);
    }
    g_string_free(tmp_output, true);

    return output;
}

gsize bv_bit_from_string(const BitVectorStringList *bv_str_list, const gchar *bv_str)
{
    int index = 0;

    for (index = 0; bv_str_list[index].name_ != NULL; index++)
    {
        if (g_ascii_strcasecmp(bv_str_list[index].name_, bv_str) == 0)
            return bv_str_list[index].bit_;
    }

    return -1;
}

void bv_from_string(BitVector *bv, const BitVectorStringList *bv_str_list, const gchar *bv_str, BitVectorStringOp bv_str_op)
{
    GError *err = NULL;
    GRegex *bv_str_regex;
    GMatchInfo *match_info;
    gsize which_bit = 0;

    if (bv_str == NULL)
        return;

    bv_str_regex = g_regex_new("\\W*(\\w+)\\W*", 0, 0, &err);
    g_regex_match(bv_str_regex, bv_str, 0, &match_info);
    while (g_match_info_matches(match_info))
    {
        gchar *word = g_match_info_expand_references(match_info, "\\1", &err);
        which_bit = bv_bit_from_string(bv_str_list, word);
        if (which_bit != -1)
        {
            switch (bv_str_op)
            {
            case BV_STR_SET:
                bv_set(bv,which_bit);
                break;
            case BV_STR_UNSET:
                bv_unset(bv,which_bit);
                break;
            case BV_STR_TOGGLE:
                if (bv_is_set(bv, which_bit))
                    bv_unset(bv,which_bit);
                else
                    bv_set(bv,which_bit);
                break;
            default:
                break;
            }
        }
        g_free(word);
        g_match_info_next(match_info, NULL);
    }
    g_match_info_free(match_info);
    g_regex_unref(bv_str_regex);
    return;
}
