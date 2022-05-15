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

#ifndef INCLUDED_SATNOGS_JSON_CONVERTER_H
#define INCLUDED_SATNOGS_JSON_CONVERTER_H

#include <satnogs/api.h>
#include <gnuradio/block.h>
#include <string>
namespace gr {
namespace satnogs {

/*!
 * \brief This block takes a PMT message from the SatNOGS decoders
 * and converts it a PMT containing the same message in JSON form.
 * This can be used for debugging and stream operations.
 * \ingroup satnogs
 *
 */
class SATNOGS_API json_converter : virtual public gr::block {
public:
  typedef boost::shared_ptr<json_converter> sptr;


  /**
  * This block takes a PMT message from the SatNOGS decoders
  * and converts it a PMT containing the same message in JSON form.
  * This can be used for debugging and stream operations.
  *
  * @param extra every JSON frame can contain an arbitrary amount of extra information.
  * Use this fill to provide a JSON-valid string with such information.
  *
  * @return shared pointer of the block instance
  */
  static sptr
  make(const std::string &extra = "");
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_JSON_CONVERTER_H */

