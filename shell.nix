{ pkgs ? import <nixpkgs> {}, stdenv ? pkgs.stdenvNoCC }:

let
  mkMiniShell = pkgs.mkShell.override { inherit stdenv; };
in mkMiniShell {
  packages = with pkgs; [
    (import ./nix/flips.nix {})
    binutils-unwrapped-all-targets
    gawk
    xxd
  ];
}
