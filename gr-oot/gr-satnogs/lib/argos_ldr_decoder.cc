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
#include <satnogs/argos_ldr_decoder.h>
#include <satnogs/ax25.h>
#include <satnogs/metadata.h>

namespace gr {
namespace satnogs {

decoder::decoder_sptr
argos_ldr_decoder::make(bool crc_check, size_t max_frame_len)
{
  return decoder::decoder_sptr(
           new argos_ldr_decoder(crc_check, max_frame_len));
}

argos_ldr_decoder::argos_ldr_decoder(bool crc_check, size_t max_frame_len) :
  decoder("argos_ldr", "1.0", sizeof(uint8_t), 2 * max_frame_len * 8),
  d_crc_check(crc_check),
  d_max_frame_len(max_frame_len),
  d_state(NO_SYNC),
  d_shift_reg(0x0),
  d_dec_b(0x0),
  d_received_bytes(0),
  d_decoded_bits(0),
  d_frame_buffer(
    new uint8_t[max_frame_len + sizeof(uint16_t)]),
  d_start_idx(0),
  d_frame_start(0),
  d_sample_cnt(0)
{
}

decoder_status_t
argos_ldr_decoder::decode(const void *in, int len)
{
  const uint8_t *input = (const uint8_t *) in;
  decoder_status_t status;

  for (int i = 0; i < len; i++) {
    d_bitstream.push_back(input[i]);
  }
  /*
   * The decoder always consumes all the available input bits and stores them
   * internally
   */
  _decode(status);
  status.consumed = len;
  return status;
}

void
argos_ldr_decoder::reset()
{
  reset_state();
}

bool
argos_ldr_decoder::_decode(decoder_status_t &status)
{
  while (1) {
    bool cont = false;
    switch (d_state) {
    case NO_SYNC:
      for (size_t i = 0; i < d_bitstream.size(); i++) {
        decode_1b(d_bitstream[i]);
        /*
         * Decrease false positives by waiting for three occurrences of the
         * AX.25 flag
         */
        const uint32_t test = ax25::sync_flag
                              | (ax25::sync_flag << 8)
                              | (ax25::sync_flag << 16);
        if (test == d_shift_reg) {
          d_bitstream.erase(d_bitstream.begin(),
                            d_bitstream.begin() + i + 1);
          /* Increment the number of items read so far */
          incr_nitems_read(i + 1);
          enter_sync_state();
          /* Mark possible start of the frame */
          d_frame_start = nitems_read();
          d_start_idx = 0;
          cont = true;
          break;
        }
      }
      if (cont) {
        continue;
      }
      incr_nitems_read(d_bitstream.size());
      d_bitstream.clear();
      return false;
    case IN_SYNC:
      /*
       * HDLC flag in most case is repeated multiple times
       */
      for (size_t i = d_start_idx; i < d_bitstream.size(); i++) {
        decode_1b(d_bitstream[i]);
        d_decoded_bits++;
        if (d_decoded_bits == 8) {
          /* Perhaps we are in frame! */
          if ((d_shift_reg & 0xFF) != ax25::sync_flag) {
            d_start_idx = i + 1;
            enter_decoding_state();
            cont = true;
            break;
          }
          d_decoded_bits = 0;
        }
      }
      if (cont) {
        continue;
      }
      d_start_idx = d_bitstream.size();
      return false;
    case DECODING:
      for (size_t i = d_start_idx; i < d_bitstream.size(); i++) {
        decode_1b(d_bitstream[i]);
        if ((d_shift_reg & 0xFF) == ax25::sync_flag) {
          d_sample_cnt = nitems_read() + i - d_frame_start;
          LOG_DEBUG("Found frame end");
          if (enter_frame_end(status)) {
            d_bitstream.erase(d_bitstream.begin(),
                              d_bitstream.begin() + i + 1);
            /* Increment the number of items read so far */
            incr_nitems_read(i + 1);
            d_start_idx = d_bitstream.size();
            return true;
          }
          cont = true;
          break;
        }
        else if (((d_shift_reg & 0xFF) & 0xfc) == 0x7c) {
          /*This was a stuffed bit */
          d_dec_b >>= 1;
        }
        else if (((d_shift_reg & 0xFF) & 0xfe) == 0xfe) {
          LOG_DEBUG("Invalid shift register value %u", d_received_bytes);
          reset_state();
          cont = true;
          break;
        }
        else {
          d_decoded_bits++;
          if (d_decoded_bits == 8) {
            d_frame_buffer[d_received_bytes++] = d_dec_b;
            d_decoded_bits = 0;

            /*Check if the frame limit was reached */
            if (d_received_bytes >= d_max_frame_len) {
              LOG_DEBUG("Wrong size");
              reset_state();
              cont = true;
              break;
            }
          }
        }
      }
      if (cont) {
        continue;
      }
      d_start_idx = d_bitstream.size();
      return false;
    default:
      LOG_ERROR("Invalid decoding state");
      reset_state();
      return false;
    }
  }
}


argos_ldr_decoder::~argos_ldr_decoder()
{
  delete[] d_frame_buffer;
}

void
argos_ldr_decoder::reset_state()
{
  d_state = NO_SYNC;
  d_dec_b = 0x0;
  d_shift_reg = 0x0;
  d_decoded_bits = 0;
  d_received_bytes = 0;
}

void
argos_ldr_decoder::enter_sync_state()
{
  d_state = IN_SYNC;
  d_dec_b = 0x0;
  d_shift_reg = 0x0;
  d_decoded_bits = 0;
  d_received_bytes = 0;
}

void
argos_ldr_decoder::enter_decoding_state()
{
  d_state = DECODING;
  d_decoded_bits = 0;
  d_received_bytes = 0;

  /*
   * Due to the possibility of bit stuffing on the first byte some special
   * handling is necessary
   */
  if (((d_shift_reg >> 16) & 0xfc) == 0x7c) {
    /*This was a stuffed bit */
    d_dec_b >>= 1;
    d_decoded_bits = 7;
  }
  else {
    d_frame_buffer[0] = d_dec_b;
    d_decoded_bits = 0;
    d_received_bytes = 1;
  }
}

bool
argos_ldr_decoder::enter_frame_end(decoder_status_t &status)
{
  /* First check if the size of the frame is valid */
  if (d_received_bytes < 1 + sizeof(uint16_t)) {
    reset_state();
    return false;
  }

  /*
   * Check if the frame is correct using the FCS field
   */
  if (frame_check()) {
    metadata::add_decoder(status.data, this);
    metadata::add_pdu(status.data, d_frame_buffer,
                      d_received_bytes - sizeof(uint16_t));
    metadata::add_time_iso8601(status.data);
    metadata::add_crc_valid(status.data, true);
    metadata::add_sample_start(status.data, d_frame_start);
    metadata::add_sample_cnt(status.data, d_sample_cnt);
    status.decode_success = true;
    reset_state();
    return true;
  }
  else if (!d_crc_check) {
    metadata::add_decoder(status.data, this);
    metadata::add_pdu(status.data, d_frame_buffer,
                      d_received_bytes - sizeof(uint16_t));
    metadata::add_time_iso8601(status.data);
    metadata::add_crc_valid(status.data, false);
    status.decode_success = true;
    LOG_DEBUG("Wrong crc");
    reset_state();
    return false;
  }
  return false;
}


inline void
argos_ldr_decoder::decode_1b(uint8_t in)
{
  /*
   * Unlike the standard AX.25, ARGOS uses HDLC but the bits are sent MS first.
   * Here we use the d_shift_reg as a 24-bit shift register
   */
  d_shift_reg = (d_shift_reg << 1) | in;
  d_shift_reg &= 0xFFFFFF;
  d_dec_b = (d_dec_b << 1) | in;
}

bool
argos_ldr_decoder::frame_check()
{
  uint16_t fcs;
  uint16_t recv_fcs = 0x0;

  /* CRC16-CCITT fails miserably if all bytes are zero */
  bool all_zeros = true;
  for (size_t i = 0; i < d_received_bytes; i++) {
    if (d_frame_buffer[i] != 0x0) {
      all_zeros = false;
      break;
    }
  }
  if (all_zeros) {
    return false;
  }

  /* Check if the frame is correct using the FCS field */
  fcs = crc::crc16_ccitt(d_frame_buffer, d_received_bytes - sizeof(uint16_t));
  recv_fcs = (((uint16_t) d_frame_buffer[d_received_bytes - 2]) << 8)
             | d_frame_buffer[d_received_bytes - 1];
  if (fcs == recv_fcs) {
    return true;
  }
  return false;
}

} /* namespace satnogs */
} /* namespace gr */
