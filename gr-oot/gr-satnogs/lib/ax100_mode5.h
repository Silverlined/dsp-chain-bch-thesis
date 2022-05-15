/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2019, Libre Space Foundation <http://libre.space>
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

#ifndef INCLUDED_SATNOGS_AX100_MODE5_H
#define INCLUDED_SATNOGS_AX100_MODE5_H

#include <satnogs/api.h>
#include <satnogs/decoder.h>
#include <satnogs/shift_reg.h>
#include <satnogs/crc.h>
#include <satnogs/whitening.h>

namespace gr {
namespace satnogs {

/*!
 * \brief This decode implements the AX100 mode 5 scheme
 *
 */
class SATNOGS_API ax100_mode5 : public decoder {
public:
  static decoder_sptr
  make(const std::vector<uint8_t> &preamble,
       size_t preamble_threshold,
       const std::vector<uint8_t> &sync,
       size_t sync_threshold,
       crc::crc_t crc,
       whitening::whitening_sptr descrambler,
       bool enable_rs);

  ax100_mode5(const std::vector<uint8_t> &preamble,
              size_t preamble_threshold,
              const std::vector<uint8_t> &sync,
              size_t sync_threshold,
              crc::crc_t crc,
              whitening::whitening_sptr descrambler,
              bool enable_rs);
  ~ax100_mode5();

  decoder_status_t
  decode(const void *in, int len);

  void
  reset();

  size_t
  input_multiple() const;

private:
  /**
   * Decoding FSM
   */
  typedef enum {
    SEARCHING,                  //!< when searching for the start of the preamble
    SEARCHING_SYNC,             //!< We have preamble, search for sync
    DECODING_FRAME_LEN,         //!< Decoding the frame length
    DECODING_PAYLOAD            //!< Decoding the payload
  } decoding_state_t;

  shift_reg                     d_preamble;
  shift_reg                     d_preamble_shift_reg;
  const size_t                  d_preamble_len;
  const size_t                  d_preamble_thrsh;
  shift_reg                     d_sync;
  shift_reg                     d_sync_shift_reg;
  const size_t                  d_sync_len;
  const size_t                  d_sync_thrsh;
  crc::crc_t                    d_crc;
  whitening::whitening_sptr     d_descrambler;
  const bool                    d_rs;
  decoding_state_t              d_state;
  size_t                        d_cnt;
  size_t                        d_len;
  size_t                        d_length_field_len;
  uint8_t                       *d_pdu;
  uint64_t                      d_frame_start;

  int
  search_preamble(const uint8_t *in, int len);

  int
  search_sync(const uint8_t *in, int len);

  int
  decode_frame_len(const uint8_t *in, int len);

  void
  decode_payload(decoder_status_t &status, const uint8_t *in, int len);

  bool
  check_crc();
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_AX100_MODE5_H */

