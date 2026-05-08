#!/bin/bash

# Ensure any error stops the script immediately
set -e

echo "Starting workflow sequence (UTC)..."

# 1. Turn outlet ON
# Replace the string below with your actual command
/usr/bin/python3 -c "import subprocess; subprocess.run(['echo', 'Turning Outlet ON'])"

# 2. Wait 2 minutes
echo "Waiting 120 seconds for power stabilization..."
sleep 120

# 3. Start the background binary
# Use & to push it to the background
/usr/bin/binary_two &
BIN_TWO_PID=$!
echo "Started binary_two with PID: $BIN_TWO_PID"

# 4. Wait 1 minute
echo "Waiting 60 seconds before launching archive binary..."
sleep 60

# 5. Generate UTC date/time and run the third binary
# Format: MM-DD-YY-HH-MM
CURRENT_TIME=$(date -u +"%m-%d-%y-%H-%M")
echo "Starting archive binary for timestamp: $CURRENT_TIME"

# This script runs until completion
/usr/bin/binary_three --csv 500000 "/archive/$CURRENT_TIME"

# 6. Once binary_three ends, kill binary_two
echo "Archive complete. Killing binary_two (PID: $BIN_TWO_PID)..."
kill $BIN_TWO_PID

# 7. Turn outlet OFF
/usr/bin/python3 -c "import subprocess; subprocess.run(['echo', 'Turning Outlet OFF'])"

echo "Workflow sequence finished successfully."