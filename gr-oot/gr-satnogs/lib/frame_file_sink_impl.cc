/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2017,2020 Libre Space Foundation <http://libre.space>
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
#include "frame_file_sink_impl.h"

#include <satnogs/base64.h>
#include <satnogs/metadata.h>

namespace gr {
namespace satnogs {

frame_file_sink::sptr
frame_file_sink::make(const std::string &prefix_name, int output_type)
{
  return gnuradio::get_initial_sptr(
           new frame_file_sink_impl(prefix_name, output_type));
}

/*
 * The private constructor
 */
frame_file_sink_impl::frame_file_sink_impl(const std::string &prefix_name,
    int output_type) :
  gr::block("frame_file_sink", gr::io_signature::make(0, 0, 0),
            gr::io_signature::make(0, 0, 0)),
  d_prefix_name(prefix_name),
  d_output_type(output_type),
  d_filename_prev(""),
  d_counter(0)
{
  message_port_register_in(pmt::mp("frame"));
  set_msg_handler(pmt::mp("frame"),
  [this](pmt::pmt_t msg) {
    this->msg_handler_frame(msg);
  });

}

/*
 * Our virtual destructor.
 */
frame_file_sink_impl::~frame_file_sink_impl()
{
}

void
frame_file_sink_impl::msg_handler_frame(pmt::pmt_t msg)
{

  /* check for the current UTC time */
  std::chrono::system_clock::time_point p2 =
    std::chrono::system_clock::now();

  char buffer[30];
  std::time_t t2 = std::chrono::system_clock::to_time_t (p2);
  struct tm *timeinfo;
  timeinfo = std::gmtime(&t2);

  std::strftime(buffer, 30, "%FT%H-%M-%S", timeinfo);
  //puts (buffer);

  /* create name of the file according prefix and timestamp */
  std::string filename;
  filename.append(d_prefix_name);
  filename.append("_");
  filename.append(buffer);


  if (filename.compare(d_filename_prev) == 0) {
    d_filename_prev.assign(filename);
    filename.append("_");
    d_counter++;
    filename.append(std::to_string(d_counter));
  }
  else {
    d_filename_prev.assign(filename);
    d_counter = 0;
  }

  const char *su;
  size_t len;

  /* Check if the message contains the legacy or the new format */
  if (pmt::is_dict(msg)) {
    pmt::pmt_t pdu = pmt::dict_ref(msg,
                                   pmt::mp(metadata::value(metadata::PDU)),
                                   pmt::PMT_NIL);
    su = (const char *) pmt::blob_data(pdu);
    len = pmt::blob_length(pdu);
  }
  else {
    su = (const char *) pmt::blob_data(msg), pmt::blob_length(msg);
    len = pmt::blob_length(msg);
  }

  switch (d_output_type) {
  case 0: {
    /* Binary form */
    std::ofstream fd(filename.c_str());
    fd.write(su, len);
    fd.close();
    break;
  }
  case 1: {
    /* aHex annotated, dd .txt to filename */
    filename.append(".txt");
    std::ofstream fd(filename.c_str());
    for (size_t i = 0; i < len; i++) {
      fd << "0x" << std::hex << std::setw(2) << std::setfill('0')
         << (uint32_t(su[i]) & 0xFF) << " ";
    }
    fd.close();
    break;
  }
  case 2: {
    /* Binary annotated, add .txt to filename */
    filename.append(".txt");
    std::ofstream fd(filename.c_str());
    for (size_t i = 0; i < len; i++) {
      fd << "0b" << std::bitset<8> (su[i]) << " ";
    }
    fd.close();
    break;
  }
  default:
    throw std::invalid_argument("Invalid format");
  }

}

} /* namespace satnogs */
} /* namespace gr */

