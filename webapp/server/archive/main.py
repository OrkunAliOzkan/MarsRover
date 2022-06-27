from datetime import datetime
import sys
import time
import threading
import os
import json
import socket

def nprint(output):
    sys.stdout.write(output + '\n')
    sys.stdout.flush()

def echo():
    while True:
        command = sys.stdin.readline().split('\n')[0]
        if command == "shutdown":
            
        sys.stdout.write(command)
        sys.stdout.flush()

def recv_post(socket):
    received = False
    try:
        recv_msg = socket.recv(1024).decode()
        received = True
    except:
        pass
    if received:
        nprint(recv_msg)
    # while True:
    #     content = client.recv(1024)
    #     if len(content) ==0:
    #         break
    #     else:
    #         nprint(content)
    # nprint("Closing connection")
    # client.close()

def send(socket, send_msg):
    try:
        socket.send(send_msg.encode())
        nprint(f"sent {send_msg}")
    except:
        pass

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_port = 8080
server_socket.bind(('0.0.0.0', 8080))
server_socket.settimeout(0.01) #10ms timeout for receives, after which silent error is thrown
server_socket.listen(0)

nprint(f"TCP server started on port {rover_port}")

x = 0
y = 0
date_ = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(time.time()))
data = '{"mode": M, "time": "'+ date_ + '", "x": "'+ str(x) + '", "y": "'+ str(y) +'"}'

rover_socket = None
rover_addr = None
threading.Thread(target=echo).start()
while True:
    rover_connected = False
    while !rover_connected:
        rover_socket, rover_addr = server_socket.accept()
    nprint("Rover connected")
    while True:
        recv_post(rover_socket)
