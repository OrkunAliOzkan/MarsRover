import pymongo
import datetime
from pprint import pprint

client = pymongo.MongoClient('mongodb+srv://bet20:TCuh4ZxixcbXFQS0@cluster0.agmw9dr.mongodb.net/?retryWrites=true&w=majority')

# # create new database called test
# db = client.test

# # create new collection in database called people
# people = db.people

# personDocument = {
#     "name": { "first": "Alan", "last": "Turing" },
#     "birth": datetime.datetime(1912, 6, 23),
#     "death": datetime.datetime(1954, 6, 7),
#     "contribs": [ "Turing machine", "Turing test", "Turingery" ],
#     "views": 1250000
# }

# print(people.insert_one(personDocument))

# print(people.find_one({ "name.last": "Turing" }))

# print(pymongo.version)

db = client.admin
serverStatusResult = db.command("serverStatus")
pprint(serverStatusResult)