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
#include <satnogs/rs_encoder.h>
#include <satnogs/libfec/fec.h>

namespace gr {
namespace satnogs {

encoder::encoder_sptr
rs_encoder::make()
{
  return encoder::encoder_sptr(new rs_encoder());
}

rs_encoder::rs_encoder()
{
}

rs_encoder::~rs_encoder()
{
}

pmt::pmt_t
rs_encoder::encode(pmt::pmt_t msg)
{
  pmt::pmt_t b;
  pmt::pmt_t meta;
  if (pmt::is_blob(msg)) {
    b = msg;
    meta = pmt::PMT_NIL;
  }
  else if (pmt::is_pair(msg)) {
    meta = pmt::car(msg);
    b = pmt::cdr(msg);
  }
  else {
    throw std::runtime_error("rs_encoder: received a malformed pdu message");
  }

  size_t pdu_len(0);
  const uint8_t *pdu = (const uint8_t *) pmt::uniform_vector_elements(b, pdu_len);
  if (pdu_len > 223) {
    throw std::runtime_error("rs_encoder: PDU received has a size larger than the maximum allowed");
  }

  uint8_t payload[255] = {0x0};
  uint8_t parity[32] = {0x0};
  std::copy(pdu, pdu + pdu_len, payload);

  /* Perform RS encoding */
  encode_rs_8(payload, parity, 223 - pdu_len);
  std::copy(pdu + pdu_len, pdu + pdu_len + 32, parity);


  /* Make a pmt compatible with the pdu to tagged stream block */
  pmt::pmt_t vecpmt(pmt::make_blob(payload, pdu_len + 32));
  pmt::pmt_t res(pmt::cons(meta, vecpmt));
  return res;
}

} /* namespace satnogs */
} /* namespace gr */





