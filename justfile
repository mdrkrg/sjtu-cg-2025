default: setup

setup:
  meson setup build --reconfigure

build: setup
  meson compile -C build

shell:
  nix-shell nix/shell.nix

run:
  nixGLIntel ./build/simple-opengl

image:
  rm -rf build
  nix-build nix/image.nix -o simple-opengl.tar.gz

dev: build run
