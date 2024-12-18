with import <nixpkgs> { };

mkShell {
  buildInputs = [
    pkg-config
    cmake
    libGLU
  ];
  # Print the SDL3 path on shell entry
  shellHook = ''
    export SDL3_PATH="/nix/store/$(ls /nix/store | grep "sdl3-3.1.6-dev")"
    echo "SDL3 is located at: $SDL3_PATH"
  '';
}
