#!/bin/bash
set -euo pipefail

# Determine script location and set project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PIPEWIRE_ROOT="$(dirname "$SCRIPT_DIR")"

# Ensure expected files exist in the project root
if [[ ! -f "$PIPEWIRE_ROOT/meson.build" || ! -d "$PIPEWIRE_ROOT/src" ]]; then
  echo "Error: Could not locate PipeWire root (expected meson.build and src/)."
  exit 1
fi

cd "$PIPEWIRE_ROOT"
echo "Running from PipeWire root: $PIPEWIRE_ROOT"

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --avb-interface)
      if [[ -n "${2:-}" ]]; then
        AVB_INTERFACE="$2"
        shift 2
      else
        echo "Error: --avb-interface requires a value"
        exit 1
      fi
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
done

echo "Using AVB interface: $AVB_INTERFACE"

# The setup command only has to be executed with initial setup
meson setup builddir || true  # avoid error if already configured

# Configure build
meson configure builddir -Dprefix=/usr -Davb-interface="$AVB_INTERFACE"

# Compile
meson compile -C builddir

cd builddir

# Install
sudo make install

# Set capabilities
sudo setcap cap_net_raw,cap_net_admin,cap_dac_override+eip /usr/bin/pipewire

# Prepare the i210 interface
sudo "$SCRIPT_DIR/prepare-traffic-shaper.sh" "$AVB_INTERFACE"
sudo "$SCRIPT_DIR/setup-vlan.sh" "$AVB_INTERFACE"

# # Start with verbose logging
# /usr/bin/pipewire-avb -v

# # Restart pipewire
# systemctl --user restart pipewire-avb.service
