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

#include "ax100_mode5.h"

#include <gnuradio/io_signature.h>
#include <satnogs/golay24.h>
#include <satnogs/metadata.h>
#include <satnogs/utils.h>
#include <satnogs/log.h>
#include <satnogs/libfec/fec.h>

namespace gr {
namespace satnogs {

decoder::decoder_sptr
ax100_mode5::make(const std::vector<uint8_t> &preamble,
                  size_t preamble_threshold,
                  const std::vector<uint8_t> &sync, size_t sync_threshold,
                  crc::crc_t crc, whitening::whitening_sptr descrambler,
                  bool enable_rs)
{
  return decoder::decoder_sptr(
           new ax100_mode5(preamble, preamble_threshold, sync, sync_threshold,
                           crc, descrambler, enable_rs));
}

ax100_mode5::ax100_mode5(const std::vector<uint8_t> &preamble,
                         size_t preamble_threshold,
                         const std::vector<uint8_t> &sync,
                         size_t sync_threshold,
                         crc::crc_t crc,
                         whitening::whitening_sptr descrambler,
                         bool enable_rs) :
  decoder("ax100_mode5", "1.0", sizeof(uint8_t), enable_rs ? 255 : 1024),
  d_preamble(preamble.size() * 8),
  d_preamble_shift_reg(preamble.size() * 8),
  d_preamble_len(preamble.size() * 8),
  d_preamble_thrsh(preamble_threshold),
  d_sync(sync.size() * 8),
  d_sync_shift_reg(sync.size() * 8),
  d_sync_len(sync.size() * 8),
  d_sync_thrsh(sync_threshold),
  d_crc(crc),
  d_descrambler(descrambler),
  d_rs(enable_rs),
  d_state(SEARCHING),
  d_cnt(0),
  d_len(0),
  d_frame_start(0),
  /* Coded Golay 24 bits */
  d_length_field_len(3),
  d_pdu(new uint8_t[1024])
{
  for (uint8_t b : preamble) {
    d_preamble <<= (b >> 7);
    d_preamble <<= ((b >> 6) & 0x1);
    d_preamble <<= ((b >> 5) & 0x1);
    d_preamble <<= ((b >> 4) & 0x1);
    d_preamble <<= ((b >> 3) & 0x1);
    d_preamble <<= ((b >> 2) & 0x1);
    d_preamble <<= ((b >> 1) & 0x1);
    d_preamble <<= (b & 0x1);
  }
  for (uint8_t b : sync) {
    d_sync <<= (b >> 7);
    d_sync <<= ((b >> 6) & 0x1);
    d_sync <<= ((b >> 5) & 0x1);
    d_sync <<= ((b >> 4) & 0x1);
    d_sync <<= ((b >> 3) & 0x1);
    d_sync <<= ((b >> 2) & 0x1);
    d_sync <<= ((b >> 1) & 0x1);
    d_sync <<= (b & 0x1);
  }

  if (d_sync_len < 8) {
    throw std::invalid_argument("SYNC word should be at least 8 bits");
  }

  if (d_preamble_len < 2 * d_preamble_thrsh) {
    throw std::invalid_argument(
      "Too many error bits are allowed for the preamble."
      "Consider lowering the threshold");
  }

  if (d_sync_len < 2 * d_sync_thrsh) {
    throw std::invalid_argument(
      "Too many error bits are allowed for the sync word. "
      "Consider lowering the threshold");
  }
}

ax100_mode5::~ax100_mode5()
{
  delete[] d_pdu;
}

decoder_status_t
ax100_mode5::decode(const void *in, int len)
{
  decoder_status_t status;
  switch (d_state) {
  case SEARCHING:
    status.consumed = search_preamble((const uint8_t *) in, len);
    break;
  case SEARCHING_SYNC:
    status.consumed = search_sync((const uint8_t *) in, len);
    break;
  case DECODING_FRAME_LEN:
    status.consumed = decode_frame_len((const uint8_t *) in, len);
    break;
  case DECODING_PAYLOAD:
    decode_payload(status, (const uint8_t *) in, len);
    break;
  default:
    throw std::runtime_error("ax100_decoder: Invalid decoding state");
  }
  incr_nitems_read(static_cast<uint64_t>(status.consumed));
  return status;
}

void
ax100_mode5::reset()
{
  if (d_descrambler) {
    d_descrambler->reset();
  }
  d_cnt = 0;
  d_state = SEARCHING;
  d_preamble_shift_reg.reset();
  d_sync_shift_reg.reset();
  LOG_DEBUG("RESET");
}

size_t
ax100_mode5::input_multiple() const
{
  return 8;
}

int
ax100_mode5::search_preamble(const uint8_t *in, int len)
{
  /* We support also cases, where a repeated preamble is not used (pure CCSDS )*/
  if (d_preamble_len == 0) {
    d_state = SEARCHING_SYNC;
    d_frame_start = nitems_read();
    return search_sync(in, len);
  }

  for (int i = 0; i < len; i++) {
    d_preamble_shift_reg <<= in[i];
    shift_reg tmp = d_preamble_shift_reg ^ d_preamble;
    if (tmp.count() <= d_preamble_thrsh) {
      d_state = SEARCHING_SYNC;
      d_cnt = 0;
      d_frame_start = nitems_read() + i + 1;
      return i + 1;
    }
  }
  return len;
}

int
ax100_mode5::search_sync(const uint8_t *in, int len)
{
  for (int i = 0; i < len; i++) {
    d_sync_shift_reg <<= in[i];
    shift_reg tmp = d_sync_shift_reg ^ d_sync;
    d_cnt++;
    if (tmp.count() <= d_sync_thrsh) {
      LOG_DEBUG("DECODING_FRAME_LEN");
      d_state = DECODING_FRAME_LEN;
      d_cnt = 0;
      return i + 1;
    }

    /* The sync word should be available by now */
    if (d_cnt > d_preamble_len * 2 + d_sync_len) {
      reset();
      return i + 1;
    }
  }
  return len;
}

int
ax100_mode5::decode_frame_len(const uint8_t *in, int len)
{
  const int s = len / 8;
  for (int i = 0; i < s; i++) {
    uint8_t b = 0x0;
    b = in[i * 8] << 7;
    b |= in[i * 8 + 1] << 6;
    b |= in[i * 8 + 2] << 5;
    b |= in[i * 8 + 3] << 4;
    b |= in[i * 8 + 4] << 3;
    b |= in[i * 8 + 5] << 2;
    b |= in[i * 8 + 6] << 1;
    b |= in[i * 8 + 7];
    d_pdu[d_cnt++] = b;

    if (d_cnt == d_length_field_len) {
      uint32_t coded_len = (d_pdu[0] << 16) | (d_pdu[1] << 8) | d_pdu[2];
      uint32_t len;
      golay24 g = golay24();
      if (g.decode24(&len, coded_len)) {
        d_len = len & 0xFF;
        LOG_DEBUG("FRAME LEN: %zu", d_len);
        if (d_len > max_frame_len()) {
          reset();
          return (i + 1) * 8;
        }
        /* All good! Proceed with decoding */
        d_cnt = 0;
        d_state = DECODING_PAYLOAD;
        memset(d_pdu, 0, max_frame_len());
        return (i + 1) * 8;
      }
      else {
        reset();
        return (i + 1) * 8;
      }
    }
  }
  return s * 8;
}

void
ax100_mode5::decode_payload(decoder_status_t &status,
                            const uint8_t *in, int len)
{
  const int s = len / 8;
  for (int i = 0; i < s; i++) {
    uint8_t b = 0x0;
    b = in[i * 8] << 7;
    b |= in[i * 8 + 1] << 6;
    b |= in[i * 8 + 2] << 5;
    b |= in[i * 8 + 3] << 4;
    b |= in[i * 8 + 4] << 3;
    b |= in[i * 8 + 5] << 2;
    b |= in[i * 8 + 6] << 1;
    b |= in[i * 8 + 7];
    d_pdu[d_cnt++] = b;

    if (d_cnt == d_len) {
      if (d_descrambler) {
        d_descrambler->descramble(d_pdu, d_pdu, d_len);
      }

      /* If RS is used try to decode the received frame */
      if (d_rs) {
        int ret = decode_rs_8(d_pdu, NULL, 0, 255 - d_len);

        /* Drop the parity */
        d_len -= 32;
        if (ret > - 1) {
          metadata::add_corrected_bits(status.data, ret);
        }
        else {
          reset();
          status.consumed = (i + 1) * 8;
          return;
        }
      }

      metadata::add_decoder(status.data, this);
      metadata::add_time_iso8601(status.data);
      metadata::add_sample_start(status.data, d_frame_start);
      metadata::add_sample_cnt(status.data,
                               nitems_read() + (i + 1) * 8 - d_frame_start);
      metadata::add_pdu(status.data, d_pdu, d_len - crc::crc_size(d_crc));
      metadata::add_crc_valid(status.data, check_crc());
      reset();
      status.decode_success = true;
      status.consumed = (i + 1) * 8;
      return;
    }
  }
  status.consumed = s * 8;
}

bool
ax100_mode5::check_crc()
{
  uint32_t crc32_c;
  uint32_t crc32_received;
  switch (d_crc) {
  case crc::CRC_NONE:
    return true;
  case crc::CRC32_C:
    crc32_c = crc::crc32_c(d_pdu, d_len - 4);
    memcpy(&crc32_received, d_pdu + d_len - 4, 4);
    crc32_received = ntohl(crc32_received);
    LOG_DEBUG("Received: 0x%04x Computed: 0x%04x", crc32_received, crc32_c);
    if (crc32_c == crc32_received) {
      return true;
    }
    return false;
  default:
    throw std::runtime_error("ax100_decoder: Invalid CRC");
  }
}

} /* namespace satnogs */
} /* namespace gr */
