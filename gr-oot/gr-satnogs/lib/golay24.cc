/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2016, 2017, 2018
 *  Libre Space Foundation <http://librespacefoundation.org/>
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

#include <satnogs/golay24.h>
#include <satnogs/utils.h>

namespace gr {
namespace satnogs {

/*
 * Matrix P was retrieved by:
 * Lin & Costello, pg 128, Ch4, "Error Control Coding", 2nd ed, Pearson.
 *
 * Matrix mentioned by Morelos-Zaragoza, Robert H. "The art of error correcting coding."
 * John Wiley & Sons, 2006 was not suitable.
 */
const std::vector<uint32_t> golay24::G_P = {
  0x8ED, 0x1DB, 0x3B5, 0x769, 0xED1, 0xDA3, 0xB47, 0x68F, 0xD1D, 0xA3B, 0x477,
  0xFFE
};

const std::vector<uint32_t> golay24::G_I = {
  0x800, 0x400, 0x200, 0x100, 0x080, 0x040, 0x020, 0x010, 0x008, 0x004, 0x002,
  0x001
};

golay24::golay24()
{
}

golay24::~golay24()
{
}

static inline uint32_t
weight(uint32_t x)
{
  return utils::bit_count(x & 0xFFF);
}

static inline uint32_t
syndrome(uint16_t x, uint16_t y)
{
  uint32_t s = 0;
  for (size_t i = 0; i < 12; i++) {
    s = (s << 1) | (weight(y & golay24::G_P[i]) % 2);
  }
  s ^= x;
  return s;
}

/**
 * Encodes a 12-bit message
 * @param in the input 12-bit message. The message should be placed at the
 * 12 LS bits
 * @param lsb_parity if set to true, the parity part is placed on the 12 LSB,
 * whereas if set to false it is placed on the 12 MSB of the 24 bit coded word
 * @return the coded 24-bit message. The message is placed at the 24 LS bits
 */
uint32_t
golay24::encode12(uint16_t in, bool lsb_parity)
{
  uint32_t c[2] =
  { 0x0, 0x0 };
  c[0] = in & 0xFFF;
  for (size_t i = 0; i < 12; i++) {
    uint32_t tmp = 0;
    for (size_t j = 0; j < 12; j++) {
      tmp ^= (((c[0] & G_P[i]) >> j) & 0x01);
    }
    c[1] = (c[1] << 1) ^ tmp;
  }
  if (lsb_parity) {
    return ((c[0] & 0xFFF) << 12) | (c[1] & 0xFFF);
  }
  return ((c[1] & 0xFFF) << 12) | (c[0] & 0xFFF);
}

/**
 * Decodes a single Golay (24, 12, 8) codeword
 *
 * @param out the 24-bit decoded message. The placement of the parity is
 * implementation specific.
 *
 * @param in the coded 24 bit code word. The message should be placed at the
 * 24 LS bits
 * @return true if the decoding was successful, false in case the error correction
 * could not be performed
 */
bool
golay24::decode24(uint32_t *out, const uint32_t in)
{
  uint32_t e[2] = {0x0, 0x0};
  uint32_t r[2] = {0x0, 0x0};
  uint32_t c[2] = {0x0, 0x0};
  uint32_t s = 0x0;
  r[0] = (in >> 12) & 0xFFF;
  r[1] = in & 0xFFF;
  s = syndrome(r[0], r[1]);
  if (weight(s) <= 3) {
    *out = ((r[0] ^ s) << 12) | (r[1]);
    return true;
  }

  for (size_t i = 0; i < 12; i++) {
    const uint16_t tmp = s ^ G_P[i];
    if (weight(tmp) <= 2) {
      *out = ((r[0] ^ tmp) << 12) | (r[1] ^ G_I[i]);
      return true;
    }
  }

  /* Compute the sP vector */
  uint32_t sP = 0;
  for (size_t i = 0; i < 12; i++) {
    sP = (sP << 1) | (weight(s & G_P[i]) % 2);
  }
  if (weight(sP) == 2 || weight(sP) == 3) {
    *out = (r[0] << 12) | (r[1] ^ sP);
    return true;
  }

  for (size_t i = 0; i < 12; i++) {
    const uint16_t tmp = sP ^ G_P[i];
    if (weight(tmp) == 2) {
      *out = ((r[0] ^ G_I[i]) << 12) | (r[1] ^ tmp);
      return true;
    }
  }
  return false;
}

} /* namespace satnogs */
} /* namespace gr */

