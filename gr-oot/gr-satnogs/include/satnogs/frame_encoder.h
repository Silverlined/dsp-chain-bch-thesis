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

#ifndef INCLUDED_SATNOGS_FRAME_ENCODER_H
#define INCLUDED_SATNOGS_FRAME_ENCODER_H

#include <satnogs/api.h>
#include <satnogs/encoder.h>
#include <gnuradio/block.h>

namespace gr {
namespace satnogs {

/*!
 * \brief <+description of block+>
 * \ingroup satnogs
 *
 */
class SATNOGS_API frame_encoder : virtual public gr::block {
public:
  typedef boost::shared_ptr<frame_encoder> sptr;

  /*!
   * \brief Return a shared_ptr to a new instance of satnogs::frame_encoder.
   *
   * To avoid accidental use of raw pointers, satnogs::frame_encoder's
   * constructor is in a private implementation
   * class. satnogs::frame_encoder::make is the public interface for
   * creating new instances.
   */
  static sptr make(encoder::encoder_sptr enc);
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_FRAME_ENCODER_H */

