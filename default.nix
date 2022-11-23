{ pkgs ? import <nixpkgs> {}, stdenv ? pkgs.stdenv }:

let
  nix-filter = import (pkgs.fetchFromGitHub {
    owner = "numtide";
    repo = "nix-filter";
    rev = "3b821578685d661a10b563cba30b1861eec05748";
    hash = "sha256-RizGJH/buaw9A2+fiBf9WnXYw4LZABB5kMAZIEE5/T8=";
  });
in stdenv.mkDerivation {
  name ="pokeemerald-randomizer";
  makeFlags = [ "PREFIX=$(out)" ];
  dontConfigure = true;
  src = nix-filter {
    root = ./.;
    include = [
      ./src
      ./patched.sha1
      ./original.sha1
      ./blacklist.default
      ./Makefile
    ];
  };
}
