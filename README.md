# pokeemerald-randomizer

Randomizes warps, and offers some QoL features

## QoL features

- New game intro sequence is lot faster
- Start menu has a Softlock option, selecting this teleports you back to the intro truck

## Usage

```bash
usage: randomizer [options]
Options
 -h, --help            display this help and exit.
 -d, --verbose         verbose output.
 -s, --seed            specify seed manually.
 --unlinked-warps      entrance and exit may be different.
 --allow-unresolved    include unpaired warps (softlock likely).
```

Output is a ROM file prefixed with either generated or specified seed number.

## Building

If you simply want a randomizer for the original Pokemon Emerald, with default custom features applied on it, then you do not need anything except `make` and a working C compiler.

```bash
make
```

## Building for a customized ROM

Symlink your pokeemerald decomp repository into randomizer's repo or point the `HACK_ROM` env variable to your hacked ROM file.
In addition to the hacked ROM file, you'll also need the `.elf` file with the same name as this is used to find offset for your rom hack.
Delete the `patched.sha1` and the `Makefile` realizes that a new `data.h` file should be generated from your ROM.
You must have a clean copy of the original ROM called `pokeemerald.gba` in the source directory or point to it with `ORIG_ROM` env variable.

The `bootstrap-hack.bash` requires some dependencies installed: `objdump xxd awk sha1sum`.

### Nix

If you build with nix (OSX, Linux) then you can just run `nix-shell` and you should have shell with all the dependencies required.

```bash
nix-shell
make
```

Similarily you can build pokeemerald also with nix:

```bash
cd pokeemerald
nix-shell
buildPhase
```
