import socketio

sio = socketio.Client()
sio.connect('http://localhost:3000')


@sio.on('ntop')
def on_message(data):
    print(f'{data}')
    
    sio.emit('pton', 'received')


