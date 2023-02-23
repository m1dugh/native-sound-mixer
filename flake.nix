{
    description = "A node package to handle low-level volume control using pulseaudio";

    outputs = { self, nixpkgs } @ inputs:
        let 
            inherit (nixpkgs) lib;
            supportedSystems = ["x86_64-linux" "aarch64-linux"];
            forAllSystems = lib.genAttrs supportedSystems;
            nixpkgsFor = forAllSystems(system: import nixpkgs {
                inherit system;
                config.allowUnfree = true;
            });
    in {
        
        devShells = forAllSystems(system:
        let
            pkgs = nixpkgsFor.${system};
        in {
            default = pkgs.mkShell {
                nativeBuildInputs = with pkgs; [
                    cmake
                    clang-tools
                    nodejs
                    python3
                ];

                buildInputs = with pkgs; [
                    libpulseaudio
                ];
            };
        });

        packages = forAllSystems(system: 
        let
            pkgs = nixpkgsFor.${system};
        in {
            default = pkgs.buildNpmPackage rec {
                pname = "native-sound-mixer";
                version = "3.4.0-win";

                src = ./.;

                nativeBuildInputs = with pkgs; [
                    cmake
                    clang-tools
                    nodejs
                    python3
                ];

                buildInputs = with pkgs; [
                    libpulseaudio
                ];

                npmDepsHash = "sha256-kmNYi9NjPJmu743RJRkSyc5DIeulmg1HiqfWCsPjXng=";
                meta = with lib; {
                    description = "A modern web UI for various torrent clients with a Node.js backend and React frontend";
                    homepage = "https://m1dugh.github.io/native-sound-mixer/";
                    license = licenses.mit;
                    maintainers = with maintainers; [ m1dugh ];
                };
            };
        });
    };
}
