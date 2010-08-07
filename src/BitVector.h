/*
Copyright (c) 2009, David Charles Haley
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. The name of the author may not be used to endorse or promote products
       derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BITVECTOR_H_
#define BITVECTOR_H_

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef BOOL
    #define BOOL char
#endif

typedef struct BitVector
{
    /** The data storage for the bitvector. */
    uint32_t* data_;
    /** How many blocks of data we have for the bitvector. */
    size_t numBlocks_;
    /** The number of bits in the bitvector. */
    size_t numBits_;
}
BitVector;

/** Create a new bitvector of size numBits.
 * All bits will be initialized to false.
 */
BitVector* bv_new(size_t numBits);

/** Create a full copy of a bitvector.
 * The resulting bitvector will have the same number of bits,
 * each with the same truth value.
 */
BitVector* bv_copy(const BitVector* other);

/** Delete a bitvector and associated memory. */
void bv_delete(BitVector* bv);

/** Assign one bitvector's value to another.
 *
 * Only works on bitvectors of equal size.
 */
void bv_assign(BitVector* dst, const BitVector* other);

/** Return true if a bit is set in a bitvector.
 * whichBit must be less than the bitvector's size.
 */
BOOL bv_is_set(const BitVector* bv, size_t whichBit);

/** Set or clear a bit in a bitvector.
 * whichBit must be less than the bitvector's size.
 */
void bv_set_bit(BitVector* bv, size_t whichBit, BOOL toWhat);

/** Set a bit in a bitvector. */
void bv_set(BitVector* bv, size_t whichBit);
/** Clear a bit in a bitvector. */
void bv_unset(BitVector* bv, size_t whichBit);

/** Clear all bits in a bitvector. */
void bv_clear(BitVector* bv);

/** Test if a bitvector is a subset of another bitvector.
 * That is, if the other bitvector has at least the same bits set.
 *
 * Only works on bitvectors of equal size.
 */
BOOL bv_is_subset(const BitVector* subset, const BitVector* superset);
        
/** Intersect one bitvector with another bitvector.
 * (In-place modification.)
 *
 * Only works on bitvectors of equal size.
 */
void bv_intersect(BitVector* bv, const BitVector* other);

/** Union one bitvector with another bitvector.
 * (In-place modification.)
 *
 * Only works on bitvectors of equal size.
 */
void bv_union(BitVector* bv, const BitVector* other);

/** Return true if two bitvectors are equal.
 * Bitvectors are equal if they have the same bits.
 */
BOOL bv_equal(const BitVector* bv1, const BitVector* bv2);

/** Return the number of set (i.e. true) bits in the bitvector. */
size_t bv_num_set(const BitVector* bv);

/** Return the total number of bits (set or not) in the bitvector. */
size_t bv_size(const BitVector* bv);

#endif


