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

#ifndef INCLUDED_SATNOGS_AX100_MODE5_ENCODER_H
#define INCLUDED_SATNOGS_AX100_MODE5_ENCODER_H

#include <satnogs/api.h>
#include <satnogs/encoder.h>
#include <satnogs/shift_reg.h>
#include <satnogs/crc.h>

#include <vector>

namespace gr {
namespace satnogs {

/*!
 * \brief This encoder implements the AX100 mode 5 framing and coding scheme
 *
 */
class SATNOGS_API ax100_mode5_encoder : public encoder {
public:
  static encoder::encoder_sptr
  make(const std::vector<uint8_t> &preamble,
       const std::vector<uint8_t> &sync,
       crc::crc_t crc,
       bool enable_rs);

  ax100_mode5_encoder(const std::vector<uint8_t> &preamble,
                      const std::vector<uint8_t> &sync,
                      crc::crc_t crc,
                      bool enable_rs);
  ~ax100_mode5_encoder();

  pmt::pmt_t
  encode(pmt::pmt_t msg);

private:
  crc::crc_t    d_crc;
  bool          d_rs;
  size_t        d_payload_start;
  uint8_t       *d_pdu;

  void
  write_16bits(uint16_t u16, size_t ind);

  void
  write_24bits(uint32_t u32, size_t ind);

  size_t
  add_crc(const uint8_t *pdu, size_t pdu_len, size_t ind);

};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_AX100_MODE5_ENCODER_H */
