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

#ifndef INCLUDED_SATNOGS_AX100_MODE6_H
#define INCLUDED_SATNOGS_AX100_MODE6_H

#include <satnogs/api.h>
#include <satnogs/decoder.h>
#include <gnuradio/digital/lfsr.h>
#include <satnogs/crc.h>
#include <satnogs/whitening.h>

#include <deque>

namespace gr {
namespace satnogs {

/*!
 * \brief AX.100 mode 6 decoder
 *
 * Mode 6 Decoder for the AX100 modem of GomSpace.
 *
 * In this particular mode, the modem encapsulates the payload inside an
 * AX.25 frame. The payload is appended with a Castagnoli CRC32, scrambled with
 * a CCSDS scrambler and encoded using the CCSDS compliant Reed Solomon.
 *
 * This non-sense framing scheme is used mainly for getting data
 * from legacy ham stations receiving and reporting AX.25 frames.
 *
 * The implementation drops any kind of AX.25 information and does not check
 * the 16-bit CRC, allowing the Reed Solomon to correct any error bits.
 *
 * \ingroup satnogs
 *
 */
class SATNOGS_API ax100_mode6 : public decoder {
public:


  static decoder::decoder_sptr
  make(crc::crc_t crc = crc::CRC32_C,
       whitening::whitening_sptr descrambler = whitening::make_ccsds(),
       bool ax25_descramble = true);

  ax100_mode6(crc::crc_t crc, whitening::whitening_sptr descrambler,
              bool ax25_descramble);

  ~ax100_mode6();

  decoder_status_t
  decode(const void *in, int len);

  void
  reset();
private:

  typedef enum {
    NO_SYNC, IN_SYNC, DECODING
  } decoding_state_t;

  /**
   * If this flag is set, the decoder operates in promiscuous mode and
   * forwards all successfully decoded frames
   */
  const crc::crc_t d_crc;
  const bool d_ax25_descramble;
  const size_t d_max_frame_len;
  whitening::whitening_sptr d_descrambler;
  decoding_state_t d_state;
  uint32_t d_shift_reg;
  uint8_t d_dec_b;
  uint8_t d_prev_bit_nrzi;
  size_t d_received_bytes;
  size_t d_decoded_bits;
  digital::lfsr d_lfsr;
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
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_AX100_MODE6_H */

