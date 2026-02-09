# wokwi-stock

基于 **ESP32 + 2.9\" 墨水屏** 的可扩展硬件项目，首个功能是股票查看（腾讯行情 API）。

## 已实现内容

- 可扩展架构：`provider`（数据源）/`feature`（功能页）/`kernel`（调度）解耦
- 首个内置功能：单股行情页（默认 `sz000001`）
- 数据源：`http://qt.gtimg.cn/q=sz000001`
- 刷新策略：60 秒拉取 + 差异刷新 + 5 分钟整屏刷新一次
- Wokwi 仿真文件：`diagram.json`、`wokwi.toml`

## 目录结构

```text
src/
  app/                # 配置 + 调度内核
  core/               # 接口与模型
  display/            # 墨水屏驱动封装
  features/           # 可插拔功能页面
  providers/          # API 数据源
```

## 快速开始（PlatformIO）

1. 安装 PlatformIO（VS Code 插件或 CLI）
2. 在项目根目录构建：

```bash
pio run -e esp32dev
```

3. 串口查看：

```bash
pio device monitor -b 115200
```

## 一键预览脚本

在仓库根目录直接运行（直达脚本）：

```bash
./run-local.sh
```

- 作用：一键构建固件 + 打印产物路径 + 拉取一次腾讯行情（用于本地快速预览）。

```bash
./run-wokwi.sh
```

- 作用：一键构建并调用 `wokwi-cli` 启动仿真（联网到 Wokwi CI）。
- 等价脚本：`./scripts/preview-local.sh` 与 `./scripts/preview-wokwi.sh`
- 首次使用需要配置 Token：

```bash
export WOKWI_CLI_TOKEN=你的_token
# 或
echo "你的_token" > ~/.wokwi-ci-token
```

## 在 Wokwi 上模拟验证

### 1) 本地构建产物

```bash
pio run -e esp32dev
```

确保以下文件存在：

- `.pio/build/esp32dev/firmware.bin`
- `.pio/build/esp32dev/firmware.elf`

### 2) 启动 Wokwi 仿真

- 用 VS Code 打开本项目（安装 Wokwi for VS Code 插件）
- 执行命令：`Wokwi: Start Simulator`
- Wokwi 会读取：
  - `diagram.json`（硬件连线）
  - `wokwi.toml`（firmware/elf 路径）

### 3) 验收清单

- 启动后串口日志出现 WiFi 连接状态和 IP
- 能看到 `[quote] ok ...` 日志，且每 60 秒拉取一次
- 墨水屏显示：代码、价格、涨跌幅、更新时间
- 断网后显示错误，恢复网络后可自动继续拉取
- 价格无变化时不频繁刷新，5 分钟触发一次整刷

## 自定义开发（新增功能）

新增一个功能页面时，步骤如下：

1. 在 `src/features/` 新建类并实现 `IAppFeature`
2. 在 `src/main.cpp` 中替换或扩展 feature 实例
3. 复用 `AppKernel` 调度逻辑，无需改 provider 与 display 层

你也可以新增数据源：实现 `IStockProvider` 即可替换腾讯接口。

## 本地扩展功能清单

- 文件：`LOCAL_EXTRA_FEATURES.md`
- 用途：记录“暂未实现但已确认方向”的后续功能，便于排期与协作。
- 更新方式：新增需求时按“价值 + 实现要点 + 验收标准”补充条目，并标注优先级（P0/P1/P2）。

## 配置项

位于 `src/app/AppConfig.h`：

- `kDefaultStockCode`：默认股票代码（如 `sz000001`）
- `kFetchIntervalMs`：拉取周期（默认 60000 ms）
- `kTencentApiHost`：腾讯 API 前缀

## 说明

- 腾讯接口返回通常是 GBK，ESP32 侧未做完整转码。若名称含非 ASCII 字符，当前回退显示代码。
- 本项目仅用于技术演示，不构成投资建议。
