#!/bin/bash

HUNT_ID=testhunt

echo "-- Test Start --"

# Clean up if needed
if [ -d "$HUNT_ID" ]; then
    echo "Cleaning previous hunt directory..."
    ./treasure_manager --remove_hunt $HUNT_ID
fi

# 1. Add treasure
echo
echo "-- Adding treasure --"
{
    echo "user1"
    echo "45.1"
    echo "25.1"
    echo "Clue for treasure 1"
    echo "100"
} | ./treasure_manager --add $HUNT_ID

# 2. List treasures
echo
echo "-- Listing treasures --"
./treasure_manager --list $HUNT_ID

# 3. View a specific treasure
echo
echo "-- Viewing Treasure ID 1 --"
./treasure_manager --view $HUNT_ID 1

# 4. Remove a treasure
echo
echo "-- Removing Treasure ID 1 --"
./treasure_manager --remove_treasure $HUNT_ID 1

# 5. List again after removal
echo
echo "-- Listing treasures after removal --"
./treasure_manager --list $H_
