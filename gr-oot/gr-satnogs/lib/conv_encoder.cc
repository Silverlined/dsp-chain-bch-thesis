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

#include <satnogs/conv_encoder.h>

namespace gr {
namespace satnogs {

/**
 * Constructor of convolutional encoder
 * @param cc_rate coding rate of encoding
 * @param max_frame_len max length of frame
 */
conv_encoder::conv_encoder(coding_rate_t cc_rate, size_t max_frame_len)
{
  d_cc_rate = cc_rate;
  d_cc_generator.set_length(2);
  d_cc_generator[0] = 0171;
  d_cc_generator[1] = 0133;
  d_conv_code.set_generator_polynomials(d_cc_generator,
                                        7); // Constraint length = 7
  d_conv_code.init_encoder();
  d_conv_code.set_start_state(0);
}

/**
 * Destructor of convolutional encoder
 */
conv_encoder::~conv_encoder()
{
}

/**
 * Encode a message
 * @param out the encoded message
 * @param in the original message
 * @param len the legth of the original message
 * @return the length of the encoded message
 */
size_t
conv_encoder::encode(uint8_t *out, const uint8_t *in, size_t len)
{
  itpp::bvec unencoded(0);
  itpp::bvec cc_encoded(0);
  for (size_t i = 0; i < len; i++) {
    unencoded = itpp::concat(unencoded, itpp::bin(in[i]));
  }
  d_conv_code.encode_tail(unencoded, cc_encoded);
  reset();
  return inv_and_puncture(out, cc_encoded);
}

/**
 * Trunc encode a message
 * @param out the encoded message
 * @param in the original message
 * @param len the legth of the original message
 * @return the length of the encoded message
 */
size_t
conv_encoder::encode_trunc(uint8_t *out, const uint8_t *in, size_t len)
{
  itpp::bvec unencoded(0);
  itpp::bvec cc_encoded(0);
  for (size_t i = 0; i < len; i++) {
    unencoded = itpp::concat(unencoded, itpp::bin(in[i]));
  }
  d_conv_code.encode_trunc(unencoded, cc_encoded);
  return inv_and_puncture(out, cc_encoded);
}

/**
 * Finalize the encoding of a message
 * @param out the encoded message
 * @return the length of the encoded message
 */
size_t
conv_encoder::finalize(uint8_t *out)
{
  itpp::bvec unencoded(0);
  itpp::bvec cc_encoded(0);
  unencoded.set_length(6, false);
  unencoded.zeros();
  size_t return_length = 0;
  d_conv_code.encode_trunc(unencoded, cc_encoded);
  return_length = inv_and_puncture(out, cc_encoded);
  reset();
  return return_length;
}

/**
 * Reset the encoder's state
 */
void
conv_encoder::reset()
{
  d_conv_code.set_start_state(0);
}

/**
 * Inverse and puncture a given message
 * @param out the result of the prosses
 * @param in the original message
 */
size_t
conv_encoder::inv_and_puncture(uint8_t *out, const itpp::bvec &in)
{
  size_t cnt = 0;
  switch (d_cc_rate) {
  case RATE_1_2:
    for (int i = 0; i < in.length() / 2; i++) {
      out[2 * i] = in[2 * i].value();
      out[2 * i + 1] = !in[2 * i + 1].value();
    }
    return in.length();
  /* Output B inversion is not applicable for punctured codes */
  case RATE_2_3:
    for (int i = 0; i < in.length(); i++) {
      if (i % 4 != 2) {
        out[cnt++] = in[i].value();
      }
    }
    return cnt;
  case RATE_3_4:
    for (int i = 0; i < in.length(); i++) {
      if (i % 6 == 2 || i % 6 == 5) {
        continue;
      }
      out[cnt++] = in[i].value();
    }
    return cnt;
  case RATE_5_6:
    for (int i = 0; i < in.length(); i++) {
      switch (i % 10) {
      case 2:
      case 5:
      case 6:
      case 9:
        break;
      default:
        out[cnt++] = in[i].value();
      }
    }
    return cnt;
  case RATE_7_8:
    for (int i = 0; i < in.length(); i++) {
      switch (i % 14) {
      case 2:
      case 4:
      case 6:
      case 9:
      case 10:
      case 13:
        break;
      default:
        out[cnt++] = in[i].value();
      }
    }
    return cnt;
  default:
    throw std::runtime_error("conv_encoder: invalid coding rate");
  }
}

}  // namespace satnogs
}  // namespace gr
