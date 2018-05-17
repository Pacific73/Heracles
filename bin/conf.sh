#!/bin/bash

HERACLES_LATENCY_FILE=/home/zcy/tailbench/tailbench-v0.9/xapian/lc.latency.95
HERACLES_MAX_LATENCY_FILE=/home/zcy/tailbench/tailbench-v0.9/xapian/lc.latency.slo
HERACLES_LOAD_FILE=/home/zcy/tailbench/tailbench-v0.9/xapian/lc.load.cur
HERACLES_MAX_LOAD_FILE=/home/zcy/tailbench/tailbench-v0.9/xapian/lc.load.max

HERACLES_DRAM_LIMIT=10240

CORE_MEMORY_SLEEP_TIME=5

TOP_SLEEP_TIME=10
TOP_DISABLE_BOUND=0.85
TOP_ENABLE_BOUND=0.8
TOP_SLOW_BE_BOUND=0.1

CGROUPS_DIR=/sys/fs/cgroup
NIC_NAME=ens2f1
# default for Linux ubuntu

HERACLES_IDLE_CORE_NUM=1

HERACLES_TOTAL_CORE_NUM=16
# cores for heracles in total
# e.g. set TOTAL_CORE_NUM=16 means core [0, 15] will be used for 
#      heracles even if there're more than 16 cores on local machine.

HERACLES_MAX_MEM_BW=16000
HERACLES_IS_NUMA=1

HERACLES_DB_PATH=tasks.db

NET_SLEEP_TIME=2
NET_TOTAL_BANDWIDTH=1e9
NET_LC_CLASSID=0x10003
NET_BE_CLASSID=0x10004
