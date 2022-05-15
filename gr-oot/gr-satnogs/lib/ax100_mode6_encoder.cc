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

#include "ax100_mode6_encoder.h"
#include <fec.h>

namespace gr {
namespace satnogs {

encoder::encoder_sptr
ax100_mode6_encoder::make(size_t preamble_len, size_t postamble_len,
                          whitening::whitening_sptr scrambler, bool scramble,
                          bool nrzi, crc::crc_t crc)
{
  return encoder::encoder_sptr
         (new ax100_mode6_encoder(preamble_len, postamble_len,
                                  scrambler, scramble, nrzi, crc));
}

ax100_mode6_encoder::ax100_mode6_encoder(size_t preamble_len,
    size_t postamble_len,
    whitening::whitening_sptr scrambler, bool scramble,
    bool nrzi, crc::crc_t crc) :
  d_max_frame_len(255),
  d_ax25("GND", 0x60, "GND", 0x61, preamble_len,
         postamble_len, scramble, nrzi),
  d_scrambler(scrambler),
  d_crc(crc)
{
  d_pdu = new uint8_t[d_max_frame_len + 4 + 32];
}

ax100_mode6_encoder::~ax100_mode6_encoder()
{
  delete[] d_pdu;
}

pmt::pmt_t
ax100_mode6_encoder::encode(pmt::pmt_t msg)
{
  pmt::pmt_t b;
  if (pmt::is_blob(msg)) {
    b = msg;
  }
  else if (pmt::is_pair(msg)) {
    b = pmt::cdr(msg);
  }
  else {
    throw std::runtime_error("ax100_mode6_encoder: received a malformed pdu message");
  }

  size_t pdu_len(0);
  const uint8_t *pdu = (const uint8_t *) pmt::uniform_vector_elements(b, pdu_len);
  if (pdu_len > d_max_frame_len) {
    throw std::runtime_error("ax100_mode6_encoder: PDU received has a size larger than the maximum allowed");
  }

  std::copy(pdu, pdu + pdu_len, d_pdu);

  // Adding CRC
  size_t step = add_crc(pdu_len);

  // RS encoding
  uint8_t parity[32] = {0x00};
  encode_rs_8(d_pdu, parity, 223 - pdu_len - step);
  std::copy(parity, parity + 32, d_pdu + pdu_len + step);

  // Scramble
  if (d_scrambler) {
    d_scrambler->reset();
    d_scrambler->scramble(d_pdu, d_pdu, pdu_len + step + 32);
  }

  pmt::pmt_t vecpmt(pmt::make_blob(d_pdu, pdu_len + step + 32));
  pmt::pmt_t res(pmt::cons(pmt::PMT_NIL, vecpmt));

  // Return the AX.25 encoding msg
  return d_ax25.encode(res);
}

size_t
ax100_mode6_encoder::add_crc(size_t ind)
{
  // Calculating and adding CRC after RS parity
  switch (d_crc) {
  case crc::CRC_NONE:
    return 0;
  case crc::CRC32_C: {
    uint32_t crc32_c = crc::crc32_c(d_pdu, ind);
    uint8_t bcrc = 0x00;
    bcrc = ((crc32_c >> 31) & 0x01) << 7;
    bcrc |= ((crc32_c >> 30) & 0x01) << 6;
    bcrc |= ((crc32_c >> 29) & 0x01) << 5;
    bcrc |= ((crc32_c >> 28) & 0x01) << 4;
    bcrc |= ((crc32_c >> 27) & 0x01) << 3;
    bcrc |= ((crc32_c >> 26) & 0x01) << 2;
    bcrc |= ((crc32_c >> 25) & 0x01) << 1;
    bcrc |= (crc32_c >> 24) & 0x01;
    d_pdu[ind] = bcrc;

    bcrc = ((crc32_c >> 23) & 0x01) << 7;
    bcrc |= ((crc32_c >> 22) & 0x01) << 6;
    bcrc |= ((crc32_c >> 21) & 0x01) << 5;
    bcrc |= ((crc32_c >> 20) & 0x01) << 4;
    bcrc |= ((crc32_c >> 19) & 0x01) << 3;
    bcrc |= ((crc32_c >> 18) & 0x01) << 2;
    bcrc |= ((crc32_c >> 17) & 0x01) << 1;
    bcrc |= (crc32_c >> 16) & 0x01;
    d_pdu[ind + 1] = bcrc;

    bcrc = ((crc32_c >> 15) & 0x01) << 7;
    bcrc |= ((crc32_c >> 14) & 0x01) << 6;
    bcrc |= ((crc32_c >> 13) & 0x01) << 5;
    bcrc |= ((crc32_c >> 12) & 0x01) << 4;
    bcrc |= ((crc32_c >> 11) & 0x01) << 3;
    bcrc |= ((crc32_c >> 10) & 0x01) << 2;
    bcrc |= ((crc32_c >> 9) & 0x01) << 1;
    bcrc |= (crc32_c >> 8) & 0x01;
    d_pdu[ind + 2] = bcrc;

    bcrc = ((crc32_c >> 7) & 0x01) << 7;
    bcrc |= ((crc32_c >> 6) & 0x01) << 6;
    bcrc |= ((crc32_c >> 5) & 0x01) << 5;
    bcrc |= ((crc32_c >> 4) & 0x01) << 4;
    bcrc |= ((crc32_c >> 3) & 0x01) << 3;
    bcrc |= ((crc32_c >> 2) & 0x01) << 2;
    bcrc |= ((crc32_c >> 1) & 0x01) << 1;
    bcrc |= crc32_c & 0x01;
    d_pdu[ind + 3] = bcrc;

    return 4;
  }
  default:
    throw std::runtime_error("ax100_mode6_encoder: Invalid CRC");
  }

  return 0;
}

} /* namespace satnogs */
} /* namespace gr */
