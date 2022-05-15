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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <volk/volk.h>
#include "frame_decoder_impl.h"
#include <satnogs/metadata.h>
namespace gr {
namespace satnogs {

frame_decoder::sptr
frame_decoder::make(decoder::decoder_sptr decoder_object, int input_size)
{
  return gnuradio::get_initial_sptr(new frame_decoder_impl(decoder_object,
                                    input_size));
}

/*
 * The private constructor
 */
frame_decoder_impl::frame_decoder_impl(decoder::decoder_sptr decoder_object,
                                       int input_size) :
  gr::sync_block("frame_decoder",
                 gr::io_signature::make2(1, 2, input_size, sizeof(float)),
                 gr::io_signature::make(0, 0, 0)),
  d_decoder(decoder_object),
  d_t_err(nullptr),
  d_t_err_acc(0.0)
{
  if (input_size != decoder_object->sizeof_input_item()) {
    throw std::invalid_argument(
      "frame_decoder: Size mismatch between the block input and the decoder");
  }

  /*
   * Each decoder implementing the generic decoder API may have special
   * requirements on the number of input items. If not, by default this will
   * set a multiple of 1 item, which corresponds to an arbitrary number of
   * samples
   */
  set_output_multiple(d_decoder->input_multiple());

  message_port_register_in(pmt::mp("reset"));
  message_port_register_out(pmt::mp("out"));

  set_msg_handler(pmt::mp("reset"),
  [this](pmt::pmt_t msg) {
    this->reset(msg);
  });

  size_t alignment = volk_get_alignment();
  d_t_err = (float *)volk_malloc(sizeof(float), alignment);
  if (!d_t_err) {
    throw std::runtime_error(alias() + ": could not allocate aligned memory");
  }
}

/*
 * Our virtual destructor.
 */
frame_decoder_impl::~frame_decoder_impl()
{
  volk_free(d_t_err);
}

void
frame_decoder_impl::reset(pmt::pmt_t m)
{
  d_decoder->reset();
}

int
frame_decoder_impl::work(int noutput_items,
                         gr_vector_const_void_star &input_items,
                         gr_vector_void_star &output_items)
{
  const void *in = input_items[0];
  decoder_status_t status = d_decoder->decode(in, noutput_items);
  /*
   * If the timing error input port is available, accumulate the
   * timing correction applied so far by the Symbol Sync block
   */
  if (input_items.size() > 1) {
    *d_t_err = 0.0f;
    volk_32f_accumulator_s32f(d_t_err, (const float *)input_items[1],
                              noutput_items);
    d_t_err_acc += *d_t_err;
  }

  if (status.decode_success) {
    if (input_items.size() > 1) {
      metadata::add_symbol_timing_error(status.data,
                                        d_t_err_acc / (nitems_read(0) + noutput_items));
    }
    message_port_pub(pmt::mp("out"), status.data);
  }

  // Tell runtime system how many output items we produced.
  return status.consumed;
}

} /* namespace satnogs */
} /* namespace gr */

