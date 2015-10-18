<?php

// example with guzzle

// first init composer => 'composer install'

include __DIR__ . '/vendor/autoload.php';

use GuzzleHttp\Client;

include __DIR__ . '/config.php';

// create the client
$client = new Client([
	'base_uri' => $url,
	'timeout'  => 2.0,
]);

try
{
	// do the query
	$response = $client->request('POST', $path, [
		'json'		=> $datas,
		'headers'	=> [
			'Accept'		=> 'application/json',
			'Content-Type'	=> 'application/json',
		],
		'auth'		=> [$key, $token],
	]);

	// print the response
	var_dump([
		$response->getStatusCode(),
		$response->getReasonPhrase()
	]);
}
catch(\Exception $e)
{
	if (method_exists($e, 'getResponse'))
	{
		// get info about the error
		$response = $e->getResponse();
		var_dump([
			$response->getStatusCode(),
			$response->getReasonPhrase(),
			$response->getBody()->getContents(),
		]);
	}
	else
	{
		var_dump($e);
	}
}
