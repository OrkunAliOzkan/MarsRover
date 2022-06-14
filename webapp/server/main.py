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
        sys.stdout.write(command)
        sys.stdout.flush()

rover_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
rover_socket.bind(('0.0.0.0', 3001))
rover_socket.listen(0)
print("TCP server started on port 3001")

def connect():
    global rover_socket, client, addr
    client, addr = rover_socket.accept()
    print("Rover connected on ", addr)

def recv_post(): 
    while True:
        content = client.recv(1024)
        if len(content) ==0:
            break
        else:
            print(content)
    print("Closing connection")
    client.close()

angle = 0
y = 0
date_ = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(time.time()))
data = '{"time": "'+ date_ + '", "a": "'+ str(angle) + '", "m": "'+ str(y) +'"}'

def send():
    rover_socket.send(data)



threading.Thread(target=echo).start()