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

#ifndef INCLUDED_SATNOGS_METADATA_SINK_IMPL_H
#define INCLUDED_SATNOGS_METADATA_SINK_IMPL_H

#include <satnogs/metadata_sink.h>

namespace gr {
namespace satnogs {

class metadata_sink_impl : public metadata_sink {

public:
  metadata_sink_impl(metadata::metadata_sptr meta,
                     const std::string &file);
  ~metadata_sink_impl();

  bool
  stop();

private:
  const bool d_enable;
  metadata::metadata_sptr d_meta;
  const std::string d_filename;

  void
  handle_msg(pmt::pmt_t msg);
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_METADATA_SINK_IMPL_H */

