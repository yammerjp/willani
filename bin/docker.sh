#!/bin/bash

REPO_DIR=$(cd "$(dirname "$0")/.."; pwd)
cd "$REPO_DIR"

sub_command="$1"

if [ "$sub_command" = "pull" ]; then
  docker build -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile
elif [ "$sub_command" = "run" ]; then
  docker run --rm -v "$REPO_DIR:/willani" -w /willani compilerbook make test
elif [ "$sub_command" = "sh" ]; then
  docker run --rm -it -v "$REPO_DIR:/willani" -w /willani compilerbook
elif [ "$sub_command" = "samplecode" ]; then
  docker run --rm -v "$REPO_DIR:/willani" -w /willani compilerbook ./run-samplecode.sh
else
  echo "usage: ./docker.sh pull ... download and build Dockerfile"
  echo "usage: ./docker.sh run  ... compile and run test"
  echo "usage: ./docker.sh sh   ... start interactive shell"
  echo "usage: ./docker.sh samplecode  ... build and run samplecode"
fi
