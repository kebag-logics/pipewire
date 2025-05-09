#!/bin/bash

# SPDX-FileCopyrightText: Copyright © 2022 Kebag-Logic
# SPDX-FileCopyrightText: Copyright © 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
# SPDX-FileCopyrightText: Copyright © 2025 Simon Gapp <simon.gapp@kebag-logic.com>
# SPDX-License-Identifier: MIT

# Part of the code taken from https://tsn.readthedocs.io/

# Use AVB_INTERFACE from environment if no argument is passed
IFACE="${1:-$AVB_INTERFACE}"

if [ -z "$IFACE" ]; then
    echo "Usage: $0 [<network-interface>]"
    echo "Selected interface: (none)"
    echo "If no interface is printed, make sure that AVB_INTERFACE is set in your environment or .bashrc"
    exit 1
fi

echo "Selected interface: $IFACE"

function kill_all() {
    echo "Stopping PTP services..."
    sudo pkill ptp4l
    sudo pkill phc2sys
    sudo timedatectl set-ntp true
}

trap kill_all SIGINT EXIT

sudo timedatectl set-ntp false

# Start ptp4l
sudo ptp4l -i "$IFACE" -f ~/linuxptp/configs/gPTP.cfg --step_threshold=1 &
PTP_PID=$!
sudo chrt -f -p 53 "$PTP_PID"

# Start phc2sys
sudo phc2sys -s "$IFACE" -c CLOCK_REALTIME --step_threshold=1 --transportSpecific=1 -w -m &
PHC2SYS_PID=$!
sudo chrt -f -p 52 "$PHC2SYS_PID"

# Set grandmaster settings
sudo pmc -u -b 0 -t 1 "SET GRANDMASTER_SETTINGS_NP clockClass 247 \
    clockAccuracy 0xfe offsetScaledLogVariance 0xffff \
    currentUtcOffset 37 leap61 0 leap59 0 currentUtcOffsetValid 1 \
    ptpTimescale 1 timeTraceable 1 frequencyTraceable 0 \
    timeSource 0xa0"

# Wait for background processes
wait "$PTP_PID"
wait "$PHC2SYS_PID"