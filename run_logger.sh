#!/usr/bin/env bash

IMU_IP="192.168.10.5"
PORT="25001"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

BIN_EXE=""
TXT_EXE=""

# Check current directory
[ -f "./bin_logger" ] && BIN_EXE="./bin_logger"
[ -f "./text_logger" ] && TXT_EXE="./text_logger"

# Check build directory
[ -f "./build/bin_logger" ] && [ -z "$BIN_EXE" ] && BIN_EXE="./build/bin_logger"
[ -f "./build/text_logger" ] && [ -z "$TXT_EXE" ] && TXT_EXE="./build/text_logger"

if [[ -z "$BIN_EXE" && -z "$TXT_EXE" ]]; then
 echo "ERROR: No logger executables found."
 echo "Checked:"
 echo "  ."
 echo "  ./build"
 exit 1
fi

# Detect if system owns IMU IP
if ifconfig | grep -q "$IMU_IP"; then
 BIND_IP="$IMU_IP"
else
 BIND_IP="0.0.0.0"
fi

echo
echo "IMU LOGGER"
echo

[[ -n "$BIN_EXE" ]] && echo "1 - Binary Logger (recommended)"
[[ -n "$TXT_EXE" ]] && echo "2 - Text Logger"
echo

read -p "Select logger [default 1]: " CHOICE
[[ -z "$CHOICE" ]] && CHOICE=1

if [[ "$CHOICE" == "1" ]]; then
 EXE="$BIN_EXE"
else
 EXE="$TXT_EXE"
fi

echo
echo "Starting $(basename "$EXE")"
echo "IP   : $BIND_IP"
echo "Port : $PORT"
echo

"$EXE" "$BIND_IP" "$PORT"