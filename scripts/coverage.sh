#!/usr/bin/env bash

pushd $(dirname $0)/..

if [ ! -d coverage ]; then
    mkdir coverage || exit 1
fi

colcon build --cmake-args -DCMAKE_CXX_FLAGS="-g --coverage" || exit 1
colcon test
colcon test-result || exit 1
lcov --capture --directory ./build/foros/CMakeFiles/foros.dir/ --output-file coverage/app.info || exit 1
lcov -r coverage/app.info "/usr*" "/opt*" "*foros_msgs*" -o coverage/app-final.info || exit 1

genhtml coverage/app-final.info --branch-coverage --output-directory coverage/ || exit 1
