#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
WOKWI_TOML="$PROJECT_ROOT/wokwi.toml"
DIAGRAM_JSON="$PROJECT_ROOT/diagram.json"
TOKEN_FILE="$HOME/.wokwi-ci-token"

extract_toml_value() {
  local key="$1"
  sed -n "s|^[[:space:]]*${key}[[:space:]]*=[[:space:]]*\"\([^\"]*\)\"[[:space:]]*$|\1|p" "$WOKWI_TOML" | head -n 1
}

ensure_required_files() {
  if [[ ! -f "$WOKWI_TOML" ]]; then
    echo "错误：未找到 $WOKWI_TOML" >&2
    exit 1
  fi

  if [[ ! -f "$DIAGRAM_JSON" ]]; then
    echo "错误：未找到 $DIAGRAM_JSON" >&2
    exit 1
  fi
}

resolve_pio_env() {
  local env_name=""

  env_name="$(extract_toml_value firmware | sed -n 's|.*\.pio/build/\([^/]*\)/firmware\.bin|\1|p')"
  if [[ -z "$env_name" && -f "$PROJECT_ROOT/platformio.ini" ]]; then
    env_name="$(sed -n 's|^\[env:\([^]]*\)\].*$|\1|p' "$PROJECT_ROOT/platformio.ini" | head -n 1)"
  fi

  if [[ -z "$env_name" ]]; then
    env_name="esp32dev"
  fi

  echo "$env_name"
}

build_firmware() {
  local pio_env="$1"

  echo "[1/4] 构建固件（env: ${pio_env}）..."

  if command -v python3 >/dev/null 2>&1 && python3 -m platformio --version >/dev/null 2>&1; then
    python3 -m platformio run -e "$pio_env"
    return
  fi

  if command -v platformio >/dev/null 2>&1; then
    platformio run -e "$pio_env"
    return
  fi

  if command -v pio >/dev/null 2>&1; then
    pio run -e "$pio_env"
    return
  fi

  echo "错误：未找到 PlatformIO。请先安装 platformio 或确保 python3 -m platformio 可用。" >&2
  exit 1
}

ensure_wokwi_cli() {
  echo "[2/4] 检查 wokwi-cli..."

  if command -v wokwi-cli >/dev/null 2>&1; then
    return
  fi

  if ! command -v curl >/dev/null 2>&1; then
    echo "错误：未找到 curl，无法自动安装 wokwi-cli。" >&2
    echo "请先安装 curl 或手动安装 wokwi-cli: https://docs.wokwi.com/wokwi-ci/cli-installation" >&2
    exit 1
  fi

  echo "未检测到 wokwi-cli，尝试自动安装（最多等待 180 秒）..."
  if ! (curl --connect-timeout 10 --max-time 180 -fsSL https://wokwi.com/ci/install.sh | sh); then
    echo "错误：自动安装 wokwi-cli 失败或超时。" >&2
    echo "请参考手动安装文档: https://docs.wokwi.com/wokwi-ci/cli-installation" >&2
    exit 1
  fi

  export PATH="$HOME/.wokwi/bin:$HOME/.local/bin:$PATH"

  if ! command -v wokwi-cli >/dev/null 2>&1; then
    echo "错误：wokwi-cli 自动安装后仍不可用，请检查安装日志或手动安装。" >&2
    exit 1
  fi
}

ensure_wokwi_token() {
  echo "[3/4] 检查 WOKWI_CLI_TOKEN..."

  if [[ -z "${WOKWI_CLI_TOKEN:-}" && -f "$TOKEN_FILE" ]]; then
    local token
    token="$(tr -d '\r\n' < "$TOKEN_FILE")"
    if [[ -n "$token" ]]; then
      export WOKWI_CLI_TOKEN="$token"
    fi
  fi

  if [[ -n "${WOKWI_CLI_TOKEN:-}" ]]; then
    return
  fi

  cat >&2 <<'MSG'
错误：未检测到 WOKWI_CLI_TOKEN。
请先在以下页面生成 Wokwi CI Token：
  https://wokwi.com/dashboard/ci

然后在终端执行（示例）：
  export WOKWI_CLI_TOKEN=你的_token

或写入默认文件（本脚本会自动读取）：
  echo "你的_token" > ~/.wokwi-ci-token
MSG
  exit 1
}

ensure_build_artifacts() {
  local firmware_rel="$1"
  local elf_rel="$2"
  local firmware_abs="$PROJECT_ROOT/$firmware_rel"
  local elf_abs="$PROJECT_ROOT/$elf_rel"

  if [[ ! -f "$firmware_abs" ]]; then
    echo "错误：未找到固件文件 $firmware_abs" >&2
    exit 1
  fi

  if [[ ! -f "$elf_abs" ]]; then
    echo "错误：未找到 ELF 文件 $elf_abs" >&2
    exit 1
  fi
}

main() {
  echo "提示：本脚本会联网到 Wokwi CI。"

  ensure_required_files

  local pio_env
  pio_env="$(resolve_pio_env)"

  local firmware_rel elf_rel
  firmware_rel="$(extract_toml_value firmware)"
  elf_rel="$(extract_toml_value elf)"

  if [[ -z "$firmware_rel" ]]; then
    firmware_rel=".pio/build/$pio_env/firmware.bin"
  fi

  if [[ -z "$elf_rel" ]]; then
    elf_rel=".pio/build/$pio_env/firmware.elf"
  fi

  build_firmware "$pio_env"
  ensure_build_artifacts "$firmware_rel" "$elf_rel"
  ensure_wokwi_cli
  ensure_wokwi_token

  echo "[4/4] 启动 Wokwi 仿真（timeout: 120000ms）..."
  wokwi-cli --timeout 120000 "$PROJECT_ROOT"
}

main "$@"
