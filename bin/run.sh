#!/bin/bash

source conf.sh

function usage(){
    help_info="
    Usage: sudo ./run.sh [lc_pid]\n"
    echo -e $help_info;
    exit 1
}

if [ $# != 1 ] ; then
    usage
fi

HERACLES_LC_PID=$1

HERACLES_LATENCY_FILE=${HERACLES_LATENCY_FILE} \
HERACLES_MAX_LATENCY_FILE=${HERACLES_MAX_LATENCY_FILE} \
HERACLES_LOAD_FILE=${HERACLES_LOAD_FILE} \
HERACLES_MAX_LOAD_FILE=${HERACLES_MAX_LOAD_FILE} \
HERACLES_DRAM_LIMIT=${HERACLES_DRAM_LIMIT} \
CORE_MEMORY_SLEEP_TIME=${CORE_MEMORY_SLEEP_TIME} \
TOP_SLEEP_TIME=${TOP_SLEEP_TIME} \
TOP_LOAD_DISABLE_BOUND=${TOP_DISABLE_BOUND} \
TOP_LOAD_ENABLE_BOUND=${TOP_ENABLE_BOUND} \
TOP_SLOW_BE_BOUND=${TOP_SLOW_BE_BOUND} \
CGROUPS_DIR=${CGROUPS_DIR} \
HERACLES_IDLE_CORE_NUM=${HERACLES_IDLE_CORE_NUM} \
HERACLES_TOTAL_CORE_NUM=${HERACLES_TOTAL_CORE_NUM} \
HERACLES_MAX_BE_CORE_NUM=${HERACLES_MAX_BE_CORE_NUM} \
HERACLES_IS_NUMA=${HERACLES_IS_NUMA} \
HERACLES_DB_PATH=${HERACLES_DB_PATH} \
NET_SLEEP_TIME=${NET_SLEEP_TIME} \
NET_TOTAL_BANDWIDTH=${NET_TOTAL_BANDWIDTH} \
NET_LC_CLASSID=${NET_LC_CLASSID} \
NET_BE_CLASSID=${NET_BE_CLASSID} \
NIC_NAME=${NIC_NAME} \
./heracles -lc ${HERACLES_LC_PID} -debug -log

