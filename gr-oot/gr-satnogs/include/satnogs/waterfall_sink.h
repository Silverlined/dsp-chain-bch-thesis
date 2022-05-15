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

#ifndef INCLUDED_SATNOGS_WATERFALL_SINK_H
#define INCLUDED_SATNOGS_WATERFALL_SINK_H

#include <satnogs/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
namespace satnogs {

/*!
 * \brief This block computes the waterfall of the incoming signal
 * and stores the result to a file.
 *
 * The file has a special header, so plotting tools can reconstruct properly
 * the spectrum.
 *
 * \ingroup satnogs
 *
 */
class SATNOGS_API waterfall_sink : virtual public gr::sync_block {
public:
  typedef boost::shared_ptr<waterfall_sink> sptr;

  /**
   * This block computes the waterfall of the incoming signal
   * and stores the result to a file.
   *
   * The file has a constant sized header of 52 bytes, so that plotting tools can
   * reconstruct properly the spectrum.
   *
   * The structure of the header is the following:
   *  - A 32 byte string containing the timestamp in
   *    ISO-8601 format. This timer has microsecond accuracy.
   *  - A 4 byte integer containing the sampling rate
   *  - A 4 byte integer with the FFT size
   *  - A 4 byte integer containing the number of FFT snapshots for one row
   *    at the waterfall
   *  - A 4 byte float with the center frequency of the observation.
   *  - A 4 byte integer indicating the endianness of the rest of the file. If
   *    set to 0 the file continues in Big endian. Otherwise, in little endian.
   *    The change of the endianness is performed to reduce the overhead at the
   *    station.
   *
   * @note All contents of the header are in Network Byte order! The rest
   * of the file is in native byte order, mainly for performance reasons.
   * Users can use data of the header to determine if their architecture match
   * the architecture of the host generated the waterfall file and act
   * accordingly.
   *
   * The file continues with information regarding the spectral content of the
   * observation.
   * Each waterfall line is prepended with a int64_t field indicating the
   * absolute time in microseconds with respect to the start of the waterfall
   * data (stored in the corresponding header field).
   * The spectral content is stored in $FFT$ float values already converted in
   * dB scale.
   *
   * @param samp_rate the sampling rate
   * @param center_freq the observation center frequency. Used only for
   * plotting reasons. For a normalized frequency x-axis set it to 0.
   * @param rps rows per second
   * @param fft_size FFT size
   * @param filename the name of the output file
   * @param mode the mode that the waterfall.
   * - 0: Simple decimation
   * - 1: Max hold
   * - 2: Mean energy
   * @return shared pointer to the object
   */
  static sptr
  make(float samp_rate, float center_freq,
       float rps, size_t fft_size,
       const std::string &filename, int mode = 0);
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_WATERFALL_SINK_H */

