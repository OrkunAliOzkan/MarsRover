const express = require('express')
const mysql = require ('mysql')
const cors = require('cors')

app = express() // Invoking express function and calling it app
app.use(cors()) // Applying use() methos to set up "cors" middelware to our express function app
port = 5000 // defining the port on which our app will listen

// Connecting to database by using specified host and name of the database and using createConnection method of mysql
const connection = mysql.createConnection({
    host:"localhost",
    database:"cps_ecc_22",
    user:"root",
    password:""
})

// Running our Express app
app.listen(port,(err) => err ? console.log(err) : console.log(`Server Running on port ${port}`))

// Connecting to databese
connection.connect(err => { err ? console.log(err) : console.log("Connection to database OK")})

// Once connected to database, we create a ROUTE that specifies the REQUEST type or method
app.get('/data', (req,res) => {
    connection.query(
        'SELECT * FROM iot', 
        (err,rows) => {
            err ? res.send(err) : res.send(rows)
        }
    )
})