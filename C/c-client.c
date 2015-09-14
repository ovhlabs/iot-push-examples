/**
 * OpenTSDB data push example in C using the libraries ssl, crypto and libcurl
 * Push data using a telnet-tls session or a https session
 *
 * To compile using gcc, run the following command:
 * gcc -o c-client c-client.c -lssl -lcrypto -lcurl
 *
 * To run the program, use the following commands
 * '$ ./c-client -https' or '$ ./c-client -tls'
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <curl/curl.h>

// Simple structure to keep track of the handle, and
// of what needs to be freed later.
typedef struct {
	int socket;
	SSL *sslHandle;
	SSL_CTX *sslContext;
} connection;

// We'll be connecting to opentsdb.iot.runabove.io
#define IOT_SERVER  "opentsdb.iot.runabove.io"

// Port 4243 for tls connection
#define TELNET_PORT 4243

// Put here your write token id
#define TOKEN_ID "w3eaaaqaa7pff"

// Put here your token key
#define TOKEN_KEY "xyz123xyz123xyz123xyz123"

// Establish a regular tcp connection
int tcpConnect () {
	int error, handle;
	struct hostent *host;
	struct sockaddr_in server;

	host = gethostbyname (IOT_SERVER);
	handle = socket (AF_INET, SOCK_STREAM, 0);
	if (handle == -1) {
		perror ("Socket");
		handle = 0;
	} else {
		server.sin_family = AF_INET;
		server.sin_port = htons (TELNET_PORT);
		server.sin_addr = *((struct in_addr *) host->h_addr);
		bzero (&(server.sin_zero), 8);

		error = connect (handle, (struct sockaddr *) &server, sizeof (struct sockaddr));
		if (error == -1) {
			perror ("Connect");
			handle = 0;
		}
	}

	return handle;
}

// Establish a telnet connection using an SSL layer
connection *sslConnect (void) {
	connection *c;

	c = malloc (sizeof (connection));
	c->sslHandle = NULL;
	c->sslContext = NULL;

	c->socket = tcpConnect ();
	if (c->socket) {
		// Register the error strings for libcrypto & libssl
		SSL_load_error_strings ();

		// Register the available ciphers and digests
		SSL_library_init ();

		// New context saying we are a client, and using SSL 2 or 3
		c->sslContext = SSL_CTX_new (SSLv23_client_method ());
		if (c->sslContext == NULL)
			ERR_print_errors_fp (stderr);

		// Create an SSL struct for the connection
		c->sslHandle = SSL_new (c->sslContext);
		if (c->sslHandle == NULL)
			ERR_print_errors_fp (stderr);

		// Connect the SSL struct to our connection
		if (!SSL_set_fd (c->sslHandle, c->socket))
			ERR_print_errors_fp (stderr);

		// Initiate SSL handshake
		if (SSL_connect (c->sslHandle) != 1)
			ERR_print_errors_fp (stderr);
	} else {
		perror ("Connect failed");
	}

	return c;
}

// Disconnect & free connection struct
void sslDisconnect (connection *c) {
	if (c->socket)
		close (c->socket);
	if (c->sslHandle) {
		SSL_shutdown (c->sslHandle);
		SSL_free (c->sslHandle);
	}
	if (c->sslContext)
		SSL_CTX_free (c->sslContext);

	free (c);
}

// Read all available text from the connection
char *sslRead (connection *c) {
	const int readSize = 1024;
	char *rc = NULL;
	int received, count = 0;
	char buffer[1024];

	if (c) {
		while (1) {
			if (!rc) {
				rc = malloc (readSize * sizeof (char) + 1);
				// Initialize rc to NULL String
				if (rc != NULL)
					rc[0] = '\0';
			} else {
				rc = realloc (rc, (count + 1) * readSize * sizeof (char) + 1);
			}

			received = SSL_read (c->sslHandle, buffer, readSize);
			buffer[received] = '\0';

			if (received > 0)
				strcat (rc, buffer);

			if (received < readSize)
				break;
			count++;
		}
	}

	return rc;
}

// Write text to the connection
void sslWrite (connection *c, char *text) {
	if (c)
		SSL_write (c->sslHandle, text, strlen (text));
}

// Send the data using a telnet session on a TLS connection
void sendByTelnet() {

	connection *c;
	char *response;

	// Initiate the connection
	c = sslConnect ();

	// Initiate the authentication
	sslWrite (c, "auth " TOKEN_ID ":" TOKEN_KEY "\n");
	response = sslRead (c);

	// Read the response from the server (should send 'ok' if the token is valid)
	printf ("auth command response\n  %s\n", response);
	free (response);

	// Send the put commands one after the other
	sslWrite (c, "put home.temp.indoor 1437591600 22.5 source=dht22\n");
	sslWrite (c, "put home.temp.outdoor 1437591600 28.2 source=ds18b20\n");
	sslWrite (c, "put home.temp.indoor 1437593400 22.1 source=dht22\n");
	sslWrite (c, "put home.temp.outdoor 1437593400 27.1 source=ds18b20\n");
	sslWrite (c, "exit\n");

	// Read the response from the server. No response means no error
	response = sslRead (c);
	printf ("put command response (if any)\n  %s\n", response);
	free (response);

	// Disconnect from the server
	sslDisconnect (c);

}

// Send the data on a OpenTSDB REST API session using a https connection
void sendByHttp() {

	// Use libcurl to manage the http communication
	CURL *curl;
	CURLcode res;
	struct curl_slist *headers = NULL;
	long http_code = 0;

	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();
	if(curl) {
		// Set header values
		headers = curl_slist_append(headers, "Accept: application/json");
		headers = curl_slist_append(headers, "Content-Type: application/json");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		// Set endpoint url
		curl_easy_setopt(curl, CURLOPT_URL, "https://" IOT_SERVER "/api/put");

		// Set verb
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");

		// Set HTTP Basic Auth
		curl_easy_setopt(curl, CURLOPT_USERPWD, TOKEN_ID ":" TOKEN_KEY);

		// Set data posted
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS,
			"[{\
				\"metric\":\"home.temp.indoor\",\
				\"timestamp\":1437591600,\
				\"value\":22.5,\
				\"tags\":{\
					\"source\":\"dht22\"\
				}\
			},\
			{\
				\"metric\":\"home.temp.outdoor\",\
				\"timestamp\":1437591600,\
				\"value\":28.2,\
				\"tags\":{\
					\"source\":\"ds18b20\"\
				}\
			},\
			{\
				\"metric\":\"home.temp.indoor\",\
				\"timestamp\":1437593400,\
				\"value\":22.1,\
				\"tags\":{\
					\"source\":\"dht22\"\
				}\
			},\
			{\
				\"metric\":\"home.temp.outdoor\",\
				\"timestamp\":1437593400,\
				\"value\":27.1,\
				\"tags\":{\
					\"source\":\"ds18b20\"\
				}\
			}]");

		// Perform the connection
		res = curl_easy_perform(curl);
		if(res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}

		// Get and print the results
		curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
		printf("response code: %ld\n", http_code);

		// Clean the result after use
		curl_easy_cleanup(curl);
	}

	// Clean all the libcurl context before quit
	curl_global_cleanup();
}

// main function, check the cli argument and run the proper method
int main (int argc, char **argv) {

	// Check command-line arguments
	if (argc >= 2) {
		if (strcmp(argv[1], "-tls") == 0) {
			sendByTelnet();
		} else if (strcmp(argv[1], "-https") == 0) {
			sendByHttp();
		} else {
			printf("usage %s -tls|-https\n", argv[0]);
		}
	} else {
		printf("usage %s -tls|-https\n", argv[0]);
	}
	return 0;
}
