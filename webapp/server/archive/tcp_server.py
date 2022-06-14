import socket
 
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)         
 
s.bind(('0.0.0.0', 3002))
s.listen(0)                 

print("Server started on port 3002")
while True:
 
    client, addr = s.accept()
    print("Rover connected")
    while True:
        content = client.recv(1024)
 
        if len(content) ==0:
           break
 
        else:
            print(content)
 
    print("Closing connection")
    client.close()