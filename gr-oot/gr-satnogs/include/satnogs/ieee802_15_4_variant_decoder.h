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

#ifndef INCLUDED_SATNOGS_IEEE802_15_4_VARIANT_DECODER_H
#define INCLUDED_SATNOGS_IEEE802_15_4_VARIANT_DECODER_H

#include <satnogs/api.h>
#include <satnogs/decoder.h>
#include <satnogs/whitening.h>
#include <satnogs/crc.h>
#include <satnogs/shift_reg.h>


namespace gr {
namespace satnogs {

/*!
 * \brief A IEEE 802.15.4 like decoder
 *
 * The IEEE 802.15.4 uses the well known preamble + sync word synchronization
 * scheme. Many popular on Cubesats ICs like the Texas Instruments CC1xxx family
 * or the AXxxxx of On Semiconductors follow this scheme. This decoder
 * class provides a generic way to decode signals following this framing
 * scheme.
 *
 */
class SATNOGS_API ieee802_15_4_variant_decoder : public decoder {
public:

  /**
   *
   * @param preamble the preamble should be a repeated word. Note that due to AGC
   * settling, the receiver may not receive the whole preamble. If the preamble
   * is indeed a repeated pattern, a portion of it can be given as parameter.
   * The block should be able to deal with this. However, a quite small subset
   * may lead to a larger number of false alarms
   *
   * @param preamble_threshold the maximum number of bits that are
   * allowed to be wrong at the preamble
   *
   * @param sync the synchronization work following the preamble
   *
   * @param sync_threshold the maximum number of bits that are
   * allowed to be wrong at the synchronization word
   *
   * @param crc the CRC scheme to use
   *
   * @param descrambler if set, data will be first descrambled by this descrambling
   * method
   *
   * @param var_len if set to true, variable length decoding is used. Otherwise,
   * the \p max_len parameter indicates the fixed frame size
   *
   * @param max_len the maximum allowed decode-able frame length
   *
   * @param rs if set, the decoder will perform RS(255,223) decoding prior the descrambling and the CRC
   *
   * @return shared pointer of the decoder
   */
  static decoder::decoder_sptr
  make(const std::vector<uint8_t> &preamble,
       size_t preamble_threshold,
       const std::vector<uint8_t> &sync,
       size_t sync_threshold,
       crc::crc_t crc,
       whitening::whitening_sptr descrambler,
       bool var_len = true,
       size_t max_len = 1024,
       bool drop_invalid = true,
       bool rs = false);

  ieee802_15_4_variant_decoder(const std::vector<uint8_t> &preamble,
                               size_t preamble_threshold,
                               const std::vector<uint8_t> &sync,
                               size_t sync_threshold,
                               crc::crc_t crc,
                               whitening::whitening_sptr descrambler,
                               bool var_len = true,
                               size_t max_len = 1024,
                               bool drop_invalid = true,
                               bool rs = false);
  ~ieee802_15_4_variant_decoder();

  decoder_status_t
  decode(const void *in, int len);

  void
  reset();

  size_t
  input_multiple() const;

private:
  /**
   * Decoding FSM states
   */
  typedef enum {
    SEARCHING,                  //!< when searching for the start of the preamble
    SEARCHING_SYNC,             //!< We have preamble, search for sync
    DECODING_GENERIC_FRAME_LEN, //!< Decoding the frame length
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
  const bool                    d_var_len;
  const bool                    d_drop_invalid;
  const bool          d_rs;
  size_t                        d_len;
  size_t                        d_length_field_len;
  decoding_state_t              d_state;
  size_t                        d_cnt;
  uint64_t                      d_frame_start_idx;
  uint8_t                       *d_pdu;

  decoder_status_t
  decode_var_len(const void *in, int len);

  decoder_status_t
  decode_const_len(const void *in, int len);

  int
  search_preamble(const uint8_t *in, int len);

  int
  search_sync(const uint8_t *in, int len);

  int
  decode_frame_len(const uint8_t *in);

  void
  decode_payload(decoder_status_t &status, const uint8_t *in, int len);

  bool
  check_crc();

};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_IEEE802_15_4_VARIANT_DECODER_H */

