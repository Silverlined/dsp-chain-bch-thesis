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

#include "ax100_mode5_encoder.h"
#include <satnogs/golay24.h>
#include <fec.h>
#include <netinet/in.h>

namespace gr {
namespace satnogs {

encoder::encoder_sptr
ax100_mode5_encoder::make(const std::vector<uint8_t> &preamble,
                          const std::vector<uint8_t> &sync,
                          crc::crc_t crc,
                          bool enable_rs)
{
  return encoder::encoder_sptr
         (new ax100_mode5_encoder(preamble, sync, crc, enable_rs));
}

ax100_mode5_encoder::ax100_mode5_encoder(const std::vector<uint8_t> &preamble,
    const std::vector<uint8_t> &sync,
    crc::crc_t crc,
    bool enable_rs) :
  d_crc(crc),
  d_rs(enable_rs)
{
  if (sync.size() * 8 < 8) {
    throw std::invalid_argument("SYNC word should be at least 8 bits");
  }

  d_pdu = new uint8_t[preamble.size() + sync.size() + 3 + 1024 + 4];

  std::copy(preamble.begin(), preamble.end(), d_pdu);
  std::copy(sync.begin(), sync.end(), d_pdu + preamble.size());

  d_payload_start = preamble.size() + sync.size();
}

ax100_mode5_encoder::~ax100_mode5_encoder()
{
  delete[] d_pdu;
}

pmt::pmt_t
ax100_mode5_encoder::encode(pmt::pmt_t msg)
{
  pmt::pmt_t b;
  if (pmt::is_blob(msg)) {
    b = msg;
  }
  else if (pmt::is_pair(msg)) {
    b = pmt::cdr(msg);
  }
  else {
    throw std::runtime_error("ax100_mode5_encoder: received a malformed pdu message");
  }

  size_t pdu_len(0);
  const uint8_t *pdu = (const uint8_t *) pmt::uniform_vector_elements(b, pdu_len);
  if (pdu_len > 1024) {
    throw std::runtime_error("ax100_mode5_encoder: PDU received has a size larger than the maximum allowed");
  }

  size_t step = d_rs ? 32 : 0;
  golay24 g = golay24();
  uint32_t enc_len = g.encode12((uint16_t)(pdu_len + step + crc::crc_size(d_crc)),
                                false);
  write_24bits(enc_len, d_payload_start);

  if (d_rs) {
    // RS encoding
    uint8_t *payload = new uint8_t[pdu_len];
    uint8_t parity[32] = {0x00};

    std::copy(pdu, pdu + pdu_len, payload);

    encode_rs_8(payload, parity, 223 - pdu_len);
    std::copy(payload, payload + pdu_len, d_pdu + d_payload_start + 3);
    std::copy(parity, parity + 32, d_pdu + d_payload_start + 3 + pdu_len);

    delete[] payload;
  }
  else {
    std::copy(pdu, pdu + pdu_len, d_pdu + d_payload_start + 3);
  }
  step += add_crc(pdu, pdu_len, d_payload_start + 3 + pdu_len + step);

  pmt::pmt_t vecpmt(pmt::make_blob(d_pdu, d_payload_start + 3 + pdu_len + step));
  pmt::pmt_t res(pmt::cons(pmt::PMT_NIL, vecpmt));
  return res;
}

void ax100_mode5_encoder::write_16bits(uint16_t u16, size_t ind)
{
  uint8_t b = 0x00;
  b = ((u16 >> 15) & 0x1) << 7;
  b |= ((u16 >> 14) & 0x1) << 6;
  b |= ((u16 >> 13) & 0x1) << 5;
  b |= ((u16 >> 12) & 0x1) << 4;
  b |= ((u16 >> 11) & 0x1) << 3;
  b |= ((u16 >> 10) & 0x1) << 2;
  b |= ((u16 >> 9) & 0x1) << 1;
  b |= (u16 >> 8) & 0x1;
  d_pdu[ind] = b;

  b = ((u16 >> 7) & 0x1) << 7;
  b |= ((u16 >> 6) & 0x1) << 6;
  b |= ((u16 >> 5) & 0x1) << 5;
  b |= ((u16 >> 4) & 0x1) << 4;
  b |= ((u16 >> 3) & 0x1) << 3;
  b |= ((u16 >> 2) & 0x1) << 2;
  b |= ((u16 >> 1) & 0x1) << 1;
  b |= u16 & 0x1;
  d_pdu[ind + 1] = b;
}

void ax100_mode5_encoder::write_24bits(uint32_t u32, size_t ind)
{
  uint8_t b = 0x00;
  b = ((u32 >> 23) & 0x01) << 7;
  b |= ((u32 >> 22) & 0x01) << 6;
  b |= ((u32 >> 21) & 0x01) << 5;
  b |= ((u32 >> 20) & 0x01) << 4;
  b |= ((u32 >> 19) & 0x01) << 3;
  b |= ((u32 >> 18) & 0x01) << 2;
  b |= ((u32 >> 17) & 0x01) << 1;
  b |= (u32 >> 16) & 0x01;
  d_pdu[ind] = b;

  write_16bits((uint16_t) u32, ind + 1);
}

size_t ax100_mode5_encoder::add_crc(const uint8_t *pdu, size_t pdu_len,
                                    size_t ind)
{
  switch (d_crc) {
  case crc::CRC_NONE:
    return 0;
  case crc::CRC32_C: {
    uint32_t crc32_c = crc::crc32_c(pdu, pdu_len);
    crc32_c = htonl(crc32_c);
    uint8_t b = 0x00;
    b = ((crc32_c >> 31) & 0x01) << 7;
    b |= ((crc32_c >> 30) & 0x01) << 6;
    b |= ((crc32_c >> 29) & 0x01) << 5;
    b |= ((crc32_c >> 28) & 0x01) << 4;
    b |= ((crc32_c >> 27) & 0x01) << 3;
    b |= ((crc32_c >> 26) & 0x01) << 2;
    b |= ((crc32_c >> 25) & 0x01) << 1;
    b |= (crc32_c >> 24) & 0x01;
    d_pdu[ind] = b;
    write_24bits(crc32_c, ind + 1);
  }
  return 4;
  default:
    return 0;
  }
}

} /* namespace satnogs */
} /* namespace gr */
