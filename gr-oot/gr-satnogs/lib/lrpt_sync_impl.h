/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2018, Libre Space Foundation <http://librespacefoundation.org/>
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

#ifndef INCLUDED_SATNOGS_LRPT_SYNC_IMPL_H
#define INCLUDED_SATNOGS_LRPT_SYNC_IMPL_H

#include <satnogs/lrpt_sync.h>
#include <satnogs/convolutional_deinterleaver.h>
#include <gnuradio/digital/constellation.h>

namespace gr {
namespace satnogs {

class lrpt_sync_impl : public lrpt_sync {
public:
  lrpt_sync_impl(size_t threshold);
  ~lrpt_sync_impl();

  int
  work(int noutput_items,
       gr_vector_const_void_star &input_items,
       gr_vector_void_star &output_items);

private:
  const size_t                          d_thresh;
  const uint64_t                        d_asm_coded;
  const size_t                          d_asm_coded_len;
  const uint64_t                        d_asm_coded_mask;
  const int                             d_window;
  const size_t                          d_coded_cadu_len;
  bool                                  d_frame_sync;
  size_t                                d_received;
  gr_complex                            d_rotate;
  digital::constellation_qpsk::sptr     d_qpsk;
  uint64_t                              d_shift_reg0;
  uint64_t                              d_shift_reg1;
  uint64_t                              d_shift_reg2;
  uint64_t                              d_shift_reg3;
  gr_complex                           *d_rotate_pi2;
  gr_complex                           *d_rotate_2pi2;
  gr_complex                           *d_rotate_3pi2;
  gr_complex                           *d_corrected;
  uint8_t                              *d_coded_cadu;

  int
  work_no_sync(const gr_complex *in, int noutput_items);

  int
  work_sync(const gr_complex *in, int noutput_items);

  bool
  found_sync(uint64_t reg);

};


} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_LRPT_SYNC_IMPL_H */

