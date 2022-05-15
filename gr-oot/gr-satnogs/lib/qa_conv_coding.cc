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
#include <satnogs/conv_decoder.h>
#include <satnogs/conv_encoder.h>
#include <satnogs/utils.h>
#include <cstring>
#include <random>

namespace gr {
namespace satnogs {

void
hard_to_soft_int8(int8_t *out, const uint8_t *in, size_t len)
{
  for (size_t i = 0; i < len; i++) {
    out[i] = ((in[i] & 0x1) * 255) - 128;
  }
}

BOOST_AUTO_TEST_CASE(simple_code_1_2)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 255);
  conv_decoder dec(conv_decoder::RATE_1_2, 2048);
  conv_encoder conv(conv_encoder::RATE_1_2, 2048);
  uint8_t raw_packed[255];
  uint8_t raw_unpacked[255 * 8];
  uint8_t coded_unpacked_hard[255 * 8];
  int8_t  coded_unpacked_soft[255 * 8];
  uint8_t decoded_unpacked[255 * 8];
  uint8_t decoded_packed[255];


  for (uint32_t i = 0; i < 48; i++) {
    raw_packed[i] = uni(mt);
  }
  utils::packed_to_unpacked<uint8_t *, uint8_t *, false>(raw_unpacked, raw_packed,
      48);

  size_t ret = conv.encode(coded_unpacked_hard, raw_unpacked, 48 * 8);
  hard_to_soft_int8(coded_unpacked_soft, coded_unpacked_hard, ret);

  ret = dec.decode(decoded_unpacked, coded_unpacked_soft, ret);
  utils::unpacked_to_packed<uint8_t *, uint8_t *, false>(decoded_packed,
      decoded_unpacked, ret);
  for (uint32_t i = 0; i < 48; i++) {
    BOOST_REQUIRE(raw_packed[i] == decoded_packed[i]);
  }
}

BOOST_AUTO_TEST_CASE(simple_code_2_3)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 255);
  conv_decoder dec(conv_decoder::RATE_2_3, 2048);
  conv_encoder  conv(conv_encoder::RATE_2_3, 2048);

  uint8_t raw_packed[255];
  uint8_t raw_unpacked[255 * 8];
  uint8_t coded_unpacked_hard[255 * 8];
  int8_t  coded_unpacked_soft[255 * 8];
  uint8_t decoded_unpacked[255 * 8];
  uint8_t decoded_packed[255];


  for (uint32_t i = 0; i < 48; i++) {
    raw_packed[i] = uni(mt);
  }
  utils::packed_to_unpacked<uint8_t *, uint8_t *, false>(raw_unpacked, raw_packed,
      48);

  size_t ret = conv.encode(coded_unpacked_hard, raw_unpacked, 48 * 8);
  hard_to_soft_int8(coded_unpacked_soft, coded_unpacked_hard, ret);

  ret = dec.decode(decoded_unpacked, coded_unpacked_soft, ret);
  utils::unpacked_to_packed<uint8_t *, uint8_t *, false>(decoded_packed,
      decoded_unpacked, ret);
  for (uint32_t i = 0; i < 48; i++) {
    BOOST_REQUIRE(raw_packed[i] == decoded_packed[i]);
  }
}

BOOST_AUTO_TEST_CASE(simple_code_3_4)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 255);
  conv_decoder dec(conv_decoder::RATE_3_4, 2048);
  conv_encoder conv(conv_encoder::RATE_3_4, 2048);

  uint8_t raw_packed[255];
  uint8_t raw_unpacked[255 * 8];
  uint8_t coded_unpacked_hard[255 * 8];
  int8_t  coded_unpacked_soft[255 * 8];
  uint8_t decoded_unpacked[255 * 8];
  uint8_t decoded_packed[255];


  for (uint32_t i = 0; i < 48; i++) {
    raw_packed[i] = uni(mt);
  }
  utils::packed_to_unpacked<uint8_t *, uint8_t *, false>(raw_unpacked, raw_packed,
      48);

  size_t ret = conv.encode(coded_unpacked_hard, raw_unpacked, 48 * 8);
  hard_to_soft_int8(coded_unpacked_soft, coded_unpacked_hard, ret);

  ret = dec.decode(decoded_unpacked, coded_unpacked_soft, ret);
  utils::unpacked_to_packed<uint8_t *, uint8_t *, false>(decoded_packed,
      decoded_unpacked, ret);
  for (uint32_t i = 0; i < 48; i++) {
    BOOST_REQUIRE(raw_packed[i] == decoded_packed[i]);
  }
}

