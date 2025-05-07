#!/bin/bash

echo "[*] Compiling sources..."
gcc -Wall -o treasure_manager treasure.c  || exit 1
gcc -Wall -o monitor monitor.c || exit 1
gcc -Wall -o treasure_hub treasure_hub.c || exit 1


REPEAT_COUNT=3

for i in $(seq 1 $REPEAT_COUNT); do
    echo
    echo "= Test Run #$i ="
    HUNT_NAME="HuntTest$i"
    TREASURE_ID=0

    echo "[*] Cleaning previous hunt..."
    rm -rf "$HUNT_NAME" "logged_hunt-$HUNT_NAME" .hub_cmd hub_output.log monitor.log 2>/dev/null

    echo "[*] Adding treasure to $HUNT_NAME..."
    echo -e "user$i\n45.$i\n21.$i\nTest clue $i\n${i}00" | ./treasure_manager --add "$HUNT_NAME"

    echo "[*] Starting treasure_hub in background..."
    (./treasure_hub &> hub_output.log) &
    HUB_PID=$!
    sleep 1

    echo "[*] Starting monitor process..."
    echo "start_monitor" > .hub_cmd
    killall -USR1 monitor 2>/dev/null
    sleep 1

    echo "[*] list_hunts..."
    echo "list_hunts" > .hub_cmd
    killall -USR1 monitor
    sleep 1

    echo "[*] list_treasures $HUNT_NAME..."
    echo "list_treasures $HUNT_NAME" > .hub_cmd
    killall -USR1 monitor
    sleep 1

    echo "[*] view_treasure $HUNT_NAME $TREASURE_ID..."
    echo "view_treasure $HUNT_NAME $TREASURE_ID" > .hub_cmd
    killall -USR1 monitor
    sleep 1

    echo "[*] Stopping monitor..."
    echo "stop_monitor" > .hub_cmd
    killall -USR2 monitor
    sleep 1

    echo "[*] Exiting treasure_hub..."
    echo "exit" > .hub_cmd
    kill "$HUB_PID" 2>/dev/null
    wait "$HUB_PID" 2>/dev/null

    echo
    echo " = Contents of logged_hunt-$HUNT_NAME ="
    cat "$HUNT_NAME/logged_hunt" || echo "(no log found)"
    echo
    echo " = Listing treasures for $HUNT_NAME ="
    ./treasure_manager --list "$HUNT_NAME"
done

echo
echo " All $REPEAT_COUNT test runs completed successfully."
