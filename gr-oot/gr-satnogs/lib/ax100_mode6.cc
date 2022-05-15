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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ax100_mode6.h"

#include <gnuradio/io_signature.h>
#include <satnogs/ax25.h>
#include <satnogs/utils.h>
#include <satnogs/libfec/fec.h>
#include <satnogs/metadata.h>

namespace gr {
namespace satnogs {

decoder::decoder_sptr
ax100_mode6::make(crc::crc_t crc, whitening::whitening_sptr descrambler,
                  bool ax25_descramble)
{
  return decoder::decoder_sptr(new ax100_mode6(crc, descrambler,
                               ax25_descramble));
}

ax100_mode6::ax100_mode6(crc::crc_t crc, whitening::whitening_sptr descrambler,
                         bool ax25_descramble) :
  decoder("ax100_mode6", "1.0", sizeof(uint8_t), 255 * 8),
  d_crc(crc),
  d_ax25_descramble(ax25_descramble),
  d_max_frame_len(255),
  d_descrambler(descrambler),
  d_state(NO_SYNC),
  d_shift_reg(0x0),
  d_dec_b(0x0),
  d_prev_bit_nrzi(0),
  d_received_bytes(0),
  d_decoded_bits(0),
  d_lfsr(0x21, 0x0, 16),
  d_frame_buffer(
    new uint8_t[d_max_frame_len + ax25::max_header_len + sizeof(uint16_t)]),
  d_start_idx(0),
  d_frame_start(0),
  d_sample_cnt(0)
{
}

decoder_status_t
ax100_mode6::decode(const void *in, int len)
{
  const uint8_t *input = (const uint8_t *) in;
  decoder_status_t status;
  if (d_ax25_descramble) {
    for (int i = 0; i < len; i++) {
      /* Perform NRZI decoding */
      uint8_t b = (~((input[i] - d_prev_bit_nrzi) % 2)) & 0x1;
      d_prev_bit_nrzi = input[i];
      b = d_lfsr.next_bit_descramble(b);
      d_bitstream.push_back(b);
    }
  }
  else {
    for (int i = 0; i < len; i++) {
      /* Perform NRZI decoding */
      uint8_t b = (~((input[i] - d_prev_bit_nrzi) % 2)) & 0x1;
      d_prev_bit_nrzi = input[i];
      d_bitstream.push_back(b);
    }
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
ax100_mode6::reset()
{
  reset_state();
}

bool
ax100_mode6::_decode(decoder_status_t &status)
{
  while (1) {
    bool cont = false;
    switch (d_state) {
    case NO_SYNC:
      for (size_t i = 0; i < d_bitstream.size(); i++) {
        decode_1b(d_bitstream[i]);
        /*
         * In case of scrambling the self synchronizing scrambler ensures
         * that enough repetitions of the AX.25 flag have been received.
         * However, this does not hold for the case of non scrambled
         * transmissions. In this case, we wait for at least three consecutive
         * AX.25 flags to reduce the false alarms. Experiments have shown
         * that due to the poor CRC there were many false positive frames.
         *
         * It is expected however, to miss some transmissions that use only one
         * AX.25 flag. We believe that such transmissions are yet rare.
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
       * Most of the transmitters repeat several times the AX.25 SYNC
       * In case of G3RUH this is mandatory to allow the self synchronizing
       * scrambler to settle
       */
      for (size_t i = d_start_idx; i < d_bitstream.size(); i++) {
        decode_1b(d_bitstream[i]);
        d_decoded_bits++;
        if (d_decoded_bits == 8) {
          /* Perhaps we are in frame! */
          if ((d_shift_reg >> 16) != ax25::sync_flag) {
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
        if ((d_shift_reg >> 16) == ax25::sync_flag) {
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
        else if (((d_shift_reg >> 16) & 0xfc) == 0x7c) {
          /*This was a stuffed bit */
          d_dec_b <<= 1;
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


ax100_mode6::~ax100_mode6()
{
  delete[] d_frame_buffer;
}

void
ax100_mode6::reset_state()
{
  if (d_descrambler) {
    d_descrambler->reset();
  }
  d_state = NO_SYNC;
  d_dec_b = 0x0;
  d_shift_reg = 0x0;
  d_decoded_bits = 0;
  d_received_bytes = 0;
}

void
ax100_mode6::enter_sync_state()
{
  d_state = IN_SYNC;
  d_dec_b = 0x0;
  d_shift_reg = 0x0;
  d_decoded_bits = 0;
  d_received_bytes = 0;
}

void
ax100_mode6::enter_decoding_state()
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
    d_dec_b <<= 1;
    d_decoded_bits = 7;
  }
  else {
    d_frame_buffer[0] = d_dec_b;
    d_decoded_bits = 0;
    d_received_bytes = 1;
  }
}

bool
ax100_mode6::enter_frame_end(decoder_status_t &status)
{
  /*
   * First check if the size of the frame is valid
   * The minimum frame is the sizeof the AX.25 header (14 bytes),
   * the AX.25 CRC and a minimum 32 byte long field for the RS
   */
  if (d_received_bytes < ax25::min_addr_len + 2 + crc::crc_size(
        crc::CRC16_AX25) + 32) {
    reset_state();
    return false;
  }

  /* If RS is enabled and the frame is larger than the RS block mark as faulty*/
  if (d_received_bytes > 255 + 2 + ax25::min_addr_len
      + crc::crc_size(crc::CRC16_AX25)) {
    reset_state();
    return false;
  }

  /* The actual payload is the AX.25 frame size minus the AX.25 overhead
   * which is a 14 byte address field, 1 byte control field, 1 byte PID field
   * and the 2 byte CRC
   */
  size_t payload_len = d_received_bytes
                       - (2 + ax25::min_addr_len + crc::crc_size(crc::CRC16_AX25));
  /* Skip the AX.25 header, not part of the RS block */
  uint8_t *payload = d_frame_buffer + 2 + ax25::min_addr_len;
  LOG_DEBUG("LEN: %u", payload_len);
  if (d_descrambler) {
    d_descrambler->descramble(payload, payload, payload_len);
  }
  int ret = decode_rs_8(payload, NULL, 0, 255 - payload_len);
  /* Discard RS parity*/
  payload_len -= 32;

  if (ret < 0) {
    LOG_DEBUG("RS failed");
    reset_state();
    return false;
  }


  switch (d_crc) {
  case crc::CRC_NONE:
    metadata::add_decoder(status.data, this);
    /*
     * Add both the AX.25 header and the RS decoded block
     * NOTE: The integrity of the AX.25 header is NOT guaranteed.
     * We do not want to loose frames due to the no-FEC part of the header.
     */
    metadata::add_pdu(status.data, d_frame_buffer,
                      2 + ax25::min_addr_len + payload_len);
    metadata::add_time_iso8601(status.data);
    metadata::add_crc_valid(status.data, false);
    metadata::add_sample_start(status.data, d_frame_start);
    metadata::add_sample_cnt(status.data, d_sample_cnt);
    metadata::add_corrected_bits(status.data, ret);
    status.decode_success = true;
    reset_state();
    return true;
  case crc::CRC32_C: {
    uint32_t crc32_c;
    uint32_t crc32_received;
    crc32_c = crc::crc32_c(payload, payload_len - 4);
    memcpy(&crc32_received, payload + payload_len - 4, 4);
    crc32_received = ntohl(crc32_received);
    LOG_DEBUG("Received: 0x%04x Computed: 0x%04x", crc32_received, crc32_c);
    if (crc32_c == crc32_received) {
      metadata::add_decoder(status.data, this);
      /*
       * Add both the AX.25 header and the RS decoded block
       * NOTE: The integrity of the AX.25 header is NOT guaranteed.
       * We do not want to loose frames due to the no-FEC part of the header.
       */
      metadata::add_pdu(status.data, d_frame_buffer,
                        2 + ax25::min_addr_len + payload_len - 4);
      metadata::add_time_iso8601(status.data);
      metadata::add_crc_valid(status.data, true);
      metadata::add_sample_start(status.data, d_frame_start);
      metadata::add_sample_cnt(status.data, d_sample_cnt);
      metadata::add_corrected_bits(status.data, ret);
      status.decode_success = true;
      reset_state();
      return true;
    }
    return false;
  }
  default:
    throw std::runtime_error("ax100_decoder: Invalid CRC");
  }
  return false;
}


inline void
ax100_mode6::decode_1b(uint8_t in)
{
  /* In AX.25 the LS bit is sent first */
  d_shift_reg = (d_shift_reg >> 1) | (in << 23);
  d_dec_b = (d_dec_b >> 1) | (in << 7);
}


} /* namespace satnogs */
} /* namespace gr */
