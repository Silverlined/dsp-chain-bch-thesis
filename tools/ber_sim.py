#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as plt
import scipy.signal
import scipy.special
from gnuradio import gr, digital, analog, blocks, fft, channels
from gnuradio.fft import window
from satellites.components.demodulators import bpsk_demodulator, fsk_demodulator
from UHF_GMSK_Demod_Hier import UHF_GMSK_Demod_Hier

###################################################
###               Abbreviations                 ###
### PR   - Pseudo Random
### PRN  - Pseudo Random Number
### MLS  - Maximum-length Sequence Generator
### LFSR - Linear-feedback Shift Register
### FFT  - Fast Fourier Transform
###################################################
###                 Methods                     ###
### Cross-correlation - Used to find a short pattern in a long sequence
### This simulation performs cross-correlation via multiplication in the frequency domain via FFTs
###################################################

###################################################
###            Variables & Constants            ###
###################################################
RAND_SEED = 43

g_test_type = "PER"
# g_test_type = "BER"
g_tex_formula = r"$1 - \left(1 - BER\right)^{1024}$"
#g_tex_formula = r"$\frac{1}{2}\exp{\left(-\frac{E_b}{2N_0}\right)}$"

### Software demodulator, tunning parameters
g_bt = 0.5
g_ted_gain = 1.47
g_clk_bw = 0.1
g_clk_limit = 0.004
g_lfsr_bits = 10
g_lp_cutoff = 9600
g_lp_decimation = 5

g_sample_rate = 96e3
g_sps = 10
g_deviation = 2400

###################################################
###           BER Simulation, flowgraph         ###
###################################################
class BERSim(gr.top_block):
    def __init__(self, ber_block, ncorrelations, lfsr_bits = g_lfsr_bits):
        gr.top_block.__init__(self)

        prn_len = 2**lfsr_bits
        prn = np.concatenate((scipy.signal.max_len_seq(lfsr_bits)[0], [1]))
        prn_fft_conj = np.conjugate(np.fft.fft(2*prn-1))

        self.source = blocks.vector_source_b(prn, True, 1, [])
        
        self.ber_block = ber_block
        
        self.char2float = blocks.char_to_float(1, 0.5)
        self.add_const = blocks.add_const_ff(-1.0)
        self.source_vector = blocks.stream_to_vector(gr.sizeof_float, prn_len)

        self.fft = fft.fft_vfc(prn_len, True, window.rectangular(prn_len), 1)
        self.prn_fft_source = blocks.vector_source_c(prn_fft_conj, True, prn_len, [])
        self.multiply_ffts = blocks.multiply_vcc(prn_len)
        self.ifft = fft.fft_vcc(prn_len, False, np.ones(prn_len)/prn_len**2, False, 1)
        self.corr_mag = blocks.complex_to_mag(prn_len)
        self.max_corr = blocks.max_ff(prn_len, 1)
        self.multiply_const = blocks.multiply_const_ff(-0.5)
        self.add_const2 = blocks.add_const_ff(0.5)
        
        self.head = blocks.head(gr.sizeof_float, ncorrelations)
        self.sink = blocks.vector_sink_f()

        self.connect(self.source, self.ber_block, self.char2float, self.add_const,
                         self.source_vector, self.fft, (self.multiply_ffts,0),
                         self.ifft, self.corr_mag, self.max_corr, self.multiply_const,
                         self.add_const2, self.head, self.sink)
        self.connect(self.prn_fft_source, (self.multiply_ffts,1))


###################################################
###            BPSK Demod, flowgraph            ###
###################################################
class BPSK(gr.hier_block2):
    def __init__(self, ebn0, nbits):
        gr.hier_block2.__init__(self, 'BPSK',
            gr.io_signature(1, 1, gr.sizeof_char),
            gr.io_signature(1, 1, gr.sizeof_char))

        self.head = blocks.head(gr.sizeof_char, nbits)
        self.pack = blocks.pack_k_bits_bb(8)
        self.bpsk_constellation = digital.constellation_bpsk().base()
        self.modulator = digital.generic_mod(
            constellation = self.bpsk_constellation,
            differential = False,
            samples_per_symbol = g_sps,
            pre_diff_code = True,
            excess_bw = 0.35,
            verbose = False,
            log = False)
        
        spb = g_sps
        self.channel = channels.channel_model(np.sqrt(spb)/10**(ebn0/20), 0, 1.0, [1], RAND_SEED, False)

        self.demod = bpsk_demodulator(g_sample_rate/g_sps, g_sample_rate, iq = True)
        self.slice = digital.binary_slicer_fb()
        self.connect(self, self.head, self.pack, self.modulator, self.channel,
                         self.demod, self.slice, self)

