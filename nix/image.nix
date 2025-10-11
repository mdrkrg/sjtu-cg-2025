{
  pkgs ? import (fetchTarball("https://github.com/NixOS/nixpkgs/archive/d0fc30899600b9b3466ddb260fd83deb486c32f1.tar.gz")) {}
}:

let
  # Import the build
  simpleScene = import ./default.nix { inherit pkgs; };

  nixGLSource = pkgs.fetchFromGitHub {
    owner = "nix-community";
    repo = "nixGL";
    rev = "a8e1ce7d49a149ed70df676785b07f63288f53c5";
    sha256 = "sha256-Ob/HuUhANoDs+nvYqyTKrkcPXf4ZgXoqMTQoCK0RFgQ=";
  };
  nixGL = import nixGLSource { inherit pkgs; };

  # Runtime dependencies
  finalEnv = pkgs.buildEnv {
    name = "app-with-nixgl";
    paths = [
      simpleScene
      nixGL.nixGLIntel
    ];
  };

in
# Build image
pkgs.dockerTools.buildImage {
  name = "simple-opengl";
  tag = "latest";

  contents = finalEnv;

  # Default command (override at runtime)
  config = {
    Cmd = [ "${simpleScene}/bin/simple-opengl" ];
  };
}