BOOST_AUTO_TEST_CASE(simple_code_5_6)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 255);
  conv_decoder dec(conv_decoder::RATE_5_6, 2048);
  conv_encoder conv(conv_encoder::RATE_5_6, 2048);

  uint8_t raw_packed[255];
  uint8_t raw_unpacked[255 * 8];
  uint8_t coded_unpacked_hard[255 * 8];
  int8_t  coded_unpacked_soft[255 * 8];
  uint8_t decoded_unpacked[255 * 8];
  uint8_t decoded_packed[255];


  for (uint32_t i = 0; i < 48; i++) {
    raw_packed[i] = uni(mt);
  }
  utils::packed_to_unpacked<uint8_t *, uint8_t *, false>(raw_unpacked, raw_packed,
      48);

  size_t ret = conv.encode(coded_unpacked_hard, raw_unpacked, 48 * 8);
  hard_to_soft_int8(coded_unpacked_soft, coded_unpacked_hard, ret);

  ret = dec.decode(decoded_unpacked, coded_unpacked_soft, ret);
  utils::unpacked_to_packed<uint8_t *, uint8_t *, false>(decoded_packed,
      decoded_unpacked, ret);
  for (uint32_t i = 0; i < 48; i++) {
    BOOST_REQUIRE(raw_packed[i] == decoded_packed[i]);
  }
}

BOOST_AUTO_TEST_CASE(simple_code_7_8)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 255);
  conv_decoder dec(conv_decoder::RATE_7_8, 2048);
  conv_encoder conv(conv_encoder::RATE_7_8, 2048);

  uint8_t raw_packed[255];
  uint8_t raw_unpacked[255 * 8];
  uint8_t coded_unpacked_hard[255 * 8];
  int8_t  coded_unpacked_soft[255 * 8];
  uint8_t decoded_unpacked[255 * 8];
  uint8_t decoded_packed[255];


  for (uint32_t i = 0; i < 48; i++) {
    raw_packed[i] = uni(mt);
  }
  utils::packed_to_unpacked<uint8_t *, uint8_t *, false>(raw_unpacked, raw_packed,
      48);

  size_t ret = conv.encode(coded_unpacked_hard, raw_unpacked, 48 * 8);
  hard_to_soft_int8(coded_unpacked_soft, coded_unpacked_hard, ret);

  ret = dec.decode(decoded_unpacked, coded_unpacked_soft, ret);
  utils::unpacked_to_packed<uint8_t *, uint8_t *, false>(decoded_packed,
      decoded_unpacked, ret);
  for (uint32_t i = 0; i < 48; i++) {
    BOOST_REQUIRE(raw_packed[i] == decoded_packed[i]);
  }
}

void
test_code(size_t msg_len, conv_decoder::coding_rate_t rate)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> randb(0, 255);
  conv_decoder dec(rate, (msg_len + 2) * 8 * 2);
  conv_encoder conv((conv_encoder::coding_rate_t) rate, (msg_len + 2) * 8 * 2);

  uint8_t *raw_packed = new uint8_t[msg_len];
  uint8_t *raw_unpacked = new uint8_t[msg_len * 8];
  uint8_t *coded_unpacked_hard = new uint8_t[(msg_len + 2) * 8 * 2];
  int8_t  *coded_unpacked_soft = new int8_t[(msg_len + 2) * 8 * 2];
  uint8_t *decoded_unpacked = new uint8_t[msg_len * 8];
  uint8_t *decoded_packed = new uint8_t[msg_len];


  for (uint32_t i = 0; i < msg_len; i++) {
    raw_packed[i] = randb(mt);
  }
  utils::packed_to_unpacked<uint8_t *, uint8_t *, false>(raw_unpacked, raw_packed,
      msg_len);

  size_t ret = conv.encode(coded_unpacked_hard, raw_unpacked, msg_len * 8);
  hard_to_soft_int8(coded_unpacked_soft, coded_unpacked_hard, ret);

  ret = dec.decode(decoded_unpacked, coded_unpacked_soft, ret);
  utils::unpacked_to_packed<uint8_t *, uint8_t *, false>(decoded_packed,
      decoded_unpacked, ret);

  for (uint32_t i = 0; i < msg_len; i++) {
    BOOST_REQUIRE(raw_packed[i] == decoded_packed[i]);
  }

  delete [] raw_packed;
  delete [] raw_unpacked;
  delete [] coded_unpacked_hard;
  delete [] coded_unpacked_soft;
  delete [] decoded_unpacked;
  delete [] decoded_packed;
}


BOOST_AUTO_TEST_CASE(various_msg_len_1_2)
{
  for (uint32_t i = 4; i < 1024; i++) {
    test_code(i, conv_decoder::RATE_1_2);
  }
}

BOOST_AUTO_TEST_CASE(various_msg_len_2_3)
{
  for (uint32_t i = 4; i < 1024; i++) {
    test_code(i, conv_decoder::RATE_2_3);
  }
}

BOOST_AUTO_TEST_CASE(various_msg_len_3_4)
{
  for (uint32_t i = 4; i < 1024; i++) {
    test_code(i, conv_decoder::RATE_3_4);
  }
}

BOOST_AUTO_TEST_CASE(various_msg_len_5_6)
{
  for (uint32_t i = 4; i < 1024; i++) {
    test_code(i, conv_decoder::RATE_5_6);
  }
}

BOOST_AUTO_TEST_CASE(various_msg_len_7_8)
{
  for (uint32_t i = 4; i < 1024; i++) {
    test_code(i, conv_decoder::RATE_7_8);
  }
}

}  // namespace satnogs
}  // namespace gr
