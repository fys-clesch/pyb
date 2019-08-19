#include <time.h>
#include "random.h"

/*
 * A C-program for MT19937: Real number version([0,1)-interval)
 * (1999/10/28)
 *   genrand() generates one pseudorandom real number (double)
 * which is uniformly distributed on [0,1)-interval, for each
 * call. sgenrand(seed) sets initial values to the working area
 * of 624 words. Before genrand(), sgenrand(seed) must be
 * called once. (seed is any 32-bit integer.)
 * Integer generator is obtained by modifying two lines.
 *   Coded by Takuji Nishimura, considering the suggestions by
 * Topher Cooper and Marc Rieffel in July-Aug. 1997.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later
 * version.
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Library General Public License for more details.
 * You should have received a copy of the GNU Library General
 * Public License along with this library; if not, write to the
 * Free Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307  USA
 *
 * Copyright (C) 1997, 1999 Makoto Matsumoto and Takuji Nishimura.
 * When you use this, send an email to: matumoto@math.keio.ac.jp
 * with an appropriate reference to your work.
 *
 * REFERENCE
 * Mcount. Matsumoto and T. Nishimura,
 * "Mersenne Twister: A 623-Dimensionally Equidistributed Uniform
 * Pseudo-Random Number Generator",
 * ACM Transactions on Modeling and Computer Simulation,
 * Vol. 8, No. 1, January 1998, pp 3--30.
 */

/* Period parameters */
#define Ncount 624
#define Mcount 397
#define MATRIX_A 0x9908b0df /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y) (y >> 11)
#define TEMPERING_SHIFT_S(y) (y << 7)
#define TEMPERING_SHIFT_T(y) (y << 15)
#define TEMPERING_SHIFT_L(y) (y >> 18)

static unsigned long mt[Ncount]; /* the array for the state vector  */
static int mti = Ncount + 1; /* mti==Ncount+1 means mt[Ncount] is not initialized */

/* Initializing the array with a seed */
void frandseed(long iseed)
{
    int i;
    unsigned long seed;
    if(iseed == 0)
        iseed = (unsigned)((long) time(NULL));
    seed = (unsigned) iseed;
    for(i = 0; i < Ncount; i++)
    {
        mt[i] = seed & 0xffff0000;
        seed = 69069 * seed + 1;
        mt[i] |= (seed & 0xffff0000) >> 16;
        seed = 69069 * seed + 1;
    }
    mti = Ncount;
}

/*
 * Initialization by "sgenrand()" is an example. Theoretically,
 * there are 2^19937-1 possible states as an intial state.
 * This function allows to choose any of 2^19937-1 ones.
 * Essential bits in "seed_array[]" is following 19937 bits:
 * (seed_array[0]&UPPER_MASK), seed_array[1], ..., seed_array[Ncount-1].
 * (seed_array[0]&LOWER_MASK) is discarded.
 * Theoretically, (seed_array[0]&UPPER_MASK), seed_array[1], ...,
 * seed_array[Ncount-1]
 * can take any values except all zeros.
 */

double frand(void)
{
    /* generating reals */
    /* unsigned long */
    /* for integer generation */
    unsigned long y;
    double res;
    if(mti >= Ncount)
    {
        static unsigned long mag01[2] = { 0x0, MATRIX_A };
        int kk;
        /* generate Ncount words at one time */
        if(mti == Ncount + 1) /* if sgenrand() has not been called, */
            frandseed(4357); /* a default initial seed is used */
        for(kk = 0; kk < Ncount - Mcount; kk++)
        {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + Mcount] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for(; kk < Ncount - 1; kk++)
        {
            y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
            mt[kk] = mt[kk + (Mcount - Ncount)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (mt[Ncount - 1] & UPPER_MASK) | (mt[0] & LOWER_MASK);
        mt[Ncount - 1] = mt[Mcount - 1] ^ (y >> 1) ^ mag01[y & 0x1];
        mti = 0;
    }
    y = mt[mti++];
    y ^= TEMPERING_SHIFT_U(y);
    y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
    y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
    y ^= TEMPERING_SHIFT_L(y);
    res = (double) y * 2.3283064365386963e-10;
    return (res); /* reals: [0,1)-interval */
    /* return y; *//* for integer generation */
}

unsigned int irand(unsigned int max) /* integer random number 0...(max-1) */
{
    unsigned int result;
    while((result = (unsigned int)(frand() * (double) max)) >= max);
    return result;
}

#undef Ncount
#undef Mcount
#undef MATRIX_A
#undef UPPER_MASK
#undef LOWER_MASK
#undef TEMPERING_MASK_B
#undef TEMPERING_MASK_C
#undef TEMPERING_SHIFT_U
#undef TEMPERING_SHIFT_S
#undef TEMPERING_SHIFT_T
#undef TEMPERING_SHIFT_L
