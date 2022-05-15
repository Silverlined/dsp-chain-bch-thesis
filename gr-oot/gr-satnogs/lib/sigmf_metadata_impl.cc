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

#include <fstream>
#include <string>
#include <gnuradio/io_signature.h>
#include <satnogs/base64.h>
#include "sigmf_metadata_impl.h"

namespace gr {
namespace satnogs {

metadata::metadata_sptr
sigmf_metadata::make(const std::string &global_segment_json)
{
  return metadata::metadata_sptr(
           new sigmf_metadata_impl(global_segment_json));
}

sigmf_metadata_impl::sigmf_metadata_impl(std::string global_segment_json)
{
  initialize_global(global_segment_json);
}

sigmf_metadata_impl::~sigmf_metadata_impl()
{
}

void
sigmf_metadata_impl::transform(pmt::pmt_t &m)
{
  auto new_annotation = sigmf::Annotation<core::DescrT, antenna::DescrT,
       ::satnogs::DescrT> ();
  auto new_capture = sigmf::Capture<core::DescrT> ();

  pmt::pmt_t v;

  v = pmt::dict_ref(m, pmt::mp(value(DECODER_PHASE_DELAY)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    d_sigmf.global.access<::satnogs::GlobalT> ().decoder_phase =
      pmt::to_uint64(v);
  }

  v = pmt::dict_ref(m, pmt::mp(value(DECODER_RESAMPLING_RATIO)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    d_sigmf.global.access<::satnogs::GlobalT> ().decoder_resampling_ratio =
      pmt::to_float(v);
  }

  /*
   * According to the sigMF specification a capture segment must contain the
   * sample_start field. However, some fields that are described in the capture
   * segment of the sigMF (e.g sample_start and datetime) are also described in
   * the annotation segment. As long as we get the metadata keys from the
   * generic satnogs metadata pmt, we dont have a way to distinguish between
   * pmts that refer to captures or annotation. Thus, if we find the unique key
   * frequency (or CENTER_FREQ of satnogs metadata) we automatically assume
   * a capture segment and we also search for sample_start, datetime and
   * global_index in the received pmt.
   */
  v = pmt::dict_ref(m, pmt::mp(value(CENTER_FREQ)), pmt::PMT_NIL);
  // The key frequency of capture segment found inside received pmt.
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    new_capture.access<core::CaptureT> ().frequency =
      pmt::to_double(v);
    // Now search for datetime and sample_start
    v = pmt::dict_ref(m, pmt::mp(value(SAMPLE_START)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_capture.access<core::CaptureT> ().sample_start =
        pmt::to_uint64(v);
    }
    v = pmt::dict_ref(m, pmt::mp(value(TIME)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_capture.access<core::CaptureT> ().datetime =
        pmt::symbol_to_string(v);
    }
    d_sigmf.captures.emplace_back(new_capture);
  }

  /*
   * Following the same logic as with the capture segments, we distinguish the
   * annotation segments using the sample_count field. Thus, if we find the
   * sample_count key (or SAMPLE_CNT of satnogs metadata) inside the received
   * pmt, then we are sure that we need to append a new annotation segment. Then
   * we try to find every other annotation key inside the pmt.
   */
  v = pmt::dict_ref(m, pmt::mp(value(SAMPLE_CNT)), pmt::PMT_NIL);
  if (!pmt::equal(v, pmt::PMT_NIL)) {
    // The sample_count key of the annotation segment found inside pmt
    new_annotation.access<core::AnnotationT> ().sample_count =
      pmt::to_uint64(v);
    // Search for every other possible annotation key of different namespaces
    v = pmt::dict_ref(m, pmt::mp(value(PDU)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      uint8_t *b = (uint8_t *) pmt::blob_data(v);
      size_t len = pmt::blob_length(v);
      new_annotation.access<::satnogs::AnnotationT> ().pdu =
        base64_encode(b, len);
    }

    v = pmt::dict_ref(m, pmt::mp(value(TIME)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_annotation.access<::satnogs::AnnotationT> ().time =
        pmt::symbol_to_string(v);
    }

    v = pmt::dict_ref(m, pmt::mp(value(DECODER_CRC_VALID)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_annotation.access<::satnogs::AnnotationT> ().decoder_crc_valid =
        pmt::to_bool(v);
    }

    v = pmt::dict_ref(m, pmt::mp(value(SAMPLE_START)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_annotation.access<core::AnnotationT> ().sample_start =
        pmt::to_uint64(v);
    }

    v = pmt::dict_ref(m, pmt::mp(value(DECODER_SYMBOL_ERASURES)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_annotation.access<::satnogs::AnnotationT> ().decoder_symbol_erasures =
        pmt::to_uint64(v);
    }

    v = pmt::dict_ref(m, pmt::mp(value(DECODER_CORRECTED_BITS)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_annotation.access<::satnogs::AnnotationT> ().decoder_corrected_bits =
        pmt::to_uint64(v);
    }

    v = pmt::dict_ref(m, pmt::mp(value(FREQ_OFFSET)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_annotation.access<::satnogs::AnnotationT> ().frequency_offset =
        pmt::to_double(v);
    }

    v = pmt::dict_ref(m, pmt::mp(value(SNR)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_annotation.access<::satnogs::AnnotationT> ().snr = pmt::to_float(v);
    }

    v = pmt::dict_ref(m, pmt::mp(value(SYMBOL_TIMING_ERROR)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_annotation.access<::satnogs::AnnotationT> ().symbol_timing_error =
        pmt::to_double(v);
    }

    v = pmt::dict_ref(m, pmt::mp(value(DECODER_NAME)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_annotation.access<::satnogs::AnnotationT> ().decoder_name =
        pmt::symbol_to_string(v);
    }

    v = pmt::dict_ref(m, pmt::mp(value(DECODER_VERSION)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_annotation.access<::satnogs::AnnotationT> ().decoder_version =
        pmt::symbol_to_string(v);
    }

    v = pmt::dict_ref(m, pmt::mp(value(ANTENNA_AZIMUTH)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_annotation.access<antenna::AnnotationT> ().azimuth_angle =
        pmt::to_double(v);
    }

    v = pmt::dict_ref(m, pmt::mp(value(ANTENNA_ELEVATION)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_annotation.access<antenna::AnnotationT> ().elevation_angle =
        pmt::to_double(v);
    }

    v = pmt::dict_ref(m, pmt::mp(value(ANTENNA_POLARIZATION)), pmt::PMT_NIL);
    if (!pmt::equal(v, pmt::PMT_NIL)) {
      new_annotation.access<antenna::AnnotationT> ().polarization =
        pmt::symbol_to_string(v);
    }

    d_sigmf.annotations.emplace_back(new_annotation);
  }
}

void
sigmf_metadata_impl::to_file(const std::string &filename)
{
  std::ofstream f(filename);
  std::stringstream json_output;
  json_output << to_sting() << std::flush;
  f << json_output.rdbuf();
  f.close();
}

const std::string
sigmf_metadata_impl::to_sting()
{
  return json(d_sigmf).dump(2);
}

void
sigmf_metadata_impl::append_capture_segment(uint64_t sample_start)
{
  auto new_capture = sigmf::Capture<core::DescrT> ();
  new_capture.access<core::CaptureT> ().sample_start = sample_start;
  d_sigmf.captures.emplace_back(new_capture);
}

void
sigmf_metadata_impl::append_capture_segment(uint64_t sample_start,
    uint64_t global_index,
    double frequency,
    std::string datetime)
{
  auto new_capture = sigmf::Capture<core::DescrT> ();
  new_capture.access<core::CaptureT> ().sample_start = sample_start;
  new_capture.access<core::CaptureT> ().global_index = global_index;
  new_capture.access<core::CaptureT> ().frequency = frequency;
  new_capture.access<core::CaptureT> ().datetime = datetime;
  d_sigmf.captures.emplace_back(new_capture);
}

void
sigmf_metadata_impl::append_capture_segment(std::string capture_json)
{
  auto new_capture = sigmf::Capture<core::DescrT> ();
  new_capture.from_json(nlohmann::json::parse(capture_json));
  d_sigmf.captures.emplace_back(new_capture);
}

void
sigmf_metadata_impl::append_annotation_segment(uint64_t sample_start,
    uint64_t sample_count)
{
  auto new_annotation = sigmf::Annotation<core::DescrT, antenna::DescrT,
       ::satnogs::DescrT> ();
  new_annotation.access<core::AnnotationT> ().sample_start = sample_start;
  new_annotation.access<core::AnnotationT> ().sample_count = sample_count;
  d_sigmf.annotations.emplace_back(new_annotation);
}

void
sigmf_metadata_impl::append_annotation_segment(uint64_t sample_start,
    uint64_t sample_count,
    std::string generator,
    std::string comment,
    double freq_lower_edge,
    double freq_upper_edge)
{
  auto new_annotation = sigmf::Annotation<core::DescrT, antenna::DescrT,
       ::satnogs::DescrT> ();
  new_annotation.access<core::AnnotationT> ().sample_start = sample_start;
  new_annotation.access<core::AnnotationT> ().sample_count = sample_count;
  new_annotation.access<core::AnnotationT> ().generator = generator;
  new_annotation.access<core::AnnotationT> ().comment = comment;
  new_annotation.access<core::AnnotationT> ().freq_lower_edge =
    freq_lower_edge;
  new_annotation.access<core::AnnotationT> ().freq_upper_edge =
    freq_upper_edge;
  d_sigmf.annotations.emplace_back(new_annotation);
}

void
sigmf_metadata_impl::append_annotation_segment(std::string annotation_json)
{
  auto new_anno = sigmf::Annotation<core::DescrT, antenna::DescrT,
       ::satnogs::DescrT> ();
  new_anno.from_json(nlohmann::json::parse(annotation_json));
  d_sigmf.annotations.emplace_back(new_anno);
}

void
sigmf_metadata_impl::parse_json(std::string filename)
{
  std::ifstream f(filename);
  std::stringstream json_output;
  json_output << f.rdbuf();
  auto as_json = json::parse(json_output);
  d_sigmf = as_json;
}

sigmf::SigMF<sigmf::Global<core::DescrT, antenna::DescrT, ::satnogs::DescrT>,
      sigmf::Capture<core::DescrT>,
      sigmf::Annotation<core::DescrT, antenna::DescrT, ::satnogs::DescrT> > &
      sigmf_metadata_impl::get_sigmf()
{
  return d_sigmf;
}

void
sigmf_metadata_impl::initialize_global(const std::string global_segment_json)
{
  d_sigmf.global.from_json(json::parse(global_segment_json)["global"]);
}

} /* namespace satnogs */
} /* namespace gr */

