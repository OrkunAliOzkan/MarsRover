import sys
import time
import threading
import tornado.web
import tornado.ioloop
import tornado.escape

def main():
    while True:
        command = sys.stdin.readline().split('\n')[0]
        sys.stdout.write(command)
        sys.stdout.flush()

# class basicRequestHandler(tornado.web.RequestHandler):
#     # def set_default_headers(self):
#     #     print("Headers Set")
#     #     self.set_header("Access-Control-Allow-Origin", "*")
#     #     self.set_header("Access-Control-Allow-Headers", "x-requested-with")
#     #     self.set_header('Access-Control-Allow-Methods', 'POST, GET, OPTIONS')
#     #     self.set_header("Access-Control-Allow-Headers", "access-control-allow-origin,authorization,content-type") 

#     # def options(self):
#     #     print("preflight")
#     #     self.set_status(204)

#     def get(self):
#         print("Get Request Received")
#         date_ = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(time.time()))
#         self.set_status(200)
#         data = '{"time": "'+ date_ + '", "x": "'+ str(x) + '", "y": "'+ str(y) +'"}'
#         print(data)
#         self.write(bytes(data, "utf-8"))
#         print("Get Request Serviced")

#     def post(self):
#         # data = tornado.escape.json_decode(self.request.body)
#         print("Post Request Received")
#         data = str(self.request.body)
#         data = data.replace(r'\n', '\n')
#         print(data)
#         self.set_status(200)
#         print("Post Request Serviced") 

# app = tornado.web.Application([
#     (r"/rover_request", basicRequestHandler),
# ])


# app.listen(3001)
# print("Python servicing rover on port 3001")
# tornado.ioloop.IOLoop.current().start()
threading.Thread(target=main).start()