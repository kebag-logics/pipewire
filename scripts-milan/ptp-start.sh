#!/bin/bash

# SPDX-FileCopyrightText: Copyright © 2022 Kebag-Logic */
# SPDX-FileCopyrightText: Copyright © 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
# SPDX-License-Identifier: MIT

# part of the code taken from https://tsn.readthedocs.io/

NIC=${1}
function kill_all()
{
	sudo pkill ptp4l
	sudo pkill phc2sys
	sudo timedatectl set-ntp true
}

trap 'kill_all' SIGINT

# fopr
sudo timedatectl set-ntp false
sudo ptp4l -i ${NIC} -f /home/alex/prjs/linuxptp/configs/gPTP.cfg --step_threshold=1 &
PTP_PID=$$
sudo chrt -f -p 53 $PTP_PID

sudo pmc -u -b 0 -t 1 "SET GRANDMASTER_SETTINGS_NP clockClass 248 \
	clockAccuracy 0xfe offsetScaledLogVariance 0xffff \
	currentUtcOffset 37 leap61 0 leap59 0 currentUtcOffsetValid 1 \
	ptpTimescale 1 timeTraceable 1 frequencyTraceable 0 \
	timeSource 0xa0"

sudo chrt -f -p 52 $PTP_PID
sudo phc2sys -s ${NIC} -c CLOCK_REALTIME --step_threshold=1 --transportSpecific=1 -w -m &
PHC2SYS_PID=$$


waitpid $PTP_PID $PHC2SYS_PID

kill_all