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

#ifndef INCLUDED_SATNOGS_AX100_MODE6_ENCODER_H
#define INCLUDED_SATNOGS_AX100_MODE6_ENCODER_H

#include <satnogs/api.h>
#include <satnogs/encoder.h>
#include <satnogs/ax25_encoder.h>
#include <satnogs/crc.h>
#include <satnogs/whitening.h>

namespace gr {
namespace satnogs {

/*!
 * \brief This encoder implements the AX100 mode 6 framing and coding scheme
 *
 */
class SATNOGS_API ax100_mode6_encoder : public encoder {
public:
  static encoder::encoder_sptr
  make(size_t preamble_len, size_t postamble_len,
       whitening::whitening_sptr scrambler, bool scramble,
       bool nrzi, crc::crc_t crc);

  ax100_mode6_encoder(size_t preamble_len, size_t postamble_len,
                      whitening::whitening_sptr scrambler, bool scramble,
                      bool nrzi, crc::crc_t crc);
  ~ax100_mode6_encoder();

  pmt::pmt_t
  encode(pmt::pmt_t msg);

private:
  const size_t              d_max_frame_len;
  ax25_encoder              d_ax25;
  whitening::whitening_sptr d_scrambler;
  crc::crc_t                d_crc;
  uint8_t                   *d_pdu;

  size_t
  add_crc(size_t ind);

};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_AX100_MODE5_ENCODER_H */
