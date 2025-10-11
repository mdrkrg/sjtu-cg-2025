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
  nixGL = import nixGLSource { inherit pkgs; };
in

pkgs.stdenv.mkDerivation rec {
  pname = "simple-opengl";
  version = "0.1.0";

  # Use the current directory as the source
  src = ./.;

  # Build inputs needed at compile and runtime
  buildInputs = with pkgs; [
    glm
    libGL
    libGLU
    glfw
    wayland
    libxkbcommon
    xorg.libX11
  ];

  # Tools needed for building
  nativeBuildInputs = with pkgs; [
    pkg-config
    cmake
    meson
    ninja
    nixGL.nixGLIntel
  ];

  # 1. Configure Phase: Run `meson setup`
  configurePhase = ''
    runHook preConfigure
    meson setup build . --prefix=$out
    runHook postConfigure
  '';

  # 2. Build Phase: Run `meson compile`
  buildPhase = ''
    runHook preBuild
    meson compile -C build
    runHook postBuild
  '';

  # 3. Install Phase: Run `meson install`
  installPhase = ''
    runHook preInstall
    meson install -C build

    mkdir -p $out/share
    cp -r $src/shaders $out/share/shaders

    runHook postInstall
  '';
}
