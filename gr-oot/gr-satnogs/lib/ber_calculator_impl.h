/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2016, Libre Space Foundation <http://librespacefoundation.org/>
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
#ifndef LIB_BER_CALCULATOR_IMPL_H_
#define LIB_BER_CALCULATOR_IMPL_H_

#include <satnogs/ber_calculator.h>
#include <deque>
#include <cstdint>

namespace gr {
namespace satnogs {

class ber_calculator_impl : public ber_calculator {
public:
  ber_calculator_impl(size_t frame_size = 64, size_t nframes = 0,
                      size_t skip = 10);

  ~ber_calculator_impl();

  bool
  stop();

private:
  const size_t d_frame_size;
  const size_t d_nframes;
  const size_t d_skip;
  uint64_t d_cnt;
  uint64_t d_inval_cnt;
  uint64_t d_last_ack;
  uint64_t d_dropped;
  uint64_t d_received;

  void
  create_pdu();

  void
  received_pdu(pmt::pmt_t msg);

  double
  fer();

  double
  ber();

  void
  print_stats();
};

} /* namespace satnogs */
} /* namespace gr */

#endif /* LIB_BER_CALCULATOR_IMPL_H_ */
