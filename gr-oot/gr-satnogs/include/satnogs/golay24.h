/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2016, 2017, 2018
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

#ifndef INCLUDED_SATNOGS_GOLAY24_H
#define INCLUDED_SATNOGS_GOLAY24_H

#include <satnogs/api.h>

#include <cstdint>
#include <vector>

namespace gr {
namespace satnogs {

/*!
 * \brief A binary Golay (24,12,8) encoder and decoder.
 *
 * A binary Golay (24,12,8) encoder and decoder. The implementation uses LUT
 * based on the process described in the book:
 *
 * Morelos-Zaragoza, Robert H. The art of error correcting coding.
 * John Wiley & Sons, 2006.
 *
 */
class SATNOGS_API golay24 {
public:
  golay24();
  ~golay24();

  static const std::vector<uint32_t>          G_P;
  static const std::vector<uint32_t>          G_I;

  uint32_t
  encode12(uint16_t in, bool lsb_parity = true);

  bool
  decode24(uint32_t *out, uint32_t in);

};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_GOLAY24_H */

