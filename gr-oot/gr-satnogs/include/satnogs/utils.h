/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2016,2018,2019 Libre Space Foundation <http://libre.space>
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

#ifndef INCLUDE_SATNOGS_UTILS_H_
#define INCLUDE_SATNOGS_UTILS_H_

#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <arpa/inet.h>
#include <itpp/itbase.h>

template<class T, class U, bool S>
class utils_helper {
public:
  static size_t
  packed_to_unpacked(T out, const U in, size_t len)
  {
    return 0;
  }

  static size_t
  unpacked_to_packed(U out, const T in, size_t len)
  {
    return 0;
  }
};

template<>
class utils_helper<uint8_t *, uint8_t *, false> {
public:
  static size_t
  packed_to_unpacked(uint8_t *out, const uint8_t *in, size_t len)
  {
    if (!out || !in || !len) {
      return 0;
    }

    for (size_t i = 0; i < len; i++) {
      *out++ = (in[i] >> 7) & 0x1;
      *out++ = (in[i] >> 6) & 0x1;
      *out++ = (in[i] >> 5) & 0x1;
      *out++ = (in[i] >> 4) & 0x1;
      *out++ = (in[i] >> 3) & 0x1;
      *out++ = (in[i] >> 2) & 0x1;
      *out++ = (in[i] >> 1) & 0x1;
      *out++ = in[i] & 0x1;
    }

    return len * 8;
  }

  static size_t
  unpacked_to_packed(uint8_t *out, const uint8_t *in, size_t len)
  {
    if (!out || !in || !len) {
      return 0;
    }

    struct h {
      uint8_t shift: 3;
    } x = {.shift = 7};

    memset(out, 0, static_cast<size_t>(std::ceil(len / 8.0)));
    for (size_t i = 0; i < len; i++) {
      out[i / 8] |= ((in[i] & 0x1) << x.shift--);
    }

    return static_cast<size_t>(std::ceil(len / 8.0));
  }
};

template<>
class utils_helper<int8_t *, uint8_t *, true> {
public:
  static size_t
  packed_to_unpacked(int8_t *out, const uint8_t *in, size_t len)
  {
    if (!out || !in || !len) {
      return 0;
    }

    for (size_t i = 0; i < len; ++i) {
      *out++ = (((in[i] >> 7) & 0x1) * 255) - 128;
      *out++ = (((in[i] >> 6) & 0x1) * 255) - 128;
      *out++ = (((in[i] >> 5) & 0x1) * 255) - 128;
      *out++ = (((in[i] >> 4) & 0x1) * 255) - 128;
      *out++ = (((in[i] >> 3) & 0x1) * 255) - 128;
      *out++ = (((in[i] >> 2) & 0x1) * 255) - 128;
      *out++ = (((in[i] >> 1) & 0x1) * 255) - 128;
      *out++ = ((in[i] & 0x1) * 255) - 128;
    }

    return len * 8;
  }

  static size_t
  unpacked_to_packed(uint8_t *out, const int8_t *in, size_t len)
  {
    if (!out || !in || !len) {
      return 0;
    }

    struct h {
      uint8_t shift: 3;
    } x = {.shift = 7};

    memset(out, 0, static_cast<size_t>(std::ceil(len / 8.0)));
    for (size_t i = 0; i < len; i++) {
      uint8_t bit = in[i] > 0 ? 1 : 0;
      out[i / 8] |= (bit << x.shift--);
    }

    return static_cast<size_t>(std::ceil(len / 8.0));
  }
};

template<>
class utils_helper<itpp::Vec<uint8_t>&, itpp::bvec &, false> {
public:
  static size_t
  packed_to_unpacked(itpp::Vec<uint8_t> &out, const itpp::bvec &in,
                     size_t len = 0)
  {
    if (!out.size() || !in.size()) {
      return 0;
    }

    size_t j = 0;
    for (size_t i = 0; i < in.size(); i++) {
      out[j++] = (in[i] >> 7) & 0x1;
      out[j++] = (in[i] >> 6) & 0x1;
      out[j++] = (in[i] >> 5) & 0x1;
      out[j++] = (in[i] >> 4) & 0x1;
      out[j++] = (in[i] >> 3) & 0x1;
      out[j++] = (in[i] >> 2) & 0x1;
      out[j++] = (in[i] >> 1) & 0x1;
      out[j++] = static_cast<bool>(in[i] & itpp::bin(0x1));
    }

    return j;
  }

  static size_t
  unpacked_to_packed(itpp::bvec &out, const itpp::Vec<uint8_t> &in,
                     size_t len = 0)
  {
    if (!out.size() || !in.size()) {
      return 0;
    }

    struct h {
      uint8_t shift: 3;
    } x = {.shift = 7};

    for (size_t i = 0; i < static_cast<size_t>(std::ceil(in.size() / 8.0)); i++) {
      out[i] = 0;
    }
    for (size_t i = 0; i < in.size(); i++) {
      ;
      out[i / 8] |= ((in[i] & 0x1) << x.shift--);
    }

    return static_cast<size_t>(std::ceil(in.size() / 8.0));
  }
};

