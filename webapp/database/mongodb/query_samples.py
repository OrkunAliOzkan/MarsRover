import pymongo
import datetime
from pprint import pprint

client = pymongo.MongoClient('mongodb+srv://bet20:TCuh4ZxixcbXFQS0@cluster0.agmw9dr.mongodb.net/?retryWrites=true&w=majority')
db = client.business

fivestar = db.reviews.find_one({'rating': 5})
pprint(fivestar)

stargroup = db.reviews.aggregate(
    [
        {
            '$group': {
                '_id': '$rating',
                'count': { '$sum':1 }
            }
        },
        {
            '$sort': {'_id':1 }
        }
    ]
)

for group in stargroup:
    print(group)
