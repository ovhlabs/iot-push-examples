###########################################################
# OpenTSDB REST API session in a HTTPS connection example #
###########################################################

import json
import requests

# Place your token id here
token_id = 'w3eaaaqaa7pff'

# Place your token here
token_key = 'xyz123xyz123xyz123xyz123'

# IoT Lab OpenTSDB Endpoint
end_point = 'https://opentsdb.iot.runabove.io/api/put'

# Raw body data to send
data = [{
				'metric':'home.temp.indoor',
				'timestamp':1437591600,
				'value':22.5,
				'tags':{
					'source':'dht22'
				}
			},
			{
				'metric':'home.temp.outdoor',
				'timestamp':1437591600,
				'value':28.2,
				'tags':{
					'source':'ds18b20'
				}
			},
			{
				'metric':'home.temp.indoor',
				'timestamp':1437593400,
				'value':22.1,
				'tags':{
					'source':'dht22'
				}
			},
			{
				'metric':'home.temp.outdoor',
				'timestamp':1437593400,
				'value':27.1,
				'tags':{
					'source':'ds18b20'
				}
			}]

try:
	# Create http session
    session = requests.session()

	# Send request and fetch response
    response = session.post(end_point, data=json.dumps(data), auth=(token_id, token_key))

    # raise error
    response.raise_for_status()

	# Print the http response code on success
	print( 'Send successful\nResponse code from server:{} '.format( response.status_code ) )

except requests.exceptions.HTTPError as e:
	print( 'HTTP code is {} and reason is {}'.format( e.response.status_code, e.response.reason ) )
