/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2016, Libre Space Foundation <http://librespacefoundation.org/>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <satnogs/whitening.h>
#include <satnogs/utils.h>

namespace gr {
namespace satnogs {

int whitening::base_unique_id = 1;


/**
 * Data whitening and de-whitening class
 * @param mask the polynomial mask
 * @param seed the initial seed
 * @param order the order of the shift register. This is equal to the
 * @param self_sync set to true to enable self synchronizing operation. In this
 * mode the input data are also fed into the LFSR of the screambler
 * number of memory stages.
 */
whitening::whitening_sptr
whitening::make(uint32_t mask, uint32_t seed, uint32_t order, bool msb,
                bool self_sync)
{
  return whitening::whitening_sptr(new whitening(mask, seed, order, msb,
                                   self_sync));
}


/**
 * CCSDS compliant data whitening
 * @param msb set to true to start the scrambling/descrambling from the
 * most significant bit of each byte
 * @return shared pointer to a CCSDS compliant scrambler/descrambler
 */
whitening::whitening_sptr
whitening::make_ccsds(bool msb)
{
  return make(0xA9, 0xFF, 7, msb);
}

/**
 * HDLC/AX.25 compliant G3RUH self synchronizing data whitening
 * @param msb set to true to start the scrambling/descrambling from the
 * most significant bit of each byte
 * @return shared pointer to an G3RUH scrambler/descrambler
 */
whitening::whitening_sptr
whitening::make_g3ruh(bool msb)
{
  return make(0x21, 0x0, 16, msb, true);
}


/**
 * Data whitening and de-whitening class
 * @param mask the polynomial mask
 * @param seed the initial seed
 * @param order the order of the shift register. This is equal to the
 * @param msb set to true to start the scrambling/descrambling from the
 * most significant bit of each byte
 * number of memory stages.
 * @param self_sync set to true to enable self synchronizing operation. In this
 * mode the input data are also fed into the LFSR of the screambler
 */
whitening::whitening(uint32_t mask, uint32_t seed, uint32_t order, bool msb,
                     bool self_sync) :
  d_msb(msb),
  d_self_sync(self_sync),
  d_lfsr(mask, seed, order),
  d_id(0)
{
  d_id = base_unique_id++;
}

whitening::~whitening()
{
}

int
whitening::unique_id()
{
  return d_id;
}

/**
 * Resets the scrambler (or the descrambler) to the initial stage and
 * the initial seed.
 */
void
whitening::reset()
{
  d_lfsr.reset();
}

/**
 * Performs data scrambling
 * @param out the output buffer
 * @param in the input buffer
 * @param len the number of the bytes to be scrambled
 */
void
whitening::scramble(uint8_t *out, const uint8_t *in, size_t len)
{
  if (d_msb) {
    if (d_self_sync) {
      for (size_t i = 0; i < len; i++) {
        uint8_t b;
        b = d_lfsr.next_bit_scramble(in[i] >> 7) << 7;
        b |= d_lfsr.next_bit_scramble((in[i] >> 6) & 0x1) << 6;
        b |= d_lfsr.next_bit_scramble((in[i] >> 5) & 0x1) << 5;
        b |= d_lfsr.next_bit_scramble((in[i] >> 4) & 0x1) << 4;
        b |= d_lfsr.next_bit_scramble((in[i] >> 3) & 0x1) << 3;
        b |= d_lfsr.next_bit_scramble((in[i] >> 2) & 0x1) << 2;
        b |= d_lfsr.next_bit_scramble((in[i] >> 1) & 0x1) << 1;
        b |= d_lfsr.next_bit_scramble(in[i] & 0x1);
        out[i] = b;
      }
    }
    else {
      for (size_t i = 0; i < len; i++) {
        uint8_t b;
        b = d_lfsr.next_bit() << 7;
        b |= d_lfsr.next_bit() << 6;
        b |= d_lfsr.next_bit() << 5;
        b |= d_lfsr.next_bit() << 4;
        b |= d_lfsr.next_bit() << 3;
        b |= d_lfsr.next_bit() << 2;
        b |= d_lfsr.next_bit() << 1;
        b |= d_lfsr.next_bit();
        out[i] = in[i] ^ b;
      }
    }
  }
  else {
    if (d_self_sync) {
      for (size_t i = 0; i < len; i++) {
        uint8_t b;
        b = d_lfsr.next_bit_scramble(in[i] & 0x1);
        b |= d_lfsr.next_bit_scramble((in[i] >> 1) & 0x1) << 1;
        b |= d_lfsr.next_bit_scramble((in[i] >> 2) & 0x1) << 2;
        b |= d_lfsr.next_bit_scramble((in[i] >> 3) & 0x1) << 3;
        b |= d_lfsr.next_bit_scramble((in[i] >> 4) & 0x1) << 4;
        b |= d_lfsr.next_bit_scramble((in[i] >> 5) & 0x1) << 5;
        b |= d_lfsr.next_bit_scramble((in[i] >> 6) & 0x1) << 6;
        b |= d_lfsr.next_bit_scramble((in[i] >> 7) & 0x1) << 7;
        out[i] = b;
      }
    }
    else {
      for (size_t i = 0; i < len; i++) {
        uint8_t b;
        b = d_lfsr.next_bit();
        b |= d_lfsr.next_bit() << 1;
        b |= d_lfsr.next_bit() << 2;
        b |= d_lfsr.next_bit() << 3;
        b |= d_lfsr.next_bit() << 4;
        b |= d_lfsr.next_bit() << 5;
        b |= d_lfsr.next_bit() << 6;
        b |= d_lfsr.next_bit() << 7;
        out[i] = in[i] ^ b;
      }
    }
  }
}

/**
 * Performs data de-scrambling
 * @param out the output buffer
 * @param in the input buffer
 * @param len the number of the bytes to be de-scrambled
 */
void
whitening::descramble(uint8_t *out, const uint8_t *in, size_t len)
{
  scramble(out, in, len);
}

/**
 * Performs data scrambling. The input and output buffer
 * contain one bit per byte
 * @param out the output buffer
 * @param in the input buffer
 * @param bits_num the number of bits to be scrambled
 */
void
whitening::scramble_one_bit_per_byte(uint8_t *out, const uint8_t *in,
                                     size_t bits_num)
{
  if (d_self_sync) {
    for (size_t i = 0; i < bits_num; i++) {
      out[i] = d_lfsr.next_bit_scramble(in[i]);
    }
  }
  else {
    for (size_t i = 0; i < bits_num; i++) {
      out[i] = in[i] ^ d_lfsr.next_bit();
    }
  }
}

/**
 * Performs data descrambling. The input and output buffer
 * contain one bit per byte
 * @param out the output buffer
 * @param in the input buffer
 * @param bits_num the number of bits to be descrambled
 */
void
whitening::descramble_one_bit_per_byte(uint8_t *out, const uint8_t *in,
                                       size_t bits_num)
{
  if (d_self_sync) {
    for (size_t i = 0; i < bits_num; i++) {
      out[i] = d_lfsr.next_bit_descramble(in[i]);
    }
  }
  else {
    for (size_t i = 0; i < bits_num; i++) {
      out[i] = in[i] ^ d_lfsr.next_bit();
    }
  }
}

} /* namespace satnogs */
} /* namespace gr */
