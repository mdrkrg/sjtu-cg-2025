{ pkgs ? import (fetchTarball("https://github.com/NixOS/nixpkgs/archive/d0fc30899600b9b3466ddb260fd83deb486c32f1.tar.gz")) {} }:

let
  # 1. Build your application using the 'default.nix' you already created.
  myApp = import ./default.nix { inherit pkgs; };

  # 2. Import the nixGL package.
  nixGLSource = pkgs.fetchFromGitHub {
    owner = "nix-community";
    repo = "nixGL";
    rev = "a8e1ce7d49a149ed70df676785b07f63288f53c5";
    sha256 = "sha256-Ob/HuUhANoDs+nvYqyTKrkcPXf4ZgXoqMTQoCK0RFgQ=";
  };
  nixGL = import nixGLSource { inherit pkgs; };

  # 3. Create a unified environment containing your app AND the nixGL wrapper.
  #    This makes both 'simple-opengl' and 'nixGLIntel' available in the final image's PATH.
  finalEnv = pkgs.buildEnv {
    name = "app-with-nixgl";
    paths = [
      myApp
      nixGL.nixGLIntel
    ];
  };

in
# 4. Build the Docker image from this unified environment.
pkgs.dockerTools.buildImage {
  name = "simple-opengl";
  tag = "latest";

  # The contents of the image come from our final environment.
  contents = finalEnv;

  # Set the default command. Users will override this at runtime.
  config = {
    Cmd = [ "${myApp}/bin/simple-opengl" ]; # Default command (will be overridden)
  };
}
