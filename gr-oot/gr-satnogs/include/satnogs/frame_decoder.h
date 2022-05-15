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

#ifndef INCLUDED_SATNOGS_FRAME_DECODER_H
#define INCLUDED_SATNOGS_FRAME_DECODER_H

#include <satnogs/api.h>
#include <gnuradio/sync_block.h>
#include <satnogs/decoder.h>

namespace gr {
namespace satnogs {

/*!
 * \brief This is a generic frame decoder block. It takes as input a
 * bit stream and produces decoded frames and their metadata.
 *
 * The decoding is performed by using a proper decoder object.
 * Each decoder implements the virtual class ::decoder()
 *
 * The frame and metadata are produced in a pmt dictionary, with the
 * keys "pdu" and "metadata".
 *
 * \ingroup satnogs
 *
 */
class SATNOGS_API frame_decoder : virtual public gr::sync_block {
public:
  typedef boost::shared_ptr<frame_decoder> sptr;

  /*!
   * \brief Return a shared_ptr to a new instance of satnogs::frame_decoder.
   * @param decoder_object the decoder object to use
   */
  static sptr make(decoder::decoder_sptr decoder_object, int input_size);
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_FRAME_DECODER_H */

