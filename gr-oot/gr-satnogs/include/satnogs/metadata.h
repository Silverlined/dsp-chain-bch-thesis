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

#ifndef INCLUDE_SATNOGS_METADATA_H_
#define INCLUDE_SATNOGS_METADATA_H_

#include <satnogs/api.h>
#include <satnogs/decoder.h>
#include <string>
#include <pmt/pmt.h>
#include <nlohmann/json.hpp>
#include <cstdint>

namespace gr {

namespace satnogs {

class SATNOGS_API metadata {
public:

  typedef boost::shared_ptr<metadata> metadata_sptr;

  typedef enum key {
    PDU = 0,
    DECODER_CRC_VALID,
    CENTER_FREQ,
    DECODER_PHASE_DELAY,
    DECODER_RESAMPLING_RATIO,
    CRC_VALID,
    FREQ_OFFSET,
    DECODER_CORRECTED_BITS,
    TIME,
    SAMPLE_START,
    SAMPLE_CNT,
    DECODER_SYMBOL_ERASURES,
    SNR,
    DECODER_NAME,
    DECODER_VERSION,
    ANTENNA_AZIMUTH,
    ANTENNA_ELEVATION,
    ANTENNA_POLARIZATION,
    SYMBOL_TIMING_ERROR,
    KEYS_NUM
  } key_t;

  static std::string
  value(const key_t &k);

  static std::string
  keys();

  static std::string
  time_iso8601();

  static void
  add_time_iso8601(pmt::pmt_t &m);

  static void
  add_pdu(pmt::pmt_t &m, const uint8_t *in, size_t len);

  static void
  add_crc_valid(pmt::pmt_t &m, bool valid);

  static void
  add_sample_start(pmt::pmt_t &m, uint64_t idx);

  static void
  add_sample_cnt(pmt::pmt_t &m, uint64_t cnt);

  static void
  add_symbol_erasures(pmt::pmt_t &m, uint32_t cnt);

  static void
  add_corrected_bits(pmt::pmt_t &m, uint32_t cnt);

  static void
  add_center_freq(pmt::pmt_t &m, double offset);

  static void
  add_freq_offset(pmt::pmt_t &m, double offset);

  static void
  add_snr(pmt::pmt_t &m, float snr);

  static void
  add_decoder(pmt::pmt_t &m, const std::string &name,
              const std::string &version);

  static void
  add_decoder(pmt::pmt_t &m, const decoder *dec);

  static void
  add_antenna_azimuth(pmt::pmt_t &m, double azimuth);

  static void
  add_antenna_elevation(pmt::pmt_t &m, double elevation);

  static void
  add_antenna_polarization(pmt::pmt_t &m, std::string polarization);

  static void
  add_phase_delay(pmt::pmt_t &m, uint64_t phase);

  static void
  add_resampling_ratio(pmt::pmt_t &m, float ratio);

  static void
  add_symbol_timing_error(pmt::pmt_t &m, double error);

  static nlohmann::json
  to_json(const pmt::pmt_t &m);

  /*!
   * Transforms the PMT metadata message of gr-satnogs to the appropriate form.
   * The form and the storage is implementation specific
   *
   * @param m the message containing a dictionary of gr-satnogs matadata
   */
  virtual void
  transform(pmt::pmt_t &m) = 0;

  /*!
   * Stores to file the metadata. The form of the data depends on the
   * implementation
   *
   * @param filename the full path to the metadata file
   */
  virtual void
  to_file(const std::string &filename) = 0;

  /*!
   *
   * @return string representation of the processed metadata
   */
  virtual const
  std::string to_sting() = 0;

  virtual ~metadata();
};

}  // namespace satnogs

}  // namespace gr


#endif /* INCLUDE_SATNOGS_METADATA_H_ */
