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

#ifndef INCLUDED_SATNOGS_REED_MULLER_H
#define INCLUDED_SATNOGS_REED_MULLER_H

#include <satnogs/api.h>
#include <cstdint>
#include <array>
#include <bitset>

namespace gr {
namespace satnogs {

/*!
 * \brief A binary Reed-Muller(1, 6) encoder and decoder.
 *
 */
class SATNOGS_API reed_muller {
public:
  reed_muller();
  ~reed_muller();

  uint64_t
  encode(const uint8_t in);

  uint8_t
  decode(const uint64_t in);

private:
  const size_t d_r;
  const size_t d_m;
  std::array<std::bitset<64>, 7> d_G;

};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_REED_MULLER_H */
