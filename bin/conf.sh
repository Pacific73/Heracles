#!/bin/bash

######################################################################
# You can change configurations of Heracles here.                    #
# And explanation for each option can be found next to each variable.#
######################################################################

########################### FILE CONF
HERACLES_LATENCY_FILE=/home/zcy/tailbench/tailbench-v0.9/xapian/lc.latency.95
# LC service continually output its latency into this file.
# Heracles will read it, then calculate statistics.

HERACLES_MAX_LATENCY_FILE=/home/zcy/tailbench/tailbench-v0.9/xapian/lc.latency.slo
# You need to manually set a maximum value for max acceptable latency.
# Heracles will make decisions according to it.

# p.s. use the same time unit in both files above. like second(s) or microsecond(ms).

HERACLES_LOAD_FILE=/home/zcy/tailbench/tailbench-v0.9/xapian/lc.load.cur
# LC service continually output its QPS into this file.
# Heracles will read it, then calculate statistics.

HERACLES_MAX_LOAD_FILE=/home/zcy/tailbench/tailbench-v0.9/xapian/lc.load.max
# You need to manually set a maximum value for max acceptable QPS.
# Heracles will make decisions in terms of it.
########################### FILE CONF END


########################### TIME CONF
CORE_MEMORY_SLEEP_TIME=5
# CoreMemoryController makes desicion every CORE_MEMORY_SLEEP_TIME seconds.

TOP_SLEEP_TIME=10
# TopController makes desicion every TOP_SLEEP_TIME seconds.

NET_SLEEP_TIME=2
# NetworkController makes desicion every NET_SLEEP_TIME seconds.

TOP_COOLING_TIME=5
# if TapController enters cooling state, it will last for TOP_COOLING_TIME seconds.
########################### TIME CONF END

########################### ENVIRONMENT CONF
CGROUPS_DIR=/sys/fs/cgroup
# cgroups path. (default value for Linux ubuntu)

HERACLES_DB_PATH=tasks.db
# BE database path.

NIC_NAME=ens2f1
# NIC device name. used for network bandwidth restriction and monitoring.

HERACLES_IS_NUMA=1
# whether cpu is NUMA, used for memory bandwidth calculation.
########################### ENVIRONMENT CONF END

########################### PARAMETER CONF
TOP_LOAD_DISABLE_BOUND=0.85
# if load percent exceeds this bound, BE will be disabled.

TOP_LOAD_ENABLE_BOUND=0.7
# if load percent is lower than this bound, BE will be enabled.

TOP_SLOW_BE_BOUND=0.4
# if slack is lower than this bound, BE's growth will be paused

HERACLES_IDLE_CORE_NUM=1
# core num reserved for OS/other tasks

HERACLES_MAX_BE_CORE_NUM=3
# max core num that can be used for BE tasks

HERACLES_TOTAL_CORE_NUM=9
# cores for heracles in total
# e.g. set TOTAL_CORE_NUM=16 means core [0, 15] will be used for 
#      heracles even if there're more than 16 cores on local machine.

HERACLES_DRAM_LIMIT=80000
# total availble memory bandwidth (in MB)

NET_TOTAL_BANDWIDTH=7500000000
# total availble network bandwidth (in bits)
########################### PARAMETER CONF END


