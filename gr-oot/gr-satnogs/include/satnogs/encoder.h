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

#ifndef INCLUDED_SATNOGS_ENCODER_H
#define INCLUDED_SATNOGS_ENCODER_H

#include <satnogs/api.h>
#include <pmt/pmt.h>
#include <deque>

namespace gr {
namespace satnogs {

/*!
 * \brief Abstract class defining the API of the SatNOGS Encoders
 *
 * Abstract class defining the API of the SatNOGS Encoders
 *
 * The gr-satnogs module tries to provide a unified encoding framework,
 * for various satellites and framing schemes.
 *
 * Specialization is performed by passing to the generic encoding block
 * (\ref frame_encoder() ) the appropriate encoder class that implements
 * this abstract class API.
 */
class SATNOGS_API encoder {
public:
  typedef boost::shared_ptr<encoder> encoder_sptr;

  static int base_unique_id;

  int
  unique_id() const;


  encoder();

  virtual ~encoder();

  virtual pmt::pmt_t
  encode(pmt::pmt_t pdu) = 0;

private:
  const int     d_id;

};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_ENCODER_H */

