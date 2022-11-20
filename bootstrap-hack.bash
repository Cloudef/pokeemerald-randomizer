#!/usr/bin/env bash
set -euo pipefail

tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT

hash objdump xxd awk sha1sum
HACK_ROM="${HACK_ROM:-pokeemerald/pokeemerald.gba}"
ORIG_ROM="${ORIG_ROM:-pokeemerald.gba}"
HACK_ELF="${HACK_ROM/.gba/.elf}"

if [[ ! -f "$HACK_ROM" ]]; then
   printf '%s: not found, you must build the hacked ROM first!\n' "$HACK_ROM"
   exit 1
fi

if [[ "${1:-}" == sha1 ]]; then
   (cd "$(dirname "$HACK_ROM")" && sha1sum "$(basename "$HACK_ROM")")
   exit $?
elif [[ ! -f patched.sha1 ]]; then
   printf 'patched.sha1: not found\n'
   exit 1
else
   patched_rom_sha1="$(cut -d' ' -f1 patched.sha1)"
   if [[ ! "$patched_rom_sha1" ]]; then
      printf 'patched.sha1: contains garbage\n'
      exit 1
   fi
fi

# rom sha1 for validating that we are working with the original ROM
rom_sha1="$(cut -d' ' -f1 original.sha1)"
printf 'static const char ORIGINAL_ROM_SHA1[] = "%s";\n' "$rom_sha1" > "$tmpdir"/data.h

# patched rom sha1 for validating we are working with the excpected base hack
printf 'static const char PATCHED_ROM_SHA1[] = "%s";\n' "$patched_rom_sha1" >> "$tmpdir"/data.h

# find the maps section offset, so we can patch the maps later
objdump --syms "$HACK_ELF" > "$tmpdir"/syms
load="0x$(objdump -h "$HACK_ELF" | grep -F '.text' | awk '{print $4}')"
printf 'static const ptrdiff_t ROM_LOAD_ADDRESS = %s;\n' "$load" >> "$tmpdir"/data.h
deref() { off="$1"; printf '%s' "0x$(xxd -s "$((off-load))" -l 4 -e "$HACK_ROM" | cut -d' ' -f2)"; }
gMapGroups="0x$(grep " gMapGroups$" "$tmpdir"/syms | cut -d' ' -f1)"
printf 'static const ptrdiff_t ROM_MAP_GROUPS_ADDRESS = %s;\n' "$gMapGroups" >> "$tmpdir"/data.h
gWildMonHeaders="0x$(grep " gWildMonHeaders$" "$tmpdir"/syms | cut -d' ' -f1)"
printf 'static const ptrdiff_t ROM_WILD_MON_HEADERS_ADDRESS = %s;\n' "$gWildMonHeaders" >> "$tmpdir"/data.h
gTrainers="0x$(grep " gTrainers$" "$tmpdir"/syms | cut -d' ' -f1)"
printf 'static const ptrdiff_t ROM_TRAINERS_ADDRESS = %s;\n' "$gTrainers" >> "$tmpdir"/data.h
gEvolutionTable="0x$(grep " gEvolutionTable$" "$tmpdir"/syms | cut -d' ' -f1)"
printf 'static const ptrdiff_t ROM_EVOLUTION_TABLE_ADDRESS = %s;\n' "$gEvolutionTable" >> "$tmpdir"/data.h
gLevelUpLearnsets="0x$(grep " gLevelUpLearnsets$" "$tmpdir"/syms | cut -d' ' -f1)"
printf 'static const ptrdiff_t ROM_LEVEL_UP_LEARNSETS_ADDRESS = %s;\n' "$gLevelUpLearnsets" >> "$tmpdir"/data.h
gBaseStats="0x$(grep " gBaseStats$" "$tmpdir"/syms | cut -d' ' -f1)"
printf 'static const ptrdiff_t ROM_BASE_STATS_ADDRESS = %s;\n' "$gBaseStats" >> "$tmpdir"/data.h
gTMHMLearnsets="0x$(grep " gTMHMLearnsets$" "$tmpdir"/syms | cut -d' ' -f1)"
printf 'static const ptrdiff_t ROM_TMHM_LEARNSETS_ADDRESS = %s;\n' "$gTMHMLearnsets" >> "$tmpdir"/data.h

# This probably isn't going to be found from other ROMs unless made external as well
sStarterMon="0x$(grep " sStarterMon$" "$tmpdir"/syms | cut -d' ' -f1)"
if [[ "$sStarterMon" != 0x ]]; then
   printf 'static const ptrdiff_t ROM_STARTER_MON_ADDRESS = %s;\n' "$sStarterMon" >> "$tmpdir"/data.h
else
   printf 'static const ptrdiff_t ROM_STARTER_MON_ADDRESS = %s;\n' "0x0" >> "$tmpdir"/data.h
fi

map_names_from_offset() {
   count=0
   off="$1"
   printf 'const char *ROM_MAP_NAMES[] = {\n'
   while : ; do
      hex="$(printf '%08x' "$off")"
      name="$(grep "^$hex.*\.rodata" "$tmpdir"/syms | awk '{print $5}')"
      [[ "$name" == "gMapGroup_"* ]] && break
      printf '"%s",\n' "$name"
      off="$((off+28))"
      count="$((count+1))"
   done
   printf '};\n'
   printf "   TotalMaps: %s\n" "$count" 1>&2
}

if [[ "$sStarterMon" != 0x ]]; then
printf '      sStarterMon: %s\n' "$sStarterMon"
fi
printf '   gTMHMLearnsets: %s\n' "$gTMHMLearnsets"
printf '      gBaseStates: %s\n' "$gBaseStats"
printf 'gLevelUpLearnsets: %s\n' "$gLevelUpLearnsets"
printf '  gEvolutionTable: %s\n' "$gEvolutionTable"
printf '        gTrainers: %s\n' "$gTrainers"
printf '  gWildMonHeaders: %s\n' "$gWildMonHeaders"
printf '       gMapGroups: %s\n' "$gMapGroups"
printf '      *gMapGroups: %s\n' "$(deref "$gMapGroups")"
first_map="$(deref "$(deref "$gMapGroups")")"
printf '     **gMapGroups: %s\n' "$first_map"
map_names_from_offset "$first_map" >> "$tmpdir"/data.h

# create BPS patch from our customized rom
flips --create --bps "$ORIG_ROM" "$HACK_ROM" "$tmpdir"/bps_patch_bin
(cd "$tmpdir" && xxd -i bps_patch_bin) >> "$tmpdir"/data.h

# and finally copy over the result
cp -f "$tmpdir/data.h" src/data.h
