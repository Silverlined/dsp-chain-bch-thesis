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

#ifndef INCLUDED_SATNOGS_CONV_ENCODER_H
#define INCLUDED_SATNOGS_CONV_ENCODER_H

#include <satnogs/api.h>
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <itpp/base/array.h>
#include <itpp/comm/convcode.h>
#include <itpp/comm/reedsolomon.h>

namespace gr {
namespace satnogs {

class SATNOGS_API conv_encoder {

  /**
  * \brief SATNOGS compatible convolutional encoder with puncturing support
  *
  */
public:
  typedef enum {
    RATE_1_2 = 0,
    RATE_2_3,
    RATE_3_4,
    RATE_5_6,
    RATE_7_8
  } coding_rate_t;

  conv_encoder(coding_rate_t cc_rate, size_t max_frame_len);
  ~conv_encoder();

  size_t
  encode(uint8_t *out, const uint8_t *in, size_t len);

  size_t
  encode_trunc(uint8_t *out, const uint8_t *in, size_t len);

  void
  reset();

  size_t
  finalize(uint8_t *out);

private:
  coding_rate_t                 d_cc_rate;
  itpp::Convolutional_Code      d_conv_code;
  itpp::ivec                    d_cc_generator;

  size_t
  inv_and_puncture(uint8_t *out, const itpp::bvec &in);
};

}
}

#endif /* INCLUDED_SATNOGS_CONV_ENCODER_H */
