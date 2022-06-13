var MongoClient = require('mongodb').MongoClient;
var url = "mongodb+srv://bet20:TCuh4ZxixcbXFQS0@cluster0.agmw9dr.mongodb.net/?retryWrites=true&w=majority";

MongoClient.connect(url, function(err, db) {
  if (err) throw err;
  console.log("Connected to MongoDB Atlas database");
  // var mars_db = db.db("missions");
  // mars_db.collection("mission_data").findOne({}, (err, result) => {
  //   if (err) throw err;
  //   console.log(result.name);
  //   db.close();
  // })
  // mars_db.close();
});

const now = new Date("2022-06-10 23:16:39.603783+00:00");
console.log(now);