/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2016,2017,2019,2020 Libre Space Foundation <http://libre.space>
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
#include "multi_format_msg_sink_impl.h"
#include <satnogs/base64.h>
#include <satnogs/metadata.h>
#include <ctime>
#include <iostream>
#include <iomanip>

namespace gr {
namespace satnogs {

multi_format_msg_sink::sptr
multi_format_msg_sink::make(size_t format,
                            bool timestamp,
                            bool out_stdout,
                            const std::string &filepath)
{
  return gnuradio::get_initial_sptr(
           new multi_format_msg_sink_impl(format, timestamp,
                                          out_stdout, filepath));
}

void
multi_format_msg_sink_impl::msg_handler_file(pmt::pmt_t msg)
{
  const char *su;
  size_t len;
  std::string s;
  char buf[256];

  /* Check if the message contains the legacy or the new format */
  if (pmt::is_dict(msg)) {
    pmt::pmt_t pdu = pmt::dict_ref(msg, pmt::mp(metadata::value(metadata::PDU)),
                                   pmt::PMT_NIL);
    su = (const char *) pmt::blob_data(pdu);
    s = std::string((const char *) pmt::blob_data(pdu), pmt::blob_length(pdu));
    len = pmt::blob_length(pdu);
  }
  else {
    su = (const char *) pmt::blob_data(msg);
    s = std::string((const char *) pmt::blob_data(msg), pmt::blob_length(msg));
    len = pmt::blob_length(msg);
  }

  if (d_timestamp) {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    strftime(buf, sizeof(buf), "%F %T %z", &tm);
    d_fos << "[" << buf << "] ";
  }

  switch (d_format) {
  case 0:
    d_fos << s << std::endl;
    break;
  case 1:
    for (size_t i = 0; i < len; i++) {
      d_fos << "0x" << std::hex << std::setw(2) << std::setfill('0')
            << (uint32_t(su[i]) & 0xFF) << " ";
    }
    d_fos << std::endl;
    break;
  case 2:
    for (size_t i = 0; i < len; i++) {
      d_fos << "0b" << std::bitset<8> (su[i]) << " ";
    }
    d_fos << std::endl;
    break;
  default:
    throw std::invalid_argument("Invalid format");
  }
}

void
multi_format_msg_sink_impl::msg_handler_stdout(pmt::pmt_t msg)
{
  const char *su;
  size_t len;
  std::string s;
  char buf[256];

  /* Check if the message contains the legacy or the new format */
  if (pmt::is_dict(msg)) {
    pmt::pmt_t pdu = pmt::dict_ref(msg, pmt::mp(metadata::value(metadata::PDU)),
                                   pmt::PMT_NIL);
    su = (const char *) pmt::blob_data(pdu);
    s = std::string((const char *) pmt::blob_data(pdu), pmt::blob_length(pdu));
    len = pmt::blob_length(pdu);
  }
  else {
    su = (const char *) pmt::blob_data(msg), pmt::blob_length(msg);
    s = std::string((const char *) pmt::blob_data(msg), pmt::blob_length(msg));
    len = pmt::blob_length(msg);
  }

  if (d_timestamp) {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    strftime(buf, sizeof(buf), "%F %T %z", &tm);
    std::cout << "[" << buf << "] ";
  }

  switch (d_format) {
  case 0: // binary
    for (size_t i = 0; i < len; i++) {
      std::cout << s[i];
    }
    std::cout << std::endl;
    break;
  case 1: // hex annotated
    for (size_t i = 0; i < len; i++) {
      std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
                << (uint32_t(su[i]) & 0xFF) << " ";
    }
    std::cout << std::endl;
    break;
  case 2: // binary annotated
    for (size_t i = 0; i < len; i++) {
      std::cout << "0b" << std::bitset<8> (su[i]) << " ";
    }
    std::cout << std::endl;
    break;
  default:
    throw std::invalid_argument("Invalid format");
  }
}

/*
 * The private constructor
 */
multi_format_msg_sink_impl::multi_format_msg_sink_impl(
  size_t format, bool timestamp, bool out_stdout,
  const std::string &filepath) :
  gr::block("multi_format_msg_sink",
            gr::io_signature::make(0, 0, 0),
            gr::io_signature::make(0, 0, 0)),
  d_format(format),
  d_timestamp(timestamp),
  d_stdout(out_stdout)
{
  message_port_register_in(pmt::mp("in"));
  if (out_stdout) {
    set_msg_handler(
      pmt::mp("in"),
    [this](pmt::pmt_t msg) {
      this->msg_handler_stdout(msg);
    });
  }
  else {
    d_fos.open(filepath);
    set_msg_handler(
      pmt::mp("in"),
    [this](pmt::pmt_t msg) {
      this->msg_handler_file(msg);
    });
  }
}

multi_format_msg_sink_impl::~multi_format_msg_sink_impl()
{
  if (!d_stdout) {
    d_fos.close();
  }
}

} /* namespace satnogs */
} /* namespace gr */

