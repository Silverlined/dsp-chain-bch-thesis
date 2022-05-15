#! /usr/bin/python
#
# gr-satnogs: SatNOGS GNU Radio Out-Of-Tree Module
#
#  Copyright (C) 2018
#  Libre Space Foundation <http://librespacefoundation.org/>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>
#

def lcm(p,q):
    p, q = abs(p), abs(q)
    m = p * q
    if not m: return 0
    while True:
        p %= q
        if not p: return m // q
        q %= p
        if not q: return m // p

def find_decimation(baudrate, min_decimation=4, audio_samp_rate=48e3,
                    multiple = 2):
    while(min_decimation * baudrate < audio_samp_rate):
        min_decimation = min_decimation + 1
    # Because we use a clock recovery that decimates,
    # we need a number that is multiple of the clock recovery decimation
    if min_decimation % multiple:
        min_decimation = min_decimation + multiple - min_decimation % multiple
    return min_decimation