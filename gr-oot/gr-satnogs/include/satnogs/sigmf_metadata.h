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

#ifndef INCLUDE_SATNOGS_SIGMF_METADATA_H
#define INCLUDE_SATNOGS_SIGMF_METADATA_H

#include <satnogs/api.h>
#include <pmt/pmt.h>
#include <satnogs/metadata.h>

namespace gr {
namespace satnogs {

/*!
 * The sigMF implementation of gr-satnogs metadata
 */
class SATNOGS_API sigmf_metadata : public metadata {
public:

  virtual ~sigmf_metadata() {};

  static metadata::metadata_sptr
  make(const std::string &global_segment_json = "{}");

  virtual void
  transform(pmt::pmt_t &m) = 0;

  virtual void
  to_file(const std::string &filename) = 0;

  virtual const std::string
  to_sting() = 0;


};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDE_SATNOGS_SIGMF_METADATA_H */

