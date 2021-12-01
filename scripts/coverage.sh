#!/usr/bin/env bash

pushd $(dirname $0)/..

if [ ! -d coverage ]; then
    mkdir coverage || exit 1
fi

colcon build --cmake-args -DCMAKE_CXX_FLAGS="-fprofile-arcs -ftest-coverage -g" || exit 1
colcon test
colcon test-result || exit 1
gcovr --xml-pretty --exclude-unreachable-branches --print-summary -o coverage/report.xml --root . -e "foros/test" -e "foros_examples" -e "foros_msgs" -e "build" -e "install/foros_msgs" --html coverage/report.html --html-detail
