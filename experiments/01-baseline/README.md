# 基线实验：`a10-cuda-smolvla-20260831-r1`

本阶段验证固定请求的正确性，建立端到端及服务端各阶段的性能基线。配置和结果仅适用于本页记录的冻结条件。

## 冻结配置

> 状态：已冻结。本节记录 2026-08-31 正式基线的冻结条件。任何冻结字段改变均须创建新的 `Configuration ID`。

### 适用范围

本记录的正式目标是当前主机上的单张 NVIDIA A10/CUDA 推理。

### 冻结条件

| 字段 | 冻结值 |
|---|---|
| 模型与 checkpoint | `vrfai/smolvla-libero-gguf@349851cd4e872d199c75e1802f037c8026767190`；本地 `models/smolvla-libero.gguf`；SHA-256 `6fb2d475c98b4c2cef3e27c4eff4e67b483740cbf983fff320a3b8a5e5f74fe8` |
| 部署路径与实现 | `vla.cpp@e9e57ea734692bbcb34528fee16318e230ba1100`；Release C++/CUDA `vla-server` |
| 依赖 | CMake FetchContent `llama.cpp@7ba604f1cb61cd14898138e9abc0b4ff2601f180`，以及 vla.cpp 构建期 CUDA hook 补丁；补丁 diff SHA-256 `2cf6db3ddb175ed4968ddddee3e2401e473d66f03c004390f7deb62123f533d0` |
| 目标设备 | NVIDIA A10，23,028 MiB，Compute Capability 8.6，persistence mode Enabled；驱动 `580.126.09` |
| 主机 | Intel Xeon Platinum 8369B，8 核 / 16 逻辑 CPU，约 58 GiB RAM，无 Swap；Ubuntu 24.04.4，内核 `6.8.0-111-generic` |
| CUDA | 驱动报告 CUDA runtime 13.0；构建 Toolkit `12.8.93`（`nvcc`） |
| 基线采集时的构建目录 | `/root/vla-smoke-c01/build-formal-a10-20260831`；制品身份以本配置记录的 SHA-256 为准，目录后来可能被 profile 构建复用，不能仅凭路径认定为基线制品；不得使用 `build-cpu/` 或历史 `build-cuda/` |
| 构建选项 | Ninja、Release、`GGML_CUDA=ON`、`GGML_CUDA_GRAPHS=ON`、`CMAKE_CUDA_ARCHITECTURES=86`、`--parallel 16` |
| 输入图像 | `src/vla.cpp/assets/front.jpg`；SHA-256 `e6f0738ce9184be421dbade5a73900fb33e44c288c475d3985054fbbc4dfb7cf` |
| 文本输入 | 固定 token IDs：`1,100,200,2` |
| 状态输入 | 32 个 `0.0f`；模型实际状态维度为 8，其余是接口填充 |
| 初始 noise | 1,600 个 `float`；第 `i` 项为 `0.001 * ((uint32_t(i) * 2654435761) % 1000) - 0.5`，`i=0…1599` |
| 服务与线程 | 单进程 `vla-server`，单客户端顺序 REQ/REP，`VLA_N_THREADS=16` |

完整环境、构建缓存摘要及基线采集时的构建制品快照见 [raw/environment-20260831.md](raw/environment-20260831.md)。

## 验证标准与测量方法

### 正确性标准


在同一个已加载服务中，固定请求的 reference 输出必须为 50×32、全部有限值；每行前 7 列是有效动作，后 25 列必须为 `0.0`。其后每个测量请求的 1,600 个 `float` action 输出必须与 reference 按位相同。

该标准只验证固定输入下的数值可重复与输出结构；不主张动作语义、LIBERO 任务成功率或机器人控制正确性。

### 性能测量方法


- 常驻服务加载模型一次；模型加载和服务启动不计入稳态样本。
- 固定请求先执行 5 次未计时 warm-up，再顺序采集 30 次；不排除样本。
- 主指标：客户端循环回路 wall time（包含 loopback ZeroMQ、Protobuf、JPEG decode 与服务端推理）。次指标：服务响应返回的 total、vision、inference、prefill、denoise 毫秒。
- 保存每次请求的原始值至 `results/samples.csv`、reference 输出至 `results/reference-actions.txt`、一致性判断至 `results/summary.json`，再从原始样本计算 min/p50/p90/max/mean。

## 复现步骤


本节固定基线 checkout、构建、服务启动、客户端参数、输出位置和验收步骤。`scripts/fixed_request_client.cpp` 是本配置唯一的固定请求客户端；其编译入口为 `scripts/build-client.sh`。客户端会拒绝非 50×32 输出、非有限值或与 reference 不同的 action。

新运行必须写入新的输出目录，不能覆盖已归档的 `results/`；它只能作为一次复现运行，不能反向改写 2026-08-31 的正式基线结论。

### 1. 构建基线

