#!/usr/bin/env bash

# ---- Usage check ----
if [ -z "$1" ]; then
    echo
    echo "Usage:"
    echo "  ./run_udp_test_sender.sh DEST_PORT [DEST_IP] [NUM_PACKETS] [INTERVAL_MS]"
    echo
    echo "Example:"
    echo "  ./run_udp_test_sender.sh 25001"
    echo "  ./run_udp_test_sender.sh 25001 127.0.0.1 10 100"
    echo
    exit 1
fi

PORT="$1"
IP="${2:-127.0.0.1}"
COUNT="${3:-1}"
INTERVAL="${4:-200}"

# ---- Locate executable ----
EXE="./udp_test_sender_boost"

if [ ! -f "$EXE" ]; then
    if [ -f "./build/bin/udp_test_sender_boost" ]; then
        EXE="./build/bin/udp_test_sender_boost"
    elif [ -f "./build/udp_test_sender_boost" ]; then
        EXE="./build/udp_test_sender_boost"
    else
        echo "Error: udp_test_sender_boost executable not found."
        exit 1
    fi
fi

echo
echo "Sending to $IP:$PORT"
echo "Packets: $COUNT"
echo "Interval: $INTERVAL ms"
echo

"$EXE" "$IP" "$PORT" "$COUNT" "$INTERVAL"