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

x = 0
y = 0

class pageHandler(tornado.web.RequestHandler):
    def get(self):
        self.render("../client/test/tornado_test.html")

class roverHandler(tornado.web.RequestHandler):
    def get(self):
        nprint("Get Request Received")
        date_ = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(time.time()))
        self.set_status(200)
        data = '{"time": "'+ date_ + '", "x": "'+ str(x) + '", "y": "'+ str(y) +'"}'
        nprint(data)
        self.write(bytes(data, "utf-8"))
        nprint("Get Request Serviced")

    def post(self):
        # data = tornado.escape.json_decode(self.request.body)
        nprint("Post Request Received")
        data = str(self.request.body)
        data = data.replace(r'\n', '\n')
        nprint(data)
        self.set_status(200)
        nprint("Post Request Serviced") 

app = tornado.web.Application([
    (r"/", roverHandler),
    (r"/test", pageHandler)
])

app.listen(3001)
nprint("Python servicing rover on port 3001")
tornado.ioloop.IOLoop.current().start()