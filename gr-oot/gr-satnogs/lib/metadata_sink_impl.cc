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

#include <gnuradio/io_signature.h>
#include "metadata_sink_impl.h"

namespace gr {
namespace satnogs {

metadata_sink::sptr
metadata_sink::make(metadata::metadata_sptr meta,
                    const std::string &file)
{
  return gnuradio::get_initial_sptr(
           new metadata_sink_impl(meta, file));
}

/*
 * The private constructor
 */
metadata_sink_impl::metadata_sink_impl(metadata::metadata_sptr meta,
                                       const std::string &file) :
  gr::block("metadata_sink",
            gr::io_signature::make(0, 0, 0),
            gr::io_signature::make(0, 0, 0)),
  d_enable(meta != nullptr),
  d_meta(meta),
  d_filename(file)
{
  message_port_register_in(pmt::mp("meta"));
  set_msg_handler(pmt::mp("meta"), [this](pmt::pmt_t msg) {
    this->handle_msg(msg);
  });
}

/*
 * Our virtual destructor.
 */
metadata_sink_impl::~metadata_sink_impl()
{
}

bool
metadata_sink_impl::stop()
{
  if (d_enable) {
    d_meta->to_file(d_filename);
  }
  return true;
}

void
metadata_sink_impl::handle_msg(pmt::pmt_t msg)
{
  if (d_enable) {
    d_meta->transform(msg);
  }
}

} /* namespace satnogs */
} /* namespace gr */

