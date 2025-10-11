{
  pkgs ? import (fetchTarball("https://github.com/NixOS/nixpkgs/archive/d0fc30899600b9b3466ddb260fd83deb486c32f1.tar.gz")) {
    config.allowUnfree = true;
  }
}:

let
  nixGLSource = pkgs.fetchFromGitHub {
    owner = "nix-community";
    repo = "nixGL";
    rev = "a8e1ce7d49a149ed70df676785b07f63288f53c5";
    sha256 = "sha256-Ob/HuUhANoDs+nvYqyTKrkcPXf4ZgXoqMTQoCK0RFgQ=";
  };

  nixGL = import nixGLSource { pkgs = pkgs; };

in pkgs.mkShell {

  buildInputs = with pkgs; [
    # Graphics and Math libs
    glm
    libGL
    libGLU
    mesa # Provides DRI drivers and debugging tools like glxinfo
    assimp.dev # Model loading

    # Windowing library
    glfw

    # Wayland and X11 libs for full display server compatibility
    wayland
    libxkbcommon
    wayland-protocols
    xorg.libX11
    xorg.libXcursor
    xorg.libXrandr
    xorg.libXinerama
    xorg.libXi
  ];

  nativeBuildInputs = with pkgs; [
    cmake
    meson
    pkg-config
    gcc14
    nixGL.nixGLIntel
  ];
}
