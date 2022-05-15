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
#include <satnogs/golay24.h>
#include <random>
#include <algorithm>

namespace gr {

namespace satnogs {

/* Inspired by the example 4.7 of
 * Lin & Costello, Ch4, "Error Control Coding", 2nd ed, Pearson.
 */

BOOST_AUTO_TEST_CASE(example_47)
{
  golay24 gol;
  uint32_t r = 0b100000110100110000000001;
  uint32_t res;
  bool ret = gol.decode24(&res, r);
  BOOST_REQUIRE(ret);
  BOOST_REQUIRE((res >> 12) == 0b100100110110);
}


BOOST_AUTO_TEST_CASE(errors_0)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint16_t> uni(0, 0xFFFF);

  uint16_t x = uni(mt) & 0xFFF;
  golay24 gol;
  uint32_t coded = gol.encode12(x);

  uint32_t res;
  bool ret = gol.decode24(&res, coded);
  BOOST_REQUIRE(ret);
  BOOST_REQUIRE((res >> 12) == x);

  coded = gol.encode12(x, false);
  ret = gol.decode24(&res, coded);
  BOOST_REQUIRE(ret);
  BOOST_REQUIRE((res & 0xFFF) == x);
}


static inline uint32_t
flip(uint32_t in, uint32_t n)
{
  if (!n || n > 24) {
    return in;
  }

  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> flip(0, 23);
  std::vector<uint8_t> idx;
  while (idx.size() < n) {
    uint8_t i = flip(mt);
    if (std::find(idx.begin(), idx.end(), i) == idx.end()) {
      idx.push_back(i);
    }
  }
  uint32_t mask = 0;
  for (uint8_t i : idx) {
    mask |= (1 << i);
  }
  return in ^ mask;
}

BOOST_AUTO_TEST_CASE(errors_1)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint16_t> uni(0, 0xFFFF);

  for (size_t i = 0; i < 2048; i++) {
    uint16_t x = uni(mt) & 0xFFF;
    golay24 gol;
    uint32_t coded = gol.encode12(x);
    /* Apply bit flip */
    uint32_t coder_error = flip(coded, 1);

    uint32_t res;
    bool ret = gol.decode24(&res, coded);
    BOOST_REQUIRE(ret);
    BOOST_REQUIRE((res >> 12) == x);

    coded = gol.encode12(x, false);
    /* Apply bit flip */
    coder_error = flip(coded, 1);
    ret = gol.decode24(&res, coded);
    BOOST_REQUIRE(ret);
    BOOST_REQUIRE((res & 0xFFF) == x);
  }
}

BOOST_AUTO_TEST_CASE(errors_3)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint16_t> uni(0, 0xFFFF);

  for (size_t i = 0; i < 2048; i++) {
    uint16_t x = uni(mt) & 0xFFF;
    golay24 gol;
    uint32_t coded = gol.encode12(x);
    /* Apply bit flip */
    uint32_t coder_error = flip(coded, 3);

    uint32_t res;
    bool ret = gol.decode24(&res, coded);
    BOOST_REQUIRE(ret);
    BOOST_REQUIRE((res >> 12) == x);

    coded = gol.encode12(x, false);
    /* Apply bit flip */
    coder_error = flip(coded, 3);
    ret = gol.decode24(&res, coded);
    BOOST_REQUIRE(ret);
    BOOST_REQUIRE((res & 0xFFF) == x);
  }
}

BOOST_AUTO_TEST_CASE(errors_4)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint16_t> uni(0, 0xFFFF);

  for (size_t i = 0; i < 2048; i++) {
    uint16_t x = uni(mt) & 0xFFF;
    golay24 gol;
    uint32_t coded = gol.encode12(x);
    /* Apply bit flip */
    uint32_t coder_error = flip(coded, 4);

    uint32_t res;
    bool ret = gol.decode24(&res, coded);
    if (ret) {
      BOOST_REQUIRE((res >> 12) == x);
    }

    coded = gol.encode12(x, false);
    /* Apply bit flip */
    coder_error = flip(coded, 4);
    ret = gol.decode24(&res, coded);
    if (ret) {
      BOOST_REQUIRE((res & 0xFFF) == x);
    }
  }
}

}  // namespace satnogs

}  // namespace gr
