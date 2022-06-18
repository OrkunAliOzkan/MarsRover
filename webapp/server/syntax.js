const pi = 3.14;

(() => {
    console.log(pi);

    const time_string = (new Date()).toISOString();

    const waypoint_data = {time: time_string, x: 100, y: 120};
    const waypoint_json = JSON.stringify(waypoint_data);
    console.log(waypoint_data);
    console.log(waypoint_json);

    const thing = {
        "time": 1002,
        "type": "rover",
        "data": {
            "posX": 400,
            "posY": 469,
            "angle": 0  
        }
    }
})();

