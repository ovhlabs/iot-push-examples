#!/bin/bash
#
# Gather Statistics from Linux Systems and
# and send them.
#
# Fork of https://github.com/reyjrar/graphite-scripts/blob/master/bin/linux_basic_performance_data.sh. 
#

LANGUAGE=en_US

# Function Declarations

JSON_CACHE=$(mktemp)
trap 'rm -f $JSON_CACHE' INT TERM EXIT
first=1

function add_metric() {
    [[ $first -ne 1 ]] && PREFIX=,
    echo $1':'$2 >> $JSON_CACHE
    first=0
}

function format() {
    cat "$JSON_CACHE"
}

#------------------------------------------------------------------------#
# Pre Check Routines
# Hard Disk Monitoring
disk_prefixes=( 'sd' 'hd' 'c0d' 'c1d' )
declare -r disks_prefixes

#------------------------------------------------------------------------#
# Disk Discovery
if [ -f /proc/partitions ]; then
    while read line
    do
        disk=`echo $line |awk '{print $4}'`;
        for prefix in "${disk_prefixes[@]}"; do
            [ -z "$disk" ] && continue;

            (( $CARBON_DEBUG )) && echo " => check: '$disk' =~ '$prefix' : $matched";
            if [[ "$disk" =~ "$prefix" ]]; then
                disks[${#disks[*]}]="$disk";
                (( $CARBON_DEBUG )) && echo "DISK: $disk";
                break
            fi;
        done;
    done < /proc/partitions;
fi

#------------------------------------------------------------------------#
# Load Average
if [ -f /proc/loadavg ]; then
    load=`cat /proc/loadavg`;
    set -- $load;
    add_metric "load.1min" "$1";
    add_metric "load.5min" "$2";
    add_metric "load.15min" "$3";
else
    : # Code to rely on uptime
fi;

# CPU Stats
if [ -x /usr/bin/mpstat ]; then
    /usr/bin/mpstat |grep '^[0-9]' | grep -v CPU | while read line; do
        set -- $line;
        cpu=$3;

        add_metric "cpu.${cpu}.user" "$4";
        add_metric "cpu.${cpu}.nice" "$5";
        add_metric "cpu.${cpu}.system" "$6";
        add_metric "cpu.${cpu}.iowait" "$7";
    done;
fi;

# IO Stats
iostat_line=`iostat |awk 'FNR==4'`;
rc=$?;
if [ $rc -eq 0 ]; then
    set -- $iostat_line;
    add_metric "iostat.user" "$1";
    add_metric "iostat.nice" "$2";
    add_metric "iostat.system" "$3";
    add_metric "iostat.iowait" "$4";
fi;

# Use Free -mo to get memory details
/usr/bin/free -mob | tail -2 | while read line; do
    set -- $line;
    k=`echo $1 | tr [A-Z] [a-z] | sed -e s/://`;
    add_metric "memory.$k.total" "$2";
    add_metric "memory.$k.used" "$3";
    add_metric "memory.$k.free" "$4";
    [ ! -z $5 ] && add_metric "memory.$k.shared" "$5";
    [ ! -z $6 ] && add_metric "memory.$k.buffers" "$6";
    [ ! -z $7 ] && add_metric "memory.$k.cached" "$7";
done;

# Disk Performance Information
if [ ${#disks} -gt 0 ]; then
    if [ -f /proc/diskstats ]; then
        while read line; do
            set -- $line;
            if [[ "${disks[@]}" =~ "$3" ]]; then
                disk=$3
                disk=${disk/\//_};
                add_metric "disks.$disk.read.issued" "$4";
                add_metric "disks.$disk.read.ms" "$7";
                add_metric "disks.$disk.write.complete" "$8";
                add_metric "disks.$disk.write.ms" "${11}";
                add_metric "disks.$disk.io.current" "${12}";
                add_metric "disks.$disk.io.ms" "${13}";
            fi;
        done < /proc/diskstats;
    fi;
fi;

# File System Data
df -Pl -x tmpfs | while read line; do
    set -- $line;

    dev=$1;
    total=$2;
    used=$3;
    available=$4;
    percentage=$5;
    path_orig=$6;

    if [[ "$dev" =~ "^\/dev" ]]; then
        case "$path_orig" in
                "/")    path="slash";;
            "/boot")    path="boot";;
                  *)    tmp=${6:1}; path=${tmp//\//_};;
        esac;
        add_metric "fs.$path.total" "$total";
        add_metric "fs.$path.used" "$used";
        add_metric "fs.$path.available" "$available";
    fi;
done;

# Grab TCP Connection Data
/bin/netstat -s --tcp |zgrep "(connections.* opening|connexions* ouvertes)" | while read line; do
    set -- $line;
    add_metric "tcp.connections.$2" "$1";
done;
tcp_failed=`/bin/netstat -s --tcp |egrep "(failed connection attempts|tentatives de connexion.*hou)"|awk '{print $1}'`;
add_metric "tcp.connections.failed" "$tcp_failed";

# Grab TCP Reset Data
/bin/netstat -s --tcp |grep reset|grep -v due |awk '{print $1 " " $NF}' | while read line; do
    set -- $line;
    add_metric "tcp.resets.$2" "$1";
done;

# Grab UDP Packet Data
/bin/netstat -s --udp|grep packets|grep -v unknown | while read line; do
    set -- $line;
    add_metric "udp.packets.$3" "$1";
done;

format