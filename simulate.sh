#!/bin/bash

: '
Usage example:
CONFIG_MAC=CONTIKIMAC \
CONFIG_TOPOLOGY_DELAY=20 \
CONFIG_TOPOLOGY_PERIOD=40 \
CONFIG_BEACON_PERIOD=60 \
CONFIG_MSG_DELAY=30 \
CONFIG_MSG_PERIOD=30 \
CONFIG_SR_MSG_DELAY=75 \
CONFIG_SR_MSG_PERIOD=10 \
./simulate.sh
'

ROOT_DIR=$(pwd)
GLOBAL_TEST_DIR="${ROOT_DIR}/test"

TEST_NAME=$(date +%Y%m%d_%H%M%S)
TEST_UDGM_NAME="UDGM"
TEST_MRM_NAME="MRM"

TEST_DIR="${GLOBAL_TEST_DIR}/${TEST_NAME}"
TEST_UDGM_DIR="${TEST_DIR}/${TEST_UDGM_NAME}"
TEST_MRM_DIR="${TEST_DIR}/${TEST_MRM_NAME}"

mkdir ${TEST_DIR} && \

# Configuration file

echo "MAC: ${CONFIG_MAC}" >> ${TEST_DIR}/config.txt && \

echo "TOPOLOGY_DELAY: ${CONFIG_TOPOLOGY_DELAY}" >> ${TEST_DIR}/config.txt && \
echo "TOPOLOGY_PERIOD: ${CONFIG_TOPOLOGY_PERIOD}" >> ${TEST_DIR}/config.txt && \

echo "BEACON_PERIOD: ${CONFIG_BEACON_PERIOD}" >> ${TEST_DIR}/config.txt && \

echo "MSG_DELAY: ${CONFIG_MSG_DELAY}" >> ${TEST_DIR}/config.txt && \
echo "MSG_PERIOD: ${CONFIG_MSG_PERIOD}" >> ${TEST_DIR}/config.txt && \

echo "SR_MSG_DELAY: ${CONFIG_SR_MSG_DELAY}" >> ${TEST_DIR}/config.txt && \
echo "SR_MSG_PERIOD: ${CONFIG_SR_MSG_PERIOD}" >> ${TEST_DIR}/config.txt

# UDGM Simulation
mkdir ${TEST_UDGM_DIR} &&
echo UDGM Simulation started && \
cooja_nogui ${ROOT_DIR}/test_nogui.csc

wait

echo UDGM Simulation terminated && \
mv ${ROOT_DIR}/test*.log ${TEST_UDGM_DIR} &&
python3 ${ROOT_DIR}/parse-stats.py ${TEST_UDGM_DIR}/test.log > ${TEST_UDGM_DIR}/result.txt

# MRM Simulation
mkdir ${TEST_MRM_DIR} &&
echo MRM Simulation started && \
cooja_nogui ${ROOT_DIR}/test_nogui_mrm.csc

wait

echo MRM Simulation terminated && \
mv ${ROOT_DIR}/test*.log ${TEST_MRM_DIR} &&
python3 ${ROOT_DIR}/parse-stats.py ${TEST_MRM_DIR}/test.log > ${TEST_MRM_DIR}/result.txt

echo results available at ${TEST_DIR}
