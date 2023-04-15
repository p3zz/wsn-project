#!/bin/bash

ROOT_DIR=$(pwd)
GLOBAL_TEST_DIR="${ROOT_DIR}/test"

TEST_NAME=$(date +%Y%m%d_%H%M%S)
TEST_UDGM_NAME="UDGM"
TEST_MRM_NAME="MRM"

TEST_DIR="${GLOBAL_TEST_DIR}/${TEST_NAME}"
TEST_UDGM_DIR="${TEST_DIR}/${TEST_UDGM_NAME}"
TEST_MRM_DIR="${TEST_DIR}/${TEST_MRM_NAME}"

# compile project
make &&

# create test directory
mkdir ${TEST_DIR} && \

# create configuration file
python3 create-config.py > ${TEST_DIR}/config.txt &&

# create UDGM folder
mkdir ${TEST_UDGM_DIR} &&

# create UDGM map file
python3 create-map.py test_nogui.csc > ${TEST_UDGM_DIR}/map.csv &&

# run UDGM simulation
echo UDGM Simulation started && \
cooja_nogui ${ROOT_DIR}/test_nogui.csc

wait

echo UDGM Simulation terminated && \

# parse UDGM stats
mv ${ROOT_DIR}/test*.log ${TEST_UDGM_DIR} &&
python3 ${ROOT_DIR}/parse-stats.py ${TEST_UDGM_DIR}/test.log > ${TEST_UDGM_DIR}/result.txt

# create MRM folder
mkdir ${TEST_MRM_DIR} &&

# create MRM map file
python3 create-map.py test_nogui_mrm.csc > ${TEST_MRM_DIR}/map.csv &&

# run MRM simulation
echo MRM Simulation started && \
cooja_nogui ${ROOT_DIR}/test_nogui_mrm.csc

wait

echo MRM Simulation terminated && \

# parse MRM stats
mv ${ROOT_DIR}/test*.log ${TEST_MRM_DIR} &&
python3 ${ROOT_DIR}/parse-stats.py ${TEST_MRM_DIR}/test.log > ${TEST_MRM_DIR}/result.txt

echo results available at ${TEST_DIR}
