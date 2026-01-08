#!/usr/bin/env bash

. /etc/os-release
case "$ID" in
  debian|ubuntu|linuxmint|pop) #debian distros off top of head
    echo ""
    ;;
  *)
    exit 0 #cant test other distros and also lazy
    ;;
esac

if [ -e .requirements_met ]; then
    exit 0
fi

set -euo pipefail

missing=0

echo "Checking dependencies..."

cmds=(
  git
  make
  gcc
  g++
  pkg-config
  protoc
  python3
  arm-none-eabi-gcc
  aarch64-linux-gnu-gcc
)

pkgs=(
  libtss2-dev
  libusb-1.0-0-dev
  libftdi-dev
  libelf-dev
  libxml2-dev
  rapidjson-dev
)

py_pkgs=(
    protobuf
    grpcio-tools
  )

for c in "${cmds[@]}"; do
  if ! command -v "$c" >/dev/null 2>&1; then
    echo "missing command: $c"
    missing=1
  fi
done

for p in "${pkgs[@]}"; do
  if ! dpkg -s "$p" >/dev/null 2>&1; then
    echo "missing package: $p"
    missing=1
  fi
done

if ! command -v pip3 >/dev/null 2>&1; then
  echo "pip3 not installed"
  missing=1
else
  for p in "${py_pkgs[@]}"; do
    if ! pip3 show "$p" >/dev/null 2>&1; then
      echo "missing pip package: $p"
      missing=1
    fi
  done
fi

echo
if [ "$missing" -ne 0 ]; then
  echo "Check docs/building.md to install required packages."
  exit 1
else
  touch .requirements_met #because requirement scan is slow
fi
