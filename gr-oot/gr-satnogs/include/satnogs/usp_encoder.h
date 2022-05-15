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
#ifndef INCLUDE_SATNOGS_USP_ENCODER_H
#define INCLUDE_SATNOGS_USP_ENCODER_H

#include <satnogs/encoder.h>
#include <satnogs/whitening.h>
#include <satnogs/conv_encoder.h>

namespace gr {
namespace satnogs {

/*!
 * \brief Unified SPUTNIX protocol (USP) frame encoder definition
 *
 */
class SATNOGS_API usp_encoder : public encoder {
public:
  static encoder::encoder_sptr
  make(const std::vector<uint8_t> &preamble,
       const std::vector<uint8_t> &sync,
       uint8_t pls_code,
       whitening::whitening_sptr scrambler);

  usp_encoder(const std::vector<uint8_t> &preamble,
              const std::vector<uint8_t> &sync,
              uint8_t pls_code,
              whitening::whitening_sptr scrambler);
  ~usp_encoder();

  pmt::pmt_t
  encode(pmt::pmt_t msg);

private:
  const size_t              d_max_frame_len;
  const size_t              d_payload_start;
  whitening::whitening_sptr d_scrambler;
  conv_encoder              d_conv;
  uint8_t                   *d_pdu;

  size_t
  final_pdu_length(size_t len) const;

};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDE_SATNOGS_USP_ENCODER_H */
