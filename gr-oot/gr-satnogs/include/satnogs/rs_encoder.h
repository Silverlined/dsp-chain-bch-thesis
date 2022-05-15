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

#ifndef INCLUDE_SATNOGS_RS_ENCODER_H_
#define INCLUDE_SATNOGS_RS_ENCODER_H_

#include <satnogs/api.h>
#include <satnogs/encoder.h>

namespace gr {
namespace satnogs {

/*!
 * \brief A simple RS (255, 223) encoder with padding support
 */
class SATNOGS_API rs_encoder : public encoder {
public:
  static encoder::encoder_sptr
  make();

  rs_encoder();
  ~rs_encoder();
private:
  pmt::pmt_t
  encode(pmt::pmt_t msg);
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDE_SATNOGS_RS_ENCODER_H_ */
