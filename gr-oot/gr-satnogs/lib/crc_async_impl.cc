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
#include "crc_async_impl.h"
#include <arpa/inet.h>

namespace gr {
namespace satnogs {

crc_async::sptr
crc_async::make(crc::crc_t crc, bool check)
{
  return gnuradio::get_initial_sptr(new crc_async_impl(crc, check));
}


/*
 * The private constructor
 */
crc_async_impl::crc_async_impl(crc::crc_t crc, bool check)
  : gr::block("crc_async",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
    d_crc(crc)
{
  message_port_register_in(pmt::mp("pdu"));
  message_port_register_out(pmt::mp("pdu"));

  if (check) {
    set_msg_handler(pmt::mp("pdu"),
    [this](pmt::pmt_t msg) {
      this->check(msg);
    });
  }
  else {
    set_msg_handler(pmt::mp("pdu"),
    [this](pmt::pmt_t msg) {
      this->append(msg);
    });
  }
}

/*
 * Our virtual destructor.
 */
crc_async_impl::~crc_async_impl()
{
}

void
crc_async_impl::append(pmt::pmt_t msg)
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
    throw std::runtime_error("crc_async: received a malformed pdu message");
  }

  size_t pdu_len(0);
  const uint8_t *pdu = (const uint8_t *) pmt::uniform_vector_elements(b, pdu_len);

  uint8_t *pdu_crc = new uint8_t[crc::crc_size(d_crc) + pdu_len];
  std::copy(pdu, pdu + pdu_len, pdu_crc);
  crc::append(d_crc, pdu_crc + pdu_len, pdu_crc, pdu_len, true);
  pmt::pmt_t vecpmt(pmt::make_blob(pdu_crc, crc::crc_size(d_crc) + pdu_len));
  pmt::pmt_t res(pmt::cons(meta, vecpmt));

  message_port_pub(pmt::mp("pdu"), res);
  delete [] pdu_crc;
}

void
crc_async_impl::check(pmt::pmt_t msg)
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
    throw std::runtime_error("crc_async: received a malformed pdu message");
  }

  size_t pdu_len(0);
  const uint8_t *pdu = (const uint8_t *) pmt::uniform_vector_elements(b, pdu_len);

  if (!crc::check(d_crc, pdu, pdu_len)) {
    return;
  }

  pmt::pmt_t vecpmt(pmt::make_blob(pdu,  pdu_len - crc::crc_size(d_crc)));
  pmt::pmt_t res(pmt::cons(meta, vecpmt));

  message_port_pub(pmt::mp("pdu"), res);
}

} /* namespace satnogs */
} /* namespace gr */



