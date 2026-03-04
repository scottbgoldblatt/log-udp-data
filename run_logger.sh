#!/usr/bin/env bash

# ===== Default values =====
DEFAULT_IP="192.168.10.5"
DEFAULT_PORT="25001"

# ===== Resolve IP =====
if [ -z "$1" ]; then
    BIND_IP="$DEFAULT_IP"
else
    BIND_IP="$1"
fi

# ===== Resolve PORT =====
if [ -z "$2" ]; then
    BIND_PORT="$DEFAULT_PORT"
else
    BIND_PORT="$2"
fi

# ===== Check executable exists =====
if [ ! -f "./hrg_logger" ]; then
    echo "ERROR: hrg_logger not found in this folder."
    echo "Make sure this script is next to the executable."
    exit 1
fi

echo "============================================"
echo "Starting hrg_logger"
echo "Bind IP   : $BIND_IP"
echo "Bind Port : $BIND_PORT"
echo "============================================"
echo

# Run the program
./hrg_logger "$BIND_IP" "$BIND_PORT"

echo
echo "Program exited."