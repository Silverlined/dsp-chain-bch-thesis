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

#include <satnogs/ax25_encoder.h>
#include <satnogs/ax25.h>
#include <satnogs/whitening.h>
#include <iostream>

namespace gr {

namespace satnogs {

/**
 * AX.25 encoder that supports the legacy hardware radios.
 *
 * The block takes as inputs blob or  GNU Radio pair PMT messages
 * and generates a byte stream.
 * Each output byte contains only one LSB, thus the output can be directly
 * used for FM modulation.
 *
 * @param dest_addr the destination callsign
 * @param dest_ssid the destination SSID
 * @param src_addr the source callsign
 * @param src_ssid the source SSID
 * @param preamble_len the number of times that the AX.25 synchronization flags
 * should be repeated in front of the frame.
 * @param postamble_len the number of times that the AX.25 synchronization flags
 * should be repeated at the end of the frame.
 * @param scramble if set to true, G3RUH scrambling will be performed
 * after bit stuffing
 * @param nrzi if set to true, the output in NRZI encoded
 *
 * @returns shared pointer to an AX.25 encoder definition
 */

encoder::encoder_sptr
ax25_encoder::make(const std::string &dest_addr, uint8_t dest_ssid,
                   const std::string &src_addr, uint8_t src_ssid, size_t preamble_len,
                   size_t postamble_len, bool scramble, bool nrzi)
{
  return encoder::encoder_sptr(new ax25_encoder(dest_addr, dest_ssid, src_addr,
                               src_ssid, preamble_len,
                               postamble_len, scramble,
                               nrzi));
}

ax25_encoder::ax25_encoder(const std::string &dest_addr, uint8_t dest_ssid,
                           const std::string &src_addr, uint8_t src_ssid,
                           size_t preamble_len, size_t postamble_len,
                           bool scramble, bool nrzi) :
  d_dest_addr(dest_addr),
  d_dest_ssid(dest_ssid),
  d_src_addr(src_addr),
  d_src_ssid(src_ssid),
  d_preamble_len(preamble_len),
  d_postamble_len(postamble_len),
  d_scramble(scramble),
  d_nrzi(nrzi)
{
  if (dest_addr.length() == 0 || dest_addr.length() > ax25::callsign_max_len) {
    throw std::invalid_argument("ax25_encoder: Invalid destination callsign");
  }

  if (src_addr.length() == 0 || src_addr.length() > ax25::callsign_max_len) {
    throw std::invalid_argument("ax25_encoder: Invalid source callsign");
  }
}

ax25_encoder::~ax25_encoder()
{
}

pmt::pmt_t
ax25_encoder::encode(pmt::pmt_t msg)
{
  pmt::pmt_t b;
  if (pmt::is_blob(msg)) {
    b = msg;
  }
  else if (pmt::is_pair(msg)) {
    b = pmt::cdr(msg);
  }
  else {
    throw std::runtime_error("ax25_encoder: received a malformed pdu message");
  }

  size_t pdu_len(0);
  const uint8_t *pdu = (const uint8_t *) pmt::uniform_vector_elements(b, pdu_len);

  /*
   * Allocate enough memory. The worst case scemario is that all frame data
   * may need bit stuffing
   */
  uint8_t *ax25_pdu = new uint8_t[d_preamble_len + d_postamble_len +
                                                 pdu_len + pdu_len / 8 + ax25::max_header_len +
                                                 4 ];
  uint8_t *tmp = new uint8_t[ax25::max_header_len + pdu_len + 2];

  size_t idx = 0;
  idx += insert_address(tmp);
  idx += insert_ctrl(tmp + idx);
  idx += insert_pid(tmp + idx);

  std::copy(pdu, pdu + pdu_len, tmp + idx);
  idx += pdu_len;
  uint16_t crc = ax25::crc(tmp, idx);
  tmp[idx++] = crc;
  tmp[idx++] = crc >> 8;

  const size_t len = idx;

  /*
   * Start placing the payload and perform bit stuffing.
   * NOTE: AX.25 handles bits LSB first.
   */
  std::fill_n(ax25_pdu, d_preamble_len, ax25::sync_flag);
  idx = d_preamble_len;
  uint8_t sr = 0;
  uint8_t enc_bits = 0;
  for (size_t i = 0; i < len * 8; i++) {
    const uint8_t bit = (tmp[i / 8] >> (i % 8)) & 0x1;
    sr = (sr << 1) | bit;
    enc_bits++;

    if (enc_bits == 8) {
      enc_bits = 0;
      ax25_pdu[idx++] = sr;
    }

    if ((sr & 0x1F) == 0x1F) {
      sr = sr << 1;
      enc_bits++;
    }

    if (enc_bits == 8) {
      enc_bits = 0;
      ax25_pdu[idx++] = sr;
    }
  }

  /*
   * Apply the postable. Due to bit stuffing, the payload may have not stopped
   * to a byte boundary
   */
  for (size_t i = 0; i < d_postamble_len * 8; i++) {
    const uint8_t bit = (ax25::sync_flag >> (i % 8)) & 0x1;
    sr = (sr << 1) | bit;
    enc_bits++;
    if (enc_bits == 8) {
      enc_bits = 0;
      ax25_pdu[idx++] = sr;
    }
  }
  /*
   * Fill with zeros the leftover bits of the last byte. This should not
   * affect the transmission of the frame
   */
  if (enc_bits) {
    ax25_pdu[idx++] = sr << (8 - enc_bits);
  }

  if (d_scramble) {
    /*
     * Frame encoder handles the bits in MS bit first order. Therefore,
     * ax25_pdu contains data in MSB order and at the same order the scrambler
     * should process them
     */
    whitening::whitening_sptr g3ruh = whitening::make_g3ruh(true);
    g3ruh->scramble(ax25_pdu, ax25_pdu, idx);
  }

  if (d_nrzi) {
    uint8_t prev = 0;
    for (size_t i = 0; i < idx; i++) {
      uint8_t inv = ~ax25_pdu[i];
      prev = (inv >> 7) ^ prev;
      uint8_t b = prev  << 7;

      prev = ((inv >> 6) & 0x1) ^ prev;
      b |= (prev & 0x1) << 6;

      prev = ((inv >> 5) & 0x1) ^ prev;
      b |= (prev & 0x1) << 5;

      prev = ((inv >> 4) & 0x1) ^ prev;
      b |= (prev & 0x1) << 4;

      prev = ((inv >> 3) & 0x1) ^ prev;
      b |= (prev & 0x1) << 3;

      prev = ((inv >> 2) & 0x1) ^ prev;
      b |= (prev & 0x1) << 2;

      prev = ((inv >> 1) & 0x1) ^ prev;
      b |= (prev & 0x1) << 1;

      prev = (inv  & 0x1) ^ prev;
      b |= (prev & 0x1);

      ax25_pdu[i] = b;
    }
  }

  /* Make a pmt compatible with the pdu to tagged stream block */
  pmt::pmt_t vecpmt(pmt::make_blob(ax25_pdu, idx));
  pmt::pmt_t res(pmt::cons(pmt::PMT_NIL, vecpmt));
  delete [] ax25_pdu;
  delete [] tmp;
  return res;
}

size_t
ax25_encoder::insert_address(uint8_t *out)
{
  size_t i;
  for (i = 0; i < d_dest_addr.length(); i++) {
    *out++ = d_dest_addr[i] << 1;
  }
  /*
   * Perhaps the destination callsign was smaller that the maximum allowed.
   * In this case the leftover bytes should be filled with space
   */
  for (; i < ax25::callsign_max_len; i++) {
    *out++ = ' ' << 1;
  }
  /* Apply SSID, reserved and C bit */
  /* FIXME: C bit is set to 0 implicitly */
  *out++ = ((0b1111 & d_dest_ssid) << 1) | 0b01100000;

  for (i = 0; i < d_src_addr.length(); i++) {
    *out++ = d_src_addr[i] << 1;
  }
  for (; i < ax25::callsign_max_len; i++) {
    *out++ = ' ' << 1;
  }
  /* Apply SSID, reserved and C bit. As this is the last address field
   * the trailing bit is set to 1.
   * /
   /* FIXME: C bit is set to 0 implicitly */
  *out++ = ((0b1111 & d_src_ssid) << 1) | 0b01100001;
  return ax25::min_addr_len;
}

size_t
ax25_encoder::insert_ctrl(uint8_t *out)
{
  *out = 0;
  return 1;
}

size_t
ax25_encoder::insert_pid(uint8_t *out)
{
  /*
   * FIXME: For now, only the "No layer 3 is implemented" information is
   * implemented
   */
  *out = 0xF0;
  return 1;
}


}  // namespace satnogs

}  // namespace gr


