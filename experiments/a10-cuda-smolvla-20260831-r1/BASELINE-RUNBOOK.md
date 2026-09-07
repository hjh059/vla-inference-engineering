# 正式基线复现说明

本文是 `a10-cuda-smolvla-20260831-r1` 的唯一正式基线复现流程。它复现冻结的基线实现和测量协议；带 NVTX 的派生 profile 构建及其采集流程分别见 [PROFILE-BUILD.md](PROFILE-BUILD.md)、[NSYS-RUNBOOK.md](NSYS-RUNBOOK.md) 和 [NCU-RUNBOOK.md](NCU-RUNBOOK.md)，不能替代本流程。

冻结的模型、环境、构建选项、输入、正确性标准和指标定义以 [CONFIGURATION.md](CONFIGURATION.md) 为准。新运行必须写入新的输出目录，不能覆盖已归档的 `results/`；它只能作为一次复现运行，不能反向改写 2026-08-31 的正式基线结论。

## 1. 构建冻结的基线

以下命令从干净的 `vla.cpp` checkout 开始执行。启用 CUDA 时，CMake 会对 FetchContent 下载的 `llama.cpp` 自动运行版本库内的 CUDA hook 脚本，无需额外手动打补丁；补丁身份、模型身份和工具链版本见 [CONFIGURATION.md](CONFIGURATION.md#冻结条件)。

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

src/vla-inference-engineering/experiments/a10-cuda-smolvla-20260831/scripts/build-client.sh
```

构建目录是冻结构建条件的一部分：更换目录可能改变 ELF `RUNPATH`，因此不能期待得到原始客户端或服务二进制的相同 SHA-256。要核验已归档的基线客户端和原始结果字节身份，可执行：

```bash
cd /root/vla-smoke-c01
sha256sum -c src/vla-inference-engineering/experiments/a10-cuda-smolvla-20260831-r1/raw/baseline-artifacts.sha256
```

该清单核验的是归档结果和此目录下的重建客户端；它不要求新的时间样本逐字节等同于历史样本。

## 2. 启动基线服务

终端 A：

```bash
cd /root/vla-smoke-c01

VLA_N_THREADS=16 ./build-formal-a10-20260831/vla-server \
  --bind tcp://127.0.0.1:5555 \
  models/smolvla-libero.gguf
```

等待服务输出 `vla-server: bound to tcp://127.0.0.1:5555. ready.`。模型加载和服务启动不计入稳态样本。

## 3. 采集 30 个正式稳态样本

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

## 4. 验收与边界

确认客户端输出 `saved 30 samples; all_bitwise_equal_reference=true`，并确认 `summary.json` 的形状为 50×32、`all_finite` 与 `all_bitwise_equal_reference` 均为 `true`。使用 [RESULTS.md](RESULTS.md) 中相同的统计方法汇总新 `samples.csv`，但不要覆盖 `results/`、修改 checksum 清单或将 profile 延迟混入这 30 个样本。

新运行的时间结果受声明环境中的正常波动影响；它用于检查该冻结配置是否可以再次执行，不允许替代、拼接或重新解释 2026-08-31 已归档的正式基线。若改变模型、设备、驱动、构建选项、输入、noise、线程数、warm-up、重复次数或统计方法，必须建立新的 `Configuration ID`。
