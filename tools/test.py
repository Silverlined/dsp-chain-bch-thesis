import sys
import numpy as np
from matplotlib import pyplot as plt

sys.path.append("./DigiCommPy")
from DigiCommPy.chapter_1.demo_scripts import sine_wave_demo
#from DigiCommPy.chapter_2.passband_modulations import gmsk_mod
from DigiCommPy.chapter_2.psd_estimates import gmsk_psd


a = np.array([0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,1,1,0,1,1,0,1,0,1,0,0,0,0,0,1,0,1,0])
#gmsk_mod(a, 5, 8, 0.5, enable_plot=True)
gmsk_psd()
plt.show()


# Interpolation is just a special type of convolution
# Integration is a low-pass filter

