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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <satnogs/ieee802_15_4_encoder.h>

namespace gr {
namespace satnogs {

encoder::encoder_sptr
ieee802_15_4_encoder::make(uint8_t preamble, size_t preamble_len,
                           const std::vector<uint8_t> &sync_word, crc::crc_t crc,
                           whitening::whitening_sptr scrambler,
                           bool var_len)
{
  return encoder::encoder_sptr(new ieee802_15_4_encoder(preamble, preamble_len,
                               sync_word, crc, scrambler, var_len));
}

ieee802_15_4_encoder::ieee802_15_4_encoder(
  uint8_t preamble, size_t preamble_len,
  const std::vector<uint8_t> &sync_word, crc::crc_t crc,
  whitening::whitening_sptr scrambler, bool var_len) :
  d_max_frame_len(255),
  d_preamle_len(preamble_len),
  d_crc(crc),
  d_scrambler(scrambler),
  d_var_len(var_len)
{
  if (!sync_word.size()) {
    throw std::invalid_argument("ieee802_15_4_encoder: SYNC word should be at least one byte");
  }
  d_buffer = new uint8_t[preamble_len + sync_word.size() + crc::crc_size(
                                        crc) + d_max_frame_len + var_len];
  std::fill_n(d_buffer, preamble_len, preamble);
  std::copy(sync_word.begin(), sync_word.end(), d_buffer + preamble_len);
  d_payload = d_buffer + preamble_len + sync_word.size();
}

ieee802_15_4_encoder::~ieee802_15_4_encoder()
{
  delete [] d_buffer;
}

pmt::pmt_t
ieee802_15_4_encoder::encode(pmt::pmt_t msg)
{
  pmt::pmt_t b;
  if (pmt::is_blob(msg)) {
    b = msg;
  }
  else if (pmt::is_pair(msg)) {
    b = pmt::cdr(msg);
  }
  else {
    throw std::runtime_error("ieee802_15_4_encoder: received a malformed pdu message");
  }

  size_t pdu_len(0);
  const uint8_t *pdu = (const uint8_t *) pmt::uniform_vector_elements(b, pdu_len);
  if (pdu_len > d_max_frame_len) {
    throw std::runtime_error("ieee802_15_4_encoder: PDU received has a size larger than the maximum allowed");
  }

  uint8_t *payload = d_payload;
  if (d_var_len) {
    *payload++ = pdu_len;
  }
  std::copy(pdu, pdu + pdu_len, payload);
  payload += pdu_len;

  /* CRC includes the length byte */
  payload += crc::append(d_crc, payload, d_payload, pdu_len + d_var_len);
  if (d_scrambler) {
    d_scrambler->reset();
    d_scrambler->scramble(d_payload, d_payload, (size_t)(payload - d_payload));
  }

  /* Make a pmt compatible with the pdu to tagged stream block */
  pmt::pmt_t vecpmt(pmt::make_blob(d_buffer, (size_t)(payload - d_buffer)));
  pmt::pmt_t res(pmt::cons(pmt::PMT_NIL, vecpmt));
  return res;
}

} /* namespace satnogs */
} /* namespace gr */

