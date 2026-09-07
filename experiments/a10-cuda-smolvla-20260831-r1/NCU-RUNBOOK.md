# Nsight Compute 采集说明

本文记录正式配置 `a10-cuda-smolvla-20260831-r1` 已验证有效的 Nsight Compute 过滤条件和复现顺序。Profile 二进制的源码 revision 和构建命令见 [`PROFILE-BUILD.md`](PROFILE-BUILD.md)；硬件计数器结果及结论边界以 [`PROFILE-ANALYSIS.md`](PROFILE-ANALYSIS.md#nsight-compute-细粒度验证) 为准。本页不重复维护构建条件或指标。

## 固定条件

- 使用 [Profile 观测构建](PROFILE-BUILD.md)中固定的 `vla.cpp@4e46904efbc57ef4ccff613c2a23cfd1cc335fed` 和对应 CUDA `vla-server`；
- 每个目标 kernel 使用全新的服务进程，使唯一计时请求为 `rid=6`；
- 客户端执行 `--warmup 5 --reps 1`；
- launch 端使用 `--target-processes all --nvtx`；
- attach 端使用 push/pop 过滤 `vla.profile@predict rid=6/`，末尾的 `/` 不可省略；
- 使用 `--nvtx-push-pop-scope process`、`--graph-profiling node` 和 `--set detailed`；
- NCU replay 会显著扰动请求时间，采集期间的延迟不得用于正式性能比较。

以下命令均从 `/root/vla-smoke-c01` 开始执行。每采集一个目标 kernel，都重新执行完整的服务启动、attach 和客户端请求顺序。

## 1. 启动待采集服务

终端 A：

```bash
cd /root/vla-smoke-c01

VLA_N_THREADS=16 ncu \
  --mode=launch \
  --port 49152 \
  --target-processes all \
  --nvtx \
  ./build-formal-a10-20260831/vla-server \
  --bind tcp://127.0.0.1:5555 \
  models/smolvla-libero.gguf
```

服务会等待 attach。完成下一节的终端 C 命令并看到服务输出 `ready.` 后，再执行客户端。

## 2. Attach 并选择目标 kernel

### `convert_unary`

终端 C：

```bash
cd /root/vla-smoke-c01/src/vla-inference-engineering/experiments/a10-cuda-smolvla-20260831-r1

ncu \
  --mode=attach \
  --hostname 127.0.0.1 \
  --port 49152 \
  --nvtx-include 'vla.profile@predict rid=6/' \
  --nvtx-push-pop-scope process \
  --kernel-name-base demangled \
  --kernel-name 'regex:.*convert_unary.*' \
  --launch-count 1 \
  --graph-profiling node \
  --set detailed \
  --force-overwrite \
  --export raw/vla-steady-r2/ncu-convert-rid6 \
  --log-file raw/vla-steady-r2/ncu-convert-rid6.log
```

### CUTLASS `Kernel2` GEMM

先停止上一轮服务并重新执行第 1 节，再在终端 C 执行：

```bash
cd /root/vla-smoke-c01/src/vla-inference-engineering/experiments/a10-cuda-smolvla-20260831-r1

ncu \
  --mode=attach \
  --hostname 127.0.0.1 \
  --port 49152 \
  --nvtx-include 'vla.profile@predict rid=6/' \
  --nvtx-push-pop-scope process \
  --kernel-name-base demangled \
  --kernel-name 'regex:.*cutlass_80_tensorop_s1688gemm_64x64_16x6_tn_align4.*' \
  --launch-count 1 \
  --graph-profiling node \
  --set detailed \
  --force-overwrite \
  --export raw/vla-steady-r2/ncu-gemm-rid6 \
  --log-file raw/vla-steady-r2/ncu-gemm-rid6.log
```

## 3. 发送固定请求

终端 B，在服务输出 `ready.` 后执行：

```bash
cd /root/vla-smoke-c01

./build-formal-a10-20260831/fixed_request_client \
  --addr tcp://127.0.0.1:5555 \
  --image src/vla.cpp/assets/front.jpg \
  --out /tmp/ncu-rid6-client \
  --warmup 5 \
  --reps 1
```

等待 NCU 完成报告写入后停止服务。客户端输出只用于确认采集请求完成及检查输出结构；本次已归档证据中，只有 `convert_unary` 采集保留了可与对应运行关联的客户端正确性工件。不得为 GEMM 报告补作或推定不存在的关联证据。

## 4. 工件验收

- 日志必须明确记录目标 kernel 完成 18 passes，不能出现“没有匹配 kernel”；
- 报告的 NVTX 元数据必须位于 `vla.profile` 的 `request rid=6` / `predict rid=6`；
- 报告、日志和实际保留的客户端工件加入 [profile 校验和清单](raw/profile-artifacts.sha256)；
- 原始工件及其用途由 [`raw/vla-steady-r2/README.md`](raw/vla-steady-r2/README.md) 维护。

所有 `.sha256` 清单均从工作区根目录验证：

```bash
cd /root/vla-smoke-c01
sha256sum -c src/vla-inference-engineering/experiments/a10-cuda-smolvla-20260831-r1/raw/profile-artifacts.sha256
```
