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

#ifndef INCLUDED_SATNOGS_LRPT_DECODER_IMPL_H
#define INCLUDED_SATNOGS_LRPT_DECODER_IMPL_H

#include <satnogs/lrpt_decoder.h>
#include <satnogs/convolutional_deinterleaver.h>
#include <satnogs/whitening.h>

namespace gr {
namespace satnogs {

class lrpt_decoder_impl : public lrpt_decoder {

public:
  lrpt_decoder_impl();
  ~lrpt_decoder_impl();

private:
  const size_t                  d_cadu_len;
  const size_t                  d_coded_cadu_len;
  const size_t                  d_mpdu_max_len;
  whitening                     d_scrambler;
  bool                          d_have_mpdu;

  uint8_t                       *d_coded_cadu_syms;
  uint8_t                       *d_cadu;
  uint8_t                       *d_mpdu;
  void                          *d_vt;

  void
  decode(pmt::pmt_t m);

  void
  decode_ccsds_packet(const uint8_t *cvcdu);
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDED_SATNOGS_LRPT_DECODER_IMPL_H */

