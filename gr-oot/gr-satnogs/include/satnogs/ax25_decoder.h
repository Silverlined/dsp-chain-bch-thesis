/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2019, 2020 Libre Space Foundation <http://libre.space>
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

#ifndef INCLUDED_SATNOGS_AX25_DECODER_H
#define INCLUDED_SATNOGS_AX25_DECODER_H

#include <satnogs/api.h>
#include <satnogs/decoder.h>
#include <gnuradio/digital/lfsr.h>

#include <deque>

namespace gr {
namespace satnogs {

/*!
 * \brief AX.25 decoder that supports the legacy hardware radios.
 *
 * This block takes as input a quadrature demodulated bit stream.
 * Each byte should contains only one bit of information at the LSB.
 *
 * The block will try to find an AX.25 frame. If the frame pass the
 * CRC check then a blob PMT message is produced at the message output
 * indicated with name 'out'. Otherwise if the frame did not pass the
 * CRC check or the size was invalid, a blob PMT message is generated at
 * the output port with the name 'fail'. This will help to recover at least
 * some bytes from a corrupted message.
 *
 * The block also supports destination callsign check. Only frames with
 * the right destination Callsign will be accepted. This feature can be
 * disabled using the promisc parameter.
 *
 * \ingroup satnogs
 *
 */
class SATNOGS_API ax25_decoder : public decoder {
public:

  /**
   * The decoder take as input a quadrature demodulated bit stream.
   * Each byte should contains only one bit of information at the LSB.
   *
   * The decoder will try to find an AX.25 frame. If the frame pass the
   * CRC check then at the metadata the CRC_VALID option will be set to true,
   * otherwise to false. CRC invalid frames are meaningful because only one
   * bit of error can cause the entire frame to fail. This will help to recover
   * at least some bytes from a corrupted message.
   *
   * The decoder also supports destination callsign check. Only frames with
   * the right destination Callsign will be accepted. This feature can be
   * disabled using the promisc parameter.
   * @param addr the Callsign of the receiver
   * @param ssid the SSID of the receiver
   * @param promisc if set to yes, the Callsign check is disabled
   * @param descramble if set to yes, the data will be descrambled prior
   * decoding using the G3RUH self-synchronizing descrambler.
   * @param max_frame_len the maximum allowed frame length
   * @param error_correction set to true to enable the 1-bit error correction
   *
   * @return a shared pointer of the decoder instance
   */
  static decoder::decoder_sptr
  make(const std::string &addr, uint8_t ssid, bool promisc = false,
       bool descramble = true, bool crc_check = true,
       size_t max_frame_len = 512,
       bool error_correction = false);

  /**
   * The decoder take as input a quadrature demodulated bit stream.
   * Each byte should contains only one bit of information at the LSB.
   *
   * The decoder will try to find an AX.25 frame. If the frame pass the
   * CRC check then at the metadata the CRC_VALID option will be set to true,
   * otherwise to false. CRC invalid frames are meaningful because only one
   * bit of error can cause the entire frame to fail. This will help to recover
   * at least some bytes from a corrupted message.
   *
   * The decoder also supports destination callsign check. Only frames with
   * the right destination Callsign will be accepted. This feature can be
   * disabled using the promisc parameter.
   * @param addr the Callsign of the receiver
   * @param ssid the SSID of the receiver
   * @param promisc if set to yes, the Callsign check is disabled
   * @param descramble if set to yes, the data will be descrambled prior
   * decoding using the G3RUH self-synchronizing descrambler.
   * @param crc_check bypass the CRC check of the frame
   * @param max_frame_len the maximum allowed frame length
   * @param error_correction set to true to enable the 1-bit error correction
   */
  ax25_decoder(const std::string &addr, uint8_t ssid, bool promisc = false,
               bool descramble = true, bool crc_check = true,
               size_t max_frame_len = 512,
               bool error_correction = false);

  ~ax25_decoder();

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
  const bool d_promisc;
  const bool d_descramble;
  const bool d_crc_check;
  const size_t d_max_frame_len;
  const bool d_error_correction;
  decoding_state_t d_state;
  uint8_t d_shift_reg;
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
  bool
  is_frame_valid();
  bool
  error_correction();
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_AX25_DECODER_H */

