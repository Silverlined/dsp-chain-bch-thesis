#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2019 Grant Iraci.
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#


import numpy
from gnuradio import gr
import pmt
import array

class var_len_packet_handler(gr.basic_block):
    
    STATE_SYNC = 0
    STATE_LEN = 1
    STATE_PAYLOAD = 2

    def __init__(self, sync, has_crc, check_crc, crc_poly=0x8005):
        gr.sync_block.__init__(self,
            name="var_len_packet_handler",
            in_sig=[numpy.uint8],
            out_sig=None)

        self.message_port_register_out(pmt.intern('out'))

        self.reg = 0
        self.sync = sync
        self.length = 0
        self.has_crc = has_crc
        self.check_crc = check_crc
        self.crc_poly = crc_poly
        self.state = self.STATE_SYNC

    def crc(self, data):
        reg = 0xFFFF
        for x in data:
            for _ in range(0, 8):
                 if(((reg & 0x8000) >> 8) ^ (x & 0x80)):
                     reg = ((reg << 1) ^ self.crc_poly ) & 0xffff
                 else:
                     reg = (reg << 1) & 0xffff
                 x = (x << 1) & 0xff
        return reg

    def send_message(self, msg):
        self.message_port_pub(pmt.intern('out'),
            pmt.cons(pmt.PMT_NIL, pmt.init_u8vector(len(msg), msg)))

    def work(self, input_items, output_items):

        in0 = input_items[0]

        for x in in0:

            if (self.state == self.STATE_PAYLOAD):
                if (self.index < self.length * 8 + (16 if self.has_crc else 0)):
                    self.buf[self.index] = x
                    self.index += 1
                if (self.index == self.length * 8 + (16 if self.has_crc else 0)):
                    self.state = self.STATE_SYNC
                    self.reg = 0
                    packed = numpy.packbits(self.buf)

                    if (self.has_crc):
                        arr = array.array('B', packed[:-2])
                    else:
                        arr = array.array('B', packed)

                    if (self.check_crc):
                        rx = (packed[-2] << 8) + packed[-1]
                        calc = self.crc([self.length] + packed[:-2].tolist())
                        if (calc == rx):
                            print('[CRC MATCH] OK')
                            self.send_message(arr)
                        else:
                            print('[crc mismatch] calc: ' + hex(calc)
                                    + '\trx: ' + hex(rx))
                    else:
                        self.send_message(arr)


            elif (self.state == self.STATE_SYNC):
                self.reg = ((self.reg << 1) & 0xFFFF) | x
                if (self.reg == self.sync):
                    self.state = self.STATE_LEN
                    self.index = 0
                    self.reg = 0
                    continue


            elif (self.state == self.STATE_LEN):
                self.reg = (self.reg << 1) | x
                self.index += 1
                if (self.index == 8):
                    self.length = self.reg
                    self.state = self.STATE_PAYLOAD
                    self.index = 0
                    self.buf = numpy.zeros(self.length * 8 +
                                (16 if self.has_crc else 0), dtype=numpy.uint8)



            

            


            

            
        return len(input_items[0])
