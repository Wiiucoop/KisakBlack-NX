#!/bin/sh
# Convert the zones the port uses into <game>/kbz/ and validate each with
# kbzdump: the multiplayer set, then the SP / Zombies set (code_*, common,
# patch, frontend, common_zombie, zombie_theater, zombie_pentagon). SP zones have no _mp
# suffix, so both share kbz/. Run from the msys2 UCRT64 shell after
# tools/ffconv/build.sh:
#   GAME=/f/pluto_t5_full_game sh tools/nx/convert-zones.sh
root=$(cd "$(dirname "$0")/../.." && pwd)
GAME=${GAME:?set GAME to the Black Ops install}
TOOLS=$root/tools/ffconv
OUT="$GAME/kbz"
mkdir -p "$OUT"
LOG=$OUT/convert.log
: > "$LOG"
for z in \
    Common/code_pre_gfx_mp   English/en_code_pre_gfx_mp \
    Common/code_post_gfx_mp  English/en_code_post_gfx_mp \
    Common/common_mp         English/en_common_mp \
    Common/ui_mp             English/en_ui_mp \
    Common/patch_mp          English/en_patch_mp \
    Common/patch_ui_mp       Common/ui_viewer_mp \
    Common/mp_nuked          English/en_mp_nuked \
    Common/code_pre_gfx      English/en_code_pre_gfx \
    Common/code_post_gfx     English/en_code_post_gfx \
    Common/common            English/en_common \
    Common/patch             English/en_patch \
    Common/frontend          Common/frontend_patch      English/en_frontend \
    Common/common_zombie     Common/common_zombie_patch English/en_common_zombie \
    Common/zombie_theater    Common/zombie_theater_patch English/en_zombie_theater \
    Common/zombie_pentagon   Common/zombie_pentagon_patch English/en_zombie_pentagon
do
    name=$(basename "$z")
    kbz="$OUT/$name.kbz"
    echo "===== $name" >> "$LOG"
    if "$TOOLS/convert.exe" "$GAME/zone/$z.ff" "$kbz" >> "$LOG" 2>&1; then
        if "$TOOLS/kbzdump.exe" "$kbz" >> "$LOG" 2>&1; then
            printf '%-22s OK        %s bytes\n' "$name" "$(stat -c %s "$kbz")"
        else
            printf '%-22s CONVERTED, kbzdump FAILED\n' "$name"
        fi
    else
        printf '%-22s CONVERT FAILED (exit %s)\n' "$name" "$?"
        rm -f "$kbz"
    fi
done
