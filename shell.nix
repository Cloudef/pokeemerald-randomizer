{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  packages = with pkgs; [
    (import ./nix/flips.nix {})
    binutils-unwrapped-all-targets
    gawk
    xxd
  ];
}
