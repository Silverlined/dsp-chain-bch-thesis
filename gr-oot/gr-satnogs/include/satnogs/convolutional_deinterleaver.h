/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2018, Libre Space Foundation <http://librespacefoundation.org/>
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

#ifndef INCLUDED_SATNOGS_CONVOLUTIONAL_DEINTERLEAVER_H
#define INCLUDED_SATNOGS_CONVOLUTIONAL_DEINTERLEAVER_H

#include <satnogs/api.h>

#include <vector>
#include <deque>
namespace gr {
namespace satnogs {

/*!
 * \brief <+description+>
 *
 */
class SATNOGS_API convolutional_deinterleaver {
public:
  convolutional_deinterleaver(size_t branches, size_t M);
  ~convolutional_deinterleaver();

  uint8_t
  decode_bit(uint8_t b);

  uint8_t
  decode_byte(uint8_t b);

  void
  reset();

private:
  const size_t d_nbranches;
  const size_t d_M;
  size_t d_idx;
  std::vector<std::deque<uint8_t>> d_branches;
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_CONVOLUTIONAL_DEINTERLEAVER_H */

