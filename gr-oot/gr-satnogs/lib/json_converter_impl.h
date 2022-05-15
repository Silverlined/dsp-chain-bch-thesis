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

#ifndef INCLUDED_SATNOGS_JSON_CONVERTER_IMPL_H
#define INCLUDED_SATNOGS_JSON_CONVERTER_IMPL_H

#include <satnogs/json_converter.h>

namespace gr {
namespace satnogs {

class json_converter_impl : public json_converter {

public:
  json_converter_impl(const std::string &extra);
  ~json_converter_impl();

  void
  convert(pmt::pmt_t m);

private:
  const std::string d_extra;
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_JSON_CONVERTER_IMPL_H */

