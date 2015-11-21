# -*- coding: utf8 -*-

import json
from json import JSONEncoder

#  
# A metric for an OpenTSDB container
#
class Metric:

    metric = ""
    value = 0.0
    timestamp = 0
    tags = {}
    
    def __init__(self, metricName, value, timestamp, tags):
        self.metric = metricName
        self.value = value
        self.timestamp = timestamp
        self.tags = tags

    def getAsJsonString(self):
        return MetricEncoder().encode(self)
        #return json.dumps(self)

class MetricEncoder(JSONEncoder):
    def default(self, o):
        return o.__dict__
