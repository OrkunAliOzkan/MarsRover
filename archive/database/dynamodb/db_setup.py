from pprint import pprint
import boto3
from boto3.dynamodb.conditions import Key, Attr

# # no need to run this function again, here for documentation
# def alien_table_setup(dynamodb=None):
#     if not dynamodb:
#         dynamodb = boto3.resource('dynamodb', region_name='us-east-1')
    
#     table = dynamodb.create_table(
#         TableName = "Targets",
#         KeySchema=[
#             {
#                 'AttributeName': 'type',    # e.g. Alien, Building, Fan 
#                 'KeyType': 'HASH'           # Partition key, on type
#             },
#             {
#                 'AttributeName': 'ID',    # e.g. some sort of ID to uniquely identify each target
#                 'KeyType': 'RANGE'           # Sort key
#             }
#         ],
#         AttributeDefinitions = [
#             {
#                 'AttributeName': 'type',    # e.g. Alien, Building, Fan 
#                 'AttributeType': 'S'           # Partition key, on type
#             },
#             {
#                 'AttributeName': 'ID',    # e.g. some sort of ID to uniquely identify each target
#                 'AttributeType': 'S'           # Sort key
#             },
#             # can have other attributes also e.g. colour for each alien
#         ],
#         # just using default values, probably doesnt matter
#         ProvisionedThroughput = {
#             'ReadCapacityUnits': 10,
#             'WriteCapacityUnits': 10
#         }
#     )
#     return table

def mission_table_setup(dynamodb=None):
    if not dynamodb:
        dynamodb = boto3.resource('dynamodb', region_name='us-east-1')
    
    table = dynamodb.create_table(
        TableName = "Missions",
        KeySchema=[
            {
                'AttributeName': 'ID',    # mission ID
                'KeyType': 'HASH'           # Partition key, on type
            },
        ],
        # can have mission ID as only attribute, store displacement/time as an array or sth
        AttributeDefinitions = [
            {
                'AttributeName': 'ID',    # mission ID
                'AttributeType': 'S'           # Partition key, on type
            },
        ], 
        # just using default values, probably doesnt matter
        ProvisionedThroughput = {
            'ReadCapacityUnits': 10,
            'WriteCapacityUnits': 10
        }
    )
    return table

# more tables (e.g. if we want to store sensor data) can be added following t

if __name__ == "__main__":
    print("hello world")
    table = mission_table_setup()
    print("Table status", table.table_status)