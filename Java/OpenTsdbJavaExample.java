/**
 * OpenTSDB data push example in Java
 * Push data using a telnet-tls session or a https session
 * Requires Java 8 because of the use of java.util.Base64 package
 *
 * To run the program with the OpenJRE VM, you can use the command
 * java OpenTsdbJavaExample -https
 * or
 * java OpenTsdbJavaExample -tls
 */

import javax.net.ssl.*;
import java.io.*;
import java.net.*;

public class OpenTsdbJavaExample {

	/**
	 * Put here your write token id
	 */
	private static final String TOKEN_ID = "w3eaaaqaa7pff";

	/**
	 * Put here your write token key
	 */
	private static final String TOKEN_KEY = "xyz123xyz123xyz123xyz123";

	/**
	 * Hostname of the endpoint
	 */
	private static final String OPENTSDB_SERVER = "opentsdb.iot.runabove.io";

	/**
	 * Port number for telnet session
	 */
	private static final int OPENTSDB_PORT = 4243;

	/**
	 * Main method
	 */
	public static void main(String args[]) {
		if (args.length > 0) {
			if ("-tls".equals(args[0])) {
				OpenTsdbJavaExample.sendByTelnet();
			} else if ("-https".equals(args[0])) {
				OpenTsdbJavaExample.sendByHttp();
			} else {
				System.out.println("Error, method "+args[0]+" unknown, please use -https or -tls");
			}
		} else {
			System.out.println("Error, no method specified, please use -https or -tls");
		}
	}

	/**
	 * OpenTSDB REST API session in a HTTPS connection example
	 */
	private static void sendByHttp() {
		HttpURLConnection conn = null;

		// Payload to send as body in json raw data
		// Prefer using a JSON library like org.json or Jackson to avoid any encoding problem
		String input = "";
		input += "[{";
		input += "	\"metric\":\"home.temp.indoor\",";
		input += "	\"timestamp\":1437591600,";
		input += "	\"value\":22.5,";
		input += "	\"tags\":{";
		input += "		\"source\":\"dht22\"";
		input += "	}";
		input += "},";
		input += "{";
		input += "	\"metric\":\"home.temp.outdoor\",";
		input += "	\"timestamp\":1437591600,";
		input += "	\"value\":28.2,";
		input += "	\"tags\":{";
		input += "		\"source\":\"ds18b20\"";
		input += "	}";
		input += "},";
		input += "{";
		input += "	\"metric\":\"home.temp.indoor\",";
		input += "	\"timestamp\":1437593400,";
		input += "	\"value\":22.1,";
		input += "	\"tags\":{";
		input += "		\"source\":\"dht22\"";
		input += "	}";
		input += "},";
		input += "{";
		input += "	\"metric\":\"home.temp.outdoor\",";
		input += "	\"timestamp\":1437593400,";
		input += "	\"value\":27.1,";
		input += "	\"tags\":{";
		input += "		\"source\":\"ds18b20\"";
		input += "	}";
		input += "}]";

		try {
			// Initialize connection parameters
			URL url = new URL("https://"+OPENTSDB_SERVER+"/api/put");
			conn = (HttpURLConnection) url.openConnection();

			// Set HTTP Basic Auth
			conn.setRequestProperty("Authorization", "Basic "+java.util.Base64.getEncoder().encodeToString((TOKEN_ID+":"+TOKEN_KEY).getBytes()));

			conn.setDoOutput(true);
			conn.setRequestMethod("POST");
			conn.setRequestProperty("Content-Type", "application/json");

			// Send the payload as outputstream
			OutputStream requestBody = conn.getOutputStream();
			requestBody.write(input.getBytes());
			requestBody.flush();

			// Parse the response from the server
			if (conn.getResponseCode() >= 400) {
				// Error from the server
				System.out.println("Send failed, HTTP error code : " + conn.getResponseCode());

				System.out.println("Output from Server, if any:");
				BufferedReader responseBody = new BufferedReader(new InputStreamReader((conn.getErrorStream())));

				// Print the response body in stdout
				String output;
				while ((output = responseBody.readLine()) != null) {
					System.out.println(output);
				}
			} else {
				// No error from the server
				BufferedReader responseBody = new BufferedReader(new InputStreamReader((conn.getInputStream())));

				// Print the response body in stdout
				String output;
				System.out.println("Send successful\nOutput from Server, if any:");
				while ((output = responseBody.readLine()) != null) {
					System.out.println(output);
				}
			}

		} catch (IOException e) {
			System.out.println(e.getMessage());
		} finally {
			conn.disconnect();
		}
     }

	/**
	 * OpenTSDB telnet session in a TLS connection example
	 */
	private static void sendByTelnet() {

        SSLSocket sslsocket = null;
        DataOutputStream outToServer = null;
        BufferedReader inFromServer = null;
        try {
			// Open the tls connection
			SSLSocketFactory factory = (SSLSocketFactory) SSLSocketFactory.getDefault();
			System.out.println("Opening connection");
			sslsocket = (SSLSocket) factory.createSocket(OPENTSDB_SERVER, OPENTSDB_PORT);

			// Initialize the read/write buffers
			outToServer  = new DataOutputStream(sslsocket.getOutputStream());
			inFromServer = new BufferedReader(new InputStreamReader(sslsocket.getInputStream()));

			// Send the auth command with the token
			outToServer.writeBytes("auth " + TOKEN_ID + ":" + TOKEN_KEY + "\n");

			// Read the auth command result
			String responseStr;
			if((responseStr = inFromServer.readLine()) != null) {
				System.out.println("Auth response: "+responseStr);
			}

			// Send the put commands and exit
			outToServer.writeBytes("put home.temp.indoor 1437591600 22.5 source=dht22\n");
			outToServer.writeBytes("put home.temp.outdoor 1437591600 28.2 source=ds18b20\n");
			outToServer.writeBytes("put home.temp.indoor 1437593400 22.1 source=dht22\n");
			outToServer.writeBytes("put home.temp.outdoor 1437593400 27.1 source=ds18b20\n");
			outToServer.writeBytes("exit\n");

			// Read the response from the server
			if((responseStr = inFromServer.readLine()) != null) {
				System.out.println("Message from server: " + responseStr);
			} else {
				System.out.println("Send complete");
			}

        } catch(IOException e) {
             System.out.println(e.getMessage());
		} finally {
			// Close the connection and the read and write buffers
			try {
				if (outToServer != null) {
					outToServer.close();
				}
			} catch (IOException e) {
				System.out.println(e.getMessage());
			}

			try {
				if (inFromServer != null) {
					inFromServer.close();
				}
			} catch (IOException e) {
				System.out.println(e.getMessage());
			}

			try {
				if (sslsocket != null) {
					sslsocket.close();
				}
			} catch (IOException e) {
				System.out.println(e.getMessage());
			}
		}
	}
}
