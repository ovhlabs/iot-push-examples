###########################################################
# OpenTSDB REST API session in a HTTPS connection example #
###########################################################

import json
import urllib2
import urllib
import base64

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
	# Create http request
	req = urllib2.Request(end_point, json.dumps(data), { 'Content-Type': 'application/json' } )

	# Setup http basic auth
	base64string = base64.encodestring( '%s:%s' % (token_id, token_key ) ).replace( '\n', '' )
	req.add_header( 'Authorization', 'Basic %s' % base64string )

	# Send request and fetch response
	response = urllib2.urlopen(req)

	# Print the http response code on success
	print( 'Send successful\nResponse code from server:{} '.format( response.getcode() ) )

except urllib2.HTTPError, e:
	print( 'HTTP code is {} and reason is {}'.format( e.code, e.reason ) )

except Exception, e:
    print( 'Exception is {} '.format( str( e ) ) )
