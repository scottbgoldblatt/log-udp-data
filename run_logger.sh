#!/usr/bin/env bash

# ===== Default values =====
DEFAULT_IP="192.168.10.5"
DEFAULT_PORT="25001"

echo
echo "============================================"
echo "IMU LOGGER LAUNCHER"
echo "============================================"
echo
echo "Select logger type:"
echo "  1 - Binary Logger (recommended)"
echo "  2 - Text Logger"
echo

read -p "Enter choice (1 or 2) [default 1]: " CHOICE
if [ -z "$CHOICE" ]; then
    CHOICE=1
fi

echo

read -p "Enter IP address [default $DEFAULT_IP]: " BIND_IP
if [ -z "$BIND_IP" ]; then
    BIND_IP="$DEFAULT_IP"
fi

read -p "Enter port [default $DEFAULT_PORT]: " BIND_PORT
if [ -z "$BIND_PORT" ]; then
    BIND_PORT="$DEFAULT_PORT"
fi

echo
echo "============================================"
echo "Starting Logger"
echo "IP   : $BIND_IP"
echo "Port : $BIND_PORT"
echo "============================================"
echo

if [ "$CHOICE" = "1" ]; then
    if [ -f "./bin_logger" ]; then
        ./bin_logger "$BIND_IP" "$BIND_PORT"
    else
        echo "ERROR: bin_logger not found in this folder."
        exit 1
    fi
elif [ "$CHOICE" = "2" ]; then
    if [ -f "./text_logger" ]; then
        ./text_logger "$BIND_IP" "$BIND_PORT"
    else
        echo "ERROR: text_logger not found in this folder."
        exit 1
    fi
else
    echo "Invalid choice."
    exit 1
fi

echo
echo "Program exited."