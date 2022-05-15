/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2020, Libre Space Foundation <http://libre.space>
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
#include <satnogs/crc.h>
#include <random>
#include <algorithm>

namespace gr {

namespace satnogs {

static size_t frame_size = 1024;

BOOST_AUTO_TEST_CASE(crc_size)
{
  BOOST_REQUIRE(crc::crc_size(crc::CRC_NONE) == 0);
  BOOST_REQUIRE(crc::crc_size(crc::CRC16_AUG_CCITT) == 2);
  BOOST_REQUIRE(crc::crc_size(crc::CRC16_AX25) == 2);
  BOOST_REQUIRE(crc::crc_size(crc::CRC16_CCITT) == 2);
  BOOST_REQUIRE(crc::crc_size(crc::CRC16_CCITT_REVERSED) == 2);
  BOOST_REQUIRE(crc::crc_size(crc::CRC16_IBM) == 2);
  BOOST_REQUIRE(crc::crc_size(crc::CRC32_C) == 4);
}

BOOST_AUTO_TEST_CASE(CRC16_AUG_CCITT)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 0xFF);
  std::uniform_int_distribution<size_t> uni_idx(0, frame_size - 1);
  std::vector<uint8_t> d(frame_size);
  std::generate(d.begin(), d.end(), [&] {
    return uni(mt);
  });

  uint16_t crc1 = crc::crc16_aug_ccitt(d.data(), frame_size);
  uint16_t crc2 = crc::crc16_aug_ccitt(d.data(), frame_size);
  BOOST_REQUIRE(crc1 == crc2);
  d[uni_idx(mt)] = uni(mt);
  BOOST_REQUIRE(crc1 != crc::crc16_aug_ccitt(d.data(), frame_size));
}

BOOST_AUTO_TEST_CASE(CRC32_C)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 0xFF);
  std::uniform_int_distribution<size_t> uni_idx(0, frame_size - 1);
  std::vector<uint8_t> d(frame_size);
  std::generate(d.begin(), d.end(), [&] {
    return uni(mt);
  });

  uint32_t crc1 = crc::crc32_c(d.data(), frame_size);
  uint32_t crc2 = crc::crc32_c(d.data(), frame_size);
  BOOST_REQUIRE(crc1 == crc2);
  d[uni_idx(mt)] = uni(mt);
  BOOST_REQUIRE(crc1 != crc::crc32_c(d.data(), frame_size));

  std::generate(d.begin(), d.end(), [&] {
    return uni(mt);
  });
  crc2 = crc::crc32_c(d.data(), frame_size);
  BOOST_REQUIRE(crc1 != crc2);
}

}  // namespace satnogs

}  // namespace gr



