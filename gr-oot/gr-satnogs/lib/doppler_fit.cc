/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2016, Libre Space Foundation <http://librespacefoundation.org/>
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
#include <satnogs/doppler_fit.h>

namespace gr {
namespace satnogs {

/**
 * Creates a Doppler frequency Lagrange extrapolation engine
 * @param degree the degree of the polynomial
 */
doppler_fit::doppler_fit(size_t degree) :
  d_degree(degree),
  d_ready(false)
{
}


/**
 * Takes a new measurement to calculate the extrapolation
 * @param x the index of the sample
 * @param y the frequency
 */
void
doppler_fit::fit(uint64_t x, double y)
{
  std::lock_guard<std::mutex> lock(d_mtx);
  if (!d_ready) {
    d_data.push_back({x, y});
    if (d_data.size() == d_degree) {
      d_ready = true;
      return;
    }
    return;
  }

  d_data.pop_front();
  d_data.push_back({x, y});
}

double
doppler_fit::L(uint64_t x)
{
  if (!d_ready) {
    return 0.0;
  }
  double ret = 0.0;
  for (size_t i = 0; i < d_degree; i++) {
    double m = d_data[i].second;
    for (size_t j = 0; j < d_degree; j++) {
      if (i != j && d_data[i].first != d_data[j].first) {
        m *= ((x - d_data[j].first) / (d_data[i].first - d_data[j].first));
      }
    }
    ret += m;
  }
  return ret;
}

/**
 * Creates a number of frequency shift predictions using the Lagrange extrapolation
 *
 * @param freqs buffer that will hold the predicted frequency differences.
 * It is responsibility of the caller to provide enough memory for at most
 * \p ncorrections double numbers.
 *
 * @param ncorrections the number predicted frequencies that the method
 * will produce.
 *
 * @param samples_per_correction the number of samples elapsed between each
 * correction.
 */
void
doppler_fit::predict_freqs(double *freqs, size_t ncorrections,
                           size_t samples_per_correction)
{
  std::lock_guard<std::mutex> lock(d_mtx);
  if (!d_ready) {
    for (size_t i = 0; i < ncorrections; i++) {
      freqs[i] = 0.0;
    }
    return;
  }

  for (size_t i = 0; i < ncorrections; i++) {
    freqs[i] = L(d_data[d_degree - 1].first + i * samples_per_correction);
  }
}

} /* namespace satnogs */
} /* namespace gr */

