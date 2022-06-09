from http.server import HTTPServer,BaseHTTPRequestHandler
import time
#########################################################
HOST = "0.0.0.0"

#HOST = "54.242.65.190"
PORT = 3001

ANG = "20"
MAG = "50"
#########################################################
class RoverHTTP(BaseHTTPRequestHandler):
    def do_GET(self):
        print("get request received")
        # How we respond to get request
        self.send_response(200)
        self.send_header("Content-type", 
                        "text/plain")
        self.end_headers()
        date_ = time.strftime("%Y-%m-%d %H:%M:%S", 
                            time.localtime(time.time()))

        self.wfile.write(bytes('{"time": "'+ date_ + 
                                '", "ang": "'+ ANG + 
				'", "mag": "'+ MAG +
                                '"}', "utf-8"))
#########################################################
    def do_POST(self):
        print("post request received")
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
#########################################################
