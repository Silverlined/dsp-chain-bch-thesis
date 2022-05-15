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


#ifndef INCLUDED_SATNOGS_SSTV_PD120_SINK_IMPL_H
#define INCLUDED_SATNOGS_SSTV_PD120_SINK_IMPL_H

#include <satnogs/sstv_pd120_sink.h>

#define PNG_DEBUG 3
#include <png++/png.hpp>

namespace gr {
namespace satnogs {

class sstv_pd120_sink_impl : public sstv_pd120_sink {
private:
  std::string d_filename_png;
  bool d_has_sync;
  bool d_initial_sync;

  float *d_line;
  size_t d_line_pos;
  size_t d_image_y;

  png::image<png::rgb_pixel> d_image;

  float to_frequency(float sample);
  int to_color(float sample);
  void ycbcr_to_rgb(int ycbcr[3], int rgb[3]);
  bool is_sync(size_t pos, const float *samples);

  void render_line();
  void blank_image();

public:
  sstv_pd120_sink_impl(const char *filename_png);
  ~sstv_pd120_sink_impl();
  bool stop();

  // Where all the action really happens
  int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_SSTV_PD120_SINK_IMPL_H */
