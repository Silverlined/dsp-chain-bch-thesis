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

#include <satnogs/usp_encoder.h>
#include <satnogs/reed_muller.h>
#include <satnogs/utils.h>
#include <fec.h>
#include <bitset>

namespace gr {
namespace satnogs {

encoder::encoder_sptr
usp_encoder::make(const std::vector<uint8_t> &preamble,
                  const std::vector<uint8_t> &sync,
                  uint8_t pls_code,
                  whitening::whitening_sptr scrambler)
{
  return encoder::encoder_sptr(new usp_encoder(preamble, sync,
                               pls_code, scrambler));
}

usp_encoder::usp_encoder(const std::vector<uint8_t> &preamble,
                         const std::vector<uint8_t> &sync,
                         uint8_t pls_code,
                         whitening::whitening_sptr scrambler) :
  d_max_frame_len(1024), // FIXME: correct max frame len,
  d_payload_start(preamble.size() + sync.size() + 8),
  d_scrambler(scrambler),
  d_conv(conv_encoder(conv_encoder::RATE_1_2, d_max_frame_len)),
  d_pdu(new uint8_t[d_payload_start + final_pdu_length(d_max_frame_len) / 8])
{
  if (pls_code > 127) {
    throw std::invalid_argument("usp_encoder: PLS Code must be less than 127 (7-bit)");
  }
  else if (pls_code % 2 != 0) {
    throw std::invalid_argument("usp_encoder: the last bit of PLS Code must be 0");
  }

  std::copy(preamble.begin(), preamble.end(), d_pdu);
  std::copy(sync.begin(), sync.end(), d_pdu + preamble.size());

  // Encode and add PLS Code
  reed_muller rm;
  std::bitset<64> bs_coded_pls(rm.encode(pls_code));
  const std::bitset<64> scramble_val(
    0b0111000110011101100000111100100101010011010000100010110111111010);
  for (size_t i = 0; i < 64; ++i) {
    bs_coded_pls[i] = (bs_coded_pls[i] + scramble_val[i]) % 2;
  }
  uint64_t coded_pls = htobe64(bs_coded_pls.to_ullong());
  std::memcpy(d_pdu + d_payload_start - sizeof(coded_pls), &coded_pls,
              sizeof(coded_pls));
}

usp_encoder::~usp_encoder()
{
  delete[] d_pdu;
}

pmt::pmt_t
usp_encoder::encode(pmt::pmt_t msg)
{
  pmt::pmt_t b;
  if (pmt::is_blob(msg)) {
    b = msg;
  }
  else if (pmt::is_pair(msg)) {
    b = pmt::cdr(msg);
  }
  else {
    throw std::runtime_error("usp_encoder: received a malformed pdu message");
  }

  size_t pdu_len(0);
  const uint8_t *pdu = (const uint8_t *) pmt::uniform_vector_elements(b, pdu_len);
  if (pdu_len > d_max_frame_len) {
    throw std::runtime_error("usp_encoder: PDU received has a size larger than the maximum allowed");
  }

  uint8_t *payload = new uint8_t[final_pdu_length(pdu_len) / 8];
  uint8_t *tmp_payload = new uint8_t[final_pdu_length(pdu_len)];
  uint8_t parity[32] = {0x00};
  std::copy(pdu, pdu + pdu_len, payload);

  // RS encoding
  encode_rs_8(payload, parity, 255 - pdu_len);
  std::copy(parity, parity + 32, payload + pdu_len);

  // Scramble
  if (d_scrambler) {
    d_scrambler->reset();
    d_scrambler->scramble(payload, payload, pdu_len + 32);
  }

  // Convolutional coding
  size_t unpacked_len = utils::packed_to_unpacked<uint8_t *, uint8_t *, false>
                        (tmp_payload, payload, pdu_len + 32);
  d_conv.reset();
  size_t conv_len = d_conv.encode(tmp_payload, tmp_payload, unpacked_len);
  // Padding
  tmp_payload[final_pdu_length(pdu_len) - 4] = 0;
  tmp_payload[final_pdu_length(pdu_len) - 3] = 0;
  tmp_payload[final_pdu_length(pdu_len) - 2] = 0;
  tmp_payload[final_pdu_length(pdu_len) - 1] = 0;
  size_t payload_len = utils::unpacked_to_packed<uint8_t *, uint8_t *, false>
                       (payload, tmp_payload, final_pdu_length(pdu_len));

  std::copy(payload, payload + payload_len, d_pdu + d_payload_start);

  delete[] payload;
  delete[] tmp_payload;

  pmt::pmt_t vecpmt(pmt::make_blob(d_pdu, d_payload_start + payload_len));
  pmt::pmt_t res(pmt::cons(pmt::PMT_NIL, vecpmt));
  return res;
}

size_t
usp_encoder::final_pdu_length(size_t len) const
{
  return (len + 32) * 16 + 6 * 2 + 4;
}

}  // namespace satnogs
}  // namespace gr
