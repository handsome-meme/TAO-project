#!/bin/bash

#set -e

print_help() {
echo "
The script builds the Barefoot Container for SHC.

Usage: $0 [<options>]

Options:
    -k, --keep-alive: Keep the build Container alive
    -j, --jobs: Number of jobs for BF SDE build (Default: 4)
    -a, --asan: Build project with enabling ASAN(Address Sanitizer)

Examples:
    $0 -k    # enter the build container in interactive mode
    $0       # build all
    $0 -a    # build all with asan enabled
"
}

SDE_VERSION="9.7.1"
JOBS=${JOBS:-4}

while (( "$#" )); do
  case "$1" in
    -k|--keep-alive)
      KEEPALIVE=true
      shift
      ;;
    -j|--jobs)
      if [ -n "$2" ] && [ ${2:0:1} != "-" ]; then
        JOBS=$2
        shift 2
      else
        echo "Error: Argument for $1 is missing" >&2
        exit 1
      fi
      ;;
    -a|--asan)
      ASAN=true
      shift
      ;;
    -h|--help)
      print_help
      exit 0
      ;;
    -*|--*=) # unsupported flags
      echo "Error: Unsupported flag $1" >&2
      print_help
      exit 1
      ;;
    *) # preserve positional arguments
      echo "Error: Unknown positional argument $1" >&2
      print_help
      exit 1
      ;;
  esac
done

SHC_DIR=${PWD}
JOBS=${JOBS:-4}
DOCKER_IMG="shc:1.0"

docker image inspect $DOCKER_IMG &> /dev/null
if [ $? -ne 0 ]; then
    echo $name" image is not existed!!!"
    # docker pull mirrors.tencent.com/ntb/$DOCKER_IMG
    # docker image tag mirrors.tencent.com/ntb/$DOCKER_IMG $DOCKER_IMG
    # docker rmi mirrors.tencent.com/ntb/$DOCKER_IMG 
else
    echo $name" build image is existed!!!"
fi

DOCKER_EXTRA_RUN_OPTS=""
if [ -n "$KEEPALIVE" ]; then
  # Running in a TTY, so run interactively (i.e. make Ctrl-C work)
  DOCKER_EXTRA_RUN_OPTS+="-it --privileged "
  CMD_OPTS="service docker start; /bin/bash"
else
  DOCKER_EXTRA_RUN_OPTS+="--privileged "
  if [ -n "$ASAN" ]; then
    CMD_OPTS="./build.sh --asan"
  else
    CMD_OPTS="./build.sh"
  fi
fi

# Build SHC 
if [ -n "$CMD_OPTS" ]; then
  echo "Building SHC"
  docker run --rm \
    $DOCKER_OPTS \
    $DOCKER_EXTRA_RUN_OPTS \
    -v $SHC_DIR:/share \
    -w /share \
    --net=host \
    $DOCKER_IMG bash -c "${CMD_OPTS}"
fi
