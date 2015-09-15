# -*- coding: utf8 -*-

###########################################################
# OpenTSDB REST API session in a HTTPS connection example #
###########################################################

from __future__ import print_function, unicode_literals
import json

# Requests library that hides much of the complexity of dealing with
# HTTP requests in Python. Can either be installed with pip:
#     pip install requests
# or with your package manager:
#     apt-get install python-requests
#     yum install python-requests
import requests

# Place your token id here
token_id = 'w3eaaaqaa7pff'

# Place your token here
token_key = 'xyz123xyz123xyz123xyz123'

# IoT Lab OpenTSDB Endpoint
end_point = 'https://opentsdb.iot.runabove.io/api/put'

# Raw body data to send
data = [
    {
        'metric': 'home.temp.indoor',
        'timestamp': 1437591600,
        'value': 22.5,
        'tags': {
            'source': 'dht22'
        }
    },
    {
        'metric': 'home.temp.outdoor',
        'timestamp': 1437591600,
        'value': 28.2,
        'tags': {
            'source': 'ds18b20'
        }
    },
    {
        'metric': 'home.temp.indoor',
        'timestamp': 1437593400,
        'value': 22.1,
        'tags': {
            'source': 'dht22'
        }
    },
    {
        'metric': 'home.temp.outdoor',
        'timestamp': 1437593400,
        'value': 27.1,
        'tags': {
            'source': 'ds18b20'
        }
    }
]

try:
    # Send request and fetch response
    response = requests.post(end_point, data=json.dumps(data),
                             auth=(token_id, token_key))

    # Raise error if any
    response.raise_for_status()

    # Print the http response code on success
    print('Send successful\nResponse code from server: {}'.
          format(response.status_code))

except requests.exceptions.HTTPError as e:
    print('HTTP code is {} and reason is {}'.format(e.response.status_code,
                                                    e.response.reason))
