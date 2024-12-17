with import <nixpkgs> { };

let
  sdl3Path = builtins.getFlake "/etc/nixos"
    .outputs.legacyPackages.x86_64-linux.sdl_pkgs.outputs.legacyPackages.x86_64-linux.sdl3;
in
mkShell {
  buildInputs = [
    sdl3Path
    pkg-config
    cmake
  ];
  # Print the SDL3 path on shell entry
  shellHook = ''
    export SDL3_PATH="${builtins.storePath inputs.sdl_pkgs.outputs.legacyPackages.x86_64-linux.sdl3}"

    echo "SDL3 is located at: $SDL3_PATH"
  '';
}
