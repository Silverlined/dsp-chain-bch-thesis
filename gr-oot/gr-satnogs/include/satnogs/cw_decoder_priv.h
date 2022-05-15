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

#ifndef CW_DECODER_PRIV_H_
#define CW_DECODER_PRIV_H_

#include <satnogs/api.h>
#include <satnogs/morse.h>
#include <satnogs/morse_tree.h>
#include <satnogs/moving_sum.h>
#include <satnogs/decoder.h>
#include <pmt/pmt.h>
#include <string>

namespace gr {
namespace satnogs {

/**
 * \brief Helper class for the CW decoder.
 *
 * Helper class for the CW decoder. This class performs the actual decoding.
 *
 */
class SATNOGS_API cw_decoder_priv {
public:

  cw_decoder_priv(double freq, float confidence, size_t dot_duration,
                  size_t min_len, size_t max_len);

  ~cw_decoder_priv();

  decoder_status_t
  decode(float newval, float snr);

  void
  reset();

private:
  typedef enum {
    NO_SYNC, SEARCH_DOT, SEARCH_DASH, SEARCH_SPACE
  } cw_dec_state_t;

  const double          d_freq;
  const float           d_confidence;
  const size_t          d_dot_duration;
  const size_t          d_dash_duration;
  const size_t          d_short_pause_duration;
  const size_t          d_long_pause_duration;
  const size_t          d_min_len;
  const size_t          d_max_len;
  const float           d_trigger_level;
  size_t                d_width;
  morse_tree            d_morse_tree;
  std::string           d_str;
  cw_dec_state_t        d_state;
  float                 d_snr;
  /**
   * Moving sum for the SNR above threshold triggers
   */
  moving_sum<float>     d_movs;
  /**
   * Local standard deviation of the triggers
   */
  moving_sum<float>     d_std_movs;
  /**
   * Moving sum for the calculation of the local std
   */
  moving_sum<float>     d_std_sq_movs;

  inline bool
  is_triggered(float meanval, float cv);

  inline bool
  check_conf_level(size_t cnt, size_t target);

  inline void
  enter_no_sync();

  inline void
  enter_search_dot();

  inline void
  enter_search_dash();

  inline void
  enter_search_space();

  inline void
  reset_snr_stats();


  std::string
  received_symbol(morse_symbol_t s);
};

} /* namespace satnogs */
} /* namespace gr */

#endif /* CW_DECODER_PRIV_H_ */
