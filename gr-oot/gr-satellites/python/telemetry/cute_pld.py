#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2021 The Regents of the University of Colorado
#
# This file is part of gr-satellites
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

from ..adapters import AffineAdapter, LinearAdapter
from construct import Adapter, BitsInteger, BitStruct, Container, Enum, Flag, \
                      Float32b, Int8ub, Int16ub, Int16sb, Int32ub, Padding, \
                      Struct

cute_pld_ccsds_header = BitStruct(
    'ccsds_version' / BitsInteger(3),
    'packet_type' / BitsInteger(1),
    'secondary_header_flag' / BitsInteger(1),
    'APID' / BitsInteger(11),
    'sequence_flags' / BitsInteger(2),
    'sequence_counter' / BitsInteger(14),
    'packet_length' / BitsInteger(16)
)

cute_pld_sw_stat = Struct(
    'ccsds_header' / cute_pld_ccsds_header,
    'shCoarse' / Int32ub,
    'shFine' / Int16ub,
    'pldSwVerMaj' / Int8ub,
    'pldSwVerMin' / Int8ub,
    'pldSwVerPatch' / Int8ub,
    'sdState' / BitStruct(
        Padding(6),
        'card1' / Enum(BitsInteger(1), DEAD=0, ALIVE=1),
        'card0' / Enum(BitsInteger(1), DEAD=0, ALIVE=1)
    ),
    'zynqTemp' / AffineAdapter(130.039011703511, 35520.15604681404, Int16ub),
    'zynqVccInt' / LinearAdapter(21739.1304347826, Int16ub),
    'zynqVccAux' / LinearAdapter(21739.1304347826, Int16ub),
    'zynqVccBram' / LinearAdapter(21739.1304347826, Int16ub),
    'zynqVccPint' / LinearAdapter(21739.1304347826, Int16ub),
    'zynqVccPAux' / LinearAdapter(21739.1304347826, Int16ub),
    'zynq_vccPdr0' / LinearAdapter(21739.1304347826, Int16ub),
    'zynqStatus' / Int8ub,
    'spare8' / Int8ub,
    'procTemp' / LinearAdapter(256.0, Int16sb),
    'ccdP5' / LinearAdapter(620.4551659097114, Int16ub),
    'ccdP15' / LinearAdapter(223.5870972357927, Int16ub),
    'ccdP32' / LinearAdapter(112.8099029045165, Int16ub),
    'ccdN5' / AffineAdapter(318.9131440052302, 2305.604898505892, Int16ub),
    'spare16' / Int16ub,
    'cmdRecvCount' / Int16ub,
    'cmdRjctCount' / Int16ub,
    'cmdSuccCount' / Int16ub,
    'cmdSuccOp' / Enum(Int16ub, NOOP=0, ARM=1, CMD_RST_STATS=2, CMD_XSUM=3,
                       CMD_ECHO_STATE=4, LOG_ROUTE=48, LOG_STATE=49,
                       LOG_RST_STATS=50, LOG_RESET_READ=52, LOG_ISSUE=53,
                       LOG_DUMP_INFO=54, LOG_SET_PUBLISH=55, MEM_DUMP=80,
                       MEM_LOAD=81, MEM_ERASE=82, MEM_XSUM=83, MEM_ABORT=84,
                       MEM_RESET=85, MEM_LOAD_DWORD=86, MEM_LOAD_WORD=87,
                       MEM_LOAD_BYTE=88, PKT_ISSUE=96, PKT_SET_RATE=97,
                       PKT_SET_STREAM=98, PKT_SET_PRIORITY=99,
                       PKT_QUERY_APID=100, PKT_SET_DELAY=101, TIME_RESET=112,
                       TBL_DUMP=128, TBL_LOAD=129, TBL_LOAD_START=130,
                       TBL_COMMIT=131, TBL_ABORT=132, TBL_VERIFY=133,
                       OS_RESET=144, OS_SEND_HK=145, FS_MKDIR=160,
                       FS_RMDIR=161, FS_RENAME=162, FS_DIRLIST=163,
                       FS_RM=164, FS_DUMP=165, FS_DEV_STAT=166, FS_LOAD=167,
                       FS_COMMIT=168, FS_FORMAT=169, FS_COPY=170,
                       FS_SD_STATE=171, TEC_ENA=176, TEC_DIS=177,
                       TEC_SET_SLEW_RATE=178, TEC_SET_TEMP=179,
                       TEC_READ_REG=180, TEC_WRITE_REG=181,
                       TEC_WRITE_DAC=182, TEC_SELECT_RTC=184, CCD_EXPOSE=192,
                       CCD_ABORT=193, CCD_SHUTTER_OVER=194,
                       CCD_SHUTTER_OPEN=195, CCD_SHUTTER_CLOSE=196,
                       CCD_SET_ID=197, CCD_COPY_IMG_SLOT=198,
                       CCD_COPY_REF_SLOT=199, CCD_ADD_REF=200,
                       CCD_MULT_REF=201, CCD_LOAD_REF=202, CCD_REPLAY=203,
                       CCD_DUMP=204, CCD_ENA=205, CCD_DIS=206, CCD_SET_DAC=207,
                       IMAGE_PROCESS=208, IMAGE_ABORT=209, MODE_STANDBY=224,
                       MODE_SCIENCE=225, CCD_STORE_REF=240),
    'cmdRjctOp' / Enum(Int16ub, NOOP=0, ARM=1, CMD_RST_STATS=2, CMD_XSUM=3,
                       CMD_ECHO_STATE=4, LOG_ROUTE=48, LOG_STATE=49,
                       LOG_RST_STATS=50, LOG_RESET_READ=52, LOG_ISSUE=53,
                       LOG_DUMP_INFO=54, LOG_SET_PUBLISH=55, MEM_DUMP=80,
                       MEM_LOAD=81, MEM_ERASE=82, MEM_XSUM=83, MEM_ABORT=84,
                       MEM_RESET=85, MEM_LOAD_DWORD=86, MEM_LOAD_WORD=87,
                       MEM_LOAD_BYTE=88, PKT_ISSUE=96, PKT_SET_RATE=97,
                       PKT_SET_STREAM=98, PKT_SET_PRIORITY=99,
                       PKT_QUERY_APID=100, PKT_SET_DELAY=101, TIME_RESET=112,
                       TBL_DUMP=128, TBL_LOAD=129, TBL_LOAD_START=130,
                       TBL_COMMIT=131, TBL_ABORT=132, TBL_VERIFY=133,
                       OS_RESET=144, OS_SEND_HK=145, FS_MKDIR=160,
                       FS_RMDIR=161, FS_RENAME=162, FS_DIRLIST=163,
                       FS_RM=164, FS_DUMP=165, FS_DEV_STAT=166, FS_LOAD=167,
                       FS_COMMIT=168, FS_FORMAT=169, FS_COPY=170,
                       FS_SD_STATE=171, TEC_ENA=176, TEC_DIS=177,
                       TEC_SET_SLEW_RATE=178, TEC_SET_TEMP=179,
                       TEC_READ_REG=180, TEC_WRITE_REG=181, TEC_WRITE_DAC=182,
                       TEC_SELECT_RTC=184, CCD_EXPOSE=192, CCD_ABORT=193,
                       CCD_SHUTTER_OVER=194, CCD_SHUTTER_OPEN=195,
                       CCD_SHUTTER_CLOSE=196, CCD_SET_ID=197,
                       CCD_COPY_IMG_SLOT=198, CCD_COPY_REF_SLOT=199,
                       CCD_ADD_REF=200, CCD_MULT_REF=201, CCD_LOAD_REF=202,
                       CCD_REPLAY=203, CCD_DUMP=204, CCD_ENA=205, CCD_DIS=206,
                       CCD_SET_DAC=207, IMAGE_PROCESS=208, IMAGE_ABORT=209,
                       MODE_STANDBY=224, MODE_SCIENCE=225, CCD_STORE_REF=240),
    'cmdFailCode' / Enum(Int8ub, SUCCESS=0, MODE=1, ARM=2, SOURCE=3, OPCODE=4,
                         METHOD=5, LENGTH=6, RANGE=7, CHECKSUM=8, PKT_TYPE=9),
    'armState' / BitStruct(
        Padding(6),
        'sc' / Enum(BitsInteger(1), OFF=0, ARMED=1),
        'dbg' / Enum(BitsInteger(1), OFF=0, ARMED=1)
    ),
    'logWriteCount' / Int16ub,
    'logDropCount' / Int16ub,
    'ccdEnaState' / Enum(Int8ub, DIS=0, ENA=1),
    'ccdCtrlState' / Enum(Int8ub, IDLE=0, ERASE=1, OPEN=2, EXPOSE=3, READOUT=4,
                          REPLAY=5, PROCESS=6, ENA=7, DIS=8),
    'ccdShutter' / Enum(Int8ub, CLOSED=0, OPEN=1),
    'shutterOverride' / Enum(Int8ub, DIS=0, ENA=1),
    'frameId' / Int32ub,
    'osCpuUsage' / Int16ub,
    'osCpuMax' / Int16ub,
    'timePPSCount' / Int16ub,
    'timeRecvCount' / Int16ub,
    'timeMissCount' / Int16ub,
    'fswMode' / Enum(Int8ub, STANDBY=0, SCIENCE=1),
    'tecState' / Enum(Int8ub, DIS=0, ENA=1),
    'tecSlewRate' / Float32b,
    'tecSetpoint' / Float32b,
    'tecCcdRtdTemp' / AffineAdapter(12.615295, 3276.7, Int16ub),
    'tecScRtd5Temp' / AffineAdapter(12.615295, 3276.7, Int16ub),
    'tecScRtd4Temp' / AffineAdapter(12.615295, 3276.7, Int16ub),
    'tecScRtd3Temp' / AffineAdapter(12.615295, 3276.7, Int16ub),
    'tecScRtd2Temp' / AffineAdapter(12.615295, 3276.7, Int16ub),
    'tecScRtd1Temp' / AffineAdapter(12.615295, 3276.7, Int16ub),
    'tecShutter' / LinearAdapter(13106.8, Int16ub),
    'tecVolt' / LinearAdapter(5234.636746976763, Int16ub),
    'tecAvgCurr' / Int16ub,
    'tecCurr' / Int16ub,
    'imgState' / Int8ub,
    'imgCurrProcType' / Enum(Int8ub, SKIP=0, RAW=1, BIN2D=2, BIAS=3, DARK=4,
                             OUTSPEC=5, SPEC1D=6, TRIM2D=7, XDISP=8)
)
