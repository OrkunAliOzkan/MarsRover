import socketio
from http.server import HTTPServer,BaseHTTPRequestHandler
import time

sio = socketio.Client()
sio.connect('http://localhost:3000')

HOST = "172.31.16.189"
#HOST = "54.242.65.190"
PORT = 8000

class RoverHTTP(BaseHTTPRequestHandler):
    def do_GET(self):
        # How we respond to get request
        self.send_response(200)
        self.send_header("Content-type", 
                        "application/json")
        self.end_headers()
        date_ = time.strftime("%Y-%m-%d %H:%M:%S", 
                            time.localtime(time.time()))

        self.wfile.write(bytes('{"time": "'+ date_ + 
                                '", "ang": "'+ ANG + 
				'", "mag": "'+ MAG +
                                '"}', "utf-8"))
#########################################################
    def do_POST(self):
        self.send_response(200)
        self.send_header("Content-type",
                        "application/json")
        self.end_headers()
        date_ = time.strftime("%Y-%m-%d %H:%M:%S", 
                            time.localtime(time.time()))
        
        self.wfile.write(bytes('{"time": "'+ date_ + 
                                '"}', "utf-8"))
#########################################################
server = HTTPServer((HOST, PORT), RoverHTTP)
print("Server active...")
server.serve_forever()
server.server_close()
print("Server inactive...")

@sio.on('ntop')
def on_message(data):
    data_list = data.split(",")
    magnitude = data_list[1]
    angle = data_list[2]
    print(f'{magnitude}, {angle}')

    sio.emit('pton', 'received')


