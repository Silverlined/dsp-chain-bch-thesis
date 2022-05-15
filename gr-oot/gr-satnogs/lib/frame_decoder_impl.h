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

#ifndef INCLUDED_SATNOGS_FRAME_DECODER_IMPL_H
#define INCLUDED_SATNOGS_FRAME_DECODER_IMPL_H

#include <satnogs/frame_decoder.h>

namespace gr {
namespace satnogs {

class frame_decoder_impl : public frame_decoder {

public:
  frame_decoder_impl(decoder::decoder_sptr decoder_object, int input_size);
  ~frame_decoder_impl();

  // Where all the action really happens
  int
  work(int noutput_items, gr_vector_const_void_star &input_items,
       gr_vector_void_star &output_items);


private:
  decoder::decoder_sptr d_decoder;
  float *d_t_err;
  double d_t_err_acc;

  void
  reset(pmt::pmt_t m);
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_FRAME_DECODER_IMPL_H */

