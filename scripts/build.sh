#!/bin/bash
mkdir -p build
cd build
cmake ..
make
mv RocketSimulation ../bin/