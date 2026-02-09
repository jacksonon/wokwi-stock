#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${REPO_ROOT}"

if ! command -v python3 >/dev/null 2>&1; then
  echo "[ERROR] python3 is required but was not found in PATH."
  exit 1
fi

PIO_CMD=()
if python3 -m platformio --version >/dev/null 2>&1; then
  PIO_CMD=(python3 -m platformio)
elif command -v platformio >/dev/null 2>&1; then
  PIO_CMD=(platformio)
else
  echo "[ERROR] PlatformIO CLI was not found."
  echo "Install it with one of:"
  echo "  pip3 install platformio"
  echo "  or install the VS Code PlatformIO extension"
  exit 1
fi

echo "==> Repo root: ${REPO_ROOT}"
echo "==> Using PlatformIO command: ${PIO_CMD[*]}"
echo "==> Building: ${PIO_CMD[*]} run -e esp32dev"
"${PIO_CMD[@]}" run -e esp32dev

BUILD_DIR="${REPO_ROOT}/.pio/build/esp32dev"
FIRMWARE_BIN="${BUILD_DIR}/firmware.bin"
FIRMWARE_ELF="${BUILD_DIR}/firmware.elf"

print_artifact() {
  local label="$1"
  local path="$2"

  if [[ -f "${path}" ]]; then
    local size_bytes
    size_bytes="$(wc -c <"${path}" | tr -d " ")"
    echo "  - ${label}: ${path} (${size_bytes} bytes)"
  else
    echo "  - ${label}: ${path} (missing)"
  fi
}

echo
echo "==> Build artifacts"
print_artifact "firmware.bin" "${FIRMWARE_BIN}"
print_artifact "firmware.elf" "${FIRMWARE_ELF}"

echo
echo "==> Pull one Tencent quote (non-fatal)"
if python3 /Users/os/.codex/skills/qt-stock-scout/scripts/qt_quote.py sz000001 --json; then
  true
else
  echo "[WARN] Quote fetch failed; continuing."
fi

echo
echo "==> Next steps"
echo "1) Serial monitor: ${PIO_CMD[*]} device monitor -b 115200"
echo "2) Wokwi in VS Code: run Wokwi: Start Simulator"
echo "   Wokwi reads diagram.json and wokwi.toml from this repo."
