#!/bin/bash

sudo pip install conan
sudo apt-get install libyaml-cpp-dev -y

mkdir -p build && cd build

conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
conan install .. && cmake -S.. -B.
