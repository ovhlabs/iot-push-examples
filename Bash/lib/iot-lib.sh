#
# IoT PAAS API put/query/utils bash functions.
#

IOT_API=https://opentsdb.iot.runabove.io
OPTS="-w {\"status\":%{http_code},\"time\":%{time_total}}\n"

[ ! -f creds ] && echo "error: please setup a creds file with write/read id and key tokens" && exit 1
source creds

# Put data
#
# @param stdin JSON data to put (http://opentsdb.net/docs/build/html/api_http/put.html)
#
put() {
    curl -s \
        -u $WRITE_TOKEN_ID:$WRITE_TOKEN_KEY \
        -XPOST "$IOT_API/api/put" \
        -d @- \
        $OPTS
}

# Query data
#
# @param stdin JSON data to qyery (http://opentsdb.net/docs/build/html/api_http/query.html)
#
query() {
    curl -s \
        -u $READ_TOKEN_ID:$READ_TOKEN_KEY \
        -XPOST "$IOT_API/api/query" \
        -d @- \
        $OPTS
}

# Utils

# Transform a key/value list in JSON 'OpenTSDB put' data.
# (http://opentsdb.net/docs/build/html/api_http/put.html)
#
# @param tag    optional JSON object for the tags (example: {"host": localhost})
#
# @return       a JSON array for the OpenTSDB put
#
kv_to_put_json() {
    local empty="{}"
    local tag=${1:-$empty}
    local now=$(date +%s)
    local comma=''
    local tags='"tags":'"$tag"
    
    echo '['
    while read line
    do
        echo "$line" | sed -r \
            -e 's|^(.*):([0-9\.]*)$|'$comma'{"metric":"\1","timestamp":'$now',"value":\2,'$tags'}|'
        comma=','
    done < /dev/stdin
    echo ']'
}
