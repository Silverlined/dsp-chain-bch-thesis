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

#ifndef INCLUDED_SATNOGS_AX100_ENCODER_H
#define INCLUDED_SATNOGS_AX100_ENCODER_H

#include <satnogs/api.h>
#include <satnogs/encoder.h>
#include <satnogs/crc.h>
#include <satnogs/whitening.h>

#include <vector>

namespace gr {
namespace satnogs {
/*!
 * \brief AX.100 frame encoder definition
 * \ingroup satnogs
 *
 */
class SATNOGS_API ax100_encoder {
public:
  static encoder::encoder_sptr
  mode5_make(const std::vector<uint8_t> &preamble,
             const std::vector<uint8_t> &sync,
             crc::crc_t crc,
             bool enable_rs);

  static encoder::encoder_sptr
  mode6_make(size_t preamble_len, size_t postamble_len,
             whitening::whitening_sptr scrambler, bool scramble,
             bool nrzi, crc::crc_t crc);

};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_AX100_ENCODER_H */
