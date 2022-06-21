const full = [
    '{"time":0,"type":"rover","data":{"posX":50,"posY":50,"angle":0}}',
    '{"time":1,"type":"rover","data":{"posX":57.5,"posY":57.5,"angle":6}}',
    '{"time":2,"type":"rover","data":{"posX":65,"posY":65,"angle":12}}',
    '{"time":3,"type":"rover","data":{"posX":72.5,"posY":72.5,"angle":18}}',
    '{"time":4,"type":"rover","data":{"posX":80,"posY":80,"angle":24}}',
    '{"time":5,"type":"rover","data":{"posX":87.5,"posY":87.5,"angle":30}}',
    '{"time":6,"type":"rover","data":{"posX":95,"posY":95,"angle":36}}',
    '{"time":7,"type":"rover","data":{"posX":102.5,"posY":102.5,"angle":42}}',
    '{"time":8,"type":"rover","data":{"posX":110,"posY":110,"angle":48}}',
    '{"time":9,"type":"rover","data":{"posX":117.5,"posY":117.5,"angle":54}}',
    '{"time":10,"type":"rover","data":{"posX":125,"posY":125,"angle":60}}',
    '{"time":11,"type":"rover","data":{"posX":132.5,"posY":132.5,"angle":66}}',
    '{"time":12,"type":"rover","data":{"posX":140,"posY":140,"angle":72}}',
    '{"time":13,"type":"rover","data":{"posX":147.5,"posY":147.5,"angle":78}}',
    '{"time":14,"type":"rover","data":{"posX":155,"posY":155,"angle":84}}',
    '{"time":15,"type":"rover","data":{"posX":162.5,"posY":162.5,"angle":90}}',
    '{"time":16,"type":"rover","data":{"posX":170,"posY":170,"angle":96}}',
    '{"time":17,"type":"rover","data":{"posX":177.5,"posY":177.5,"angle":102}}',
    '{"time":18,"type":"rover","data":{"posX":185,"posY":185,"angle":108}}',
    '{"time":19,"type":"rover","data":{"posX":192.5,"posY":192.5,"angle":114}}',
    '{"time":20,"type":"rover","data":{"posX":200,"posY":200,"angle":120}}',
    '{"time":21,"type":"rover","data":{"posX":207.5,"posY":207.5,"angle":126}}',
    '{"time":22,"type":"rover","data":{"posX":215,"posY":215,"angle":132}}',
    '{"time":23,"type":"rover","data":{"posX":222.5,"posY":222.5,"angle":138}}',
    '{"time":24,"type":"rover","data":{"posX":230,"posY":230,"angle":144}}',
    '{"time":25,"type":"rover","data":{"posX":237.5,"posY":237.5,"angle":150}}',
    '{"time":26,"type":"rover","data":{"posX":245,"posY":245,"angle":156}}',
    '{"time":27,"type":"rover","data":{"posX":252.5,"posY":252.5,"angle":162}}',
    '{"time":28,"type":"rover","data":{"posX":260,"posY":260,"angle":168}}',
    '{"time":29,"type":"rover","data":{"posX":267.5,"posY":267.5,"angle":174}}',
    '{"time":30,"type":"rover","data":{"posX":275,"posY":275,"angle":180}}',
    '{"time":31,"type":"rover","data":{"posX":282.5,"posY":282.5,"angle":186}}',
    '{"time":32,"type":"rover","data":{"posX":290,"posY":290,"angle":192}}',
    '{"time":33,"type":"rover","data":{"posX":297.5,"posY":297.5,"angle":198}}',
    '{"time":34,"type":"rover","data":{"posX":305,"posY":305,"angle":204}}',
    '{"time":35,"type":"rover","data":{"posX":312.5,"posY":312.5,"angle":210}}',
    '{"time":36,"type":"rover","data":{"posX":320,"posY":320,"angle":216}}',
    '{"time":37,"type":"rover","data":{"posX":327.5,"posY":327.5,"angle":222}}',
    '{"time":38,"type":"rover","data":{"posX":335,"posY":335,"angle":228}}',
    '{"time":39,"type":"rover","data":{"posX":342.5,"posY":342.5,"angle":234}}',
    '{"time":40,"type":"rover","data":{"posX":350,"posY":350,"angle":240}}',
    '{"time":41,"type":"rover","data":{"posX":357.5,"posY":357.5,"angle":246}}',
    '{"time":42,"type":"rover","data":{"posX":365,"posY":365,"angle":252}}',
    '{"time":43,"type":"rover","data":{"posX":372.5,"posY":372.5,"angle":258}}',
    '{"time":44,"type":"rover","data":{"posX":380,"posY":380,"angle":264}}',
    '{"time":45,"type":"rover","data":{"posX":387.5,"posY":387.5,"angle":270}}',
    '{"time":46,"type":"rover","data":{"posX":395,"posY":395,"angle":276}}',
    '{"time":47,"type":"rover","data":{"posX":402.5,"posY":402.5,"angle":282}}',
    '{"time":48,"type":"rover","data":{"posX":410,"posY":410,"angle":288}}',
    '{"time":49,"type":"rover","data":{"posX":417.5,"posY":417.5,"angle":294}}',
    '{"time":50,"type":"rover","data":{"posX":425,"posY":425,"angle":300}}',
    '{"time":51,"type":"rover","data":{"posX":432.5,"posY":432.5,"angle":306}}',
    '{"time":52,"type":"rover","data":{"posX":440,"posY":440,"angle":312}}',
    '{"time":53,"type":"rover","data":{"posX":447.5,"posY":447.5,"angle":318}}',
    '{"time":54,"type":"rover","data":{"posX":455,"posY":455,"angle":324}}',
    '{"time":55,"type":"rover","data":{"posX":462.5,"posY":462.5,"angle":330}}',
    '{"time":56,"type":"rover","data":{"posX":470,"posY":470,"angle":336}}',
    '{"time":57,"type":"rover","data":{"posX":477.5,"posY":477.5,"angle":342}}',
    '{"time":58,"type":"rover","data":{"posX":485,"posY":485,"angle":348}}',
    '{"time":59,"type":"rover","data":{"posX":492.5,"posY":492.5,"angle":354}}'
  ]

