/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2021, Libre Space Foundation <http://libre.space>
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

#include <satnogs/reed_muller.h>
#include <cmath>

namespace gr {
namespace satnogs {

reed_muller::reed_muller() :
  d_r(1),
  d_m(6),
  d_G({
  std::bitset<64>(0b1111111111111111111111111111111111111111111111111111111111111111),
  std::bitset<64>(0b0101010101010101010101010101010101010101010101010101010101010101),
  std::bitset<64>(0b0011001100110011001100110011001100110011001100110011001100110011),
  std::bitset<64>(0b0000111100001111000011110000111100001111000011110000111100001111),
  std::bitset<64>(0b0000000011111111000000001111111100000000111111110000000011111111),
  std::bitset<64>(0b0000000000000000111111111111111100000000000000001111111111111111),
  std::bitset<64>(0b0000000000000000000000000000000011111111111111111111111111111111)
})
{
}

reed_muller::~reed_muller()
{
}

uint64_t reed_muller::encode(const uint8_t in)
{
  std::bitset<7> bs_in(in);
  int coded_num[64] = {0};

  for (size_t i = 0; i < bs_in.size(); ++i) {
    if (bs_in[i]) {
      for (size_t j = 0; j < 64; ++j) {
        coded_num[j] += static_cast<int>(d_G[bs_in.size() - i - 1][63 - j]);
      }
    }
  }

  std::bitset<64> coded_bits(0b0);
  for (size_t i = 0; i < 64; ++i) {
    coded_bits[63 - i] = static_cast<bool>(coded_num[i] % 2);
  }

  return static_cast<uint64_t>(coded_bits.to_ullong());
}

uint8_t reed_muller::decode(const uint64_t in)
{
  const std::bitset<64> bs_in(in);
  std::bitset<7> out(0);

  for (int ind = 5; ind >= 0; --ind) {
    std::bitset<64> places(
      0b1111111111111111111111111111111111111111111111111111111111111111);
    int i = 63; // MS-bit
    int step = static_cast<int>(pow(2, 5 - ind));

    int ones_cnt = 0;
    while (places.count() != 0) {
      while (!places[i]) {
        --i;
      }

      ones_cnt += (bs_in[i] + bs_in[i - step]) % 2;
      places[i] = false;
      places[i - step] = false;
    }

    out[ind] = ones_cnt >= 16;
  }

  std::bitset<64> total(0);
  for (int i = 63; i >= 0; --i) {
    total[i] = (bs_in[i] + out[5] * d_G[1][i] + out[4] * d_G[2][i]
                + out[3] * d_G[3][i] + out[2] * d_G[4][i]
                + out[1] * d_G[5][i] + out[0] * d_G[6][i]) % 2;
  }
  out[6] = total.count() >= 32;

  return static_cast<uint8_t>(out.to_ulong());
}

} /* namespace satnogs */
} /* namespace gr */
