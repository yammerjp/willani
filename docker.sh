#!/bin/bash

SCRIPT_DIR=$(cd "$(dirname "$0")"; pwd)
cd "$SCRIPT_DIR"

sub_command="$1"

if [ "$sub_command" = "pull" ]; then
  docker build -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile
elif [ "$sub_command" = "run" ]; then
  docker run --rm -v "$SCRIPT_DIR:/willani" -w /willani compilerbook make test
else
  echo "usage: ./docker.sh pull ... download and build Dockerfile"
  echo "usage: ./docker.sh run ... compile and run test"
fi