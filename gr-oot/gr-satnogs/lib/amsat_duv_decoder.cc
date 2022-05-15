/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2019, Libre Space Foundation <http://libre.space>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <satnogs/amsat_duv_decoder.h>
#include <satnogs/metadata.h>
#include <gnuradio/blocks/count_bits.h>
#include <satnogs/libfec/fec.h>

namespace gr {
namespace satnogs {

/**
 * Actual frame size without RS padding and parity.
 * 6 bytes are header, 58 payload
 */
const size_t amsat_duv_decoder::amsat_fox_duv_frame_size = 6 + 58;

const uint8_t amsat_duv_decoder::amsat_fox_spacecraft_id[] {
  0x1 /* FOX-1A */,
  0x2 /* FOX-1B */,
  0x3 /* FOX-1C */,
  0x4 /* FOX-1D */,
  0x5 /* FOX-1E */
};

/**
 * Creates a shared pointer to a amsat_duv_decoder object
 * @param control_symbol the control symbol indicating the start of a frame
 * @param max_frame_len the maximum frame length
 * @return a shared pointer to a amsat_duv_decoder object
 */
decoder::decoder_sptr
amsat_duv_decoder::make(const std::string &control_symbol,
                        size_t max_frame_len)
{
  return decoder::decoder_sptr(
           new amsat_duv_decoder(control_symbol, max_frame_len));
}

/**
 *
 * Creates a decoder for the 80b10 DUV coding scheme of the AMSAT FOX satellites
 * @param control_symbol the control symbol indicating the start of a frame
 * @param max_frame_len the maximum frame length
 */
amsat_duv_decoder::amsat_duv_decoder(const std::string &control_symbol,
                                     size_t max_frame_len) :
  decoder("amsat_duv", "1.0", 1, max_frame_len),
  d_erasure_cnt(0),
  d_control_symbol_pos(0),
  d_control_symbol_neg(0),
  d_data_reg(0),
  d_wrong_bits(0),
  d_wrong_bits_neg(0),
  d_nwrong(0),
  d_nwrong_neg(0),
  d_word_cnt(0),
  d_state(SEARCH_SYNC),
  d_bitstream_idx(0)
{
  d_8b_words = new uint8_t[max_frame_len];
  d_erasures_indexes = new int[max_frame_len];

  if (!set_access_code(control_symbol)) {
    throw std::out_of_range("control_symbol is not 10 bits");
  }
}

bool
amsat_duv_decoder::set_access_code(const std::string &control_symbol)
{
  unsigned len = control_symbol.length();       // # of bytes in string

  /* if the control sequence is not 10-bit then throw exception */
  if (len != 10) {
    return false;
  }

  for (size_t i = 0; i < len; i++) {
    d_control_symbol_pos = (d_control_symbol_pos << 1)
                           | (control_symbol[i] & 0x1);
  }
  d_control_symbol_neg = (~d_control_symbol_pos) & 0x3FF;
  return true;
}

amsat_duv_decoder::~amsat_duv_decoder()
{
  delete[] d_8b_words;
  delete[] d_erasures_indexes;
}

void
amsat_duv_decoder::process_10b(uint16_t word, size_t write_pos)
{
  uint16_t diff_bits = 0;
  uint8_t min_pos = 0;
  uint8_t min_dist = 11;
  uint8_t curr_dist = 0;
  size_t i = 0;

  /* Check for the disparity -1 */
  while ((i < 256) && (min_dist > 0)) {

    diff_bits = (word ^ (d_lookup_8b10b[0][i])) & 0x3FF;
    curr_dist = gr::blocks::count_bits16(diff_bits);

    if (curr_dist < min_dist) {
      min_dist = curr_dist;
      min_pos = i;
    }
    i++;
  }

  /* Check for the disparity +1 */
  i = 0;
  while ((i < 256) && (min_dist > 0)) {

    diff_bits = (word ^ (d_lookup_8b10b[1][i])) & 0x3FF;
    curr_dist = gr::blocks::count_bits16(diff_bits);

    if (curr_dist < min_dist) {
      min_dist = curr_dist;
      min_pos = i;
    }
    i++;
  }

  /* report that there is erasure to this 10 bits */
  d_8b_words[write_pos] = min_pos;

  /* If we did not found a perfect match, mark this index as erasure */
  if (min_dist != 0) {
    d_erasures_indexes[d_erasure_cnt++] = write_pos;
  }
}

inline uint16_t
amsat_duv_decoder::pack_10b_word(size_t idx)
{
  return (((uint16_t) d_bitstream[idx] & 0x1) << 9)
         | (((uint16_t) d_bitstream[idx + 1] & 0x1) << 8)
         | (((uint16_t) d_bitstream[idx + 2] & 0x1) << 7)
         | (((uint16_t) d_bitstream[idx + 3] & 0x1) << 6)
         | (((uint16_t) d_bitstream[idx + 4] & 0x1) << 5)
         | (((uint16_t) d_bitstream[idx + 5] & 0x1) << 4)
         | (((uint16_t) d_bitstream[idx + 6] & 0x1) << 3)
         | (((uint16_t) d_bitstream[idx + 7] & 0x1) << 2)
         | (((uint16_t) d_bitstream[idx + 8] & 0x1) << 1)
         | (d_bitstream[idx + 9] & 0x1);
}

static inline bool
is_spacecraft_valid(uint8_t id)
{
  for (size_t i = 0; i < sizeof(amsat_duv_decoder::amsat_fox_spacecraft_id);
       i++) {
    if (amsat_duv_decoder::amsat_fox_spacecraft_id[i] == id) {
      return true;
    }
  }
  return false;
}

decoder_status_t
amsat_duv_decoder::decode(const void *in, int len)
{
  const uint8_t *input = (const uint8_t *) in;
  decoder_status_t status;
  int ret;
  uint16_t word;
  size_t available;

  /* Due to internal buffering we consume all the availabele symbols */
  for (int i = 0; i < len; i++) {
    d_bitstream.push_back(input[i]);
  }
  status.consumed = len;

  while (1) {
    bool cont = false;
    if (d_bitstream.size() < 11) {
      return status;
    }
    switch (d_state) {
    case SEARCH_SYNC:
      for (size_t i = 0; i < d_bitstream.size(); i++) {
        d_data_reg = (d_data_reg << 1) | (d_bitstream[i] & 0x1);
        d_wrong_bits = (d_data_reg ^ d_control_symbol_pos) & 0x3FF;
        d_wrong_bits_neg = (d_data_reg ^ d_control_symbol_neg) & 0x3FF;
        d_nwrong = gr::blocks::count_bits16(d_wrong_bits);
        d_nwrong_neg = gr::blocks::count_bits16(d_wrong_bits_neg);

        /* we found the controls symbol */
        if ((d_nwrong == 0) || (d_nwrong_neg == 0)) {
          d_erasure_cnt = 0;
          d_word_cnt = 0;
          d_state = DECODING;
          if (i > 10) {
            d_bitstream_idx = 9;
            d_bitstream.erase(d_bitstream.begin(),
                              d_bitstream.begin() + i + 1 - 9);
          }
          else {
            d_bitstream_idx = i;
            d_bitstream.pop_front();
          }
          return status;
        }
      }
      /* No SYNC found on the entire buffer. Clear it and return */
      d_bitstream.clear();
      return status;
    case DECODING:
      available = d_bitstream.size() - d_bitstream_idx;
      if (available < 10) {
        return status;
      }
      /*
       * From now one, we have a SYNC so we process the data
       * in chunks of 10 bits
       */
      for (size_t i = 0; i < available / 10; i++, d_bitstream_idx += 10) {
        word = pack_10b_word(d_bitstream_idx);

        /* Revert 10b to 8b and accumulate! */
        process_10b(word, d_word_cnt);
        d_word_cnt++;

        if (d_word_cnt == max_frame_len()) {
          d_state = SEARCH_SYNC;
          d_data_reg = 0;

          if (d_erasure_cnt > 0) {
            ret = decode_rs_8(d_8b_words, d_erasures_indexes, d_erasure_cnt,
                              255 - max_frame_len());
          }
          else {
            ret = decode_rs_8(d_8b_words, NULL, 0, 255 - max_frame_len());
          }
          if (ret > -1) {
            uint8_t fox_id = d_8b_words[0] & 0x7;
            if (is_spacecraft_valid(fox_id)) {
              metadata::add_decoder(status.data, this);
              metadata::add_pdu(status.data, d_8b_words,
                                amsat_fox_duv_frame_size);
              metadata::add_symbol_erasures(status.data, d_erasure_cnt);
              metadata::add_corrected_bits(status.data, ret);
              metadata::add_time_iso8601(status.data);
              status.decode_success = true;
              d_bitstream.erase(d_bitstream.begin(),
                                d_bitstream.begin() + (i + 1) * 10 + 1);
              return status;
            }
          }
          /* Frame could not be decoded. Retry to sync */
          cont = true;
          break;
        }
      }

      if (cont) {
        continue;
      }
      return status;
    default:
      throw std::invalid_argument(
        "amsat_duv_decoder: Invalid decoding state");
    }
  }
}

void
amsat_duv_decoder::reset()
{
  d_erasure_cnt = 0;
  d_word_cnt = 0;
  d_state = SEARCH_SYNC;
  d_bitstream.clear();
  d_bitstream_idx = 0;
}

} /* namespace satnogs */
} /* namespace gr */

