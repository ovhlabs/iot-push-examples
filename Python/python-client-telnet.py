# -*- coding: utf8 -*-

#######################################################
# OpenTSDB telnet session in a TLS connection example #
#######################################################

from __future__ import print_function, unicode_literals
from socket import socket, AF_INET, SOCK_STREAM, SHUT_RDWR
from ssl import wrap_socket, PROTOCOL_TLSv1

# Place your token here
token_id = 'w3eaaaqaa7pff'
token_key = 'xyz123xyz123xyz123xyz123'
opentsdb_host = 'opentsdb.iot.runabove.io'
opentsdb_port = 4243

# Create socket
client_socket = socket(AF_INET, SOCK_STREAM)
client_socket.settimeout(1)

tls_client = wrap_socket(client_socket, ssl_version=PROTOCOL_TLSv1)

print('Opening connection')
# Connect to the echo server
tls_client.connect((opentsdb_host, opentsdb_port))

print('Authenticating')
# Send auth command
tls_client.send(('auth {}:{}\n'.format(token_id, token_key)).encode('utf-8'))

# Read received data
data_in = tls_client.recv(1024)

# Decode and print message
response = data_in.decode()
print('Read response: ' + response)

if response == 'ok\n':
    # Send data (replace metric, timestamp, data and tags with your own values)
    print('Sending data')
    tls_client.send(b'put home.temp.indoor 1437591600 22.5 source=dht22\n')
    tls_client.send(b'put home.temp.outdoor 1437591600 28.2 source=ds18b20\n')
    tls_client.send(b'put home.temp.indoor 1437593400 22.1 source=dht22\n')
    tls_client.send(b'put home.temp.outdoor 1437593400 27.1 source=ds18b20\n')
    print('Data sent')
else:
    print('Auth failed, not sending data.')

# Send exit command to close connection
tls_client.send(b'exit\n')

# Close the socket
tls_client.shutdown(SHUT_RDWR)
tls_client.close()
