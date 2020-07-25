#!/bin/bash

REPO_DIR=$(cd "$(dirname "$0")"; pwd)
cd "$REPO_DIR"

sub_command="$1"

if   [ "$sub_command" = "pull"   ]; then
  docker build -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile
elif [ "$sub_command" = "sh"     ]; then
  docker run --rm -it -v "$REPO_DIR:/willani" -w /willani compilerbook
elif [ "$sub_command" = "test"   ]; then
  docker run --rm -v "$REPO_DIR:/willani" -w /willani compilerbook make test
elif [ "$sub_command" = "self"   ]; then
  docker run --rm -v "$REPO_DIR:/willani" -w /willani compilerbook ./self.sh
elif [ "$sub_command" = "sample" ]; then
  docker run --rm -v "$REPO_DIR:/willani" -w /willani compilerbook make sample
elif [ "$sub_command" = "debug"  ]; then
  docker run --rm -v "$REPO_DIR:/willani" -w /willani compilerbook make debug
else
  echo "usage: ./docker.sh pull   ... download and build Dockerfile"
  echo "usage: ./docker.sh sh     ... start interactive shell"
  echo "usage: ./docker.sh test   ... build and run test/test.c"
  echo "usage: ./docker.sh self   ... check selfhosted compiler with running self.sh"
  echo "usage: ./docker.sh sample ... build and run sample.c"
  echo "usage: ./docker.sh debug  ... build and run sample.c on gdb"
fi
