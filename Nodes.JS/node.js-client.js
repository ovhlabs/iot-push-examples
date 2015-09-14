/**
 * Nodes.js sample code to put data in IoT Lab using a http session
 */

// Library required to execute POST requests
var request = require("request");

var TOKEN_ID = "w3eaaaqaa7pff"; // Your token id goes here
var TOKEN_KEY = "xyz123xyz123xyz123xyz123"; // your token goes here

// Data object to send
var data = [{
			metric:"home.temp.indoor",
			timestamp:1437591600,
			value:22.5,
			tags:{
				source:"dht22"
			}
		},
		{
			metric:"home.temp.outdoor",
			timestamp:1437591600,
			value:28.2,
			tags:{
				source:"ds18b20"
			}
		},
		{
			metric:"home.temp.indoor",
			timestamp:1437593400,
			value:22.1,
			tags:{
				source:"dht22"
			}
		},
		{
			metric:"home.temp.outdoor",
			timestamp:1437593400,
			value:27.1,
			tags:{
				source:"ds18b20"
			}
		}];


// Send post request to the IoT Lab OpenTSDB
request({
	uri: "https://opentsdb.iot.runabove.io/api/put",
	auth: {
		user: TOKEN_ID,
		pass: TOKEN_KEY,
		sendImmediately: true
	},
	method: "POST",
	json: data
}, function (error, response, body) {
	if (error) {
		console.log(error);
	} else if (response.statusCode >= 400) {
		console.log(body);
		console.log(response.statusMessage);
	} else {
		console.log("Success!");
	}
});
