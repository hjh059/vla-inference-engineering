# 当前正式环境采集（2026-08-31）

> 本记录对应 `a10-cuda-smolvla-20260831-r1`。

## 硬件与系统

| 项目 | 记录值 |
|---|---|
| OS | Ubuntu 24.04.4 LTS (`noble`) |
| Kernel | `6.8.0-111-generic` |
| CPU | Intel(R) Xeon(R) Platinum 8369B CPU @ 2.90GHz |
| CPU 拓扑 | 1 socket、8 cores、每核 2 threads、16 online CPUs、单 NUMA node |
| RAM | 62,794,043,392 bytes（约 58 GiB） |
| Swap | 0 bytes |
| GPU | NVIDIA A10，23,028 MiB，Compute Capability 8.6，persistence mode Enabled |
| NVIDIA 驱动 | `580.126.09`；`nvidia-smi` 报告 CUDA Version 13.0 |
| CUDA Toolkit | `nvcc` 12.8.93，build `cuda_12.8.r12.8/compiler.35583870_0` |

GPU 信息来自 2026-08-31 交互终端成功执行的：

```bash
nvidia-smi --query-gpu=name,memory.total,driver_version,compute_cap,persistence_mode --format=csv,noheader
# NVIDIA A10, 23028 MiB, 580.126.09, 8.6, Enabled
```

## 构建工具与依赖

| 项目 | 记录值 |
|---|---|
| CMake | 3.28.3 |
| Ninja | 1.11.1 |
| GCC/G++ | 13.3.0 |
| Python | 3.12.3 |
| Protobuf | 3.21.12 |
| ZeroMQ | 4.3.5 |
| vla.cpp | `e9e57ea734692bbcb34528fee16318e230ba1100` |
| llama.cpp | `7ba604f1cb61cd14898138e9abc0b4ff2601f180` + 构建期 hook patch |

## 独立 CUDA 构建

```bash
cmake -S /root/vla-smoke-c01/src/vla.cpp \
  -B /root/vla-smoke-c01/build-formal-a10-20260831 \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DGGML_CUDA=ON \
  -DGGML_CUDA_GRAPHS=ON \
  -DCMAKE_CUDA_ARCHITECTURES=86
cmake --build /root/vla-smoke-c01/build-formal-a10-20260831 --parallel 16
```

构建缓存 SHA-256：`cd1cf78a32195711d2f6339e808191557e3c9e6e425b646ccecb6347e026040f`。

| 制品 | SHA-256 |
|---|---|
| `build-formal-a10-20260831/vla-cli` | `392018490e3ca9d7b9d228e11e161a0fbf9c71f57ffc86610c7c9801fedfb43c` |
| `build-formal-a10-20260831/vla-server` | `c88519243a3f070608272e9a68ee6720a19f926066ea1fa9e094d52e1be33d09` |
| `build-formal-a10-20260831/vla-bench` | `291e363b67368f382795820c35e93673bc936e618f787e70d9a4bdfa35db9d9c` |

模型、输入资产和 CUDA hook patch 属于冻结配置，其字节身份只在上级 [`CONFIGURATION.md`](../CONFIGURATION.md#冻结条件) 中维护。`llama.cpp` 工作树包含 vla.cpp 构建时加入的 CUDA hook 修改。
