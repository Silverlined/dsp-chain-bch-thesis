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

#ifndef INCLUDED_SATNOGS_ARGOS_LDR_DECODER_H
#define INCLUDED_SATNOGS_ARGOS_LDR_DECODER_H

#include <satnogs/api.h>
#include <satnogs/decoder.h>

#include <deque>

namespace gr {
namespace satnogs {

/*!
 * \brief ARGOS Low Data Rate HDLC decoder
 *
 * \ingroup satnogs
 *
 */
class SATNOGS_API argos_ldr_decoder : public decoder {
public:

  /**
   * The decoder take as input a quadrature demodulated bit stream.
   * Each byte should contains only one bit of information at the LSB.
   *
   * propagated
   * @param max_frame_len the maximum allowed frame length
   *
   * @return a shared pointer of the decoder instance
   */
  static decoder::decoder_sptr
  make(bool crc_check = true, size_t max_frame_len = 64);

  /**
  /**
   * The decoder take as input a quadrature demodulated bit stream.
   * Each byte should contains only one bit of information at the LSB.
   *
   * propagated
   * @param max_frame_len the maximum allowed frame length
   *
   * @return a shared pointer of the decoder instance
   */
  argos_ldr_decoder(bool crc_check = true, size_t max_frame_len = 64);

  ~argos_ldr_decoder();

  decoder_status_t
  decode(const void *in, int len);

  void
  reset();
private:

  typedef enum {
    NO_SYNC, IN_SYNC, DECODING
  } decoding_state_t;

  const bool d_crc_check;
  const size_t d_max_frame_len;
  decoding_state_t d_state;
  uint32_t d_shift_reg;
  uint8_t d_dec_b;
  size_t d_received_bytes;
  size_t d_decoded_bits;
  uint8_t *d_frame_buffer;
  std::deque<uint8_t> d_bitstream;
  size_t d_start_idx;
  uint64_t d_frame_start;
  uint64_t d_sample_cnt;

  void
  reset_state();
  void
  enter_sync_state();
  void
  enter_decoding_state();
  bool
  enter_frame_end(decoder_status_t &status);

  bool
  _decode(decoder_status_t &status);

  inline void
  decode_1b(uint8_t in);
  bool
  frame_check();
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_ARGOS_LDR_DECODER_H */

