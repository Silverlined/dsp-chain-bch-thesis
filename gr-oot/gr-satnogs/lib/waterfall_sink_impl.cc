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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "waterfall_sink_impl.h"
#include <satnogs/log.h>
#include <satnogs/utils.h>
#include <satnogs/date.h>

namespace gr {
namespace satnogs {

waterfall_sink::sptr
waterfall_sink::make(float samp_rate, float center_freq, float rps,
                     size_t fft_size, const std::string &filename, int mode)
{
  return gnuradio::get_initial_sptr(
           new waterfall_sink_impl(samp_rate, center_freq, rps, fft_size, filename,
                                   mode));
}

/*
 * The private constructor
 */
waterfall_sink_impl::waterfall_sink_impl(float samp_rate, float center_freq,
    float rps, size_t fft_size,
    const std::string &filename,
    int mode) :
  gr::sync_block("waterfall_sink",
                 gr::io_signature::make(1, 1, sizeof(gr_complex)),
                 gr::io_signature::make(0, 0, 0)),
  d_samp_rate(samp_rate),
  d_center_freq(center_freq),
  d_fft_size(fft_size),
  d_mode((wf_mode_t) mode),
  d_refresh((d_samp_rate / fft_size) / rps),
  d_fft_cnt(0),
  d_fft_shift((size_t)(ceil(fft_size / 2.0))),
  d_samples_cnt(0),
  d_fft(fft_size)
{
  const int alignment_multiple = volk_get_alignment()
                                 / (fft_size * sizeof(gr_complex));
  set_alignment(std::max(1, alignment_multiple));
  set_output_multiple(fft_size);

  d_shift_buffer = (gr_complex *) volk_malloc(fft_size * sizeof(gr_complex),
                   volk_get_alignment());
  if (!d_shift_buffer) {
    LOG_ERROR("Could not allocate aligned memory");
    throw std::runtime_error("Could not allocate aligned memory");
  }

  d_hold_buffer = (float *) volk_malloc(fft_size * sizeof(gr_complex),
                                        volk_get_alignment());
  if (!d_hold_buffer) {
    LOG_ERROR("Could not allocate aligned memory");
    throw std::runtime_error("Could not allocate aligned memory");
  }
  memset(d_hold_buffer, 0, fft_size * sizeof(gr_complex));

  d_tmp_buffer = (float *) volk_malloc(fft_size * sizeof(float),
                                       volk_get_alignment());
  if (!d_tmp_buffer) {
    LOG_ERROR("Could not allocate aligned memory");
    throw std::runtime_error("Could not allocate aligned memory");
  }

  d_fos.open(filename, std::ios::binary | std::ios::trunc);
  if (d_fos.fail()) {
    throw std::runtime_error("Could not create file for writing");
  }
}

bool
waterfall_sink_impl::start()
{
  /*
   * Append header for proper plotting. We do it on the start() to reduce
   * as much as possible the delay between the start of the observation tagging
   * and the fist invocation of the work() method.
   */
  apply_header();
  return true;
}

/*
 * Our virtual destructor.
 */
waterfall_sink_impl::~waterfall_sink_impl()
{
  d_fos.close();
  volk_free(d_shift_buffer);
  volk_free(d_hold_buffer);
  volk_free(d_tmp_buffer);
}

int
waterfall_sink_impl::work(int noutput_items,
                          gr_vector_const_void_star &input_items,
                          gr_vector_void_star &output_items)
{
  const gr_complex *in = (const gr_complex *) input_items[0];
  size_t n_fft = ((size_t) noutput_items / d_fft_size);

  switch (d_mode) {
  case WATERFALL_MODE_DECIMATION:
    compute_decimation(in, n_fft);
    break;
  case WATERFALL_MODE_MAX_HOLD:
    compute_max_hold(in, n_fft);
    break;
  case WATERFALL_MODE_MEAN:
    compute_mean(in, n_fft);
    break;
  default:
    LOG_ERROR("Wrong waterfall mode");
    throw std::runtime_error("Wrong waterfall mode");
    return -1;
  }
  return n_fft * d_fft_size;
}

void
waterfall_sink_impl::compute_decimation(const gr_complex *in, size_t n_fft)
{
  size_t i;
  gr_complex *fft_in;
  for (i = 0; i < n_fft; i++) {
    d_fft_cnt++;
    if (d_fft_cnt == d_refresh) {
      fft_in = d_fft.get_inbuf();
      memcpy(fft_in, in + i * d_fft_size, d_fft_size * sizeof(gr_complex));
      d_fft.execute();
      /* Perform FFT shift */
      memcpy(d_shift_buffer, &d_fft.get_outbuf()[d_fft_shift],
             sizeof(gr_complex) * (d_fft_size - d_fft_shift));
      memcpy(&d_shift_buffer[d_fft_size - d_fft_shift],
             &d_fft.get_outbuf()[0], sizeof(gr_complex) * d_fft_shift);

      /* Compute the energy in dB */
      volk_32fc_s32f_x2_power_spectral_density_32f(d_hold_buffer,
          d_shift_buffer,
          (float) d_fft_size, 1.0,
          d_fft_size);
      /* Write the result to the file */
      write_timestamp();
      d_fos.write((char *) d_hold_buffer, d_fft_size * sizeof(float));
      d_fft_cnt = 0;
    }
    d_samples_cnt += d_fft_size;
  }
}

void
waterfall_sink_impl::compute_max_hold(const gr_complex *in, size_t n_fft)
{
  size_t i;
  size_t j;
  gr_complex *fft_in;
  for (i = 0; i < n_fft; i++) {
    fft_in = d_fft.get_inbuf();
    memcpy(fft_in, in + i * d_fft_size, d_fft_size * sizeof(gr_complex));
    d_fft.execute();
    /* Perform FFT shift */
    memcpy(d_shift_buffer, &d_fft.get_outbuf()[d_fft_shift],
           sizeof(gr_complex) * (d_fft_size - d_fft_shift));
    memcpy(&d_shift_buffer[d_fft_size - d_fft_shift], &d_fft.get_outbuf()[0],
           sizeof(gr_complex) * d_fft_shift);

    /* Normalization factor */
    volk_32fc_s32fc_multiply_32fc(d_shift_buffer, d_shift_buffer,
                                  1.0 / d_fft_size, d_fft_size);

    /* Compute the mag^2 */
    volk_32fc_magnitude_squared_32f(d_tmp_buffer, d_shift_buffer, d_fft_size);
    /* Max hold */
    volk_32f_x2_max_32f(d_hold_buffer, d_hold_buffer, d_tmp_buffer,
                        d_fft_size);
    d_fft_cnt++;
    if (d_fft_cnt == d_refresh) {
      /* Compute the energy in dB */
      for (j = 0; j < d_fft_size; j++) {
        d_hold_buffer[j] = 10.0 * log10f(d_hold_buffer[j] + 1.0e-20);
      }

      /* Write the result to the file */
      write_timestamp();
      d_fos.write((char *) d_hold_buffer, d_fft_size * sizeof(float));

      /* Reset */
      d_fft_cnt = 0;
      memset(d_hold_buffer, 0, d_fft_size * sizeof(float));
    }
    d_samples_cnt += d_fft_size;
  }
}

void
waterfall_sink_impl::apply_header()
{
  header_t h;
  memset(h.start_time, 0, 32);
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
  d_start = tp;
  std::string s = date::format("%FT%TZ",
                               date::floor<std::chrono::microseconds> (tp));
  std::strncpy(h.start_time, s.c_str(), 32);

  /* Before writing to the file convert all values to Network Byte Order */
  h.fft_size = htonl(d_fft_size);
  h.samp_rate = htonl(d_samp_rate);
  h.nfft_per_row = htonl(d_refresh);
  uint32_t tmp = htonl(*((uint32_t *) &d_center_freq));
  memcpy(&h.center_freq, &tmp, sizeof(uint32_t));
  h.center_freq = *((float *) &tmp);
  h.endianness = !(1 == htonl(1));

  /*
   * Write the header. Make a dummy serialization to avoid padding and
   * alignment issues
   */
  d_fos.write(h.start_time, 32);
  d_fos.write((char *)&h.fft_size, sizeof(uint32_t));
  d_fos.write((char *)&h.samp_rate, sizeof(uint32_t));
  d_fos.write((char *)&h.nfft_per_row, sizeof(uint32_t));
  d_fos.write((char *)&h.center_freq, sizeof(float));
  d_fos.write((char *)&h.endianness, sizeof(uint32_t));
}

void
waterfall_sink_impl::write_timestamp()
{
  std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
  int64_t x = std::chrono::duration_cast<std::chrono::microseconds> (
                tp - d_start).count();
  d_fos.write((char *)&x, sizeof(int64_t));
}

void
waterfall_sink_impl::compute_mean(const gr_complex *in, size_t n_fft)
{
  size_t i;
  gr_complex *fft_in;
  for (i = 0; i < n_fft; i++) {
    fft_in = d_fft.get_inbuf();
    memcpy(fft_in, in + i * d_fft_size, d_fft_size * sizeof(gr_complex));
    d_fft.execute();
    /* Perform FFT shift */
    memcpy(d_shift_buffer, &d_fft.get_outbuf()[d_fft_shift],
           sizeof(gr_complex) * (d_fft_size - d_fft_shift));
    memcpy(&d_shift_buffer[d_fft_size - d_fft_shift], &d_fft.get_outbuf()[0],
           sizeof(gr_complex) * d_fft_shift);

    /* Accumulate the complex numbers  */
    volk_32f_x2_add_32f(d_hold_buffer, d_hold_buffer, (float *) d_shift_buffer,
                        2 * d_fft_size);
    d_fft_cnt++;
    if (d_fft_cnt == d_refresh) {
      /*
       * Compute the energy in dB performing the proper normalization
       * before any dB calculation, emulating the mean
       */
      volk_32fc_s32f_x2_power_spectral_density_32f(
        d_hold_buffer, (gr_complex *) d_hold_buffer,
        (float) d_fft_cnt * d_fft_size, 1.0, d_fft_size);

      /* Write the result to the file */
      write_timestamp();
      d_fos.write((char *) d_hold_buffer, d_fft_size * sizeof(float));

      /* Reset */
      d_fft_cnt = 0;
      memset(d_hold_buffer, 0, 2 * d_fft_size * sizeof(float));
    }
    d_samples_cnt += d_fft_size;
  }
}

} /* namespace satnogs */
} /* namespace gr */

