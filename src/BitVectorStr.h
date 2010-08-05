/*
 * BitVectorStr.h
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

#ifndef BUCKETMUD_BITVECTORSTR_H_
#define BUCKETMUD_BITVECTORSTR_H_

typedef struct BitVectorStringList
{
    gchar *name_;
    gsize bit_;
} BitVectorStringList;

typedef enum { BV_STR_SET, BV_STR_UNSET, BV_STR_TOGGLE } BitVectorStringOp;


gchar *bv_to_string(const BitVector *, const BitVectorStringList *);
gsize bv_bit_from_string(const BitVectorStringList *, const gchar *);
void bv_from_string(BitVector *, const BitVectorStringList *, const gchar *, BitVectorStringOp);

#endif // BUCKETMUD_BITVECTORSTR_H_

