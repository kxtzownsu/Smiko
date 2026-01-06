#!/usr/bin/env bash
# Copyright 2025 HavenOverflow
# Use of this code is permissible so long as the appropriate credit
# is provided. See the LICENSE file for more info.

function hex2dec() {
    echo $(printf '%d' "$1")
}

function cpatoff() {
    dd if="$1" of="$2" status=progress bs=1 seek=$(hex2dec "$3") conv=notrunc
}

function cleanup() {
    # Scary as hell
    if echo "$wdir" | grep -q "/tmp"; then
        rm -rf "$wdir"
    fi
}

function main() {
    local out="$1"
    local file=
    local i=0

    wdir=$(mktemp -d)

    if [[ $# -lt 3 ]]; then
        echo "Error: Expected more input arguments!"
        exit 1
    fi

    # Generate a brand new 512KiB flash image.
    head -c 524288 /dev/zero | tr '\000' '\377' >"$wdir/out.bin"
    cpatoff "$2" "$wdir/out.bin" 0x0      # Write RO_A
    cpatoff "$3" "$wdir/out.bin" 0x4000   # Write RW_A
    
    # Copy the B sections if they're provided too.
    if [ $# -gt 2 ]; then
        cpatoff "$4" "$wdir/out.bin" 0x40000  # Write RO_B  
    fi
    if [ $# -gt 3 ]; then
        cpatoff "$5" "$wdir/out.bin" 0x44000  # Write RW_B
    fi
    cp "$wdir/out.bin" "$out"

    echo "New GSC firmware image packed at $out!"
}

if [ "$0" == "$BASH_SOURCE" ]; then
    trap cleanup EXIT
    main "$@"
fi