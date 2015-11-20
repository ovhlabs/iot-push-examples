# -*- coding: utf8 -*-

###############################################################################################
#                   OpenTSDB REST API session in a HTTPS connection library                   #
# from https://github.com/runabove/iot-push-examples/blob/master/Python/python-client-http.py #
###############################################################################################

from __future__ import print_function, unicode_literals
from socket import socket, AF_INET, SOCK_STREAM, SHUT_RDWR
from ssl import wrap_socket, PROTOCOL_TLSv1
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
    	client_socket = socket(AF_INET, SOCK_STREAM)
		client_socket.settimeout(1)

		tls_client = wrap_socket(client_socket, ssl_version=PROTOCOL_TLSv1)

		print('Opening connection')
		# Connect to the echo server
		tls_client.connect((openTSDBProfile.url, openTSDBProfile.port))

		print('Authenticating')
		# Send auth command
		tls_client.send(('auth {}:{}\n'.format(openTSDBProfile.token_id, openTSDBProfile.token_password)).encode('utf-8'))

		# Read received data
		data_in = tls_client.recv(1024)

		# Decode and print message
		response = data_in.decode()
		print('Read response: ' + response)

		if response == 'ok\n':
		    # Send data
		    print('Sending data')
		    tagsAsString = ""
		    for key, value in metric.tags.iteritems():
		    	tagsAsString = tagsAsString + " " + key + "=" + value


		    tls_client.send(b'put '+ metric.metric +' '+ metric.timestamp +' '+ metric.value + ' ' + tagsAsString + '\n')
		    print('Data sent')
		else:
		    print('Auth failed, not sending data.')

		# Send exit command to close connection
		tls_client.send(b'exit\n')

		# Close the socket
		tls_client.shutdown(SHUT_RDWR)
		tls_client.close()