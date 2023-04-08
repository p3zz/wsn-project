#!/bin/bash
DATE=$(date +%Y%m%d_%H%M%S)

ROOT_DIR=$(pwd)
TEST_NAME="${DATE}"
TEST_DIR="${ROOT_DIR}/test/${TEST_NAME}"

mkdir ${TEST_DIR} && \
echo ${TEST_NAME} && \
cooja_nogui ${ROOT_DIR}/test*.csc &>/dev/null