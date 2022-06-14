import sys
import time
import threading
import os
import json

#################################
# UTILITY FUNCTIONS #

def nprint(output):
    sys.stdout.write(output + '\n')
    sys.stdout.flush()

def echo():
    while True:
        command = sys.stdin.readline().split('\n')[0]
        sys.stdout.write(command)
        sys.stdout.flush()

#################################
# TCP SERVER#
# import socket
 
# s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)         
 
# s.bind(('0.0.0.0', 3002))
# s.listen(0)                 

# print("TCP server started on port 3002")

# def recv_post():
#     while True:
#         client, addr = s.accept()
#         print("Rover connected")
#         while True:
#             content = client.recv(1024)
#             if len(content) ==0:
#                 break
#             else:
#                 print(content)
#         print("Closing connection")
#         client.close()

#################################
import tornado.web
import tornado.ioloop
import tornado.escape

angle = 45
y = 1000

class pageHandler(tornado.web.RequestHandler):
    def get(self):
        self.render("../client/test/tornado_test.html")

class roverHandler(tornado.web.RequestHandler):
    def get(self):
        nprint("Get Request Received")
        date_ = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(time.time()))
        self.set_status(200)
        data = '{"time": "'+ date_ + '", "a": "'+ str(angle) + '", "m": "'+ str(y) +'"}'
        self.write(bytes(data, "utf-8"))
        nprint("Get Request Serviced")
        nprint("\n");

    def post(self):
        # data = tornado.escape.json_decode(self.request.body)
        #nprint("Post Request Received")
        data = str(self.request.body)
        #data = data.replace(r'\n', '\n')
        #nprint(data)
        print(data)
        self.set_status(200)
        #nprint("Post Request Serviced") 
        #nprint("\n");

app = tornado.web.Application([
    (r"/", roverHandler),
    (r"/test", pageHandler)
])

app.listen(3001)

nprint("Python servicing rover on port 3001")

tornado.ioloop.IOLoop.current().start()
threading.Thread(target=echo).start()
# threading.Thread(target=recv_post).start()