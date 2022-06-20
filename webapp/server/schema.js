const route_example = [{"time":15,"type":"rover","data":{"posX":162.5,"posY":162.5,"angle":90}},
  {"time":16,"type":"rover","data":{"posX":170,"posY":170,"angle":96}},
  {"time":17,"type":"rover","data":{"posX":177.5,"posY":177.5,"angle":102}}]

const mission_1 = {
    "id": 601,
    "times": [tstart, tend],
    "duration_sec": 600,
    "starting_pos" : [30, 2000],
    "total_path": 1245,
    "route_data": route_example,
    "num_targets_found" : 4,
    "targets": {
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
    // "route": [[timestamp, x, y], etc.] uncomment when fully implemented
}

