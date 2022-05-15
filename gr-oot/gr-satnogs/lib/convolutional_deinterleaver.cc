/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2018, Libre Space Foundation <http://librespacefoundation.org/>
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
#include <satnogs/convolutional_deinterleaver.h>

namespace gr {
namespace satnogs {

convolutional_deinterleaver::convolutional_deinterleaver(size_t branches,
    size_t M) :
  d_nbranches(branches),
  d_M(M),
  d_idx(0)
{
  for (size_t i = 0; i < d_nbranches; i++) {
    d_branches.push_back(std::deque<uint8_t>((d_nbranches - 1 - i) * M, 0));
  }
}

convolutional_deinterleaver::~convolutional_deinterleaver()
{
}

uint8_t
convolutional_deinterleaver::decode_bit(uint8_t b)
{
  uint8_t ret;
  d_branches[d_idx].push_back(b);
  ret = d_branches[d_idx].front();
  d_branches[d_idx].pop_front();
  d_idx = (d_idx + 1) % d_nbranches;
  return ret;
}

uint8_t
convolutional_deinterleaver::decode_byte(uint8_t b)
{
  uint8_t ret = 0;
  ret = decode_bit(b >> 7) << 7;
  ret |= decode_bit((b >> 6) & 0x1) << 6;
  ret |= decode_bit((b >> 5) & 0x1) << 5;
  ret |= decode_bit((b >> 4) & 0x1) << 4;
  ret |= decode_bit((b >> 3) & 0x1) << 3;
  ret |= decode_bit((b >> 2) & 0x1) << 2;
  ret |= decode_bit((b >> 1) & 0x1) << 1;
  ret |= decode_bit(b & 0x1);
  return ret;
}

void
convolutional_deinterleaver::reset()
{
  d_branches.clear();
  for (size_t i = 0; i < d_nbranches; i++) {
    d_branches.push_back(std::deque<uint8_t>((d_nbranches - 1 - i) * d_M, 0));
  }
  d_idx = 0;
}

} /* namespace satnogs */
} /* namespace gr */

