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

#ifndef INCLUDE_SATNOGS_AX25_ENCODER_H_
#define INCLUDE_SATNOGS_AX25_ENCODER_H_


#include <satnogs/encoder.h>


namespace gr {
namespace satnogs {

/*!
 * \brief AX.25 frame encoder definition
 */
class SATNOGS_API ax25_encoder : public encoder {
public:

  static encoder::encoder_sptr
  make(const std::string &dest_addr, uint8_t dest_ssid,
       const std::string &src_addr, uint8_t src_ssid, size_t preamble_len = 16,
       size_t postamble_len = 16, bool scramble = true,
       bool nrzi = true);

  ax25_encoder(const std::string &dest_addr, uint8_t dest_ssid,
               const std::string &src_addr, uint8_t src_ssid,
               size_t preamble_len, size_t postamble_len,
               bool scramble = true, bool nrzi = true);
  ~ax25_encoder();

  pmt::pmt_t
  encode(pmt::pmt_t msg);

private:
  const std::string d_dest_addr;
  const uint8_t     d_dest_ssid;
  const std::string d_src_addr;
  const uint8_t d_src_ssid;
  const size_t d_preamble_len = 16;
  const size_t d_postamble_len = 16;
  const bool d_scramble;
  const bool d_nrzi;

  size_t
  insert_address(uint8_t *out);

  size_t
  insert_ctrl(uint8_t *out);

  size_t
  insert_pid(uint8_t *out);
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDE_SATNOGS_AX25_ENCODER_H_ */
