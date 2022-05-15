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

#include <satnogs/cw_decoder_priv.h>
#include <satnogs/log.h>
#include <cmath>
#include <iostream>
#include <satnogs/utils.h>
#include <satnogs/metadata.h>

namespace gr {
namespace satnogs {


cw_decoder_priv::cw_decoder_priv(double freq, float confidence,
                                 size_t dot_duration,
                                 size_t min_len, size_t max_len) :
  d_freq(freq),
  d_confidence(confidence),
  d_dot_duration(dot_duration - dot_duration / 4),
  d_dash_duration(3 * dot_duration - dot_duration / 4),
  d_short_pause_duration(3 * dot_duration - dot_duration / 4),
  d_long_pause_duration(7 * dot_duration - dot_duration / 4),
  d_min_len(min_len),
  d_max_len(max_len),
  /* Take into consideration the offset added for the coefficient of variation
   * calculation
   */
  d_trigger_level(dot_duration  * confidence),
  d_width(0),
  d_morse_tree('?', max_len),
  d_str(""),
  d_state(NO_SYNC),
  d_snr(0.0f),
  /*
   * Initializing the moving sums to a non-zero value is essential for the
   * coefficient of variation calculation stability
   */
  d_movs(dot_duration / 2, 1.0f),
  d_std_movs(5, 1.0f),
  d_std_sq_movs(5, 1.0f)
{
}

std::string
cw_decoder_priv::received_symbol(morse_symbol_t s)
{
  bool res = false;
  switch (s) {
  case MORSE_DOT:
  case MORSE_DASH:
  case MORSE_S_SPACE:
    res = d_morse_tree.received_symbol(s);
    break;
  case MORSE_L_SPACE:
    /*
     * Inject a character separator, for the morse decoder to commit
     * the outstanding character
     */
    res = d_morse_tree.received_symbol(MORSE_S_SPACE);
    /* Just ignore the word separator if no word is yet decoded */
    if (d_morse_tree.get_word_len() == 0) {
      res = true;
    }
    else {
      d_str = d_str.append(d_morse_tree.get_word());
      d_str = d_str.append(" ");
      d_morse_tree.reset();
    }
    break;
  case MORSE_INTRA_SPACE:
    /*Ignore it */
    break;
  case MORSE_END_MSG_SPACE:
    if (d_str.length() > d_min_len) {
      d_morse_tree.reset();
      std::string s = d_str;
      d_str = "";
      return s;
    }
    break;
  default:
    LOG_ERROR("cw_ecoder: Invalid symbol");
    break;
  }

  /*
  * If the decoding return false, it means that either an non decode-able
  * character situation occurred or the maximum word limit reached
  */
  if (!res && d_morse_tree.get_max_word_len() == d_morse_tree.get_word_len()) {
    std::string s = d_str.append(d_morse_tree.get_word());
    d_str = "";
    d_morse_tree.reset();
    return s;
  }
  return std::string("");
}

inline bool
cw_decoder_priv::is_triggered(float meanval, float cv)
{
  if (meanval > d_trigger_level && cv < (1.0f - d_confidence)) {
    return true;
  }
  return false;
}

inline bool
cw_decoder_priv::check_conf_level(size_t cnt, size_t target)
{
  return ((float)cnt > target * d_confidence);
}

/**
 * The decoder work as follows. The moving sum of the FFT triggers are passed.
 * This moving sum should exchibit a plateu when the CW is on. Depending on the
 * width of the plateu, it could be a dot or dash. In addition during a plateu
 * a small standard deviation should be observed.
 */
decoder_status_t
cw_decoder_priv::decode(float newval, float snr)
{
  decoder_status_t status;
  float mv;
  float mu;
  float x;
  float std_val;

  /* We need to calculate the coefficient of variation, the mean cannot be
   * zero. We add a constant offset and we take that offset into consideration
   * for every logic that is affected
   */
  mv = d_movs.insert(newval + 1.0f);

  /* Compute the stdfilt value based on the new value */
  mu = d_std_movs.insert(mv) / 5.0f;
  x = d_std_sq_movs.insert(mv * mv);
  std_val = std::sqrt(x / 5.0f - mu * mu);
  bool triggered = is_triggered(mv, std_val / mu);
  switch (d_state) {
  case NO_SYNC:
    if (triggered) {
      enter_search_dot();
    }
    else {
      d_width++;
      if (d_width > 3 * d_long_pause_duration) {
        std::string s = received_symbol(MORSE_END_MSG_SPACE);
        if (s.length() > d_min_len) {
          metadata::add_time_iso8601(status.data);
          metadata::add_freq_offset(status.data, d_freq);
          metadata::add_snr(status.data, d_snr);
          metadata::add_pdu(status.data, (const uint8_t *)s.c_str(),
                            s.length());
          status.decode_success = true;
        }
        reset_snr_stats();
        enter_no_sync();
      }
    }
    break;
  case SEARCH_DOT:
    if (triggered) {
      d_width++;
      d_snr = std::max(d_snr, snr);
      if (d_width > d_dot_duration) {
        enter_search_dash();
      }
    }
    else {
      if (check_conf_level(d_width, d_dot_duration)) {
        received_symbol(MORSE_DOT);
      }
      enter_search_space();
    }
    break;
  case SEARCH_DASH:
    if (triggered) {
      d_snr = std::max(d_snr, snr);
      d_width++;
    }
    else {
      /* Avoid DC or harmonics spikes */
      if (d_width > d_dash_duration * 1.75) {
        d_state = NO_SYNC;
        d_width = 0;
      }
      else if (check_conf_level(d_width, d_dash_duration)) {
        received_symbol(MORSE_DASH);
      }
      else {
        received_symbol(MORSE_DOT);
      }
      enter_search_space();
    }
    break;
  case SEARCH_SPACE:
    if (triggered) {
      if (check_conf_level(d_width, d_long_pause_duration)) {
        received_symbol(MORSE_L_SPACE);
      }
      else if (check_conf_level(d_width, d_short_pause_duration)) {
        received_symbol(MORSE_S_SPACE);
      }
      enter_search_dot();
    }
    else {
      d_width++;
      if (check_conf_level(d_width, d_long_pause_duration)) {
        received_symbol(MORSE_L_SPACE);
        enter_no_sync();
      }
    }
    break;
  }
  return status;
}

inline void
cw_decoder_priv::enter_no_sync()
{
  d_width = 0;
  d_state = NO_SYNC;
}

inline void
cw_decoder_priv::enter_search_dot()
{
  d_width = 0;
  d_state = SEARCH_DOT;
}

inline void
cw_decoder_priv::enter_search_dash()
{
  d_state = SEARCH_DASH;
}

inline void
cw_decoder_priv::enter_search_space()
{
  d_state = SEARCH_SPACE;
  d_width = 0;
}

inline void
cw_decoder_priv::reset_snr_stats()
{
  d_snr = 0.0f;
}

void
cw_decoder_priv::reset()
{
  d_morse_tree.reset();
  d_width = 0;
  d_state = NO_SYNC;
}


cw_decoder_priv::~cw_decoder_priv()
{
}

} /* namespace satnogs */
} /* namespace gr */


