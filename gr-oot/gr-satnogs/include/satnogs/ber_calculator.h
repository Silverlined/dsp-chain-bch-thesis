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

#ifndef INCLUDE_SATNOGS_BER_CALCULATOR_H_
#define INCLUDE_SATNOGS_BER_CALCULATOR_H_

#include <satnogs/api.h>
#include <gnuradio/block.h>
#include <string>

namespace gr {
namespace satnogs {

/*!
 * \brief A block that calculates the BER of a packet passed system using
 * the FER (Frame Error Rate)
 * \ingroup satnogs
 *
 */
class SATNOGS_API ber_calculator : virtual public gr::block {
public:
  typedef boost::shared_ptr<ber_calculator> sptr;

  static sptr
  make(size_t frame_size = 64, size_t nframes = 0, size_t skip = 10);
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDE_SATNOGS_BER_CALCULATOR_H_ */
