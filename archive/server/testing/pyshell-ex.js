// // method 1
// const {PythonShell} = require('python-shell');
// let pyshell = new PythonShell('echo_text.py', { mode: 'text' });
// // let output = "";
// pyshell.stdout.on('data', (data) => {
//     // output += '' + data;
//     console.log(`Python out: ${data}`);
// });

// pyshell.send('hello').send('world').end(function (err) {
//     console.log(output === ('hello' + '\n' + 'world' + '\n'));
//     console.log(output);
// });
// pyshell.on('close', () => console.log("Process ended"));