## Bash functions for the Metrics data platform API.

Create a [`creds`](creds.example) file with your tokens info.

Source [lib/metrics-functions.sh](lib/metrics-functions.sh) and write some Bash:

```
#!/bin/bash
set -euo pipefail

source lib/metrics-functions.sh
```

### Put

```
echo '{
    "metric":"foo",
    "timestamp":'$(date +%s)',
    "value":'0.$RANDOM',
    "tags":{}
}' | put
```

Or send key:value list with `kv_to_put_json` function:

```
echo '\
foo:42
bar:123' \
    | kv_to_put_json \
    | put
```

### Query

```
echo '{
    "start":0,
    "queries":[{
        "metric":"foo",
        "aggregator":"avg",
        "downsample":"10s-avg",
        "tags":{}
    }]
}' | query
```

### Delete

```
echo '{
    "start":0,
    "queries":[{
        "metric":"foo",
        "aggregator":"avg",
        "downsample":"10s-avg",
        "tags":{}
    }]
}' | delete
```

### List Metrics

```
listMetrics
```
