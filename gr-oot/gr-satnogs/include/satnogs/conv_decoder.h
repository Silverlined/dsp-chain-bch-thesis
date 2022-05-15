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

#ifndef INCLUDED_SATNOGS_CONV_DECODER_H
#define INCLUDED_SATNOGS_CONV_DECODER_H

#include <satnogs/api.h>
#include <cstddef>
#include <cstdint>

namespace gr {
namespace satnogs {

/*!
 * \brief SATNOGS compatible convolutional decoder with puncturing support
 *
 */
class SATNOGS_API conv_decoder {
public:
  typedef enum {
    RATE_1_2 = 0,
    RATE_2_3,
    RATE_3_4,
    RATE_5_6,
    RATE_7_8
  } coding_rate_t;

  conv_decoder(coding_rate_t coding_rate, size_t max_frame_len);
  virtual ~conv_decoder();

  void
  reset();

  size_t
  decode_trunc(uint8_t *out, const int8_t *in, size_t len);

  size_t
  decode(uint8_t *out, const int8_t *in, size_t len);

private:
  const coding_rate_t   d_rate;
  size_t                d_trunc_depth;
  size_t                d_long_trunc_depth;
  uint8_t               *d_syms;
  uint8_t               *d_unpacked;
  bool                  d_first_block;
  void                  *d_vp;
  uint32_t              d_last_state;

  size_t
  decode_block(uint8_t *out, const int8_t *in, size_t len);

  size_t
  decode_block_1_2(uint8_t *out, const int8_t *in, size_t len);

  size_t
  decode_block_2_3(uint8_t *out, const int8_t *in, size_t len);

  size_t
  decode_block_3_4(uint8_t *out, const int8_t *in, size_t len);

  size_t
  decode_block_5_6(uint8_t *out, const int8_t *in, size_t len);

  size_t
  decode_block_7_8(uint8_t *out, const int8_t *in, size_t len);
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_CONV_DECODER_H */
