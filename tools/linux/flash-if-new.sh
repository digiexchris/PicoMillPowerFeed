#!/bin/bash
# filepath: /home/chris/repos/PicoMillPowerFeed/tools/smart_flash.sh

# Get the binary path from arguments
BINARY="$1"

# Set the frequency for flashing
FREQUENCY="$2"
if [ -z "$FREQUENCY" ]; then
  FREQUENCY="5000000"  # Default to 5MHz if not specified
fi

# Get the build directory
BUILD_DIR=$(dirname "$BINARY")
TIMESTAMP_FILE="${BUILD_DIR}/flash_timestamp"

# Echo what we're doing
echo "Checking if flashing is needed for: $BINARY"

# If timestamp file doesn't exist or is older than binary, flash is needed
if [ ! -f "$TIMESTAMP_FILE" ] || [ "$BINARY" -nt "$TIMESTAMP_FILE" ]; then
    echo "Binary has changed or first run, flashing..."
    
    # Flash using PyOCD
    echo "Using PyOCD with frequency $FREQUENCY Hz"
    pyocd flash --target rp2040 --frequency "$FREQUENCY" "$BINARY"
    
    # Check if flash was successful
    if [ $? -eq 0 ]; then
        # Update timestamp
        touch "$TIMESTAMP_FILE"
        echo "Flash successful, timestamp updated."
        exit 0
    else
        echo "Flash failed!"
        exit 1
    fi
else
    echo "Binary unchanged, skipping flash."
    exit 0
fi