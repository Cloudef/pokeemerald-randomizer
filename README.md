# pokeemerald-randomizer

Randomizes warps, and offers some QoL features

Try it online here: https://cloudef.github.io/pokeemerald-randomizer/

## QoL features

- New game intro sequence is lot faster
- Start menu has a Softlock option, selecting this teleports you back to the intro truck

## Usage

```
usage: pokeemerald-randomizer [options]
Options
 -h, --help            display this help and exit.
 -d, --verbose         verbose output.
 -o, --output          specify output path.
 -s, --seed            specify seed manually.
 -f, --filter          provide custom warp blacklist.
 --unlinked-warps      entrance and exit may be different.
 --allow-unresolved    include unpaired warps (softlock likely).
 --no-filter           do not use the default warp blacklist.
 --randomize-mons      randomize wild encounters.
 --randomize-trainers  randomize trainer battles.
 --test-rng            print out some random numbers and exit.
```

Output is a ROM file prefixed with either generated or specified seed number.

## Building

If you simply want a randomizer for the original Pokemon Emerald, with default custom features applied on it, then you do not need anything except `make` and a working C compiler.

```
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

```
nix-shell
make
```

Similarily you can build pokeemerald also with nix:

```
cd pokeemerald
nix-shell
buildPhase
```
