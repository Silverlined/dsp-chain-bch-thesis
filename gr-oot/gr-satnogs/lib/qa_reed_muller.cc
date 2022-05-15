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
#include <satnogs/reed_muller.h>
#include <random>
#include <bitset>
#include <vector>

namespace gr {
namespace satnogs {

BOOST_AUTO_TEST_CASE(rm_encode_test_0)
{
  reed_muller rm;
  uint8_t num = 0b1000000;
  uint64_t encoded_num = rm.encode(num);
  BOOST_REQUIRE(encoded_num ==
                0b1111111111111111111111111111111111111111111111111111111111111111);
}

BOOST_AUTO_TEST_CASE(rm_encode_test_1)
{
  reed_muller rm;
  uint8_t num = 0b0000011;
  uint64_t encoded_num = rm.encode(num);
  BOOST_REQUIRE(encoded_num ==
                0b0000000000000000111111111111111111111111111111110000000000000000);
}

BOOST_AUTO_TEST_CASE(rm_test_0)
{
  reed_muller rm;
  uint8_t num = 0b1010110;
  uint64_t encoded_num = rm.encode(num);
  uint8_t decoded_num = rm.decode(encoded_num);
  BOOST_REQUIRE(decoded_num == num);
}

BOOST_AUTO_TEST_CASE(rm_test_1)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 127);

  uint8_t num = uni(mt);
  reed_muller rm;
  uint64_t coded = rm.encode(num);

  uint8_t res = rm.decode(coded);
  BOOST_REQUIRE(res == num);
}

BOOST_AUTO_TEST_CASE(rm_errors_0)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 127);

  uint8_t num = uni(mt);
  reed_muller rm;
  uint64_t coded = rm.encode(num);

  uint8_t res = rm.decode(coded);
  BOOST_REQUIRE(res == num);

  /* Apply bit flip */
  std::bitset<64> bs_coded(coded);
  std::uniform_int_distribution<uint8_t> ri(0, 63);
  bs_coded.flip(ri(mt));
  uint64_t coded_error = static_cast<uint64_t>(bs_coded.to_ullong());
  res = rm.decode(coded_error);
  BOOST_REQUIRE(res == num);
}

BOOST_AUTO_TEST_CASE(rm_errors_1)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 127);

  uint8_t num = uni(mt);
  reed_muller rm;
  uint64_t coded = rm.encode(num);

  uint8_t res = rm.decode(coded);
  BOOST_REQUIRE(res == num);

  for (int j = 0; j < 2048; ++j) {
    /* Apply random number of bit flips */
    std::bitset<64> bs_coded(coded);
    std::uniform_int_distribution<uint8_t> flip_times_gen(0, 15);
    std::uniform_int_distribution<uint8_t> ind_gen(0, 63);
    uint8_t flip_times = flip_times_gen(mt);
    std::vector<uint8_t> indx;
    for (int i = 0; i < flip_times; ++i) {
      uint8_t ind = ind_gen(mt);
      while (std::find(indx.begin(), indx.end(), ind) != indx.end()) {
        ind = ind_gen(mt);
      }
      indx.push_back(ind);
      bs_coded.flip(ind);
    }
    uint64_t coded_error = static_cast<uint64_t>(bs_coded.to_ullong());
    res = rm.decode(coded_error);
    BOOST_REQUIRE(res == num);
  }
}

}  // namespace satnogs
}  // namespace gr