###################################################
###            GFSK Demod, flowgraph            ###
###################################################
class FSK(gr.hier_block2):
    def __init__(self, ebn0, nbits):
        gr.hier_block2.__init__(self, 'FSK',
            gr.io_signature(1, 1, gr.sizeof_char),
            gr.io_signature(1, 1, gr.sizeof_char))

        self.head = blocks.head(gr.sizeof_char, nbits)
        self.pack = blocks.pack_k_bits_bb(8)
        self.modulator = digital.gmsk_mod(
            samples_per_symbol = g_sps,
            bt = g_bt,
            verbose = False,
            log = False)
        
        spb = g_sps
        self.channel = channels.channel_model(np.sqrt(spb)/10**(ebn0/20), 0, 1.0, [1], RAND_SEED, False)

        #self.demod = fsk_demodulator(g_sample_rate/g_sps, g_sample_rate, g_deviation = g_deviation, iq = True)
        self.demod = UHF_GMSK_Demod_Hier(ted_gain=g_ted_gain, clk_bw=g_clk_bw, lp_decimation=g_lp_decimation)
        self.slice = digital.binary_slicer_fb()

        self.connect(self, self.head, self.pack, self.modulator, self.channel,
                         self.demod, self.slice, self)

def compute_ber(ber_block_class, ebn0, drop_correlations = 2):
    ncorrelations = 100000
    nbits = int( ncorrelations * 2 ** g_lfsr_bits * 1.1 ) # 1.1 to add a bit of margin
    gr_flowgraph = BERSim( ber_block_class( ebn0, nbits ), ncorrelations )
    print(f'Computing FSK {g_test_type} for EbN0 = {ebn0:.01f} dB')
    gr_flowgraph.run()
    return np.average(gr_flowgraph.sink.data()[drop_correlations:])

def ber_bpsk_awgn(ebn0):
    """Calculates theoretical bit error rate in AWGN (for BPSK and given Eb/N0)"""
    return 0.5 * scipy.special.erfc( 10 ** ( ebn0 / 20 ) )

def ber_fsk_awgn(ebn0):
    """Calculates theoretical bit error rate in AWGN (for FSK and given Eb/N0)"""
    return 0.5 * np.exp( -0.5 * 10 ** ( ebn0 / 10 ) )

def plot_bpsk():
    ebn0s = np.arange(-2, 10.5, 0.5)
    bers = [compute_ber(BPSK, ebn0) for ebn0 in ebn0s]
    bers_theory = [ber_bpsk_awgn(ebn0) for ebn0 in ebn0s]

    fig, ax = plt.subplots()
    ax.semilogy(ebn0s, bers_theory, '.-', label = 'Theoretical BPSK BER')
    ax.semilogy(ebn0s, bers, '.-', label = 'gr-satellites demodulator')
    ax.set_title('BPSK BER')
    ax.set_xlabel('Eb/N0 (dB)')
    ax.set_ylabel('BER')
    ax.legend()
    ax.grid()
    fig.savefig('ber_bpsk.png')
    print('Saved output to ber_bpsk.png')

def plot_fsk():
    ebn0s = np.arange(5, 23, 0.5)
    bers = [compute_ber(FSK, ebn0) for ebn0 in ebn0s]
    bers_theory = [ber_fsk_awgn(ebn0) for ebn0 in ebn0s]

    ### Convert to PER
    if (g_test_type == "PER"):
        bers = [1 - (1 - ber) ** 1024 for ber in bers]
        bers_theory = [1 - (1 - ber_theory) ** 1024 for ber_theory in bers_theory]

    print("GNUR:")
    for ber, ebn0 in zip(bers, ebn0s):
        print(f"PER {ber:.2e} at Eb/No: {ebn0:.2f} dB")

    print("Theory:")
    for ber, ebn0 in zip(bers_theory, ebn0s):
        print(f"PER {ber:.2e} at Eb/No: {ebn0:.2f} dB")

    fig, ax = plt.subplots()
    ax.semilogy(ebn0s, bers_theory, '.-', label = f'Theoretical non-coherent FSK {g_test_type}\n {g_tex_formula}')
    ax.semilogy(ebn0s, bers, '.-', label = f'GNURadio Software Demodulator')
    ax.set_title(f'FSK {g_test_type}')
    ax.set_xlabel('Eb/N0 (dB)')
    #ax.set_ylabel(r'$\log_{10}$' f'{g_test_type}')
    ax.set_ylabel(f'{g_test_type}')
    if ( g_test_type == "BER" ):  
        ax.set_ylim([10e-9, 10e-1])
    elif ( g_test_type == "PER" ):
        ax.set_ylim([10e-6, 10e-1])
    ax.legend()
    ax.grid()
    fig.savefig(f'results/{g_test_type}/bt{g_bt}/{g_test_type}_fsk_TED{g_ted_gain}_CLK{g_clk_bw}_CLKLIMIT{g_clk_limit}_BT{g_bt}_LPCUTOFF{g_lp_cutoff}_LFSRbits{g_lfsr_bits}_LPdecimation{g_lp_decimation}.png')
    print('Saved output to ber_fsk.png')

if __name__ == '__main__':
    #print('Computing BPSK BER')
    #plot_bpsk()

    print(f'Computing FSK {g_test_type}')
    plot_fsk()    

    exit(0)
