import socketio
import tornado.web
import tornado.ioloop
import tornado.escape
import json

import time

sio = socketio.Client()
sio.connect('http://localhost:3000')

x = 0
y = 0

@sio.on('ntop')
def on_message(data):
    data_list = data.split(",")
    global x
    x = data_list[1]
    global y
    y = data_list[2]
    print(f'{x}, {y}')

class basicRequestHandler(tornado.web.RequestHandler):
    # def set_default_headers(self):
    #     print("Headers Set")
    #     self.set_header("Access-Control-Allow-Origin", "*")
    #     self.set_header("Access-Control-Allow-Headers", "x-requested-with")
    #     self.set_header('Access-Control-Allow-Methods', 'POST, GET, OPTIONS')
    #     self.set_header("Access-Control-Allow-Headers", "access-control-allow-origin,authorization,content-type") 

    def options(self):
        print("preflight")
        self.set_status(204)

    def get(self):
        print("Get Request Received")
        date_ = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(time.time()))
        self.set_status(200)
        data = '{"time": "'+ date_ + '", "x": "'+ str(x) + '", "y": "'+ str(y) +'"}'
        print(data)
        self.write(bytes(data, "utf-8"))
        print("Get Request Serviced")

    def post(self):
        # data = tornado.escape.json_decode(self.request.body)
        print("Post Request Received")
        data = str(self.request.body)
        data = data.replace(r'\n', '\n')
        print(data)
        self.set_status(200)
        print("Post Request Serviced") 

app = tornado.web.Application([
    (r"/rover_request", basicRequestHandler),
])

app.listen(3001)
print("I'm listening on port 3001")
tornado.ioloop.IOLoop.current().start()