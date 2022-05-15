#!/usr/bin/env python3 

import zmq
import serial
import pmt
from time import sleep
from multiprocessing import Process, Queue, Lock

port_push = 6001
port_pull = 6002

def send_serial(l, serialPort):
    try:
        while 1:
            print("SERIAL_WRITE")
            message = pullQueue.get()
            l.acquire()
            print(message)
            serialPort.write(bytearray(message[10:]))
            serialPort.flush()
            l.release()
    except KeyboardInterrupt:
        pass

def read_serial(l, serialPort):
    try:
        while 1:
            l.acquire()
            print("SERIAL_READ")
            message = serialPort.read(144)
            print(bytearray(message))
            l.release()
            pushQueue.put(message)
            sleep(0.1)
    except KeyboardInterrupt:
        pass

def client_pull(pullQueue):
    print("PULL")
    
    context = zmq.Context()
    socket_pull = context.socket(zmq.PULL)
    socket_pull.connect ("tcp://localhost:%s" % port_pull)
    print ("Connected to server with port %s" % port_pull)
    # Initialize poll set
    poller = zmq.Poller()
    poller.register(socket_pull, zmq.POLLIN)

    isRunning = True
    timetick = 100
    try:
        while isRunning:
            obj = dict(poller.poll(timetick))
            if socket_pull in obj and obj[socket_pull] == zmq.POLLIN:
                message = socket_pull.recv()
                pullQueue.put(message)
    except KeyboardInterrupt:
        pass

def server_push(pushQueue):
    print("PUSH")

    context = zmq.Context()
    socket = context.socket(zmq.PUSH)
    socket.bind("tcp://127.0.0.1:%s" % port_push)
    print ("Running server on port: ", port_push)
    isRunning = True
    try:
        while isRunning:
            pkt = pushQueue.get()
            socket.send(pkt)
    except KeyboardInterrupt:
        pass

if __name__ == "__main__":
    serialPort = serial.Serial('/dev/ttyACM1', 115200, serial.EIGHTBITS, serial.PARITY_NONE, serial.STOPBITS_ONE, timeout=1)
    pushQueue = Queue()
    pullQueue = Queue()
    lock = Lock()

    Process(target=client_pull, args=(pullQueue,)).start()
    Process(target=send_serial, args=(lock, serialPort,)).start()
    Process(target=read_serial, args=(lock, serialPort,)).start()
    Process(target=server_push, args=(pushQueue,)).start()


