#!/usr/bin/env python3

import logging
import sys

import matplotlib
import numpy as np

matplotlib.use('Agg')

import matplotlib.pyplot as plt  # isort:skip # noqa: E402 # pylint: disable=C0411,C0412,C0413

from datetime import datetime

LOGGER = logging.getLogger(__name__)

OFFSET_IN_STDS = -2.0
SCALE_IN_STDS = 8.0


class EmptyArrayError(Exception):
    """
    Empty data array exception
    """


def _read_waterfall(datafile_path):
    """
    Read waterfall data file

    :param datafile_path: Path to data file
    :type datafile_path: str
    :return: Waterfall data
    :rtype: dict
    """
    LOGGER.info('Reading waterfall file')

    datafile = open(datafile_path, mode='rb')

    waterfall = {
        'timestamp': np.fromfile(datafile, dtype='|S32', count=1)[0],
        'nchan': np.fromfile(datafile, dtype='>i4', count=1)[0],
        'samp_rate': np.fromfile(datafile, dtype='>i4', count=1)[0],
        'nfft_per_row': np.fromfile(datafile, dtype='>i4', count=1)[0],
        'center_freq': np.fromfile(datafile, dtype='>f4', count=1)[0],
        'endianess': np.fromfile(datafile, dtype='>i4', count=1)[0]
    }
    data_dtypes = np.dtype([('tabs', 'int64'), ('spec', 'float32', (waterfall['nchan'], ))])
    waterfall['data'] = np.fromfile(datafile, dtype=data_dtypes)
    if waterfall['data'].size == 0:
        raise EmptyArrayError

    datafile.close()

    return waterfall


def _compress_waterfall(waterfall):
    """
    Compress spectra of waterfall

    :param waterfall: Watefall data
    :type waterfall: dict
    :return: Compressed spectra
    :rtype: dict
    """
    spec = waterfall['data']['spec']
    std = np.std(spec, axis=0)
    offset = np.mean(spec, axis=0) + OFFSET_IN_STDS * std
    scale = SCALE_IN_STDS * std / 255.0
    values = np.clip((spec - offset) / scale, 0.0, 255.0).astype('uint8')

    return {'offset': offset, 'scale': scale, 'values': values}


def _get_waterfall(datafile_path):
    """
    Get waterfall data

    :param datafile_path: Path to data file
    :type datafile_path: str_array
    :return: Waterfall data including compressed data
    :rtype: dict
    """
    waterfall = _read_waterfall(datafile_path)

    nint = waterfall['data']['spec'].shape[0]
    waterfall['trel'] = np.arange(nint) * waterfall['nfft_per_row'] * waterfall['nchan'] / float(
        waterfall['samp_rate'])
    waterfall['freq'] = np.linspace(-0.5 * waterfall['samp_rate'],
                                    0.5 * waterfall['samp_rate'],
                                    waterfall['nchan'],
                                    endpoint=False)
    waterfall['compressed'] = _compress_waterfall(waterfall)

    return waterfall


class Waterfall():  # pylint: disable=R0903
    """
    Parse waterfall data file

    :param datafile_path: Path to data file
    :type datafile_path: str_array
    """
    def __init__(self, datafile_path):
        """
        Class constructor
        """
        self.data = _get_waterfall(datafile_path)

    def plot(self, figure_path, vmin=None, vmax=None):
        """
        Plot waterfall into a figure

        :param figure_path: Path of figure file to save
        :type figure_path: str
        :param value_range: Minimum and maximum value range
        :type value_range: tuple
        """
        tmin = np.min(self.data['data']['tabs'] / 1000000.0)
        tmax = np.max(self.data['data']['tabs'] / 1000000.0)
        fmin = np.min(self.data['freq'] / 1000.0)
        fmax = np.max(self.data['freq'] / 1000.0)
        if vmin is None or vmax is None:
            vmin = -100
            vmax = -50
            c_idx = self.data['data']['spec'] > -200.0
            if np.sum(c_idx) > 100:
                data_mean = np.mean(self.data['data']['spec'][c_idx])
                data_std = np.std(self.data['data']['spec'][c_idx])
                vmin = data_mean - 2.0 * data_std
                vmax = data_mean + 6.0 * data_std
        plt.figure(figsize=(10, 20))
        plt.imshow(self.data['data']['spec'],
                   origin='lower',
                   aspect='auto',
                   interpolation='None',
                   extent=[fmin, fmax, tmin, tmax],
                   vmin=vmin,
                   vmax=vmax,
                   cmap='viridis')
        plt.xlabel('Frequency (kHz)')
        plt.ylabel('Time (seconds)')
        fig = plt.colorbar(aspect=50)
        fig.set_label('Power (dB)')
        plt.savefig(figure_path, bbox_inches='tight')
        plt.close()


if __name__ == "__main__":
    wat = Waterfall(sys.argv[1])
    timestamp = datetime.now().strftime("%m-%d-%YT%H-%M-%S")

    wat.plot("waterfall_"+timestamp+".png")
