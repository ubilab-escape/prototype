#!/bin/bash
# This script checks which floppy disk is in which drive
# 

timeout -s SIGKILL 3 bash -c "fdisk -l | grep \"/dev/sd\\|Disk identifier: 0x00\""

