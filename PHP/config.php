<?php

// config
$token = ''; // set the token here
$secret = ''; // set the secret here

$host = 'opentsdb.iot.runabove.io';
$url = 'https://opentsdb.iot.runabove.io';
$path = '/api/put';

$datas = [
	[
		'metric'	=>  'temp.home',
		'timestamp'	=> time(),
		'value'		=> 1234,
		'tags'		=> [
			// uncoment to get an error
			// 'tag3' => 123,
			'tag1'	=> 'abc1',
			'tag2'	=> 'def2',
		],
	]
];
