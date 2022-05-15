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

#include "ber_calculator_impl.h"
#include <random>
#include <cstring>
#include <satnogs/crc.h>
#include <satnogs/whitening.h>
#include <satnogs/metadata.h>
#include <satnogs/utils.h>
#include <thread>

namespace gr {
namespace satnogs {

/**
 * Creates the BER calculator block
 * @param frame_size the frame size
 * @param nframes the number of frames to send for the test. If set to 0 the
 * test continues until the user terminates the flowgraph
 * @param skip skip the given number frames at the start of the experiment.
 * Useful to deal with buffering and alignment issues
 * @return shared pointer to a BER calculator block
 */
ber_calculator::sptr
ber_calculator::make(size_t frame_size, size_t nframes, size_t skip)
{
  return gnuradio::get_initial_sptr(
           new ber_calculator_impl(frame_size, nframes, skip));
}

ber_calculator_impl::ber_calculator_impl(size_t frame_size, size_t nframes,
    size_t skip)
  : gr::block("ber_calculator",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
    d_frame_size(frame_size),
    d_nframes(nframes),
    d_skip(skip),
    d_cnt(0),
    d_inval_cnt(0),
    d_last_ack(0),
    d_dropped(0),
    d_received(0)
{
  if (frame_size < 8) {
    throw std::invalid_argument(alias() +
                                ": Minimum supported frame size is 8 bytes");
  }

  message_port_register_in(pmt::mp("trigger"));
  message_port_register_in(pmt::mp("received"));
  message_port_register_out(pmt::mp("pdu"));

  set_msg_handler(pmt::mp("trigger"),
  [this](pmt::pmt_t msg) {
    this->create_pdu();
  });

  set_msg_handler(pmt::mp("received"),
  [this](pmt::pmt_t msg) {
    this->received_pdu(msg);
  });
}

ber_calculator_impl::~ber_calculator_impl()
{
}

bool
ber_calculator_impl::stop()
{
  print_stats();
  return true;
}

/**
 * Appends an PDU that was sent from the transmitter at the DB.
 * @param msg
 */
void
ber_calculator_impl::create_pdu()
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<uint8_t> uni(0, 0xFF);
  std::vector<uint8_t> d(d_frame_size);
  std::generate(d.begin(), d.end(), [&] {
    return uni(mt);
  });

  memcpy(d.data(), &d_cnt, sizeof(uint64_t));
  d_cnt++;
  const size_t crc_size = crc::crc_size(crc::crc_t::CRC32_C);
  crc::append(crc::crc_t::CRC32_C, d.data() + d_frame_size - crc_size,
              d.data(), d_frame_size - crc_size);
  /* CCSDS variant to avoid using the same scrambler by an encoder */
  whitening::whitening_sptr w = whitening::make(0xA9, 0xAA, 7, true);
  w->scramble(d.data(), d.data(), d_frame_size);
  message_port_pub(pmt::mp("pdu"), pmt::cons(pmt::PMT_NIL,
                   pmt::init_u8vector(d_frame_size, d)));

  if (d_nframes != 0 && d_cnt == d_nframes) {
    system_handler(pmt::cons(pmt::intern("done"), pmt::mp(true)));
  }
}

/**
 * Appends an PDU that was sent from the transmitter at the DB
 * @param msg
 */
void
ber_calculator_impl::received_pdu(pmt::pmt_t msg)
{
  pmt::pmt_t b;
  /*
   * Try to support most of the known PDU formats, including GNU Radio,
   * raw blobs and SatNOGS dictionaries
   */
  if (pmt::is_blob(msg)) {
    b = msg;
  }
  else if (pmt::dict_has_key(msg, pmt::mp(metadata::value(metadata::PDU)))) {
    b = pmt::dict_ref(msg, pmt::mp(metadata::value(metadata::PDU)),
                      pmt::PMT_NIL);
  }
  else if (pmt::is_pair(msg)) {
    b = pmt::cdr(msg);
  }
  else {
    throw std::runtime_error(alias() + ": received a malformed pdu message");
  }
  /* Descramble it and check for the frame validity */
  size_t len;
  uint8_t *d;
  if (pmt::is_blob(b)) {
    len = pmt::blob_length(b);
    d = (uint8_t *)pmt::blob_data(b);
  }
  else {
    d = (uint8_t *)pmt::uniform_vector_writable_elements(b, len);
  }
  if (len != d_frame_size) {
    d_inval_cnt++;
    return;
  }

  /* CCSDS variant to avoid using the same scrambler by an encoder */
  whitening::whitening_sptr w = whitening::make(0xA9, 0xAA, 7, true);
  w->descramble(d, d, d_frame_size);
  if (!crc::check(crc::crc_t::CRC32_C, d, d_frame_size)) {
    d_inval_cnt++;
    return;
  }

  uint64_t cnt = 0;
  memcpy(&cnt, d, sizeof(uint64_t));
  if (cnt < d_last_ack || cnt > d_cnt) {
    d_inval_cnt++;
    return;
  }

  if (cnt < d_skip) {
    d_last_ack = cnt + 1;
    return;
  }

  d_dropped += (cnt - d_last_ack);
  d_last_ack = cnt + 1;
  d_received++;
}

/**
 *
 * @return the FER (Frame Error Rate).
 * @note the FER does not take into consideration the last unacknowledged frames,
 * to mitigate buffering issues inside the processing chain
 */
double
ber_calculator_impl::fer()
{
  if (d_last_ack == 0) {
    return 1.0;
  }
  return (double)d_dropped / d_last_ack;
}

/**
 * Calculates the BER based on the FER
 * The formula is: \f$ FER = 1 - (1 - BER)^{N} \f$ where \f$ N \f$ is the number
 * of bits of the frame. Assuming that the preamble, SFD are not affecting a lot
 * the result the BER can be calculated as:
 * \f$ BER = 1 - 10^{\frac{log_{10}{1-FER}}{N}}\f$
  * @return the estimated BER
 */
double
ber_calculator_impl::ber()
{
  return 1.0 - std::pow(10.0, std::log10(1.0 - fer()) / d_frame_size * 8);
}

void
ber_calculator_impl::print_stats()
{
  std::cout << "Frames sent     : " << d_cnt << std::endl;
  std::cout << "Frames received : " << d_received << std::endl;
  std::cout << "Frames lost     : " << d_dropped << std::endl;
  std::cout << "Frames invalid  : " << d_inval_cnt << std::endl;
  std::cout << "FER             : " << fer() << std::endl;
  std::cout << "BER estimation  : " << ber() << std::endl;
}

} /* namespace satnogs */
} /* namespace gr */
