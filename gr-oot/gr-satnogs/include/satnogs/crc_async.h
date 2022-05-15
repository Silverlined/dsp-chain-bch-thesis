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

#ifndef INCLUDE_SATNOGS_CRC_ASYNC_H_
#define INCLUDE_SATNOGS_CRC_ASYNC_H_


#include <satnogs/api.h>
#include <gnuradio/block.h>
#include <satnogs/crc.h>

namespace gr {
namespace satnogs {

/*!
 * \brief An extended version of the CRC Async block of GNU Radio
 * \ingroup satnogs
 *
 */
class SATNOGS_API crc_async : virtual public gr::block {
public:
  typedef boost::shared_ptr<crc_async> sptr;

  /*!
   * \brief Return a shared_ptr to a new instance of satnogs::crc_async.
   *
   * To avoid accidental use of raw pointers, satnogs::crc_async
   * constructor is in a private implementation
   * class. satnogs::crc_async::make is the public interface for
   * creating new instances.
   */
  static sptr make(crc::crc_t crc, bool check = false);
};

} // namespace satnogs
} // namespace gr


#endif /* INCLUDE_SATNOGS_CRC_ASYNC_H_ */
