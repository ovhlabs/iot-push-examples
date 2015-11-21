# -*- coding: utf8 -*-

#
# Your OpenTSDB Account
#
# Only use the default constructor with your tokens
# and server URI
#

class OpenTSDBProfile:

    token_id = ""
    token_password = ""
    url = ""
    port = 0

    def __init__(self, token_id, token_password, url, port):
        self.token_id = token_id
        self.token_password = token_password
        self.url = url
        self.port = port
