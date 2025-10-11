#!/usr/bin/env bash

IMAGE_NAME=simple-opengl
IMAGE=./simple-opengl.tar.gz

if [[ -v CONTAINER_RUNTIME ]]; then
  runtime=$CONTAINER_RUNTIME
else
  runtime=docker # podman
fi

function setup() {
  if [ "$runtime" = "docker" ]; then
    # WARN: insecure for X11 authentication
    xhost +local:docker
  fi
}

function cleanup() {
  if [ "$runtime" = "docker" ]; then
    xhost -local:docker
  fi
}

function run() {
  setup
  $runtime run -it --rm \
    --device=/dev/dri:/dev/dri \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix:ro \
    -e XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR \
    -v $XDG_RUNTIME_DIR:$XDG_RUNTIME_DIR:ro \
    -w /share \
    simple-opengl:latest \
    nixGLIntel simple-opengl
  cleanup
}

function load() {
  $runtime load -i $IMAGE
}

function main() {
  if ! $runtime images | grep $IMAGE_NAME &>/dev/null; then
    load
  fi

  run
}

main
