# -*- coding: utf8 -*-

###############################################################################################
#                   OpenTSDB REST API session in a HTTPS connection library                   #
# from https://github.com/runabove/iot-push-examples/blob/master/Python/python-client-http.py #
###############################################################################################

from __future__ import print_function, unicode_literals
import json

# Requests library that hides much of the complexity of dealing with
# HTTP requests in Python. Can either be installed with pip:
#     pip install requests
# or with your package manager:
#     apt-get install python-requests
#     yum install python-requests
import requests
from Metric import Metric
from OpenTSDBProfile import OpenTSDBProfile

class OpenTSDBPusher:

    def pushData(self, openTSDBProfile, metric):
        try:
            # Send request and fetch response
            response = requests.post(openTSDBProfile.url, data=metric.getAsJsonString(),
                                     auth=(openTSDBProfile.token_id, openTSDBProfile.token_password))

            # Raise error if any
            response.raise_for_status()

            # Print the http response code on success
            print('Send successful\nResponse code from server: {}'.
                  format(response.status_code))

        except requests.exceptions.HTTPError as e:
            print('HTTP code is {} and reason is {}'.format(e.response.status_code,
                                                            e.response.reason))
            print('data : ' + metric.getAsJsonString())
