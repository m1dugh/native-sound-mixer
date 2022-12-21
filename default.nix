{ pkgs ? import <nixpkgs> {}}:

with pkgs;

stdenv.mkDerivation {
    name = "native-sound-mixer";
    src = ./.;

    nativeBuildInputs = [
        cmake
        clang-tools
        nodejs
        python3
    ];

    buildInputs = [
        libpulseaudio
    ];
}
