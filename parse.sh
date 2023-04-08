#!/bin/bash

TEST_NAME=$1
ROOT_DIR=$(pwd)
TEST_DIR="${ROOT_DIR}/test/${TEST_NAME}"

mv ${ROOT_DIR}/test.log ${ROOT_DIR}/test_dc.log ${TEST_DIR} && \
python3 parse-stats.py ${TEST_DIR}/test.log > ${TEST_DIR}/result.txt