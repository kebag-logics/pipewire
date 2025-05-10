#!/bin/bash

# SPDX-FileCopyrightText: Copyright © 2022 Kebag-Logic */
# SPDX-FileCopyrightText: Copyright © 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
# SPDX-License-Identifier: MIT

# Some of hte code is taken from https://tsn.readthedocs.io/
NIC=${1}

# Clean Everything first
sudo tc qdisc del dev ${NIC} parent root handle 6666 mqprio \
	num_tc 3 \
	map 2 2 1 0 2 2 2 2 2 2 2 2 2 2 2 2 \
	queues 1@0 1@1 2@2 \
	hw 0 > /dev/null  2>&1

# Tune up the system
sysctl -w net.core.default_qdisc=pfifo_fast
sudo sysctl -w net.core.wmem_max=90299200
sudo sysctl -w net.core.wmem_default=90299200

# Big Big assumption is that the system is running a i210/226
sudo modprobe -r igb
sudo modprobe igb

# Increase the number of descriptor to be used
sudo ethtool -G ${NIC} rx 64
sudo ethtool -G ${NIC} tx 64

# Create the MQPrio mapping to Traffic class to Queue
sudo tc qdisc add dev ${NIC} parent root handle 6666 mqprio \
	num_tc 3 \
	map 2 2 1 0 2 2 2 2 2 2 2 2 2 2 2 2 \
	queues 1@0 1@1 2@2 \
	hw 0

# Setup the QDisc for the traffic shapper
# The qdisc value here is set to transmit ONLY 1 Stream
# Calculation are done accordingly to https://tsn.readthedocs.io/qdiscs.html#configuring-cbs-qdisc
sudo tc qdisc replace dev ${NIC} parent 6666:1 cbs \
	idleslope 98688 sendslope -901312 hicredit 153 locredit -1389 \
	offload 1

# Set up the ETF for a 125us tx time
sudo tc qdisc add dev ${NIC} parent 6666:1 etf \
	clockid CLOCK_TAI \
	delta 500000 \
	offload

tc qdisc show dev ${NIC}
