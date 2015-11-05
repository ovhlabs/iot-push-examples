<?php
# Place your write token id here 
$token_id = 'jpbabaeayub543';

# Place your write token here
$token_key = '7cCA7GpIAChqYbNIxUrr7Uwk3';


# IoT Lab OpenTSDB Endpoint
$end_point = 'https://opentsdb.iot.runabove.io/api/put';

$data = [
    [
        'metric'=>'home.temp.indoor',
        'timestamp'=> time(),
        'value'=> rand(20, 35),
        'tags'=> [
            'source'=>'dht22'
        ]
    ],
    [
        'metric'=>'home.temp.outdoor',
        'timestamp'=> time(),
        'value'=> rand(5, 25),
        'tags'=> [
            'source'=>'dht22'
        ]
    ]
]
;



$curl = curl_init($end_point);
curl_setopt($curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
curl_setopt($curl, CURLOPT_USERPWD, $token_id.":".$token_key); //Your credentials goes here
curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
curl_setopt($curl, CURLOPT_VERBOSE, false);
curl_setopt($curl, CURLOPT_HEADER, false);
curl_setopt($curl, CURLOPT_POST, true);
curl_setopt($curl, CURLOPT_POSTFIELDS, json_encode($data));
curl_setopt($curl, CURLOPT_SSL_VERIFYPEER, false); //IMP if the url has https and you don't want to verify source certificate

$curl_response = curl_exec($curl);
curl_close($curl);

