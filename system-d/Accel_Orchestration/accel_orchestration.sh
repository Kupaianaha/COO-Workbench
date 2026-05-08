#!/bin/bash

# Ensure any error stops the script immediately
set -e

echo "Starting workflow sequence (UTC)..."

# 1. Turn outlet ON
# Replace the string below with your actual command
echo "modify -s k2aopower OUTLET_HC1=on"

# 2. Wait 2 minutes
echo "Waiting 120 seconds for power stabilization..."
sleep 120

# 3. Start the background binary
# Navigate to the directory and start the binary in the background
cd /home/nfiudev/HISPEC/Accel_System || exit
./KPIC_AccelReadout_ShmWriter &

# Capture the Process ID
BIN_TWO_PID=$!

echo "Started KPIC_AccelReadout_ShmWriter with PID: $BIN_TWO_PID"

# 4. Wait 1 minute
echo "Waiting 60 seconds before launching archive binary..."
sleep 60

# 5. Generate UTC date/time and run the third binary
# Format: MM-DD-YY-HH-MM
CURRENT_TIME=$(date -u +"%m-%d-%y-%H-%M")
CURRENT_DATE=$(date -u +"%m-%d-%y")
echo "Starting archive binary for timestamp: $CURRENT_TIME"

# This script runs until completion
cd /home/nfiudev/HISPEC/Accel_System || exit
./KPIC_AccelReadout_ShmReader --csv 500000 "/home/nfiudev/HISPEC/Accel_System/archive/$CURRENT_DATE/$CURRENT_TIME"

# 6. Once binary_three ends, kill binary_two
echo "Archive complete. Killing KPIC_AccelReadout_ShmWriter (PID: $BIN_TWO_PID)..."
kill $BIN_TWO_PID

# 7. Turn outlet OFF
echo "modify -s k2aopower OUTLET_HC1=off"

echo "Workflow sequence finished successfully."