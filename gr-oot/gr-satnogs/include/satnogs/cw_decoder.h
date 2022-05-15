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

#ifndef INCLUDED_SATNOGS_CW_DECODER_H
#define INCLUDED_SATNOGS_CW_DECODER_H

#include <satnogs/api.h>
#include <satnogs/decoder.h>
#include <satnogs/moving_sum.h>
#include <satnogs/cw_decoder_priv.h>
#include <gnuradio/fft/fft.h>
#include <cstdlib>
#include <vector>
#include <deque>



namespace gr {
namespace satnogs {

/*!
 * \brief CW decoder based on overlapping FFT
 *
 */
class SATNOGS_API cw_decoder : public decoder {
public:
  static decoder::decoder_sptr
  make(double samp_rate, int fft_len,
       int overlapping,
       float wpm = 22.0,
       float snr = 10.0,
       float confidence = 0.9,
       size_t channels = 16,
       size_t min_frame_size = 5,
       size_t max_frame_size = 256);

  cw_decoder(double samp_rate,
             int fft_len,
             int overlapping,
             float wpm,
             float snr,
             float confidence,
             size_t channels,
             size_t min_frame_size,
             size_t max_frame_size);
  ~cw_decoder();


  decoder_status_t
  decode(const void *in, int len);

  void
  reset();

  size_t
  input_multiple() const;

private:
  const double                  d_samp_rate;
  const int                     d_fft_len;
  const int                     d_overlapping;
  const float                   d_wpm;
  const float                   d_snr;
  const float                   d_confidence;
  const size_t                  d_min_frame_size;
  const int                     d_new_samples;
  const int                     d_shift_len;
  const float                   d_fft_duration;
  const size_t                  d_dot_duration;
  int                           d_channels_num;
  int                           d_channel_carriers;
  int                           d_nf_est_remaining;
  int                           d_dirty_samples;
  std::vector<cw_decoder_priv *>d_decoders;
  gr::fft::fft_complex          *d_fft;
  gr_complex                    *d_tmp_buf;
  float                         *d_nf_buf;
  float                         *d_psd;
  std::deque<decoder_status_t>  d_frames;

  void
  process_psd();

  void
  process_windows();

  void
  calc_nf();
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_CW_DECODER_H */

