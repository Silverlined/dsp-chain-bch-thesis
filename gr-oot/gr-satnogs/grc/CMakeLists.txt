# Copyright 2011 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA

list(APPEND debug_blocks
    satnogs_cw_encoder.block.yml
    satnogs_cw_to_symbol.block.yml
    satnogs_debug_msg_source_raw.block.yml
    satnogs_debug_msg_source.block.yml
    satnogs_morse_debug_source.block.yml
)

list(APPEND enabled_blocks
    satnogs_amsat_duv_decoder.block.yml
    satnogs_argos_ldr_decoder.block.yml
    satnogs_ax25_decoder.block.yml
    satnogs_ax25_encoder.block.yml
    satnogs_ax100_mode5_decoder.block.yml
    satnogs_ax100_mode6_decoder.block.yml
    satnogs_ax100_mode5_encoder.block.yml
    satnogs_ax100_mode6_encoder.block.yml
    satnogs_usp_encoder.block.yml
    satnogs_ber_calculator.block.yml
    satnogs_coarse_doppler_correction_cc.block.yml
    satnogs_constellation_modulator.block.yml
    satnogs_crc_async.block.yml
    satnogs_cw_decoder.block.yml
    satnogs_doppler_compensation.block.yml
    satnogs_doppler_correction_cc.block.yml
    satnogs_frame_decoder.block.yml
    satnogs_frame_encoder.block.yml
    satnogs_frame_file_sink.block.yml
    satnogs_ieee802_15_4_encoder.block.yml
    satnogs_ieee802_15_4_variant_decoder.block.yml
    satnogs_metadata_sink.block.yml
    satnogs_iq_sink.block.yml
    satnogs_json_converter.block.yml
    satnogs_lrpt_decoder.block.yml
    satnogs_lrpt_sync.block.yml
    satnogs_morse_decoder.block.yml
    satnogs_multi_format_msg_sink.block.yml
    satnogs_noaa_apt_sink.block.yml
    satnogs_ogg_encoder.block.yml
    satnogs_ogg_source.block.yml
    satnogs_rs_encoder.block.yml
    satnogs_sigmf_metadata.block.yml
    satnogs_sstv_pd120_sink.block.yml
    satnogs_tcp_rigctl_msg_source.block.yml
    satnogs_udp_msg_sink.block.yml
    satnogs_udp_msg_source.block.yml
    satnogs_waterfall_sink.block.yml
    satnogs_whitening_ccsds.block.yml
    satnogs_whitening_g3ruh.block.yml
    satnogs_whitening.block.yml
    satnogs.tree.yml
)

if(${INCLUDE_DEBUG_BLOCKS})
	list(APPEND enabled_blocks ${debug_blocks})
endif()

install(FILES
    ${enabled_blocks}
    DESTINATION share/gnuradio/grc/blocks
)
