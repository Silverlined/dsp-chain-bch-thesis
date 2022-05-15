/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2020 Libre Space Foundation <http://libre.space>
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

#include <satnogs/utils.h>
#include <cstring>
#include <cmath>
#include <cstdio>
#include <arpa/inet.h>

namespace gr {
namespace satnogs {

constexpr uint8_t utils::s_bytes_reversed[];

/**
 * Computes the Mean Absolute Percentage Error
 * @param ref the reference value
 * @param estimation the estimated value
 * @return the mean absolute percentage error
 */
double
utils::mape(double ref, double estimation)
{
  return std::abs(ref - estimation) / ref;
}

uint64_t
utils::htonll(uint64_t x)
{
  if (1 == htonl(1)) {
    return x;
  }
  return ((((uint64_t)htonl(x)) & 0xFFFFFFFF) << 32) | htonl(x >> 32);
}

uint64_t
utils::ntohll(uint64_t x)
{
  if (1 == htonl(1)) {
    return x;
  }
  return ((((uint64_t)ntohl(x)) & 0xFFFFFFFF) << 32) | ntohl(x >> 32);
}

/**
 * Counts the number of active bits in x
 * @param x the number to count the number of active bits
 */
uint32_t
utils::bit_count(uint32_t x)
{
  /*
   * Some more magic from
   * http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
   */
  x = x - ((x >> 1) & 0x55555555);
  x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
  return (((x + (x >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

/**
 * Reverse the bits of the byte b.
 * @param b the byte to be mirrored.
 */
uint8_t
utils::reverse_byte(uint8_t b)
{
  return s_bytes_reversed[b];
}

/**
 * Reverse the bits of the 32-bit number i
 * @param i the 32-bit value to mirror
 * @return mirrored i
 */
uint32_t
utils::reverse_uint32_bytes(uint32_t i)
{
  return (s_bytes_reversed[i & 0xff] << 24)
         | (s_bytes_reversed[(i >> 8) & 0xff] << 16)
         | (s_bytes_reversed[(i >> 16) & 0xff] << 8)
         | (s_bytes_reversed[(i >> 24) & 0xff]);
}

/**
 * Reverse the bits of the 64-bit number x
 * @param x the 64-bit value to mirror
 * @return mirrored x
 */
uint64_t
reverse_uint64_bytes(uint64_t x)
{
  x = (x & 0x00000000FFFFFFFF) << 32 | (x & 0xFFFFFFFF00000000) >> 32;
  x = (x & 0x0000FFFF0000FFFF) << 16 | (x & 0xFFFF0000FFFF0000) >> 16;
  x = (x & 0x00FF00FF00FF00FF) << 8 | (x & 0xFF00FF00FF00FF00) >> 8;
  return x;
}

void
utils::print_pdu(const uint8_t *buf, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    printf("0x%02x ", buf[i]);
  }
  printf("\n");
}

}
}

