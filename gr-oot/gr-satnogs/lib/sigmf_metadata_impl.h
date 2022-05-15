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

#ifndef INCLUDE_SATNOGS_SIGMF_METADATA_IMPL_H
#define INCLUDE_SATNOGS_SIGMF_METADATA_IMPL_H

#include <satnogs/sigmf_metadata.h>
#include <libsigmf/sigmf.h>
#include <libsigmf/sigmf_antenna_generated.h>
#include <libsigmf/sigmf_satnogs_generated.h>


namespace gr {
namespace satnogs {

/*!
 * \brief <+description+>
 *
 */
class SATNOGS_API sigmf_metadata_impl : public sigmf_metadata {
public:
  sigmf_metadata_impl(std::string global_segment_json);

  ~sigmf_metadata_impl();

  metadata::metadata_sptr
  make(const std::string &global_segment_json);

  void
  transform(pmt::pmt_t &m);

  void
  to_file(const std::string &filename);


  const std::string
  to_sting();

  void
  append_capture_segment(uint64_t sample_start);

  void
  append_capture_segment(uint64_t sample_start, uint64_t global_index,
                         double frequency, std::string datetime);

  void
  append_capture_segment(std::string capture_json);

  void
  append_annotation_segment(uint64_t sample_start, uint64_t sample_count);

  void
  append_annotation_segment(uint64_t sample_start, uint64_t sample_count,
                            std::string generator, std::string comment,
                            double freq_lower_edge,
                            double freq_upper_edge);

  void
  append_annotation_segment(std::string annotation_json);

  void
  parse_json(std::string filename);

  sigmf::SigMF <
  sigmf::Global<core::DescrT, antenna::DescrT, ::satnogs::DescrT>,
        sigmf::Capture<core::DescrT>,
        sigmf::Annotation<core::DescrT, antenna::DescrT, ::satnogs::DescrT> > &
        get_sigmf();

private:

  sigmf::SigMF <
  sigmf::Global<core::DescrT, antenna::DescrT, ::satnogs::DescrT>,
        sigmf::Capture<core::DescrT>,
        sigmf::Annotation<core::DescrT, antenna::DescrT, ::satnogs::DescrT> > d_sigmf;

  std::string d_metafile_name;
  std::string d_metadata_name;

  void
  initialize_global(const std::string global_segment_json);
};

} // namespace satnogs
} // namespace gr

#endif /* INCLUDE_SATNOGS_SIGMF_METADATA_IMPL_H */

