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
#include <boost/test/unit_test.hpp>
#include <satnogs/utils.h>
#include <random>
#include <itpp/itbase.h>

namespace gr {
namespace satnogs {

BOOST_AUTO_TEST_CASE(uint8_test)
{
  size_t ret;
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 255);
  uint8_t orig[255];
  uint8_t unpacked[255 * 8];
  uint8_t packed[255];

  for (size_t i = 0; i < 255; i++) {
    orig[i] = uni(mt);
  }
  ret = utils::packed_to_unpacked<uint8_t *, uint8_t *, false>(unpacked, orig,
        255);
  BOOST_REQUIRE(ret == 255 * 8);
  ret = utils::unpacked_to_packed<uint8_t *, uint8_t *, false>(packed, unpacked,
        ret);
  BOOST_REQUIRE(ret == 255);
  for (size_t i = 0; i < 255; i++) {
    BOOST_REQUIRE(packed[i] == orig[i]);
  }
}

BOOST_AUTO_TEST_CASE(vec_test)
{
  size_t ret;
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 1);
  itpp::bvec orig(255);
  itpp::Vec<uint8_t> unpacked(255 * 8);
  itpp::bvec packed(255);

  for (size_t i = 0; i < 255; i++) {
    orig[i] = uni(mt);
  }
  ret = utils::packed_to_unpacked<itpp::Vec<uint8_t>&, itpp::bvec &, false>
        (unpacked, orig);
  BOOST_REQUIRE(ret == 255 * 8);
  ret = utils::unpacked_to_packed<itpp::bvec &, itpp::Vec<uint8_t>&, false>
        (packed, unpacked);
  BOOST_REQUIRE(ret == 255);
  for (size_t i = 0; i < 255; i++) {
    BOOST_REQUIRE(packed[i] == orig[i]);
  }
}

BOOST_AUTO_TEST_CASE(uint8_soft_test)
{
  size_t ret;
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 255);
  uint8_t orig[255];
  int8_t unpacked[255 * 8];
  uint8_t packed[255];

  for (size_t i = 0; i < 255; i++) {
    orig[i] = uni(mt);
  }
  ret = utils::packed_to_unpacked<int8_t *, uint8_t *, true>(unpacked, orig, 255);
  BOOST_REQUIRE(ret == 255 * 8);
  ret = utils::unpacked_to_packed<uint8_t *, int8_t *, true>(packed, unpacked,
        ret);
  BOOST_REQUIRE(ret == 255);
  for (int i = 0; i < 255; i++) {
    BOOST_REQUIRE(packed[i] == orig[i]);
  }
}

BOOST_AUTO_TEST_CASE(vec_soft_test)
{
  size_t ret;
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 1);
  itpp::bvec orig(255);
  itpp::Vec<int8_t> unpacked(255 * 8);
  itpp::bvec packed(255);

  for (size_t i = 0; i < 255; i++) {
    orig[i] = uni(mt);
  }
  ret = utils::packed_to_unpacked<itpp::Vec<int8_t>&, itpp::bvec &, true>
        (unpacked, orig);
  BOOST_REQUIRE(ret == 255 * 8);
  ret = utils::unpacked_to_packed<itpp::bvec &, itpp::Vec<int8_t>&, true>
        (packed, unpacked);
  BOOST_REQUIRE(ret == 255);
  for (size_t i = 0; i < 255; i++) {
    BOOST_REQUIRE(packed[i] == orig[i]);
  }
}

}  // namespace satnogs
}  // namespace gr
