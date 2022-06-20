const mongo = require('mongodb');

(async function main() {
    var url = "mongodb+srv://bet20:TCuh4ZxixcbXFQS0@cluster0.agmw9dr.mongodb.net/?retryWrites=true&w=majority";
    const client = new mongo.MongoClient(url);
    try {
        await client.connect();
        console.log("Connected to cluster");

        

        // await listDatabases(client);

        // const mars_db = client.db("missions");
        
        // await printCollections(mars_db);

        // await printDocs(mars_db, "mission_data");
    } catch (e) {
        console.log(e);
    } finally {
        await client.close();
    }
})();

async function printDocs(db, col) {
    document_list = await db.collection(col).find({}).toArray();
    console.log(document_list);
}

async function printCollections(db) {
    collection_list = await db.listCollections().toArray();
    console.log(collection_list);
    console.log("Collections:");
    collection_list.forEach(collection => console.log(` - ${collection.name}`));
}

async function listDatabases(client) {
    databasesList = await client.db().admin().listDatabases();

    console.log("Databases:");
    databasesList.databases.forEach(db => console.log(` - ${db.name}`));
}

// MongoClient.connect(url, function(err, db) {
//     if (err) throw err;
//     console.log("Connected to MongoDB Atlas database");
//     var mars_db = db.db("missions");
//     // mars_db.collection("mission_data").findOne({}, (err, result) => {
//     //   if (err) throw err;
//     //   console.log(result.name);
//     //   db.close();
//     // })
//     // mars_db.close();
//     mars_db.collectionNames( (err, collections) => {
//         console.log(collections);
//     })
// });

const now = new Date("2022-06-10 23:16:39.603783+00:00");
console.log(now);