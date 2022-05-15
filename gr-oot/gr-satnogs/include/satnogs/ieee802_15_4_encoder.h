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

#ifndef INCLUDED_SATNOGS_IEEE802_15_4_ENCODER_H
#define INCLUDED_SATNOGS_IEEE802_15_4_ENCODER_H

#include <satnogs/api.h>
#include <satnogs/crc.h>
#include <satnogs/whitening.h>
#include <satnogs/encoder.h>

namespace gr {
namespace satnogs {

/*!
 * \brief An IEEE802.15.4 frame encoder with some extended parameterization
 */
class SATNOGS_API ieee802_15_4_encoder : public encoder {
public:
  static encoder::encoder_sptr
  make(uint8_t preamble, size_t preamble_len,
       const std::vector<uint8_t> &sync_word, crc::crc_t crc,
       whitening::whitening_sptr scrambler,
       bool var_len = true);

  ieee802_15_4_encoder(uint8_t preamble, size_t preamble_len,
                       const std::vector<uint8_t> &sync_word, crc::crc_t crc,
                       whitening::whitening_sptr scrambler,
                       bool var_len);
  ~ieee802_15_4_encoder();

  pmt::pmt_t
  encode(pmt::pmt_t msg);
private:
  const size_t                  d_max_frame_len;
  const size_t                  d_preamle_len;
  const crc::crc_t              d_crc;
  whitening::whitening_sptr     d_scrambler;
  const bool                    d_var_len;
  uint8_t                       *d_buffer;
  uint8_t                       *d_payload;
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_IEEE802_15_4_ENCODER_H */

