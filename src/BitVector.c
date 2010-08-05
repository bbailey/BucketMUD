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

#include "BitVector.h"

// for assert
#include <assert.h>

// for memcpy
#include <string.h>

// Local prototypes
static void initialize_bits_in_16bits();
static size_t uint32_bitcount(uint32_t n);

///////////////////////////////////////////////////////////////////////////////

// We're using uint32_t for the blocks, so we have 32 bits.
// THIS VALUE MUST BE CHANGED IF YOU CHANGE THE SIZE OF THE BLOCKS.
const size_t BITS_PER_BLOCK = 32;

/** Create a new bitvector of size numBits.
 * All bits will be initialized to false.
 */
BitVector* bv_new(size_t numBits)
{
    // Make sure the equality testing map has been initialized.
    initialize_bits_in_16bits();

    BitVector* bv = (BitVector*) malloc(sizeof(BitVector));

    bv->numBits_ = numBits;

    // Count the number of blocks
    bv->numBlocks_ = numBits / BITS_PER_BLOCK;
    // If there's a remainder, we need another one
    if (numBits % BITS_PER_BLOCK != 0) {
        bv->numBlocks_++;
    }

    // Create the blocks. (calloc initializes them to zero.)
    bv->data_ = (uint32_t*) calloc(bv->numBlocks_, sizeof(uint32_t));

    return bv;
}

/** Create a full copy of a bitvector.
 * The resulting bitvector will have the same number of bits,
 * each with the same truth value.
 */
BitVector* bv_copy(const BitVector* other)
{
    BitVector* bv = (BitVector*) malloc(sizeof(BitVector));

    bv->numBits_ = other->numBits_;
    bv->numBlocks_ = other->numBlocks_;

    // Create the blocks -- but don't initialize them.
    bv->data_ = (uint32_t*) malloc(bv->numBlocks_ * sizeof(uint32_t));

    // Copy the bits from 'other' into 'bv'.
    memcpy(bv->data_, other->data_, bv->numBlocks_ * sizeof(uint32_t));

    return bv;
}

/** Delete a bitvector and associated memory. */
void bv_delete(BitVector* bv)
{
    free(bv->data_);
    free(bv);
}

/** Assign one bitvector's value to another.
 *
 * Only works on bitvectors of equal size.
 */
void bv_assign(BitVector* dst, const BitVector* other)
{
    assert(dst->numBits_ == other->numBits_);

    memcpy(dst->data_, other->data_, other->numBlocks_ * sizeof(uint32_t));
}

/** Return true if a bit is set in a bitvector.
 * whichBit must be less than the bitvector's size.
 */
BOOL bv_is_set(const BitVector* bv, size_t whichBit)
{
    assert(whichBit < bv->numBits_);

    // figure out what block to be in.
    size_t block = whichBit / BITS_PER_BLOCK;
    // figure out what bit to grab.
    size_t bit = whichBit - (block*BITS_PER_BLOCK);

    assert(bit < BITS_PER_BLOCK);

    return (bv->data_[block] & (0x1u << bit) ) != 0;
}

/** Set or clear a bit in a bitvector.
 * whichBit must be less than the bitvector's size.
 */
void bv_set_bit(BitVector* bv, size_t whichBit, BOOL toWhat)
{
    assert(whichBit < bv->numBits_);

    // figure out what block to be in.
    size_t block = whichBit / BITS_PER_BLOCK;
    // figure out what bit to grab.
    size_t bit = whichBit - (block*BITS_PER_BLOCK);

    assert(bit < BITS_PER_BLOCK);

    if (toWhat) {
        bv->data_[block] |= (0x1u << bit);
    }
    else {
        bv->data_[block] ^= (0x1u << bit);
    }
}

/** Set a bit in a bitvector. */
void bv_set(BitVector* bv, size_t whichBit)
{
    bv_set_bit(bv, whichBit, 1);
}

/** Clear a bit in a bitvector. */
void bv_unset(BitVector* bv, size_t whichBit)
{
    bv_set_bit(bv, whichBit, 0);
}

/** Clear all bits in a bitvector. */
void bv_clear(BitVector* bv)
{
    size_t block = 0;
    for (block = 0; block < bv->numBlocks_; block++) {
        bv->data_[block] = 0;
    }
}

/** Test if a bitvector is a subset of another bitvector.
 * That is, if the other bitvector has at least the same bits set.
 *
 * Only works on bitvectors of equal size.
 */
BOOL bv_is_subset(const BitVector* subset, const BitVector* superset)
{
    size_t block = 0;
    assert(subset->numBits_ == superset->numBits_);

    // for each block, make sure all bits in this are present in other
    for (block = 0; block < subset->numBlocks_; block++)
    {
        // XOR: figure out what bits are in one but not both
        uint32_t mask = subset->data_[block] ^ superset->data_[block];

        // AND: make sure that none of those bits are in this one
        if ( (subset->data_[block] & mask) != 0 ) {
            return 0;
        }
    }

    return 1;
}
        
/** Intersect one bitvector with another bitvector.
 * (In-place modification.)
 *
 * Only works on bitvectors of equal size.
 */
void bv_intersect(BitVector* bv, const BitVector* other)
{
    size_t block = 0;
    assert(bv->numBits_ == other->numBits_);

    for (block = 0; block < bv->numBlocks_; block++) {
        bv->data_[block] &= other->data_[block];
    }
}

/** Union one bitvector with another bitvector.
 * (In-place modification.)
 *
 * Only works on bitvectors of equal size.
 */
void bv_union(BitVector* bv, const BitVector* other)
{
    size_t block = 0;
    assert(bv->numBits_ == other->numBits_);

    for (block = 0; block < bv->numBlocks_; block++) {
        bv->data_[block] |= other->data_[block];
    }
}

/** Return true if two bitvectors are equal.
 * Bitvectors are equal if they have the same bits.
 */
BOOL bv_equal(const BitVector* bv1, const BitVector* bv2)
{
    size_t block = 0;
    assert(bv1->numBits_ == bv2->numBits_);
    
    for (block = 0; block < bv1->numBlocks_; block++) {
        if (bv1->data_[block] != bv2->data_[block]) {
            return 0;
        }
    }

    return 1;
}

/** Return the number of set (i.e. true) bits in the bitvector. */
size_t bv_num_set(const BitVector* bv)
{
    size_t block = 0;
    size_t numBits = 0;

    for (block = 0; block < bv->numBlocks_; block++) {
        numBits += uint32_bitcount(bv->data_[block]);
    }

    return numBits;
}

/** Return the total number of bits (set or not) in the bitvector. */
size_t bv_size(const BitVector* bv)
{
    return bv->numBits_;
}

///////////////////////////////////////////////////////////////////////////////

// A pre-populated map from 16-bit integer
// to the number of bits in that integer.
static char bits_in_16bits[0x1u << 16];

// Initialize the above bits_in_16bits map.
// (Will only initialize it once.)
static void initialize_bits_in_16bits()
{
    size_t i = 0;
    static int initialized = 0;
    if (initialized) {
        return;
    }

    // Iteratively count the number of bits in each 16-bit integer.
    // Doesn't have to be fast since we only do this once.
    for (i = 0; i < (0x1u << 16); i++)
    {
        size_t numBits = 0;

        size_t bit = i;
        while (bit) 
        {
            numBits += bit & 0x1u;
            bit >>= 1;
        }

        bits_in_16bits[i] = numBits;
    }

    initialized = 1;
}

static size_t uint32_bitcount(uint32_t n)
{
    return bits_in_16bits[ n        & 0xffffu]
        +  bits_in_16bits[(n >> 16) & 0xffffu];
}