template<>
class utils_helper<itpp::Vec<int8_t>&, itpp::bvec &, true> {
public:
  static size_t
  packed_to_unpacked(itpp::Vec<int8_t> &out, const itpp::bvec &in, size_t len = 0)
  {
    if (!out.size() || !in.size()) {
      return 0;
    }

    size_t j = 0;
    for (size_t i = 0; i < in.size(); ++i) {
      out[j++] = (((in[i] >> 7) & 0x1) * 255) - 128;
      out[j++] = (((in[i] >> 6) & 0x1) * 255) - 128;
      out[j++] = (((in[i] >> 5) & 0x1) * 255) - 128;
      out[j++] = (((in[i] >> 4) & 0x1) * 255) - 128;
      out[j++] = (((in[i] >> 3) & 0x1) * 255) - 128;
      out[j++] = (((in[i] >> 2) & 0x1) * 255) - 128;
      out[j++] = (((in[i] >> 1) & 0x1) * 255) - 128;
      out[j++] = (static_cast<bool>(in[i] & itpp::bin(0x1)) * 255) - 128;
    }

    return j;
  }

  static size_t
  unpacked_to_packed(itpp::bvec &out, const itpp::Vec<int8_t> &in, size_t len = 0)
  {
    if (!out.size() || !in.size()) {
      return 0;
    }

    struct h {
      uint8_t shift: 3;
    } x = {.shift = 7};

    for (size_t i = 0; i < static_cast<size_t>(std::ceil(in.size() / 8.0)); i++) {
      out[i] = 0;
    }
    for (size_t i = 0; i < in.size(); i++) {
      uint8_t bit = in[i] > 0 ? 1 : 0;
      out[i / 8] |= (bit << x.shift--);
    }

    return static_cast<size_t>(std::ceil(in.size() / 8.0));
  }
};

namespace gr {

namespace satnogs {

/**
 * @brief Several bit-level utility methods, frequently used in the
 * encoding/decoding process
 *
 * @ingroup satnogs
 */
class utils {
public:
  static double
  mape(double ref, double estimation);

  static uint64_t
  htonll(uint64_t x);

  static uint64_t
  ntohll(uint64_t x);

  static uint32_t
  bit_count(uint32_t x);

  static uint8_t
  reverse_byte(uint8_t b);

  static uint32_t
  reverse_uint32_bytes(uint32_t i);

  static uint64_t
  reverse_uint64_bytes(uint64_t x);

  static void
  print_pdu(const uint8_t *buf, size_t len);

  /**
   * Template function that unpack the bits of the list-like object with numbers in
   * @param out the list with the unpacked bits
   * @param in the list with the packed bits
   * @param len the size of the in
   * @return the size of the out
   */
  template<class T, class U, bool S>
  static size_t
  packed_to_unpacked(T out, const U in, size_t len = 0)
  {
    return utils_helper<T, U, S>::packed_to_unpacked(out, in, len);
  }

  /**
   * Template function that pack the bits of the list-like object with numbers in
   * @param out the list with the packed bits
   * @param in the list with the unpacked bits
   * @param len the size of the in
   * @return the size of the out
   */
  template<class T, class U, bool S>
  static size_t
  unpacked_to_packed(T out, const U in, size_t len = 0)
  {
    return utils_helper<U, T, S>::unpacked_to_packed(out, in, len);
  }

private:
  static constexpr uint8_t s_bytes_reversed[256] = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0,
    0x30, 0xB0, 0x70, 0xF0, 0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68,
    0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 0x04, 0x84,
    0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34,
    0xB4, 0x74, 0xF4, 0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
    0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 0x02, 0x82, 0x42,
    0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2,
    0x72, 0xF2, 0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A,
    0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA, 0x06, 0x86, 0x46, 0xC6,
    0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76,
    0xF6, 0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E,
    0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 0x01, 0x81, 0x41, 0xC1, 0x21,
    0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59,
    0xD9, 0x39, 0xB9, 0x79, 0xF9, 0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5,
    0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5, 0x0D,
    0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD,
    0x3D, 0xBD, 0x7D, 0xFD, 0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63,
    0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 0x0B, 0x8B,
    0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B,
    0xBB, 0x7B, 0xFB, 0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
    0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 0x0F, 0x8F, 0x4F,
    0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF,
    0x7F, 0xFF
  };
};

}  // namespace satnogs

}  // namespace gr

#endif /* INCLUDE_SATNOGS_UTILS_H_ */
