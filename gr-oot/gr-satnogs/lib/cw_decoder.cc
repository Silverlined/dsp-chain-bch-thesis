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
#include <satnogs/cw_decoder.h>
#include <satnogs/metadata.h>
#include <volk/volk.h>

namespace gr {
namespace satnogs {

decoder::decoder_sptr
cw_decoder::make(double samp_rate, int fft_len, int overlapping,
                 float wpm, float snr, float confidence,
                 size_t channels,
                 size_t min_frame_size,
                 size_t max_frame_size)
{
  return decoder::decoder_sptr(
           new cw_decoder(samp_rate, fft_len, overlapping, wpm, snr, confidence,
                          channels, min_frame_size, max_frame_size));
}

cw_decoder::cw_decoder(double samp_rate, int fft_len, int overlapping,
                       float wpm, float snr,
                       float confidence,
                       size_t channels,
                       size_t min_frame_size,
                       size_t max_frame_size) :
  decoder("cw", "1.0", sizeof(gr_complex), max_frame_size),
  d_samp_rate(samp_rate),
  d_fft_len(fft_len),
  d_overlapping(overlapping),
  d_wpm(wpm),
  d_snr(snr),
  d_confidence(confidence),
  d_min_frame_size(min_frame_size),
  d_new_samples(fft_len - overlapping),
  d_shift_len(std::floor(fft_len / 2.0)),
  d_fft_duration(d_new_samples / samp_rate),
  d_dot_duration((1.2 / wpm) / d_fft_duration),
  d_nf_est_remaining(100 * d_fft_len),
  d_dirty_samples(10 * fft_len)
{
  if (min_frame_size >= max_frame_size) {
    throw std::invalid_argument("cw_decoder: Wrong minimum or maximum frame length");
  }

  if (min_frame_size > 512) {
    throw std::invalid_argument("cw_decoder: Invalid minimum frame length");
  }

  if (wpm < 1.0 || wpm > 100.0) {
    throw std::invalid_argument("cw_decoder: Invalid WPM");
  }

  if (fft_len <= overlapping) {
    throw std::invalid_argument("cw_decoder: Invalid overlapping value");
  }

  if (confidence < 0.0 || confidence > 1.0) {
    throw std::invalid_argument("cw_decoder: Invalid confidence level");
  }

  d_fft = new gr::fft::fft_complex(fft_len, true, 1);
  d_tmp_buf = (gr_complex *)volk_malloc(fft_len * sizeof(gr_complex),
                                        volk_get_alignment());
  if (!d_tmp_buf) {
    throw std::runtime_error("cw_decoder: could not allocate aligned memory");
  }
  d_psd = (float *)volk_malloc(fft_len * sizeof(float),
                               volk_get_alignment());
  if (!d_psd) {
    throw std::runtime_error("cw_decoder: could not allocate aligned memory");
  }

  d_nf_buf = (float *)volk_malloc(fft_len * sizeof(float),
                                  volk_get_alignment());
  if (!d_nf_buf) {
    throw std::runtime_error("cw_decoder: could not allocate aligned memory");
  }
  std::fill(d_nf_buf, d_nf_buf + d_fft_len, -170.0f);

  d_channels_num = channels;
  while (fft_len % (fft_len / d_channels_num)) {
    d_channels_num++;
  }
  d_channel_carriers = fft_len / d_channels_num;

  if (d_channel_carriers * (samp_rate / fft_len) < 200) {
    throw std::invalid_argument("cw_decoder: Too many sub-channels");
  }

  for (int i = 0 ; i < d_channels_num; i++) {
    double freq = -samp_rate / 2 + (i * d_channel_carriers
                                    + d_channel_carriers / 2) * samp_rate / fft_len;
    d_decoders.push_back(new cw_decoder_priv(freq, confidence,
                         d_dot_duration,
                         min_frame_size, max_frame_size));
  }
}

cw_decoder::~cw_decoder()
{
  volk_free(d_tmp_buf);
  volk_free(d_psd);
  volk_free(d_nf_buf);
  delete d_fft;
  for (cw_decoder_priv *i : d_decoders) {
    delete i;
  }
}

decoder_status_t
cw_decoder::decode(const void *in, int len)
{
  decoder_status_t status;
  const gr_complex *input = (const gr_complex *)in;

  const int fft_frames = len / d_new_samples;
  for (int i = 0; i < fft_frames; i++) {
    memcpy(d_tmp_buf, d_fft->get_inbuf() + d_new_samples,
           d_overlapping * sizeof(gr_complex));
    memcpy(d_tmp_buf + d_overlapping,
           input + i * d_new_samples, d_new_samples * sizeof(gr_complex));
    memcpy(d_fft->get_inbuf(), d_tmp_buf, d_fft_len * sizeof(gr_complex));
    d_fft->execute();
    /* Log scale is not a requirement, but let's start with something familiar */
    volk_32fc_s32f_x2_power_spectral_density_32f(d_psd, d_fft->get_outbuf(),
        d_fft_len, 1.0, d_fft_len);
    /*FFT shift */
    std::rotate(d_psd, d_psd + d_shift_len, d_psd + d_fft_len);
    /* Drop the first samples that will probably pollute our NF estimation */
    if (d_dirty_samples > 0) {
      d_dirty_samples -= d_new_samples;
    }
    else if (d_nf_est_remaining > 0) {
      calc_nf();
      d_nf_est_remaining -= d_new_samples;
    }
    else {
      process_psd();
    }
  }
  /*
   * Check if any decoded frames exist. As multiple windows may produce
   * frames at the same time, propagate them in a serial way,
   * just to keep compatibility with the decoder API
   */
  if (d_frames.size() > 0) {
    status = d_frames.front();
    d_frames.pop_front();
  }
  status.consumed = fft_frames * d_new_samples;
  return status;
}

void
cw_decoder::process_psd()
{
  bool trigger = false;
  for (int win_i = 0; win_i < d_channels_num; win_i++) {
    trigger = false;
    for (int i = 0; i < d_channel_carriers; i++) {
      if (d_psd[win_i * d_channel_carriers + i] > d_nf_buf[win_i * d_channel_carriers
          + i]  + d_snr) {
        decoder_status_t d = d_decoders[win_i]->decode(1.0f,
                             d_psd[win_i * d_channel_carriers + i] - d_nf_buf[win_i * d_channel_carriers +
                                 i]);
        if (d.decode_success) {
          metadata::add_decoder(d.data, this);
          d_frames.push_back(d);
        }
        trigger = true;
        break;
      }
    }
    if (!trigger) {
      decoder_status_t d = d_decoders[win_i]->decode(0.0f, 0.0f);
      if (d.decode_success) {
        metadata::add_decoder(d.data, this);
        d_frames.push_back(d);
      }
    }
  }
}

void
cw_decoder::calc_nf()
{
  volk_32f_x2_max_32f(d_nf_buf, d_psd, d_nf_buf, d_fft_len);
}

void
cw_decoder::process_windows()
{
  for (int i = 0; i < d_channels_num; i++) {
    d_decoders[i]->reset();
  }
}


void
cw_decoder::reset()
{
  for (int i = 0; i < d_channels_num; i++) {
    d_decoders[i]->reset();
  }
}

size_t
cw_decoder::input_multiple() const
{
  return d_new_samples;
}

} /* namespace satnogs */
} /* namespace gr */