以下命令从干净的 `vla.cpp` checkout 开始执行。启用 CUDA 时，CMake 会对 FetchContent 下载的 `llama.cpp` 自动运行版本库内的 CUDA hook 脚本，无需额外手动打补丁；补丁身份、模型身份和工具链版本见 [实验配置](README.md#冻结条件)。

```bash
cd /root/vla-smoke-c01

git -C src/vla.cpp switch --detach e9e57ea734692bbcb34528fee16318e230ba1100
git -C src/vla.cpp rev-parse HEAD
# 必须输出：e9e57ea734692bbcb34528fee16318e230ba1100

cmake -S src/vla.cpp \
  -B build-formal-a10-20260831 \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DGGML_CUDA=ON \
  -DGGML_CUDA_GRAPHS=ON \
  -DCMAKE_CUDA_ARCHITECTURES=86
cmake --build build-formal-a10-20260831 --parallel 16

bash src/vla-inference-engineering/experiments/01-baseline/scripts/build-client.sh
```

构建目录是冻结构建条件的一部分：更换目录可能改变 ELF `RUNPATH`，因此不能期待得到原始客户端或服务二进制的相同 SHA-256。要核验已归档的基线客户端和原始结果字节身份，可执行：

```bash
cd /root/vla-smoke-c01
sha256sum -c src/vla-inference-engineering/experiments/01-baseline/raw/baseline-artifacts.sha256
```

该清单核验的是归档结果和此目录下的重建客户端；它不要求新的时间样本逐字节等同于历史样本。

### 2. 启动服务

终端 A：

```bash
cd /root/vla-smoke-c01

VLA_N_THREADS=16 ./build-formal-a10-20260831/vla-server \
  --bind tcp://127.0.0.1:5555 \
  models/smolvla-libero.gguf
```

等待服务输出 `vla-server: bound to tcp://127.0.0.1:5555. ready.`。模型加载和服务启动不计入稳态样本。

### 3. 发送固定请求并采集样本

终端 B：

```bash
cd /root/vla-smoke-c01

./build-formal-a10-20260831/fixed_request_client \
  --addr tcp://127.0.0.1:5555 \
  --image src/vla.cpp/assets/front.jpg \
  --out /tmp/a10-cuda-smolvla-20260831-r1-baseline-repro \
  --warmup 5 \
  --reps 30
```

客户端先执行 5 次未计时 warm-up，再顺序保存 30 个样本。输出目录包含 `samples.csv`、`reference-actions.txt` 和 `summary.json`；第一个计时请求生成 reference，后续 29 个请求必须与其 1,600 个 `float` action 值按位相同。

### 4. 验收输出与检查工件

确认客户端输出 `saved 30 samples; all_bitwise_equal_reference=true`，并确认 `summary.json` 的形状为 50×32、`all_finite` 与 `all_bitwise_equal_reference` 均为 `true`。使用 [结果汇总](README.md#实验结果) 中相同的统计方法汇总新 `samples.csv`，但不要覆盖 `results/`、修改 checksum 清单或将 profile 延迟混入这 30 个样本。

新运行的时间结果受声明环境中的正常波动影响；它用于检查该冻结配置是否可以再次执行，不允许替代、拼接或重新解释 2026-08-31 已归档的正式基线。若改变模型、设备、驱动、构建选项、输入、noise、线程数、warm-up、重复次数或统计方法，必须建立新的 `Configuration ID`。

## 实验结果

基线采集日期：2026-08-31；结果来自无 profiler 条件下的正式测量，适用范围和测量协议见上文。

### 正确性结果

常驻 `vla-server` 在模型加载后接受同一固定 JPEG、token、32 维零状态和 1,600 维固定 noise。执行 5 次未计时 warm-up 后采集 30 次顺序请求。

| 检查 | 结果 |
|---|---|
| 输出形状 | 30 次均为 50×32 |
| 有限性 | 30 次均无 `NaN`/`Inf` |
| 填充契约 | reference 输出每行后 25 列均为 `0.0` |
| 数值一致性 | 29 个后续样本均与第一个计时样本的 1,600 个 `float` action 值按位相同；客户端摘要为 `all_bitwise_equal_reference=true` |

这只建立固定输入下的结构与数值可重复性，不支持动作语义、LIBERO 成功率或机器人控制正确性结论。

### 性能结果

统计来自 `results/samples.csv` 的全部 30 个样本；未排除样本。单位均为 ms。

| 指标 | min | p50 | p90 | max | mean | stddev |
|---|---:|---:|---:|---:|---:|---:|
| 客户端端到端 wall time | 69.994 | 70.499 | 71.665 | 72.232 | 70.751 | 0.623 |
| 服务端 total | 67.285 | 67.786 | 68.946 | 69.506 | 68.027 | 0.609 |
| 服务端 vision | 18.696 | 18.749 | 18.895 | 19.184 | 18.793 | 0.108 |
| 服务端 inference | 48.062 | 48.494 | 49.646 | 50.288 | 48.735 | 0.588 |

### 结果解读与适用范围

按均值分解，服务端 inference 占 total 约 71.6%，vision 约 27.6%，其余服务端开销约 0.5 ms；客户端 loopback 协议、序列化和请求边界相对服务端 total 的均值差约 2.723 ms。这是端到端分解观察，不是主瓶颈归因。

## 工件索引

| 路径 | 内容 |
|---|---|
| [scripts/fixed_request_client.cpp](scripts/fixed_request_client.cpp) | 固定请求客户端，各阶段复用 |
| [scripts/build-client.sh](scripts/build-client.sh) | 客户端构建脚本 |
| [raw/environment-20260831.md](raw/environment-20260831.md) | 环境、构建缓存摘要及制品快照 |
| [raw/server-baseline.log](raw/server-baseline.log) | 基线服务日志 |
| [raw/fixed-request-client.log](raw/fixed-request-client.log) | 客户端运行日志 |
| [results/samples.csv](results/samples.csv) | 逐次测量原始样本 |
| [results/summary.json](results/summary.json) | 正确性摘要 |
| [results/reference-actions.txt](results/reference-actions.txt) | 参考动作输出 |
| [raw/baseline-artifacts.sha256](raw/baseline-artifacts.sha256) | 基线客户端及结果工件校验清单 |
