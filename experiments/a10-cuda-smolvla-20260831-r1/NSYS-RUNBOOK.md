# Nsight Systems 隔离稳态采集说明

本文记录 `vla-steady-r2` 成功采集所对应的可复现流程。Profile 二进制的源码 revision 和构建命令见 [`PROFILE-BUILD.md`](PROFILE-BUILD.md)；冻结输入和客户端协议见 [`CONFIGURATION.md`](CONFIGURATION.md)。

原始逐字 shell transcript 当时没有归档。以下命令由已归档 `nsys.nsys-rep` 的 capture metadata、导出 SQLite、Git 历史中的采集方案和固定客户端参数共同还原；其中会话、服务命令、工作目录、trace 类型、CUDA Graph node、线程数及 `rid=1–12` 范围均可由现有工件复核。

## 1. 启动服务但暂不 collection

终端 A：

```bash
cd /root/vla-smoke-c01

VLA_N_THREADS=16 nsys launch \
  --session-new vla-steady-r2-repro \
  --trace=cuda,nvtx \
  --cuda-graph-trace=node \
  --wait=primary \
  ./build-formal-a10-20260831/vla-server \
  --bind tcp://127.0.0.1:5555 \
  models/smolvla-libero.gguf
```

等待服务输出 `vla-server: bound to tcp://127.0.0.1:5555. ready.`。

## 2. 在 collection 外完成正式预热

终端 B：

```bash
cd /root/vla-smoke-c01

./build-formal-a10-20260831/fixed_request_client \
  --addr tcp://127.0.0.1:5555 \
  --image src/vla.cpp/assets/front.jpg \
  --out /tmp/vla-steady-r2-repro-warmup \
  --warmup 5 \
  --reps 1
```

客户端要求 `--reps >= 1`，因此额外请求也发生在 collection 外，不进入 profile 分析。

## 3. 开始 collection

终端 C：

```bash
cd /root/vla-smoke-c01

nsys start \
  --session vla-steady-r2-repro \
  --sample=none \
  --cpuctxsw=none \
  --export=sqlite \
  --output /tmp/vla-steady-r2-repro-nsys
```

## 4. 发送 2 个丢弃请求和 10 个正式请求

终端 B：

```bash
cd /root/vla-smoke-c01

./build-formal-a10-20260831/fixed_request_client \
  --addr tcp://127.0.0.1:5555 \
  --image src/vla.cpp/assets/front.jpg \
  --out /tmp/vla-steady-r2-repro-results \
  --warmup 2 \
  --reps 10
```

该客户端进程从 `rid=1` 开始：两次 warm-up 对应 `rid=1–2`，只用于消除 collection 启动扰动；保存到 `samples.csv` 的 10 个正式请求对应 `rid=3–12`。

## 5. 停止 collection

终端 C：

```bash
nsys stop --session vla-steady-r2-repro
nsys shutdown --session vla-steady-r2-repro
```

随后停止终端 A 中的服务。本复现流程输出到新的 `/tmp/vla-steady-r2-repro-*` 路径，不覆盖当前正式工件。若需要形成新的正式证据，必须使用新的 run ID 和工件目录，不得写入现有 `raw/vla-steady-r2/`。

## 6. 验收

```bash
cd /root/vla-smoke-c01

nsys stats \
  --report nvtx_sum \
  --format csv \
  /tmp/vla-steady-r2-repro-nsys.sqlite
```

必须确认：

- 恰有 12 个闭合的 `request` 和 12 个闭合的嵌套 `predict` 范围；
- `rid=1–2` 不进入正式汇总，正式范围仅为 `predict rid=3–12`；
- 每个正式范围包含 CUDA Runtime 和 GPU kernel 活动；
- 客户端保存 10 个正式样本，输出均满足冻结正确性标准；
- profile 延迟只用于范围和热点分析，不替代 30 样本正式性能基线。

当前归档结果、范围验收和指标见 [`PROFILE-ANALYSIS.md`](PROFILE-ANALYSIS.md#隔离稳态-profilevla-steady-r2)，工件用途和大小见 [`raw/vla-steady-r2/README.md`](raw/vla-steady-r2/README.md)。
