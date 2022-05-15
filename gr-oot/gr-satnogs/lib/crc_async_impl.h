/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2020, Libre Space Foundation <http://libre.space>
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

#ifndef LIB_CRC_ASYNC_IMPL_H_
#define LIB_CRC_ASYNC_IMPL_H_

#include <satnogs/crc_async.h>

namespace gr {
namespace satnogs {

class crc_async_impl : public crc_async {

public:
  crc_async_impl(crc::crc_t crc, bool check);
  ~crc_async_impl();

private:
  crc::crc_t d_crc;

  void
  append(pmt::pmt_t msg);

  void
  check(pmt::pmt_t msg);
};

} // namespace satnogs
} // namespace gr

#endif /* LIB_CRC_ASYNC_IMPL_H_ */
