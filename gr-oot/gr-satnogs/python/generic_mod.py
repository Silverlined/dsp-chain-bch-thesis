#
# Copyright 2005,2006,2007,2009,2011 Free Software Foundation, Inc.
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
# Boston, MA 02110-1301, USA.
#

"""
Generic modulation and demodulation.
"""

# See gnuradio-examples/python/digital for examples

from __future__ import print_function
from __future__ import absolute_import
from __future__ import unicode_literals

from gnuradio import gr, blocks, filter, analog, digital
import math


# default values (used in __init__ and add_options)
_def_samples_per_symbol = 2
_def_excess_bw = 0.35
_def_verbose = False
_def_log = False

# Frequency correction
_def_freq_bw = 2*math.pi/100.0
# Symbol timing recovery
_def_timing_bw = 2*math.pi/100.0
_def_timing_max_dev = 1.5
# Fine frequency / Phase correction
_def_phase_bw = 2*math.pi/100.0
# Number of points in constellation
_def_constellation_points = 16
# Whether differential coding is used.
_def_differential = False


# /////////////////////////////////////////////////////////////////////////////
#                             Generic modulator
# /////////////////////////////////////////////////////////////////////////////

class generic_mod(gr.hier_block2):
    """
    Hierarchical block for RRC-filtered differential generic modulation.

    The input is a byte stream (unsigned char) and the
    output is the complex modulated signal at baseband.

    Args:
        constellation: determines the modulation type (gnuradio.digital.digital_constellation)
        samples_per_symbol: samples per baud >= 2 (float)
        differential: whether to use differential encoding (boolean)
        pre_diff_code: whether to use apply a pre-differential mapping (boolean)
        excess_bw: Root-raised cosine filter excess bandwidth (float)
        verbose: Print information about modulator? (boolean)
        log: Log modulation data to files? (boolean)
    """

    def __init__(self, constellation,
                 differential=_def_differential,
                 samples_per_symbol=_def_samples_per_symbol,
                 pre_diff_code=True,
                 excess_bw=_def_excess_bw):

        gr.hier_block2.__init__(self, "generic_mod",
                                gr.io_signature(1, 1, gr.sizeof_char),       # Input signature
                                gr.io_signature(1, 1, gr.sizeof_gr_complex)) # Output signature

        self._constellation = constellation
        self._samples_per_symbol = samples_per_symbol
        self._excess_bw = excess_bw
        self._differential = differential
        # Only apply a predifferential coding if the constellation also supports it.
        self.pre_diff_code = pre_diff_code and self._constellation.apply_pre_diff_code()

        if self._samples_per_symbol < 2:
            raise TypeError("sps must be >= 2, is %f" % self._samples_per_symbol)

        arity = pow(2,self.bits_per_symbol())

        # turn bytes into k-bit vectors
        self.bytes2chunks = \
            blocks.packed_to_unpacked_bb(self.bits_per_symbol(), gr.GR_MSB_FIRST)

        if self.pre_diff_code:
            self.symbol_mapper = digital.map_bb(self._constellation.pre_diff_code())

        if differential:
            self.diffenc = digital.diff_encoder_bb(arity)

        self.chunks2symbols = digital.chunks_to_symbols_bc(self._constellation.points())

        # pulse shaping filter
        nfilts = 32
        ntaps = nfilts * 11 * int(self._samples_per_symbol)    # make nfilts filters of ntaps each
        self.rrc_taps = filter.firdes.root_raised_cosine(
            nfilts,          # gain
            nfilts,          # sampling rate based on 32 filters in resampler
            1.0,             # symbol rate
            self._excess_bw, # excess bandwidth (roll-off factor)
            ntaps)
        self.rrc_filter = filter.pfb_arb_resampler_ccf(self._samples_per_symbol,
                                                       self.rrc_taps)
        # Declare the group delay so the scheduler can properly propagate the
        # tags
        self.rrc_filter.declare_sample_delay((11 * self._samples_per_symbol) // 2)

        # Connect
        self._blocks = [self, self.bytes2chunks]
        if self.pre_diff_code:
            self._blocks.append(self.symbol_mapper)
        if differential:
            self._blocks.append(self.diffenc)
        self._blocks += [self.chunks2symbols, self.rrc_filter, self]
        self.connect(*self._blocks)
    
    def samples_per_symbol(self):
        return self._samples_per_symbol

    def bits_per_symbol(self):   # static method that's also callable on an instance
        return self._constellation.bits_per_symbol()
    
