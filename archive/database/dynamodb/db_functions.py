from pprint import pprint
import boto3
from boto3.dynamodb.conditions import Key, Attr

def delete_table(tbl_name, dynamodb=None):
    if not dynamodb:
        dynamodb = boto3.resource('dynamodb', region_name="us-east-1")
    table = dynamodb.Table(tbl_name)
    table.delete()

def put_mission(mid, minfo, dynamodb=None):
    if not dynamodb:
        dynamodb = boto3.resource('dynamodb', region_name='us-east-1')

    table = dynamodb.Table('Missions')
    response = table.put_item(
       Item={
            'ID': mid,
            'info': {
                'target_locations': []
            }
        }
    )
    return response

def get_mission(mid, dynamodb=None):
    if not dynamodb:
        dynamodb = boto3.resource('dynamodb', region_name='us-east-1')

    table = dynamodb.Table('Missions')

    try:
        response = table.get_item(Key={'ID': mid})
    except ClientError as e:
        print(e.response['Error']['Message'])
    else:
        return response['Item']

def all_missions(dynamodb=None):
    if not dynamodb:
        dynamodb = boto3.resource('dynamodb', region_name='us-east-1')
    
    table = dynamodb.Table('Missions')
    
    response = table.scan()
    data = response['Items']

    while 'LastEvaluatedKey' in response:
        response = table.scan()
        data.extend(response['Items'])
    return data
    

if __name__ == "__main__":
    print(all_missions())

