{ pkgs ? import <nixpkgs> {}}:

with pkgs;

stdenv.mkDerivation {
    name = "native-sound-mixer";
    src = ./.;

    nativeBuildInputs = [
        nodejs
        python3
    ];

    buildInputs = [
        libpulseaudio
    ];
}
