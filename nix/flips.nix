{ pkgs ? import <nixpkgs> {}, lib ? pkgs.lib, stdenv ? pkgs.stdenv, fetchFromGitHub ? pkgs.fetchFromGitHub }:

stdenv.mkDerivation {
  pname = "flips-cli";
  version = "unstable-2021-10-28";

  src = fetchFromGitHub {
    owner = "Alcaro";
    repo = "Flips";
    rev = "3a8733e74c9bdbb6b89da2b45913a0be3d0e1866";
    sha256 = "1jik580mz2spik5mgh60h93ryaj5x8dffncnr1lwija0v803xld7";
  };

  dontConfigure = true;
  buildPhase = "c++ *.c *.cpp -O3 -o flips";
  installPhase = "install -Dm755 flips $out/bin/flips";

  meta = with lib; {
    description = "A patcher for IPS and BPS files (CLI)";
    homepage = "https://github.com/Alcaro/Flips";
    license = licenses.gpl3Plus;
  };
}

