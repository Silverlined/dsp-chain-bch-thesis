/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2019, 2020 Libre Space Foundation <http://libre.space>
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

#ifndef INCLUDED_SATNOGS_CRC_H
#define INCLUDED_SATNOGS_CRC_H

#include <satnogs/api.h>

namespace gr {
namespace satnogs {

/*!
 * CRC class providing a range of different CRC calculation static methods
 *
 */
class SATNOGS_API crc {
public:
  typedef enum crc_type {
    CRC_NONE = 0,
    CRC16_CCITT,
    CRC16_AUG_CCITT,
    CRC16_CCITT_REVERSED,
    CRC16_AX25,
    CRC16_IBM,
    CRC32_C,
    CRC_METHODS_NUM
  } crc_t;

  static uint16_t
  crc16_ccitt_reversed(const uint8_t *data, size_t len);

  static uint16_t
  crc16_ccitt(const uint8_t *data, size_t len);

  static uint16_t
  crc16_aug_ccitt(const uint8_t *data, size_t len);

  static uint16_t
  crc16_ax25(const uint8_t *data, size_t len);

  static uint16_t
  crc16_ibm(const uint8_t *data, size_t len);

  static uint32_t
  crc32_c(const uint8_t *data, size_t len);

  static size_t
  crc_size(crc_t t);

  static size_t
  append(crc_t t, uint8_t *out, const uint8_t *data, size_t len, bool nbo = true);

  static bool
  check(crc_t t, const uint8_t *data, size_t len, bool nbo = true);

private:
  static const uint16_t crc16_ccitt_table_reverse[256];
  static const uint16_t crc16_ccitt_table[256];
  static const uint32_t crc32_c_table[256];
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_CRC_H */
