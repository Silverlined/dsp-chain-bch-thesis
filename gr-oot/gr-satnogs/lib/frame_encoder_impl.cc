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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "frame_encoder_impl.h"

namespace gr {
namespace satnogs {

frame_encoder::sptr
frame_encoder::make(encoder::encoder_sptr enc)
{
  return gnuradio::get_initial_sptr
         (new frame_encoder_impl(enc));
}


/*
 * The private constructor
 */
frame_encoder_impl::frame_encoder_impl(encoder::encoder_sptr enc)
  : gr::block("frame_encoder",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
    d_encoder(enc)
{
  message_port_register_in(pmt::mp("pdu"));
  message_port_register_out(pmt::mp("pdu"));

  set_msg_handler(pmt::mp("pdu"),
  [this](pmt::pmt_t msg) {
    this->encode(msg);
  });
}

/*
 * Our virtual destructor.
 */
frame_encoder_impl::~frame_encoder_impl()
{
}

void
frame_encoder_impl::encode(pmt::pmt_t pdu)
{
  message_port_pub(pmt::mp("pdu"), d_encoder->encode(pdu));
}

} /* namespace satnogs */
} /* namespace gr */

