<?php

// example with curl

include __DIR__ . '/config.php';
$jsonDatas = json_encode($datas);

// init curl
$process = curl_init($host);

// config the curl command
curl_setopt($process, CURLOPT_URL, $url . $path);
// get header in the return content
curl_setopt($process, CURLOPT_HEADER, 1);
curl_setopt($process, CURLOPT_USERPWD, $key . ':' . $token);
curl_setopt($process, CURLOPT_POST, 1);
// post data
curl_setopt($process, CURLOPT_POSTFIELDS, $jsonDatas);
// get the content
curl_setopt($process, CURLOPT_RETURNTRANSFER, true);

// do the query
$return = curl_exec($process);
$infos = curl_getinfo($process);
curl_close($process);

// print result
var_dump([
	$infos['http_code'],
	$return,
]);
