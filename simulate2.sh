#!/bin/bash

COOJA_CONFIG_PATH=$1
DEST_DIR=$2

# compile project
make &&

# create csv of the map
python3 $(pwd)/create-map.py ${COOJA_CONFIG_PATH} > ${DEST_DIR}/map.csv &&

# run simulation
echo Simulation started && \
cooja_nogui ${COOJA_CONFIG_PATH} | grep 'Test script at'

wait

echo Simulation terminated &&

# parse stats
mv $(pwd)/test*.log ${DEST_DIR} &&
echo Parsing stats &&
python3 $(pwd)/parse-stats.py ${DEST_DIR}/test.log > ${DEST_DIR}/result.txt