const route_example = [{"time":15,"type":"rover","data":{"posX":162.5,"posY":162.5,"angle":90}},
  {"time":16,"type":"rover","data":{"posX":170,"posY":170,"angle":96}},
  {"time":17,"type":"rover","data":{"posX":177.5,"posY":177.5,"angle":102}}]

const targets = {
    "alien": {
        "red": {
            "posX": 100,
            "posY": 500
        },
        "blue": {
            "posX": 200,
            "posY": 500
        },
        "green": {
            "posX": 300,
            "posY": 500
        }
    },
    "fan": {
        "posX": 50,
        "posY": 300
    }
}

const mission_1 = {
    "_id": 601,
    "times": {"start": new Date("2022-06-01 20:00:00Z"), "end": new Date("2022-06-01 20:10:00Z")},
    "duration_ms": 600,
    "starting_pos" : {"posX": 30, "posY": 30},
    "total_path": 5000,
    "route_data": route_example,
    "num_targets_found" : 4,
    "targets": targets
    // "route": [[timestamp, x, y], etc.] uncomment when fully implemented
}

const mission_2 = {
    "_id": 602,
    "times": {"start": new Date("2022-06-02 20:00:00Z"), "end": new Date("2022-06-02 20:11:00Z")},
    "duration_ms": 660,
    "starting_pos" : {"posX": 30, "posY": 30},
    "total_path": 4000,
    "route_data": route_example,
    "num_targets_found" : 3,
    "targets": targets
    // "route": [[timestamp, x, y], etc.] uncomment when fully implemented
}

const mission_3 = {
    "_id": 603,
    "times": {"start": new Date("2022-06-03 20:00:00Z"), "end": new Date("2022-06-02 20:12:00Z")},
    "duration_ms": 720,
    "starting_pos" : {"posX": 30, "posY": 30},
    "total_path": 5000,
    "route_data": route_example,
    "num_targets_found" : 5,
    "targets": targets
    // "route": [[timestamp, x, y], etc.] uncomment when fully implemented
}
