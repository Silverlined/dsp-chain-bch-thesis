/* -*- c++ -*- */
/*
 * gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
 *
 *  Copyright (C) 2018, Libre Space Foundation <http://librespacefoundation.org/>
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
#include "lrpt_decoder_impl.h"
#include <satnogs/log.h>
#include <satnogs/utils.h>
#include <satnogs/libfec/fec.h>

namespace gr {
namespace satnogs {

lrpt_decoder::sptr
lrpt_decoder::make()
{
  return gnuradio::get_initial_sptr(new lrpt_decoder_impl());
}

/*
 * The private constructor
 */
lrpt_decoder_impl::lrpt_decoder_impl()
  : gr::block("lrpt_decoder",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(0, 0, 0)),
    /*
     * Metop violates the standard as many times as possible...
     * The frame should contain 128 RS check symbols at the end.
     * For some unknown reasons, it seems that the RS encoding is not performed.
     * Thus, they dropped the check symbols at the end of the frame.
     */
    d_cadu_len(1020 + 4 - 128),
    d_coded_cadu_len(1020 * 2 + 4 * 2 - 128 * 2),
    d_mpdu_max_len(59400),
    d_scrambler(0x2A9, 0xFF, 7),
    d_have_mpdu(false)
{
  message_port_register_in(pmt::mp("cadu"));
  message_port_register_out(pmt::mp("frame"));

  set_msg_handler(pmt::mp("cadu"),
  [this](pmt::pmt_t msg) {
    this->decode(msg);
  });

  d_vt = create_viterbi27(d_cadu_len * 8);
  if (!d_vt) {
    throw std::runtime_error("lrpt_decoder: Failed to init Viterbi decoder");
  }

  int polys[2] = {0x4f, 0x6d};
  set_viterbi27_polynomial(polys);

  d_cadu = new uint8_t[d_cadu_len];
  d_coded_cadu_syms = new uint8_t[d_coded_cadu_len * 8];
  d_mpdu = new uint8_t[d_mpdu_max_len];

}

/*
 * Our virtual destructor.
 */
lrpt_decoder_impl::~lrpt_decoder_impl()
{

  delete [] d_cadu;
  delete [] d_coded_cadu_syms;
  delete [] d_mpdu;
}

void
lrpt_decoder_impl::decode(pmt::pmt_t m)
{
  const uint8_t *coded_cadu = (const uint8_t *)pmt::blob_data(m);
  if (pmt::blob_length(m) != d_coded_cadu_len) {
    LOG_ERROR("Wrong CADU size");
    return;
  }

  init_viterbi27(d_vt, 0);

  for (size_t i = 0; i < d_coded_cadu_len; i++) {
    d_coded_cadu_syms[i * 8] = 0xFF * (coded_cadu[i] >> 7);
    d_coded_cadu_syms[i * 8 + 1] = 0xFF * ((coded_cadu[i] >> 6) & 0x1);
    d_coded_cadu_syms[i * 8 + 2] = 0xFF * ((coded_cadu[i] >> 5) & 0x1);
    d_coded_cadu_syms[i * 8 + 3] = 0xFF * ((coded_cadu[i] >> 4) & 0x1);
    d_coded_cadu_syms[i * 8 + 4] = 0xFF * ((coded_cadu[i] >> 3) & 0x1);
    d_coded_cadu_syms[i * 8 + 5] = 0xFF * ((coded_cadu[i] >> 2) & 0x1);
    d_coded_cadu_syms[i * 8 + 6] = 0xFF * ((coded_cadu[i] >> 1) & 0x1);
    d_coded_cadu_syms[i * 8 + 7] = 0xFF * ((coded_cadu[i] & 0x1));
  }

  /* Convolutional decoding */
  update_viterbi27_blk(d_vt, d_coded_cadu_syms, d_cadu_len * 8);
  chainback_viterbi27(d_vt, d_cadu, d_cadu_len * 8, 0);

  /* Descrambling */
  d_scrambler.reset();
  d_scrambler.descramble(d_cadu + 4, d_cadu + 4, d_cadu_len - 4);
  decode_ccsds_packet(d_cadu + 4);
}


void
lrpt_decoder_impl::decode_ccsds_packet(const uint8_t *cvcdu)
{
  /* Check first the VCDU version and if encryption is off */
  if ((cvcdu[0] >> 6) != 0x1) {
    return;
  }
  if (cvcdu[6] != 0x0 || cvcdu[7] != 0x0) {
    return;
  }

  /* Check if the VCDU contans data */
  //if((cvcdu[8] >> 3) != 0x0 && (cvcdu[8] >> 3) != 0x1f) {
  //  return;
  //}

  const uint8_t *mpdu = cvcdu + 10;
  /* Check CCSDS packet version and type */
  //if( (mpdu[0] >> 5) != 0x0) {
  //  return;
// }

  uint32_t vcdu_seq = 0;
  vcdu_seq = cvcdu[2];
  vcdu_seq = (vcdu_seq << 8) | cvcdu[3];
  vcdu_seq = (vcdu_seq << 8) | cvcdu[4];

  uint16_t hdr_ptr = 0;
  hdr_ptr = cvcdu[8] & 0x7;
  hdr_ptr = (hdr_ptr << 8) | cvcdu[9];

  /* Try to find the start of a MPDU */
  if (!d_have_mpdu) {
    if (hdr_ptr != 0) {
      return;
    }
    d_have_mpdu = true;
  }
  message_port_pub(pmt::mp("frame"), pmt::make_blob(cvcdu, d_cadu_len - 4));
}


} /* namespace satnogs */
} /* namespace gr */
