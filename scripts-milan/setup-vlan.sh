#!/bin/bash

NIC=${1}

sudo modprobe 8021q
sudo ip link add link ${NIC} name ${NIC}.2 type vlan id 2
sudo ip link set dev ${NIC}.2 up
