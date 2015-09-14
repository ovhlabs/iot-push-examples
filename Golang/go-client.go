// OpenTSDB data push example in Go lang
// Push data using a telnet-tls session or a https session
//
// The parameters are set in the file iotpush-config.json and the data file specified by the configuration
// in this example, the data file is iotpush-data.txt
//
// iotpush-config.json parameters:
// - dataFile: the path to the data file
// - sendType: type of protocol used, https or tls
// - endPoint: the address of the endpoint, for example opentsdb-internal.iot.runabove.io:4243 for a telnet session or https://opentsdb.iot.runabove.io/api/put for a http session
// - tokenWrite: A valid IoT Lab WRITE token
// - tagList: A list of tags that will be added to all values put, format key:value
//
// data file format:
// <metric> <timestamp> <value> <additional tags>
// - metric: the metric name to use for this data
// - timestamp: the timestamp of the data in Unix Epoch format in seconds
// - value: the value of the data
// - additional tags: any additional tag you want to add to your data
package main

import (
	"bufio"
	"bytes"
	"crypto/tls"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"strconv"
	"strings"
	"time"
)

// Structure of the config file
type Config struct {
	Endpoint string            `json:"endPoint"` // url of the endpoint
	TokenId  string            `json:"tokenId"`  // Write token id
	TokenKey string            `json:"tokenKey"` // Token key
	TagList  map[string]string `json:"tagList"`  // a list of tags to add to all put values
	SendType string            `json:"sendType"` // send type (https or tls)
	DataFile string            `json:"dataFile"` // file containing the data to send
}

// Structure of the data file
type Data struct {
	MetricName string            `json:"metric"`    // Metric value
	Timestamp  int64             `json:"timestamp"` // Timestamp in Unix Epoch format in seconds
	Data       float64           `json:"value"`     // Data payload
	TagList    map[string]string `json:"tags"`      // a list of tags to add to this payload
}

// Main function
func main() {
	// Open the file iotpush.conf in the same directory
	config, err := getConfig("iotpush-config.json")

	if err == nil {
		if dataList, err := getData(config); err == nil {
			// Checks the SendType option and use the proper function
			if config.SendType == "tls" {
				fmt.Println("type tls")
				if err = SendByTelnet(dataList, config.Endpoint, config.TokenId, config.TokenKey); err == nil {
					fmt.Println("Send complete")
				} else {
					fmt.Println(err)
				}
			} else if config.SendType == "https" {
				fmt.Println("type https")
				if err = SendByHttp(dataList, fmt.Sprint("https://", config.Endpoint, "/api/put"), config.TokenId, config.TokenKey); err == nil {
					fmt.Println("Send complete")
				} else {
					fmt.Println(err)
				}
			} else {
				fmt.Println("Error, send type unknown")
			}
		} else {
			fmt.Println(err)
		}
	} else {
		fmt.Println(err)
	}
}

// Open the config file and return the parsed configuration
func getConfig(configFile string) (*Config, error) {
	var config Config

	data, err := ioutil.ReadFile(configFile)
	if err != nil {
		return nil, errors.New(fmt.Sprint("Error reading config file, ", err))
	}

	if err = json.Unmarshal(data, &config); err == nil {
		return &config, nil
	} else {
		return nil, errors.New(fmt.Sprint("Error loading config file, ", err))
	}
}

// Open the data file and return the parsed payload
func getData(config *Config) ([]Data, error) {
	inFile, err := os.Open(config.DataFile)
	if err != nil {
		return nil, errors.New(fmt.Sprint("Error reading data file: ", err))
	}
	var dataList []Data
	defer inFile.Close()
	scanner := bufio.NewScanner(inFile)
	scanner.Split(bufio.ScanLines)

	for scanner.Scan() {
		if oneLine := strings.Split(scanner.Text(), " "); len(oneLine) >= 3 {
			// Line must contain a metric name, a timestamp in UNIX EPOCH format, and a value
			var toAppend = true
			var oneData Data
			oneData.TagList = make(map[string]string)
			for key, value := range config.TagList {
				oneData.TagList[key] = value
			}
			for i, elt := range oneLine {
				if i == 0 {
					// Metric name
					oneData.MetricName = elt
				} else if i == 1 {
					// Epoch timestamp
					oneData.Timestamp, err = strconv.ParseInt(elt, 10, 64)
					if err != nil {
						oneData.Timestamp = time.Now().Unix()
					}
				} else if i == 2 {
					// data
					oneData.Data, err = strconv.ParseFloat(elt, 64)
					if err != nil {
						toAppend = false
					}
				} else {
					// tag (format key=value)
					oneTag := strings.Split(elt, "=")
					if len(oneTag) == 2 {
						oneData.TagList[oneTag[0]] = oneTag[1]
					}
				}
			}
			if toAppend {
				dataList = append(dataList, oneData)
			}
		}
	}
	return dataList, nil
}

// Converts a tag list to a string
func TagListToString(tagList map[string]string) string {
	values := make([]string, 0, len(tagList))
	for key, value := range tagList {
		values = append(values, fmt.Sprintf("%s=%s", key, value))
	}
	return strings.Join(values, " ")
}

// Push data using a telnet tls session
func SendByTelnet(dataList []Data, endpoint string, tokenId string, tokenKey string) error {
	host, _, err := net.SplitHostPort(endpoint)
	if err != nil {
		return err
	}

	var readLine []byte
	tlc := &tls.Config{
		ServerName:         host,
	}

	fmt.Println("Open connection")
	connTls, err := tls.Dial("tcp", endpoint, tlc)
	defer connTls.Close()

	if err != nil {
		return errors.New(fmt.Sprint("Error opening connexion:", err))
	}

	fmt.Println("Connection open, sending auth command")
	putLine := fmt.Sprintf("auth %s:%s\n", tokenId, tokenKey)
	fmt.Fprintf(connTls, putLine)

	message, err := bufio.NewReader(connTls).ReadString('\n')
	if err != nil {
		return err
	}

	fmt.Print("Response to auth command: ", message)
	for _, oneData := range dataList {
		putLine := fmt.Sprintf("put %s %d %f %s\n", oneData.MetricName, oneData.Timestamp, oneData.Data, TagListToString(oneData.TagList))
		fmt.Fprintf(connTls, putLine)

		length, err := bufio.NewReader(connTls).Read(readLine)
		if err != nil {
			return err
		}

		if length > 0 {
			fmt.Println(fmt.Sprint("Error pushing data", putLine, "response is: ", string(readLine)))
		}
	}
	fmt.Fprintf(connTls, "exit\n")
	return nil
}

// Push the data using the OpenTSDB REST API
func SendByHttp(dataList []Data, endpoint string, tokenId string, tokenKey string) error {

	var err error
	client := http.Client{}
	b := make([]byte, 0)
	if dataList != nil {
		b, err = json.Marshal(dataList)
		if err != nil {
			return err
		}
	}

	req, err := http.NewRequest("POST", endpoint, bytes.NewReader(b))
	if err == nil {
		req.SetBasicAuth(tokenId, tokenKey)
		resp, err := client.Do(req)
		defer resp.Body.Close()

		if err != nil {
			return err
		}

		body, err := ioutil.ReadAll(resp.Body)
		if err != nil {
			return err
		}
		fmt.Println(fmt.Sprintf("Response from server (if any): %s", string(body)))
	}
	return err
}
