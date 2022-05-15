/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2021, Libre Space Foundation <http://libre.space>
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

#include <satnogs/ax100_encoder.h>

#include "ax100_mode5_encoder.h"
#include "ax100_mode6_encoder.h"

namespace gr {
namespace satnogs {

encoder::encoder_sptr
ax100_encoder::mode5_make(const std::vector<uint8_t> &preamble,
                          const std::vector<uint8_t> &sync,
                          crc::crc_t crc,
                          bool enable_rs)
{
  return ax100_mode5_encoder::make(preamble, sync, crc, enable_rs);
}

encoder::encoder_sptr
ax100_encoder::mode6_make(size_t preamble_len, size_t postamble_len,
                          whitening::whitening_sptr scrambler, bool scramble,
                          bool nrzi, crc::crc_t crc)
{
  return ax100_mode6_encoder::make(preamble_len, postamble_len,
                                   scrambler, scramble, nrzi, crc);
}

} /* namespace satnogs */
} /* namespace gr */

