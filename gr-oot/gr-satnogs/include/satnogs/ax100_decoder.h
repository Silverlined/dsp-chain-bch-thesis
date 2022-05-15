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

#ifndef INCLUDED_SATNOGS_AX100_DECODER_H
#define INCLUDED_SATNOGS_AX100_DECODER_H

#include <satnogs/api.h>
#include <satnogs/decoder.h>
#include <satnogs/shift_reg.h>
#include <satnogs/crc.h>
#include <satnogs/whitening.h>

namespace gr {
namespace satnogs {

/*!
 * \brief This decoder tries to cover all the supported framing and coding
 * schemes of the AX100 product from GOMSpace. It also supports some variants
 * that have appear in different missions, including excet the ASM and a
 * repeated preamble
 *
 */
class SATNOGS_API ax100_decoder {
public:
  static decoder::decoder_sptr
  mode5_make(const std::vector<uint8_t> &preamble,
             size_t preamble_threshold,
             const std::vector<uint8_t> &sync,
             size_t sync_threshold,
             crc::crc_t crc,
             whitening::whitening_sptr descrambler,
             bool enable_rs);

  static decoder::decoder_sptr
  mode6_make(crc::crc_t crc = crc::CRC32_C,
             whitening::whitening_sptr descrambler = whitening::make_ccsds(),
             bool ax25_descramble = true);

};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_AX100_DECODER_H */

