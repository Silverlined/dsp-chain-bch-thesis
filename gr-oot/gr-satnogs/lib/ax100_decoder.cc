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

#include "ax100_mode5.h"
#include "ax100_mode6.h"

#include <gnuradio/io_signature.h>
#include <satnogs/ax100_decoder.h>

namespace gr {
namespace satnogs {

decoder::decoder_sptr
ax100_decoder::mode5_make(const std::vector<uint8_t> &preamble,
                          size_t preamble_threshold,
                          const std::vector<uint8_t> &sync,
                          size_t sync_threshold,
                          crc::crc_t crc, whitening::whitening_sptr descrambler,
                          bool enable_rs)
{
  return ax100_mode5::make(preamble, preamble_threshold, sync, sync_threshold,
                           crc, descrambler, enable_rs);
}

decoder::decoder_sptr
ax100_decoder::mode6_make(crc::crc_t crc,
                          whitening::whitening_sptr descrambler,
                          bool ax25_descramble)
{
  return ax100_mode6::make(crc, descrambler, ax25_descramble);
}

} /* namespace satnogs */
} /* namespace gr */
