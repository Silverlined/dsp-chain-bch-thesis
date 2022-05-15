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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "lrpt_sync_impl.h"
#include <satnogs/log.h>
#include <satnogs/utils.h>

#include <volk/volk.h>
#include <gnuradio/blocks/count_bits.h>

namespace gr {
namespace satnogs {

lrpt_sync::sptr
lrpt_sync::make(size_t threshold)
{
  return gnuradio::get_initial_sptr(new lrpt_sync_impl(threshold));
}

/*
 * The private constructor
 */
lrpt_sync_impl::lrpt_sync_impl(size_t threshold) :
  gr::sync_block("lrpt_sync",
                 gr::io_signature::make(1, 1, sizeof(gr_complex)),
                 gr::io_signature::make(0, 0, 0)),
  d_thresh(threshold),
  d_asm_coded(0xd49c24ff2686b),
  d_asm_coded_len(52),
  d_asm_coded_mask((1ULL << d_asm_coded_len) - 1),
  /*
   * We process the data in a multiple of 2 frames and a UW
   * sync word
   */
  d_window((72 + 8) / 2),
  /* Each CADU has the 4 byte ASM and a VCDU of 1020 bytes*/
  /*
   * NOTE:
   * Metop violates the standard as many times as possible...
   * The frame should contain 128 RS check symbols at the end.
   * For some unknown reasons, it seems that the RS encoding is not performed.
   * Thus, they dropped the check symbols at the end of the frame.
   */
  d_coded_cadu_len(1020 * 2 + 4 * 2 - 128 * 2),
  d_frame_sync(false),
  d_received(0),
  d_rotate(1.0, 0.0),
  d_qpsk(digital::constellation_qpsk::make()),
  d_shift_reg0(0x0),
  d_shift_reg1(0x0),
  d_shift_reg2(0x0),
  d_shift_reg3(0x0)
{
  set_output_multiple(d_window);
  const int alignment_multiple = volk_get_alignment() / sizeof(gr_complex);
  set_alignment(std::max(1, alignment_multiple));
  d_rotate_pi2 = (gr_complex *) volk_malloc(d_window * sizeof(gr_complex),
                 volk_get_alignment());
  if (!d_rotate_pi2) {
    throw std::runtime_error("lrpt_sync: Could not allocate memory");
  }

  d_rotate_2pi2 = (gr_complex *) volk_malloc(d_window * sizeof(gr_complex),
                  volk_get_alignment());
  if (!d_rotate_2pi2) {
    volk_free(d_rotate_pi2);
    throw std::runtime_error("lrpt_sync: Could not allocate memory");
  }

  d_rotate_3pi2 = (gr_complex *) volk_malloc(d_window * sizeof(gr_complex),
                  volk_get_alignment());
  if (!d_rotate_3pi2) {
    volk_free(d_rotate_pi2);
    volk_free(d_rotate_2pi2);
    throw std::runtime_error("lrpt_sync: Could not allocate memory");
  }

  d_corrected = (gr_complex *)volk_malloc(d_window * sizeof(gr_complex),
                                          volk_get_alignment());
  if (!d_corrected) {
    volk_free(d_rotate_pi2);
    volk_free(d_rotate_2pi2);
    volk_free(d_rotate_3pi2);
    throw std::runtime_error("lrpt_sync: Could not allocate memory");
  }

  uint64_t asm_coded = utils::htonll(d_asm_coded);
  d_coded_cadu = new uint8_t[d_coded_cadu_len];
  memcpy(d_coded_cadu, &asm_coded, sizeof(uint64_t));
  d_received =  sizeof(uint64_t);

  message_port_register_out(pmt::mp("cadu"));
}

/*
 * Our virtual destructor.
 */
lrpt_sync_impl::~lrpt_sync_impl()
{
  volk_free(d_rotate_pi2);
  volk_free(d_rotate_2pi2);
  volk_free(d_rotate_3pi2);
  volk_free(d_corrected);
  delete [] d_coded_cadu;
}

bool
lrpt_sync_impl::found_sync(uint64_t reg)
{
  return blocks::count_bits64((reg ^ d_asm_coded) & d_asm_coded_mask)
         <= d_thresh;
}


int
lrpt_sync_impl::work_no_sync(const gr_complex *in, int noutput_items)
{
  uint32_t bits;
  int multiple = noutput_items / d_window;
  for (int i = 0; i < multiple; i++) {
    volk_32fc_s32fc_multiply_32fc(d_rotate_pi2, in + i * d_window,
                                  gr_complex(0.0, 1.0), d_window);
    volk_32fc_s32fc_multiply_32fc(d_rotate_2pi2, in + i * d_window,
                                  gr_complex(-1.0, 0.0), d_window);
    volk_32fc_s32fc_multiply_32fc(d_rotate_3pi2, in + i * d_window,
                                  gr_complex(0.0, -1.0), d_window);
    /*
     * Search for the sync pattern, rotating the QPSK constellation on
     * all possible positions
     */
    for (int j = 0; j < d_window; j++) {
      bits = d_qpsk->decision_maker(in + i * d_window + j);
      //bits = (d_conv_deinter.decode_bit(bits >> 1) << 1) | d_conv_deinter.decode_bit(bits & 0x1);
      d_shift_reg0 = (d_shift_reg0 << 2) | bits;
      if (found_sync(d_shift_reg0)) {
        d_rotate = gr_complex(1.0, 0.0);
        d_frame_sync = true;
        uint64_t asm_coded = utils::htonll(d_shift_reg0);
        memcpy(d_coded_cadu, &asm_coded, sizeof(uint64_t));
        return i * d_window + j + 1;
      }

      bits = d_qpsk->decision_maker(d_rotate_pi2 + j);
      //bits = (d_conv_deinter.decode_bit(bits >> 1) << 1) | d_conv_deinter.decode_bit(bits & 0x1);
      d_shift_reg1 = (d_shift_reg1 << 2) | bits;
      if (found_sync(d_shift_reg1)) {
        d_rotate = gr_complex(0.0, 1.0);
        d_frame_sync = true;
        uint64_t asm_coded = utils::htonll(d_shift_reg1);
        memcpy(d_coded_cadu, &asm_coded, sizeof(uint64_t));
        return i * d_window + j + 1;
      }

      bits = d_qpsk->decision_maker(d_rotate_2pi2 + j);
      //bits = (d_conv_deinter.decode_bit(bits >> 1) << 1) | d_conv_deinter.decode_bit(bits & 0x1);
      d_shift_reg2 = (d_shift_reg2 << 2) | bits;
      if (found_sync(d_shift_reg2)) {
        d_rotate = gr_complex(-1.0, 0.0);
        d_frame_sync = true;
        uint64_t asm_coded = utils::htonll(d_shift_reg2);
        memcpy(d_coded_cadu, &asm_coded, sizeof(uint64_t));
        return i * d_window + j + 1;
      }

      bits = d_qpsk->decision_maker(d_rotate_3pi2 + j);
      //bits = (d_conv_deinter.decode_bit(bits >> 1) << 1) | d_conv_deinter.decode_bit(bits & 0x1);
      d_shift_reg3 = (d_shift_reg3 << 2) | bits;
      if (found_sync(d_shift_reg3)) {
        d_rotate = gr_complex(0.0, -1.0);
        d_frame_sync = true;
        uint64_t asm_coded = utils::htonll(d_shift_reg3);
        memcpy(d_coded_cadu, &asm_coded, sizeof(uint64_t));
        return i * d_window + j + 1;
      }
    }
  }
  return noutput_items;
}

int
lrpt_sync_impl::work_sync(const gr_complex *in, int noutput_items)
{
  uint8_t b;
  int multiple = noutput_items / d_window;
  for (int i = 0; i < multiple; i++) {
    volk_32fc_s32fc_multiply_32fc(d_corrected, in + i * d_window,
                                  d_rotate, d_window);
    for (int j = 0; j < d_window; j += 4) {
      b = 0;
      b = d_qpsk->decision_maker(d_corrected + j) << 6;
      b |= d_qpsk->decision_maker(d_corrected + j + 1) << 4;
      b |= d_qpsk->decision_maker(d_corrected + j + 2) << 2;
      b |= d_qpsk->decision_maker(d_corrected + j + 3);

      d_coded_cadu[d_received++] = b;
      if (d_received == d_coded_cadu_len) {
        d_received = sizeof(uint64_t);
        d_frame_sync = false;
        message_port_pub(pmt::mp("cadu"),
                         pmt::make_blob(d_coded_cadu, d_coded_cadu_len));
        return i * d_window + j + 4;
      }
    }
  }
  return noutput_items;
}


int
lrpt_sync_impl::work(int noutput_items,
                     gr_vector_const_void_star &input_items,
                     gr_vector_void_star &output_items)
{
  const gr_complex *in = (const gr_complex *) input_items[0];
  uint32_t bits;
  if (!d_frame_sync) {
    return work_no_sync(in, noutput_items);
  }
  return work_sync(in, noutput_items);
}

} /* namespace satnogs */
} /* namespace gr */

