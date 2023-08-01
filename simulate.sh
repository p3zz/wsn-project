#!/bin/bash

CSC_CONFIG_PATH=$1

CSC_CONFIG=$(basename ${CSC_CONFIG_PATH})

ROOT_DIR=$(pwd)
GLOBAL_TEST_DIR="${ROOT_DIR}/test"

TEST_NAME=$(date +%Y%m%d_%H%M%S)
TEST_UDGM_NAME="udgm"
TEST_MRM_NAME="mrm"

TEST_DIR="${GLOBAL_TEST_DIR}/${TEST_NAME}"

TEST_CSC="${TEST_DIR}/${CSC_CONFIG}"
TEST_UDGM_DIR="${TEST_CSC}/${TEST_UDGM_NAME}"
TEST_MRM_DIR="${TEST_CSC}/${TEST_MRM_NAME}"


mkdir ${GLOBAL_TEST_DIR}

mkdir ${TEST_DIR} && \

# create test directory
mkdir ${TEST_CSC} && \

# create UDGM folder
mkdir ${TEST_UDGM_DIR} && \

./simulate2.sh ${CSC_CONFIG_PATH}/udgm.csc ${TEST_UDGM_DIR} &&

# create UDGM folder
mkdir ${TEST_MRM_DIR} &&

# simulate MRM
./simulate2.sh ${CSC_CONFIG_PATH}/mrm.csc ${TEST_MRM_DIR} &&

echo ${CSC_CONFIG} completed
