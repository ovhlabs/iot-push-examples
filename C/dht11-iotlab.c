/**
 *  dht11.c:
 *  Read temperature and humidity from a dht11 sensor on a Raspberry pi,
 *  then send it to the RunAbove IoT lab opentsdb server via a telnet session on a TLS connection
 *  the metrics used are <metric.prefix>.temp and <metric.prefix>.hum with the current timestamp
 *
 *  You need the libraries libssl-dev and wiringPi to compile
 *
 *  Compilation: gcc -o dht11-iotlab dht11-iotlab.c -lwiringPi -lssl -lcrypto
 *
 *  Usage: sudo ./dht11-iotlab <WRITE_TOKEN> <metric.prefix> [additional_tags]
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <wiringPi.h>

// We'll connect to opentsdb.iot.runabove.io
#define IOT_SERVER  "opentsdb.iot.runabove.io"

// Port 4243 for tls connection
#define TELNET_PORT 4243

#define WORDLENGTH  8
#define MAXTIMINGS  42

// Set here the pin where you have plugged your DHT11
#define DHTPIN      7

int dht11_dat[5] = { 0, 0, 0, 0, 0 };

enum status{valid, invalid};

typedef struct {
	char hum[WORDLENGTH+1];
	char temp[WORDLENGTH+1];
	int status;
} sensor_data;

sensor_data read_dht11_dat() {
	uint8_t laststate  = HIGH;
	uint8_t counter    = 0;
	uint8_t j        = 0, i;
	sensor_data data;

	dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;

	/* pull pin down for 18 milliseconds */
	pinMode( DHTPIN, OUTPUT );
	digitalWrite( DHTPIN, LOW );
	delay( 18 );
	/* then pull it up for 40 microseconds */
	digitalWrite( DHTPIN, HIGH );
	delayMicroseconds( 40 );
	/* prepare to read the pin */
	pinMode( DHTPIN, INPUT );

	/* detect change and read data */
	for ( i = 0; i < MAXTIMINGS; i++ ) {
		counter = 0;
		while ( digitalRead( DHTPIN ) == laststate ) {
			counter++;
			delayMicroseconds( 1 );
			if ( counter == 255 ) {
				break;
			}
		}
		laststate = digitalRead( DHTPIN );

		if ( counter == 255 )
			break;

		/* ignore first 3 transitions */
		if ( (i >= 4) && (i % 2 == 0) ) {
			/* shove each bit into the storage bytes */
			dht11_dat[j / 8] <<= 1;
			if ( counter > 16 )
				dht11_dat[j / 8] |= 1;
			j++;
		}
	}

	/**
	* check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
	* set sensor_data values if data is good
	* set sensor_data values if data is good
	*/
	if ( (j >= 40) && (dht11_dat[4] == ( (dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF) ) ) {
		snprintf(data.hum, WORDLENGTH, "%d.%d", dht11_dat[0], dht11_dat[1]);
		snprintf(data.temp, WORDLENGTH, "%d.%d", dht11_dat[2], dht11_dat[3]);
		data.status = valid;
	} else {
		data.status = invalid;
	}
	return data;
}

// Simple structure to keep track of the handle, and
// of what needs to be freed later.
typedef struct {
	int socket;
	SSL *sslHandle;
	SSL_CTX *sslContext;
} ssl_connection;

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
ssl_connection *sslConnect (void) {
	ssl_connection *c;

	c = malloc (sizeof (ssl_connection));
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
void sslDisconnect (ssl_connection *c) {
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
char *sslRead (ssl_connection *c) {
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
void sslWrite (ssl_connection *c, char *text) {
	if (c)
		SSL_write (c->sslHandle, text, strlen (text));
}

// Send the data using a telnet session on a TLS connection
void sendByTelnet(sensor_data data, char * token, char * metric, char * additional_tags) {

	ssl_connection *c;
	char *response, *response_auth, query[1024];
	time_t now;

	// Initiate the connection
	c = sslConnect ();

	// Initiate the authentication
	snprintf(query, 1023, "auth %s\n", token);
	sslWrite (c, query);
	response_auth = sslRead (c);

	// Read the response from the server (should send 'ok' if the token is valid)
	if (0 == strcmp("ok\n", response_auth)) {
		time(&now);

		// Send the put commands one after the other
		snprintf(query, 1023, "put %s.hum %ld %s %s\n", metric, now, data.hum, additional_tags);
		sslWrite (c, query);

		snprintf(query, 1023, "put %s.temp %ld %s %s\n", metric, now, data.temp, additional_tags);
		sslWrite (c, query);
	} else {
		printf("Error auth, response is %s\n", response_auth);
	}

	free (response_auth);

	sslWrite (c, "exit\n");
	// Read the response from the server. No response means no error
	response = sslRead (c);
	if (0 != strcmp("", response)) {
		printf ("Command put error\n  \"%s\"\n", response);
	}
	free(response);

	// Disconnect from the server
	sslDisconnect (c);
}

/**
 * Main function
 *
 * Parse the command line arguments, then every 5 minutes reads the sensor data
 * If the data is valid, push it to IoT lab
 *
 */
int main( int argc, char ** argv ) {
	char * additional_tags;
	size_t len = 0;
	int i;
	sensor_data data;

	if ( wiringPiSetup() == -1 )
	exit(EXIT_FAILURE);

	// Parse arguments
	if (argc >= 3) {
		if (argc >= 4) {
			// Concat additional tags arguments in a single char*
			for (i=3; i<argc; i++) {
				len += strlen(argv[i]);
			}
			len += (argc-2);
			additional_tags = malloc(len*sizeof(char));
			additional_tags[0] = '\0';
			for (i=3; i<argc; i++) {
				strcat(additional_tags, argv[i]);
				strcat(additional_tags, " ");
			}
		} else {
			additional_tags = malloc(sizeof(char));
			additional_tags[0] = '\0';
		}
		while (1) {
			data = read_dht11_dat();
			if (data.status == valid) {
				printf("%ld Send data hum=%s temp=%s\n", time(NULL), data.hum, data.temp);
				sendByTelnet(data, argv[1], argv[2], additional_tags);
			} else {
				printf("%ld Error getting sensor data, abort push data\n", time(NULL));
			}
			sleep(5*60); // Restart every 5 minutes
		}
	} else {
		printf("Usage %s token-id:token-key metric [additional tags]\n", argv[0]);
	}

	free(additional_tags);
	return(EXIT_SUCCESS);
}
