{ pkgs ? import <nixpkgs> {}, stdenv ? pkgs.stdenv }:

let
  nix-filter = import (pkgs.fetchFromGitHub {
    owner = "numtide";
    repo = "nix-filter";
    rev = "3b821578685d661a10b563cba30b1861eec05748";
    hash = "sha256-RizGJH/buaw9A2+fiBf9WnXYw4LZABB5kMAZIEE5/T8=";
  });

  wasi-stdenv = (import (pkgs.fetchFromGitHub {
    owner = "Cloudef";
    repo = "nix-zig-stdenv";
    rev = "9a7fb06077c993994bbf0d14d1770b547206f121";
    hash = "sha256-EK64gl3tt+yTMOTlqP173Ql+B4qLAPRyYu3Cx9i0aUw=";
  }) { target = "wasm32-unknown-wasi"; }).stdenv;

  base = stdenv: stdenv.mkDerivation {
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
  };
in (base stdenv).overrideAttrs (o: {
  passthru.wasi = (base wasi-stdenv).overrideAttrs (o: {
    buildFlags = o.makeFlags ++ [ "randomizer.wasm" ];
  });
})
