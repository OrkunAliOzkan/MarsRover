import json
_rover = {
    "time": 1002,
    "type": "rover",
    "data": {
        "posX": 400,
        "posY": 469,
        "angle": 0  
    }
}

_alien = {
    "time": 1002,
    "type": "alien",
    "colour": "blue",
    "data": {
        "posX": 150,
        "posY": 70,
    }
}

duration = 3000
packet_period = 50
packet_list = []

num_packets = int (duration / packet_period)
for i in range(0, num_packets):
    tmp = {
        "time": i,
        "type": "rover",
        "data": {
            "posX": 50 + 450 / num_packets * i,
            "posY": 50 + 450 / num_packets * i,
            "angle": i * 360 / num_packets  
        }
    }
    packet_json = json.dumps(tmp)
    packet_list.append(packet_json)

for i in packet_list:
    print(i)


# packet_json = json.dumps(_rover)
# print(packet_json)