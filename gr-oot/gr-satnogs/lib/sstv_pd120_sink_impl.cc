/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2019, 2020,
 *  Libre Space Foundation <http://librespacefoundation.org/>
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
#include "sstv_pd120_sink_impl.h"
#include <satnogs/metadata.h>

namespace gr {
namespace satnogs {

const size_t image_width = 640;
const size_t image_height = 496;
const size_t sync_length = 105;
const size_t sync_thresh = 100;
const size_t porch_length = 10;
const size_t line_length = sync_length + porch_length + 4 * image_width;

const float max_dev = 600;
const float center = 1750;
const float min_freq = 1200;
const float max_freq = 2300;

const float color_low = 1500;
const float color_high = max_freq;


sstv_pd120_sink::sptr
sstv_pd120_sink::make(const char *filename_png)
{
  return gnuradio::get_initial_sptr
         (new sstv_pd120_sink_impl(filename_png));
}

/*
 * The private constructor
 */
sstv_pd120_sink_impl::sstv_pd120_sink_impl(const char *filename_png)
  : gr::sync_block("sstv_pd120_sink",
                   gr::io_signature::make(1, 1, sizeof(float)),
                   gr::io_signature::make(0, 0, 0)),
    d_filename_png(filename_png),
    d_has_sync(false),
    d_initial_sync(true),
    d_line_pos(0),
    d_image_y(0)
{
  set_history(sync_length);
  d_line = new float[line_length];

  d_image = png::image<png::rgb_pixel>(image_width, image_height);
  blank_image();
}

/*
 * Our virtual destructor.
 */
sstv_pd120_sink_impl::~sstv_pd120_sink_impl()
{
  delete[] d_line;
}

bool
sstv_pd120_sink_impl::stop()
{
  /*
   * If there is an semi-decoded image, write it to
   * a file and then exit
   */
  if (d_image_y) {
    std::string file_name = std::string(d_filename_png) + "_"
                            + metadata::time_iso8601() + ".png";
    d_image.write(file_name.c_str());
  }
  return true;
}

float
sstv_pd120_sink_impl::to_frequency(float sample)
{
  float freq = center + sample * max_dev;
  freq = std::max(min_freq, freq);
  freq = std::min(max_freq, freq);
  return freq;
}

int
sstv_pd120_sink_impl::to_color(float sample)
{
  sample = (sample - color_low) / (color_high - color_low);
  sample = sample * 255.0f;
  sample = std::max(sample, 0.0f);
  sample = std::min(sample, 255.0f);
  return int(sample);
}

void
sstv_pd120_sink_impl::ycbcr_to_rgb(int ycbcr[3], int rgb[3])
{
  int y = ycbcr[0];
  int cb = ycbcr[1];
  int cr = ycbcr[2];

  //https://stackoverflow.com/questions/4041840/function-to-convert-ycbcr-to-rgb/15333019#15333019
  cr = cr - 128;
  cb = cb - 128;
  int r = y + 45 * cr / 32;
  int g = y - (11 * cb + 23 * cr) / 32;
  int b = y + 113 * cb / 64;

  rgb[0] = std::min(255, std::max(r, 0));
  rgb[1] = std::min(255, std::max(g, 0));
  rgb[2] = std::min(255, std::max(b, 0));
}

bool
sstv_pd120_sink_impl::is_sync(size_t pos, const float *samples)
{
  size_t count = 0;
  for (size_t i = 0; i < sync_length; i++) {
    float sample = to_frequency(samples[pos - (sync_length - 1) + i]);
    if (sample < color_low) {
      count += 1;
    }
  }

  bool res = count > sync_thresh && !d_has_sync;
  d_has_sync = count > sync_thresh;

  if (res) {
    d_initial_sync = false;
  }

  return res;
}


void
sstv_pd120_sink_impl::render_line()
{
  int start_pos = d_line_pos - sync_length - 4 * image_width;
  if (start_pos < 0) {
    return;
  }

  for (size_t x = 0; x < image_width; x++) {
    int y0 = to_color(d_line[start_pos + x]);
    int cr = to_color(d_line[start_pos + image_width + x]);
    int cb = to_color(d_line[start_pos + 2 * image_width + x]);
    int y1 = to_color(d_line[start_pos + 3 * image_width + x]);

    int rgb0[3] = {0, 0, 0};
    int ycrcb0[] = {y0, cb, cr};
    ycbcr_to_rgb(ycrcb0, rgb0);

    d_image.set_pixel(x, d_image_y, png::rgb_pixel(rgb0[0], rgb0[1], rgb0[2]));

    int rgb1[] = {0, 0, 0};
    int ycrcb1[] = {y1, cb, cr};
    ycbcr_to_rgb(ycrcb1, rgb1);
    d_image.set_pixel(x, d_image_y + 1, png::rgb_pixel(rgb1[0], rgb1[1], rgb1[2]));
  }
  d_image_y += 2;

  if (d_image_y >= image_height) {
    /* Write the decoded image before moving to the next one */
    std::string file_name = std::string(d_filename_png) + "_"
                            + metadata::time_iso8601() + ".png";
    d_image.write(file_name.c_str());

    d_image_y = 0;
    d_initial_sync = true;
    blank_image();
  }
}

void
sstv_pd120_sink_impl::blank_image()
{
  for (size_t y = 0; y < image_height; y++) {
    for (size_t x = 0; x < image_width; x++) {
      d_image.set_pixel(x, y, png::rgb_pixel(0, 0, 0));
    }
  }
}

int
sstv_pd120_sink_impl::work(int noutput_items,
                           gr_vector_const_void_star &input_items,
                           gr_vector_void_star &output_items)
{
  const float *in = (const float *) input_items[0];

  for (size_t i = sync_length - 1;
       i < noutput_items + sync_length - 1; i++) {

    float sample = to_frequency(in[i]);
    d_line[d_line_pos] = sample;
    d_line_pos++;

    if (is_sync(i, in) || d_line_pos >= line_length) {
      if (d_initial_sync) {
        d_image_y = 0;
      }
      else if (!d_initial_sync && d_line_pos > line_length - porch_length) {
        render_line();
      }
      d_line_pos = 0;
    }
  }

  // Tell runtime system how many output items we produced.
  return noutput_items;
}

} /* namespace satnogs */
} /* namespace gr */
