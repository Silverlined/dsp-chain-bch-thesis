/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2017,2019 Libre Space Foundation <http://libre.space>
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

#ifndef INCLUDED_SATNOGS_WATERFALL_SINK_IMPL_H
#define INCLUDED_SATNOGS_WATERFALL_SINK_IMPL_H

#include <satnogs/waterfall_sink.h>
#include <volk/volk.h>
#include <gnuradio/fft/fft.h>
#include <iostream>
#include <fstream>
#include <chrono>

namespace gr {
namespace satnogs {

class waterfall_sink_impl : public waterfall_sink {
private:

  /**
   * Waterfall header data.
   * This structure is only for readability purposes and make more clear to
   * possible users the structure of the header.
   */
  typedef struct {
    char start_time[32];        /**< String with the start of the waterfall in ISO-8601 format */
    uint32_t samp_rate;         /**< The sampling rate of the waterfall */
    uint32_t fft_size;          /**< The FFT size of the flowgraph */
    uint32_t nfft_per_row;      /**< The number of FFTs performed to plot one row */
    float center_freq;          /**< The center frequency of the observation. Just for viasualization purposes */
    uint32_t endianness;        /**< The endianness of the rest of the file. Should be 0 for big endian */
  } header_t;

  /**
   * The different types of operation of the waterfall
   */
  typedef enum {
    WATERFALL_MODE_DECIMATION = 0,//!< WATERFALL_MODE_DECIMATION Performs just a decimation and computes the energy only
    WATERFALL_MODE_MAX_HOLD = 1,  //!< WATERFALL_MODE_MAX_HOLD compute the max hold energy of all the FFT snapshots between two consecutive pixel rows
    WATERFALL_MODE_MEAN = 2       //!< WATERFALL_MODE_MEAN compute the mean energy of all the FFT snapshots between two consecutive pixel rows
  } wf_mode_t;

  const float d_samp_rate;
  const float d_center_freq;
  const size_t d_fft_size;
  wf_mode_t d_mode;
  size_t d_refresh;
  size_t d_fft_cnt;
  size_t d_fft_shift;
  size_t d_samples_cnt;
  fft::fft_complex d_fft;
  gr_complex *d_shift_buffer;
  float *d_hold_buffer;
  float *d_tmp_buffer;
  std::ofstream d_fos;
  std::chrono::system_clock::time_point d_start;

  void
  apply_header();

  void
  write_timestamp();

public:
  waterfall_sink_impl(float samp_rate, float center_freq, float rps,
                      size_t fft_size, const std::string &filename, int mode);
  ~waterfall_sink_impl();

  bool
  start();

  int
  work(int noutput_items, gr_vector_const_void_star &input_items,
       gr_vector_void_star &output_items);

  void
  compute_decimation(const gr_complex *in, size_t n_fft);

  void
  compute_max_hold(const gr_complex *in, size_t n_fft);

  void
  compute_mean(const gr_complex *in, size_t n_fft);
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_WATERFALL_SINK_IMPL_H */

