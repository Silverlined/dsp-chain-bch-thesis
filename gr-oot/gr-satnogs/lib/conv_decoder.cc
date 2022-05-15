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

#include <satnogs/conv_decoder.h>
#include <satnogs/libfec/fec.h>
#include <cstring>
#include <iostream>

namespace gr {
namespace satnogs {

/**
 * Constructor for convolutional decoder
 * @param coding_rate the rate of the coding
 * @param max_frame_len max length of the frame
 */
conv_decoder::conv_decoder(coding_rate_t coding_rate, size_t max_frame_len) :
  d_rate(coding_rate),
  d_trunc_depth(0),
  d_syms(nullptr),
  d_unpacked(nullptr),
  d_first_block(true),
  d_vp(0),
  d_last_state(0)
{
  /*
   * The truncate depth greatly affects the performance and the computational
   * demands of the Viterbi decoder.
   *
   * Based on the findings of B.  Moision. “A  Truncation  Depth  Rule
   * of  Thumb  for  Convolutional  Codes.”    In
   * Information  Theory  and  Applications  Workshop
   * (January  27  2008-February  1  2008,  San Diego, California)
   * , 555-557.  New York: IEEE, 2008. mentioned by  SATNOGS 130.1-G-2,
   * we set the truncate depth to $3 \times (K/1-R)$
   */
  switch (coding_rate) {
  case RATE_1_2:
    d_trunc_depth = 3 * (7 / (1 - 1.0 / 2.0));
    while (d_trunc_depth % 2) {
      d_trunc_depth++;
    }
    break;
  case RATE_2_3:
    d_trunc_depth = 3 * (7 / (1 - 2.0 / 3.0));
    while (d_trunc_depth % 3) {
      d_trunc_depth++;
    }
    break;
  case RATE_3_4:
    d_trunc_depth = 3 * (7 / (1 - 3.0 / 4.0));
    while (d_trunc_depth % 4) {
      d_trunc_depth++;
    }
    break;
  case RATE_5_6:
    d_trunc_depth = 3 * (7 / (1 - 5.0 / 6.0));
    while (d_trunc_depth % 6) {
      d_trunc_depth++;
    }
    break;
  case RATE_7_8:
    d_trunc_depth = 3 * (7 / (1 - 7.0 / 8.0));
    while (d_trunc_depth % 8) {
      d_trunc_depth++;
    }
    break;
  default:
    d_trunc_depth = 0;
    throw std::invalid_argument("conv_decoder: Invalid coding rate");
  }

  /*
   * In some cases, when the remaining traceback depth is small, it is better
   * to let the trellis evolve further a bit, rather than doing traceback
   * and reset
   */
  d_long_trunc_depth = d_trunc_depth + d_trunc_depth / 4;
  while (d_long_trunc_depth % 2) {
    d_long_trunc_depth++;
  }
  d_syms = new uint8_t[2 * d_long_trunc_depth];
  d_unpacked = new uint8_t[2 * d_long_trunc_depth / 2];

  int polys[2];

  /* Symbol invertion is performed only for coding rate 1/2 */
  if (coding_rate == RATE_1_2) {
    polys[0] = V27POLYB;
    polys[1] = -V27POLYA;
  }
  else {
    polys[0] = V27POLYB;
    polys[1] = V27POLYA;
  }
  /*
   * The maximum space required would be the
   * traceback depth / 2  + the number of punctured bits. For simplicity
   * we allocate twice the traceback depth to cover all possible cases
   */
  d_vp = create_viterbi27(2 * d_long_trunc_depth);
  if (!d_vp) {
    throw std::runtime_error("conv_decoder: Could not create Viterbi instance");
  }
  set_viterbi27_polynomial(polys);
  init_viterbi27(d_vp, d_last_state);
}

/**
 * Destructor of convolutional decoder
 */
conv_decoder::~conv_decoder()
{
  delete_viterbi27(d_vp);
  delete [] d_syms;
  delete [] d_unpacked;
}

/**
 * Trunc decode a message
 * @param out the decoded message
 * @param in the original message
 * @param len the legth of the original message
 * @return the length of the decoded message
 */
size_t
conv_decoder::decode_trunc(uint8_t *out, const int8_t *in, size_t len)
{
  return -1;
}

/**
 * Reset the state of the decoder
 */
void
conv_decoder::reset()
{
  d_first_block = true;
  d_last_state = 0;
}

/**
 * Decode a message block by block
 * @param out the decoded message
 * @param in the original message
 * @param len the legth of the original message
 * @return the length of the decoded message
 */
size_t
conv_decoder::decode(uint8_t *out, const int8_t *in, size_t len)
{
  size_t idx = 0;
  size_t remaining = len;
  size_t ret;
  reset();
  /* Decode the blocks that fill entirely the whole traceback depth */
  for (size_t i = 0; i < len; i += d_trunc_depth) {
    /*
     * if the remaining bits are within the margin (traceback + traceback/4)
     * perform the whole decoding. Eventually this will result to better
     * performance
     */
    if (remaining < d_long_trunc_depth) {
      ret = decode_block(out + idx, in + i, remaining);
      idx += ret;
      return idx;
    }
    ret = decode_block(out + idx, in + i, d_trunc_depth);
    remaining -= d_trunc_depth;
    idx += ret;
  }

  /* Decode any remaining bits */
  ret = decode_block(out + idx, in + (len / d_trunc_depth) * d_trunc_depth,
                     len - (len / d_trunc_depth) * d_trunc_depth);
  idx += ret;
  return idx;
}

/**
 * Decode a single block based on coding rate
 * @param out the decoded block
 * @param in the original block
 * @param len the legth of the original block
 * @return the length of the decoded block
 */
size_t
conv_decoder::decode_block(uint8_t *out, const int8_t *in, size_t len)
{
  switch (d_rate) {
  case RATE_1_2:
    return decode_block_1_2(out, in, len);
  case RATE_2_3:
    return decode_block_2_3(out, in, len);
  case RATE_3_4:
    return decode_block_3_4(out, in, len);
  case RATE_5_6:
    return decode_block_5_6(out, in, len);
  case RATE_7_8:
    return decode_block_7_8(out, in, len);
  default:
    throw std::invalid_argument("conv_decoder: Invalid coding rate");
    return 0;
  }
  return 0;
}

/**
 * Decode a message with coding rate 1/2
 * @param out the decoded message
 * @param in the original message
 * @param len the legth of the original message
 * @return the length of the decoded message
 */
size_t
conv_decoder::decode_block_1_2(uint8_t *out, const int8_t *in, size_t len)
{
  if (len < 2) {
    return 0;
  }
  init_viterbi27(d_vp, d_last_state);
  /* Convert to libfec compatible soft symbols */
  for (uint32_t i = 0; i < len; i++) {
    d_syms[i] = ((uint8_t)in[i] + 128);
  }

  update_viterbi27_blk(d_vp, d_syms, len / 2);
  int state = chainback_viterbi27_unpacked_trunc(d_vp, d_unpacked, len / 2);
  d_last_state = (uint32_t)state;

  /* Skip the first 6 bits if this was the first block */
  size_t nbits = len / 2;
  if (d_first_block) {
    d_first_block = false;
    memcpy(out, d_unpacked + 6, nbits - 6);
    return nbits - 6;
  }
  memcpy(out, d_unpacked, nbits);
  return nbits;
}

/**
 * Decode a message with coding rate 2/3
 * @param out the decoded message
 * @param in the original message
 * @param len the legth of the original message
 * @return the length of the decoded message
 */
size_t
conv_decoder::decode_block_2_3(uint8_t *out, const int8_t *in, size_t len)
{
  if (len < 3) {
    return 0;
  }
  size_t nsyms = 0;
  init_viterbi27(d_vp, d_last_state);

  /* Convert to libfec compatible soft symbols */
  for (uint32_t i = 0; i < len; i++) {
    d_syms[nsyms++] = ((uint8_t)in[i] + 128);
    if (i % 3 == 1) {
      d_syms[nsyms++] = 127;
    }
  }

  update_viterbi27_blk(d_vp, d_syms, nsyms / 2);
  int state = chainback_viterbi27_unpacked_trunc(d_vp, d_unpacked, nsyms / 2);
  d_last_state = (uint32_t)state;

  /* Repack bits, skipping the first 6 bits if this was the first block */
  size_t nbits = nsyms / 2;
  if (d_first_block) {
    d_first_block = false;
    memcpy(out, d_unpacked + 6, nbits - 6);
    return nbits - 6;
  }
  memcpy(out, d_unpacked, nbits);
  return nbits;
}

/**
 * Decode a message with coding rate 3/4
 * @param out the decoded message
 * @param in the original message
 * @param len the legth of the original message
 * @return the length of the decoded message
 */
size_t
conv_decoder::decode_block_3_4(uint8_t *out, const int8_t *in, size_t len)
{
  if (len < 4) {
    return 0;
  }
  size_t nsyms = 0;
  init_viterbi27(d_vp, d_last_state);

  /* Convert to libfec compatible soft symbols */
  for (uint32_t i = 0; i < len; i++) {
    d_syms[nsyms++] = ((uint8_t)in[i] + 128);
    if (i % 4 == 1 || i % 4 == 3) {
      d_syms[nsyms++] = 127;
    }
  }

  update_viterbi27_blk(d_vp, d_syms, nsyms / 2);
  int state = chainback_viterbi27_unpacked_trunc(d_vp, d_unpacked, nsyms / 2);
  d_last_state = (uint32_t)state;

  /* Repack bits, skipping the first 6 bits if this was the first block */
  size_t nbits = nsyms / 2;
  if (d_first_block) {
    d_first_block = false;
    memcpy(out, d_unpacked + 6, nbits - 6);
    return nbits - 6;
  }
  memcpy(out, d_unpacked, nbits);
  return nbits;
}

/**
 * Decode a message with coding rate 5/6
 * @param out the decoded message
 * @param in the original message
 * @param len the legth of the original message
 * @return the length of the decoded message
 */
size_t
conv_decoder::decode_block_5_6(uint8_t *out, const int8_t *in, size_t len)
{
  if (len < 6) {
    return 0;
  }
  size_t nsyms = 0;
  init_viterbi27(d_vp, d_last_state);

  /* Convert to libfec compatible soft symbols */
  for (uint32_t i = 0; i < len; i++) {
    d_syms[nsyms++] = ((uint8_t)in[i] + 128);
    switch (i % 6) {
    case 1:
      d_syms[nsyms++] = 127;
      break;
    case 3:
      d_syms[nsyms++] = 127;
      d_syms[nsyms++] = 127;
      break;
    case 5:
      d_syms[nsyms++] = 127;
      break;
    }
  }

  update_viterbi27_blk(d_vp, d_syms, nsyms / 2);
  int state = chainback_viterbi27_unpacked_trunc(d_vp, d_unpacked, nsyms / 2);
  d_last_state = (uint32_t)state;

  /* Repack bits, skipping the first 6 bits if this was the first block */
  size_t nbits = nsyms / 2;
  if (d_first_block) {
    d_first_block = false;
    memcpy(out, d_unpacked + 6, nbits - 6);
    return nbits - 6;
  }
  memcpy(out, d_unpacked, nbits);
  return nbits;
}

/**
 * Decode a message with coding rate 7/8
 * @param out the decoded message
 * @param in the original message
 * @param len the legth of the original message
 * @return the length of the decoded message
 */
size_t
conv_decoder::decode_block_7_8(uint8_t *out, const int8_t *in, size_t len)
{
  if (len < 8) {
    return 0;
  }
  size_t nsyms = 0;
  init_viterbi27(d_vp, d_last_state);

  /* Convert to libfec compatible soft symbols */
  for (uint32_t i = 0; i < len; i++) {
    d_syms[nsyms++] = ((uint8_t)in[i] + 128);
    switch (i % 8) {
    case 1:
    case 2:
    case 3:
      d_syms[nsyms++] = 127;
      break;
    case 5:
      d_syms[nsyms++] = 127;
      d_syms[nsyms++] = 127;
      break;
    case 7:
      d_syms[nsyms++] = 127;
      break;
    }
  }

  update_viterbi27_blk(d_vp, d_syms, nsyms / 2);
  int state = chainback_viterbi27_unpacked_trunc(d_vp, d_unpacked, nsyms / 2);
  d_last_state = (uint32_t)state;

  /* Repack bits, skipping the first 6 bits if this was the first block */
  size_t nbits = nsyms / 2;
  if (d_first_block) {
    d_first_block = false;
    memcpy(out, d_unpacked + 6, nbits - 6);
    return nbits - 6;
  }
  memcpy(out, d_unpacked, nbits);
  return nbits;
}

}  // namespace satnogs
}  // namespace gr
