import sys
import time
import threading
import os
import json

def nprint(output):
    sys.stdout.write(output + '\n')
    sys.stdout.flush()

def echo():
    while True:
        command = sys.stdin.readline().split('\n')[0]
        sys.stdout.write(command)
        sys.stdout.flush()

threading.Thread(target=echo).start()

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
        nprint("Post Request Received")
        data = str(self.request.body)
        data = data.replace(r'\n', '\n')
        nprint(data)
        self.set_status(200)
        nprint("Post Request Serviced") 
        nprint("\n");

app = tornado.web.Application([
    (r"/", roverHandler),
    (r"/test", pageHandler)
])

app.listen(3001)
nprint("Python servicing rover on port 3001")
tornado.ioloop.IOLoop.current().start()