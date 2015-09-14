/**
 * JQuery sample code to put data in IoT Lab using a http session
 */

var TOKEN_ID = "w3eaaaqaa7pff"; // Your token id goes here
var TOKEN_KEY = "xyz123xyz123xyz123xyz123"; // your token key goes here

// Data object to send
var data =
		[{
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

// Use Ajax POST method to send data to the IoT Lab OpenTSDB
$.ajax("https://opentsdb.iot.runabove.io/api/put", {
	method: "POST",
	contentType: "application/json",
	beforeSend: function (xhr) {
		xhr.setRequestHeader ("Authorization", "Basic "+btoa(TOKEN_ID+":"+TOKEN_KEY));
	},
	data: JSON.stringify(data)
}).then(function () {
	alert("Success!");
}, function (error) {
	alert("Error: " + error.responseText);
});
