import boto3
db = boto3.resource('dynamodb', region_name='us-east-1')
print(list(db.tables.all()))

