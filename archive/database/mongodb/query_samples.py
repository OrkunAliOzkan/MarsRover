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

conditions = {'_id': fivestar.get('_id')}
updated_contents = {'$inc': {'likes': 666},  
                    '$set': {'cuisine': "Chinese", 'name': "Sha Xian Xiao Chi", 'rating': 69}
                    }

result = db.reviews.update_one(conditions, updated_contents)
print("Number of documents modified: " + str(result.modified_count))

updated = db.reviews.find_one({'_id': fivestar.get('_id')})
print("The updated document:")
pprint(updated)
