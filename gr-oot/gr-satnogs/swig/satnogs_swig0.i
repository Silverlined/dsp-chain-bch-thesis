/* -*- c++ -*- */

#define SATNOGS_API

%include <typemaps.i>
%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "satnogs_swig0_doc.i"


%template(whitening_sptr) boost::shared_ptr<gr::satnogs::whitening>;
%nodefaultctor gr::satnogs::decoder;
%template(decoder_sptr) boost::shared_ptr<gr::satnogs::decoder>;

%nodefaultctor gr::satnogs::encoder;
%template(encoder_sptr) boost::shared_ptr<gr::satnogs::encoder>;

%nodefaultctor gr::satnogs::metadata;
%template(metadata_sptr) boost::shared_ptr<gr::satnogs::metadata>;

%{
#include "satnogs/amsat_duv_decoder.h"
#include "satnogs/argos_ldr_decoder.h"
#include "satnogs/ax100_decoder.h"
#include "satnogs/ax100_encoder.h"
#include "satnogs/usp_encoder.h"
#include "satnogs/ax25_decoder.h"
#include "satnogs/ax25_encoder.h"
#include "satnogs/ber_calculator.h"
#include "satnogs/morse_tree.h"
#include "satnogs/multi_format_msg_sink.h"
#include "satnogs/cw_decoder.h"
#include "satnogs/udp_msg_source.h"
#include "satnogs/tcp_rigctl_msg_source.h"
#include "satnogs/decoder.h"
#include "satnogs/doppler_correction_cc.h"
#include "satnogs/encoder.h"
#include "satnogs/frame_decoder.h"
#include "satnogs/frame_encoder.h"
#include "satnogs/whitening.h"
#include "satnogs/udp_msg_sink.h"
#include "satnogs/coarse_doppler_correction_cc.h"
#include "satnogs/waterfall_sink.h"
#include "satnogs/ogg_encoder.h"
#include "satnogs/ogg_source.h"
#include "satnogs/noaa_apt_sink.h"
#include "satnogs/sstv_pd120_sink.h"
#include "satnogs/frame_file_sink.h"
#include "satnogs/metadata.h"
#include "satnogs/metadata_sink.h"
#include "satnogs/iq_sink.h"
#include "satnogs/json_converter.h"
#include "satnogs/lrpt_decoder.h"
#include "satnogs/lrpt_sync.h"
#include "satnogs/crc.h"
#include "satnogs/crc_async.h"
#include "satnogs/ieee802_15_4_encoder.h"
#include "satnogs/ieee802_15_4_variant_decoder.h"
#include "satnogs/rs_encoder.h"
#include "satnogs/sigmf_metadata.h"
%}


%include "satnogs/whitening.h"
%include "satnogs/morse_tree.h"
%include "satnogs/decoder.h"
%include "satnogs/encoder.h"
%include "satnogs/amsat_duv_decoder.h"
%include "satnogs/argos_ldr_decoder.h"
%include "satnogs/ax25_decoder.h"
%include "satnogs/ax25_encoder.h"
%include "satnogs/ber_calculator.h"
%include "satnogs/cw_decoder.h"
%include "satnogs/metadata.h"
%include "satnogs/rs_encoder.h"
%include "satnogs/sigmf_metadata.h"


/* crc.h should come first. All classes using it should be included afterwards */ 
%include "satnogs/crc.h"
%include "satnogs/ieee802_15_4_encoder.h"
%include "satnogs/ieee802_15_4_variant_decoder.h"
%include "satnogs/ax100_decoder.h"
%include "satnogs/ax100_encoder.h"
%include "satnogs/usp_encoder.h"

%include "satnogs/ber_calculator.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, ber_calculator);

%include "satnogs/crc_async.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, crc_async);

%include "satnogs/multi_format_msg_sink.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, multi_format_msg_sink);

%include "satnogs/udp_msg_source.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, udp_msg_source);

%include "satnogs/tcp_rigctl_msg_source.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, tcp_rigctl_msg_source);

%include "satnogs/frame_decoder.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, frame_decoder);

%include "satnogs/frame_encoder.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, frame_encoder);

%include "satnogs/doppler_correction_cc.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, doppler_correction_cc);

%include "satnogs/udp_msg_sink.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, udp_msg_sink);

%include "satnogs/coarse_doppler_correction_cc.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, coarse_doppler_correction_cc);

%include "satnogs/waterfall_sink.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, waterfall_sink);

%include "satnogs/ogg_encoder.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, ogg_encoder);

%include "satnogs/ogg_source.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, ogg_source);

%include "satnogs/noaa_apt_sink.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, noaa_apt_sink);

%include "satnogs/sstv_pd120_sink.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, sstv_pd120_sink);

%include "satnogs/frame_file_sink.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, frame_file_sink);

%include "satnogs/metadata_sink.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, metadata_sink);

%include "satnogs/iq_sink.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, iq_sink);

%include "satnogs/json_converter.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, json_converter);

%include "satnogs/lrpt_sync.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, lrpt_sync);

%include "satnogs/lrpt_decoder.h"
GR_SWIG_BLOCK_MAGIC2(satnogs, lrpt_decoder);

