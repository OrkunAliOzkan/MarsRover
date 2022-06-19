const pi = 3.14;

function getRandomInt(min, max) {
    min = Math.ceil(min);
    max = Math.floor(max);
    return Math.floor(Math.random() * (max - min) + min); //The maximum is exclusive and the minimum is inclusive
}

function genPacketsSmall() {
    // generate small 60 packets
    const duration = 3000;
    const packet_period = 50;
    let packet_list = [];

    const num_packets = Math.floor(duration / packet_period)
    for (let i = 0; i < num_packets; i++){
        let tmp = {
            "time": i,
            "type": "rover",
            "data": {
                "posX": 50 + 450 / num_packets * i,
                "posY": 50 + 450 / num_packets * i,
                "angle": i * 360 / num_packets  
            }
        }
        let packet_json = JSON.stringify(tmp);
        packet_list.push(packet_json);
    }

    // packet_list.forEach(item => console.log(item));
    return packet_list;
}

var state = {
    x: 12
};

(() => {
    genPacketsSmall();
    state.y = 123;
    console.log(state);
    // console.log(pi);

    // const time_string = (new Date()).toISOString();

    // const waypoint_data = {time: time_string, x: 100, y: 120};
    // const waypoint_json = JSON.stringify(waypoint_data);
    // console.log(waypoint_data);
    // console.log(waypoint_json);

    // const thing = {
    //     "time": 1002,
    //     "type": "rover",
    //     "data": {
    //         "posX": 400,
    //         "posY": 469,
    //         "angle": 0  
    //     }
    // }

    // generate small 36000 packets
    // const duration = 3000;
    // const packet_period = 50;
    // let packet_list = [];

    // //const num_packets = Math.floor(duration / packet_period)
    // for (let i = 0; i < 36000; i++){
    //     let tmp = {
    //         "time": i,
    //         "type": "rover",
    //         "data": {
    //             "posX": getRandomInt(100, 500),
    //             "posY": getRandomInt(100, 500),
    //             "angle": getRandomInt(0, 360), 
    //         }
    //     }
    //     let packet_json = JSON.stringify(tmp);
    //     packet_list.push(packet_json);
    // }
    // // for (let i = 0; i < 500; i++) {
    // //     console.log(packet_list[i]);
    // // }
    // packet_list.forEach(item => console.log(item));
})();

