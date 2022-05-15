/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2016,2018 Libre Space Foundation <http://librespacefoundation.org/>
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

#ifndef INCLUDE_SATNOGS_AX25_H_
#define INCLUDE_SATNOGS_AX25_H_

#include <satnogs/crc.h>
#include <satnogs/log.h>
#include <limits.h>
#include <stdint.h>
#include <string>

namespace gr {

namespace satnogs {

class ax25 {
public:
  static const size_t min_addr_len = 14;
  static const size_t max_addr_len = (2 * 7 + 8 * 7);
  static const size_t max_ctrl_len = 2;
  static const size_t pid_len = 1;
  static const size_t max_header_len = (max_addr_len + max_ctrl_len + pid_len);
  static const uint8_t sync_flag = 0x7e;
  static const size_t callsign_max_len = 6;

  /**
   * AX.25 Frame types
   */
  typedef enum {
    I_FRAME, //!< Information frame
    S_FRAME, //!< Supervisory frame
    U_FRAME, //!< Unnumbered frame
    UI_FRAME //!< Unnumbered information frame
  } frame_type_t;

  static uint16_t
  crc(const uint8_t *buffer, size_t len)
  {
    return crc::crc16_ax25(buffer, len);
  }
};

}  // namespace satnogs
}  // namespace gr

#endif /* INCLUDE_SATNOGS_AX25_H_ */
