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

#include <satnogs/metadata.h>
#include <satnogs/base64.h>
#include <satnogs/date.h>
#include <stdexcept>
#include <chrono>

namespace gr {

namespace satnogs {

metadata::~metadata()
{

}

std::string
metadata::keys()
{
  std::string s = "[";
  for (size_t i = 0; i < KEYS_NUM - 1; i++) {
    s.append(value((key_t) i));
    s.append(", ");
  }
  s.append(value((key_t)(KEYS_NUM - 1)));
  s.append("]");
  return s;
}

/**
 * Return the string representation of the @a k.
 * This string can be
 * @param k the key enumeration
 * @return string corresponding to the key @a k value
 */
std::string
metadata::value(const key_t &k)
{
  switch (k) {
  case PDU:
    return "pdu";
  case DECODER_CRC_VALID:
    return "decoder_crc_valid";
  case CENTER_FREQ:
    return "center_freq";
  case FREQ_OFFSET:
    return "freq_offset";
  case DECODER_CORRECTED_BITS:
    return "decoder_corrected_bits";
  case TIME:
    return "time";
  case SAMPLE_START:
    return "sample_start";
  case SAMPLE_CNT:
    return "sample_cnt";
  case DECODER_SYMBOL_ERASURES:
    return "decoder_symbol_erasures";
  case SNR:
    return "snr";
  case DECODER_NAME:
    return "decoder_name";
  case DECODER_VERSION:
    return "decoder_version";
  case ANTENNA_AZIMUTH:
    return "antenna_azimuth";
  case ANTENNA_ELEVATION:
    return "antenna_elevation";
  case ANTENNA_POLARIZATION:
    return "antenna_polarization";
  case DECODER_PHASE_DELAY:
    return "decoder_phase";
  case DECODER_RESAMPLING_RATIO:
    return "decoder_resampling_ratio";
  case SYMBOL_TIMING_ERROR:
    return "symbol_timing_error";
  default:
    throw std::invalid_argument("metadata: invalid key");
  }
}

/**
 *
 * @return string with the date in ISO 8601 format
 */
std::string
metadata::time_iso8601()
{
  /* check for the current UTC time */
  std::chrono::system_clock::time_point tp =
    std::chrono::system_clock::now();

  return date::format("%FT%TZ", date::floor<std::chrono::microseconds>(tp));
}

/**
 * Adds the system current time in ISO 8601  sting format
 * @param m reference to a PMT dictionary
 */
void
metadata::add_time_iso8601(pmt::pmt_t &m)
{
  m = pmt::dict_add(m, pmt::mp(value(TIME)), pmt::mp(time_iso8601()));
}

void
metadata::add_pdu(pmt::pmt_t &m, const uint8_t *in, size_t len)
{
  m = pmt::dict_add(m, pmt::mp(value(PDU)), pmt::make_blob(in, len));
}

/**
 * Adds the CRC validity value
 * @param m reference to a PMT dictionary
 */
void
metadata::add_crc_valid(pmt::pmt_t &m, bool valid)
{
  m = pmt::dict_add(m, pmt::mp(value(DECODER_CRC_VALID)), pmt::from_bool(valid));
}

void
metadata::add_sample_start(pmt::pmt_t &m, uint64_t idx)
{
  m = pmt::dict_add(m, pmt::mp(value(SAMPLE_START)), pmt::from_uint64(idx));
}

void
metadata::add_sample_cnt(pmt::pmt_t &m, uint64_t cnt)
{
  m = pmt::dict_add(m, pmt::mp(value(SAMPLE_CNT)), pmt::from_uint64(cnt));
}

void
metadata::add_symbol_erasures(pmt::pmt_t &m, uint32_t cnt)
{
  m = pmt::dict_add(m, pmt::mp(value(DECODER_SYMBOL_ERASURES)),
                    pmt::from_uint64(cnt));
}

void
metadata::add_corrected_bits(pmt::pmt_t &m, uint32_t cnt)
{
  m = pmt::dict_add(m, pmt::mp(value(DECODER_CORRECTED_BITS)),
                    pmt::from_uint64(cnt));
}

void
metadata::add_center_freq(pmt::pmt_t &m, double freq)
{
  m = pmt::dict_add(m, pmt::mp(value(CENTER_FREQ)), pmt::from_double(freq));
}

void
metadata::add_freq_offset(pmt::pmt_t &m, double offset)
{
  m = pmt::dict_add(m, pmt::mp(value(FREQ_OFFSET)), pmt::from_double(offset));
}

void
metadata::add_snr(pmt::pmt_t &m, float snr)
{
  m = pmt::dict_add(m, pmt::mp(value(SNR)), pmt::from_float(snr));
}

void
metadata::add_antenna_azimuth(pmt::pmt_t &m, double azimuth)
{
  m = pmt::dict_add(m, pmt::mp(value(ANTENNA_AZIMUTH)),
                    pmt::from_double(azimuth));
}

void
metadata::add_antenna_elevation(pmt::pmt_t &m, double elevation)
{
  m = pmt::dict_add(m, pmt::mp(value(ANTENNA_ELEVATION)),
                    pmt::from_double(elevation));
}

void
metadata::add_antenna_polarization(pmt::pmt_t &m, std::string polarization)
{
  m = pmt::dict_add(m, pmt::mp(value(ANTENNA_POLARIZATION)),
                    pmt::mp(polarization));
}

void
metadata::add_decoder(pmt::pmt_t &m, const std::string &name,
                      const std::string &version)
{
  m = pmt::dict_add(m, pmt::mp(value(DECODER_NAME)), pmt::mp(name));
  m = pmt::dict_add(m, pmt::mp(value(DECODER_VERSION)), pmt::mp(version));
}

void
metadata::add_phase_delay(pmt::pmt_t &m, uint64_t phase_delay)
{
  m = pmt::dict_add(m, pmt::mp(value(DECODER_PHASE_DELAY)),
                    pmt::mp(phase_delay));
}

void
metadata::add_resampling_ratio(pmt::pmt_t &m, float ratio)
{
  m = pmt::dict_add(m, pmt::mp(value(DECODER_RESAMPLING_RATIO)),
                    pmt::mp(ratio));
}

void
metadata::add_symbol_timing_error(pmt::pmt_t &m, double error)
{
  m = pmt::dict_add(m, pmt::mp(value(SYMBOL_TIMING_ERROR)),
                    pmt::from_double(error));
}


/**
 * Adds to the m the name and the version of the decoder dec
 * @param m reference to a PMT dictionary
 * @param dec pointer to a decoder object. If null the call of this method
 * has no effect
 */
void
metadata::add_decoder(pmt::pmt_t &m, const decoder *dec)
{
  if (!dec) {
    return;
  }
  m = pmt::dict_add(m, pmt::mp(value(DECODER_NAME)), pmt::mp(dec->name()));
  m = pmt::dict_add(m, pmt::mp(value(DECODER_VERSION)), pmt::mp(dec->version()));
}


nlohmann::json
metadata::to_json(const pmt::pmt_t &m)
{
  nlohmann::json j;
  pmt::pmt_t v = pmt::dict_ref(m, pmt::mp(value(PDU)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    uint8_t *b = (uint8_t *) pmt::blob_data(v);
    size_t len = pmt::blob_length(v);
    j[value(PDU)] = base64_encode(b, len);
  }

  v = pmt::dict_ref(m, pmt::mp(value(TIME)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(TIME)] = pmt::symbol_to_string(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(DECODER_CRC_VALID)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(DECODER_CRC_VALID)] = pmt::to_bool(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(SAMPLE_START)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(SAMPLE_START)] = pmt::to_uint64(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(SAMPLE_CNT)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(SAMPLE_CNT)] = pmt::to_uint64(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(DECODER_SYMBOL_ERASURES)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(DECODER_SYMBOL_ERASURES)] = pmt::to_uint64(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(DECODER_CORRECTED_BITS)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(DECODER_CORRECTED_BITS)] = pmt::to_uint64(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(CENTER_FREQ)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(CENTER_FREQ)] = pmt::to_double(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(FREQ_OFFSET)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(FREQ_OFFSET)] = pmt::to_double(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(SNR)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(SNR)] = pmt::to_float(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(ANTENNA_AZIMUTH)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(ANTENNA_AZIMUTH)] = pmt::to_double(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(ANTENNA_ELEVATION)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(ANTENNA_ELEVATION)] = pmt::to_double(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(ANTENNA_POLARIZATION)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(ANTENNA_POLARIZATION)] = pmt::symbol_to_string(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(DECODER_PHASE_DELAY)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(DECODER_PHASE_DELAY)] = pmt::to_uint64(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(DECODER_RESAMPLING_RATIO)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(DECODER_RESAMPLING_RATIO)] = pmt::to_float(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(DECODER_NAME)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(DECODER_NAME)] = pmt::symbol_to_string(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(DECODER_VERSION)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(DECODER_VERSION)] = pmt::symbol_to_string(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(SYMBOL_TIMING_ERROR)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    j[value(SYMBOL_TIMING_ERROR)] = pmt::to_double(v);
  }
  return j;
}

}  // namespace satnogs

}  // namespace gr
