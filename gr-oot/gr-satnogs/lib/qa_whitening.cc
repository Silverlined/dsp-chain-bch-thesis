/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2019, Libre Space Foundation <http://libre.space>
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
#include <boost/test/unit_test.hpp>
#include <satnogs/whitening.h>
#include <random>

namespace gr {

namespace satnogs {

#define LEN 512

void
whitening_ccsds(bool msb)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 0xFF);

  whitening::whitening_sptr scr = whitening::make_ccsds();
  whitening::whitening_sptr descr = whitening::make_ccsds();

  uint8_t orig[LEN];
  uint8_t scrambled[LEN];
  uint8_t descrambled[LEN];

  for (size_t i = 0; i < LEN; i++) {
    orig[i] = uni(mt);
  }

  scr->scramble(scrambled, orig, LEN);
  descr->descramble(descrambled, scrambled, LEN);

  printf("Original: ");
  for (size_t i = 0; i < LEN; i++) {
    printf("0x%02x ", orig[i]);
  }
  printf("\n");

  printf("Scrambled: ");
  for (size_t i = 0; i < LEN; i++) {
    printf("0x%02x ", scrambled[i]);
  }
  printf("\n");

  printf("Descrambled: ");
  for (size_t i = 0; i < LEN; i++) {
    printf("0x%02x ", descrambled[i]);
  }
  printf("\n");

  for (size_t i = 0; i < LEN; i++) {
    BOOST_REQUIRE(orig[i] == descrambled[i]);
  }
}


BOOST_AUTO_TEST_CASE(whitening_ccsds_msb)
{
  whitening_ccsds(true);
}

BOOST_AUTO_TEST_CASE(whitening_ccsds_lsb)
{
  whitening_ccsds(false);
}

BOOST_AUTO_TEST_CASE(whitening_ccsds_unpacked)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 1);

  whitening::whitening_sptr scr = whitening::make_ccsds();
  whitening::whitening_sptr descr = whitening::make_ccsds();

  uint8_t orig[LEN];
  uint8_t scrambled[LEN];
  uint8_t descrambled[LEN];

  for (size_t i = 0; i < LEN; i++) {
    orig[i] = uni(mt);
  }

  scr->scramble_one_bit_per_byte(scrambled, orig, LEN);
  descr->descramble_one_bit_per_byte(descrambled, scrambled, LEN);

  printf("Original: ");
  for (size_t i = 0; i < LEN; i++) {
    printf("0x%02x ", orig[i]);
  }
  printf("\n");

  printf("Scrambled: ");
  for (size_t i = 0; i < LEN; i++) {
    printf("0x%02x ", scrambled[i]);
  }
  printf("\n");

  printf("Descrambled: ");
  for (size_t i = 0; i < LEN; i++) {
    printf("0x%02x ", descrambled[i]);
  }
  printf("\n");

  for (size_t i = 0; i < LEN; i++) {
    BOOST_REQUIRE(orig[i] == descrambled[i]);
  }
}


}  // namespace satnogs

}  // namespace gr
