import pymongo
import datetime
from pprint import pprint

client = pymongo.MongoClient('mongodb+srv://bet20:TCuh4ZxixcbXFQS0@cluster0.agmw9dr.mongodb.net/?retryWrites=true&w=majority')
db = client.missions

tstart = 0
tend = 0

### Store Data ###
# Collection 1
mission_data = db.mission_data

mission = {
    "id": 601,
    "times": [tstart, tend],
    "starting_pos" : [tstart, tend]
    # "route": [[timestamp, x, y], etc.] uncomment when fully implemented
}

#######
# INSERT INTO COLLECTION #
mission_data.insert_one(mission)
######

# Collection 2
targets = db.targets

target0 = {
    "tid": 2,
    "mid": 601,
    "location": [12.5, 25], 
    "type": "alien", 
    "time_found": 10, 
    "colour": 0x123f
}

target1 = {
    "tid": 1,
    "mid": 603,
    "location": [100, 15], 
    "type": "building", 
    "time_found": 61, 
}

#######
# INSERT INTO COLLECTION #
# targets.insert_one(target0)
######

"""
This is a 'normalised' model rather than an 'embedded' model
In an embedded model all the related elements are in the same document
Just one document for all details of a mission, rather than separate documents for each target etc.
"""


### Query Data ###
