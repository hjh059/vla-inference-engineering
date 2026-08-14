# C-01 smoke 环境准备、构建与证据采集命令

> 本文记录项目负责人补充的 2026-08-11 历史执行命令，并用 2026-08-14 当前工作区可核对的信息补齐变量值和工作目录。它不是可直接自动执行的安装脚本。

## 补齐依据与证据边界

- 现有证据文件名确认 `VLA_EVIDENCE_TAG=20260811-c01`；
- `build-cuda/CMakeCache.txt` 确认 `CMAKE_HOME_DIRECTORY=/root/vla-smoke-c01/src/vla.cpp`，因此历史命令中的 `cmake -S .` 从该目录执行；
- 当前工作区 `nproc` 为 8；历史构建命令使用 `--parallel "$(nproc)"`，因此按当前环境解析为 8；
- 构建缓存确认 Release、Ninja、`GGML_CUDA=ON`、`GGML_CUDA_GRAPHS=ON` 和 `CMAKE_CUDA_ARCHITECTURES=86`；历史配置命令没有显式传入 `GGML_CUDA_GRAPHS`，该值来自当时解析后的缓存；
- `FetchContent` 工作树确认 `llama.cpp` 为 `75a48a90559abf65df3f3616a53bb16e5afb9d07`；
- 下方软件包版本来自 2026-08-14 当前工作区，不作为 2026-08-11 安装时版本的原始证明；
- 当前会话无法连接 NVIDIA 驱动；2026-08-11 的 A10、驱动和 CUDA 运行环境仍以 [`raw/environment.txt`](raw/environment.txt) 为原始证据。

## 历史执行命令

以下命令由项目负责人确认为此前执行步骤；只补入了已由当前工作区核对的变量值和执行目录。

```bash
export VLA_SMOKE_ROOT=/root/vla-smoke-c01
export VLA_EVIDENCE_TAG=20260811-c01
cd "$VLA_SMOKE_ROOT/src/vla.cpp"

apt-get update
apt-get install -y \
  build-essential \
  cmake \
  ninja-build \
  git \
  curl \
  ca-certificates \
  time

cmake \
  -S . \
  -B "$VLA_SMOKE_ROOT/build-cuda" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DGGML_CUDA=ON \
  -DCMAKE_CUDA_ARCHITECTURES=86

cmake \
  --build "$VLA_SMOKE_ROOT/build-cuda" \
  --parallel "$(nproc)"

sha256sum \
  "$VLA_SMOKE_ROOT/models/smolvla-libero.gguf" \
  | tee "$VLA_SMOKE_ROOT/evidence/model-${VLA_EVIDENCE_TAG}.sha256"

sha256sum \
  "$VLA_SMOKE_ROOT/src/vla.cpp/assets/front.jpg" \
  | tee "$VLA_SMOKE_ROOT/evidence/input-${VLA_EVIDENCE_TAG}.sha256"

sha256sum \
  "$VLA_SMOKE_ROOT/build-cuda/vla-cli" \
  | tee "$VLA_SMOKE_ROOT/evidence/vla-cli-${VLA_EVIDENCE_TAG}.sha256"

{
  date --iso-8601=seconds
  uname -a

  echo "vla.cpp revision:"
  git -C "$VLA_SMOKE_ROOT/src/vla.cpp" rev-parse HEAD

  echo "GPU:"
  nvidia-smi \
    --query-gpu=name,memory.total,driver_version,compute_cap \
    --format=csv,noheader

  echo "CUDA Toolkit:"
  nvcc --version

  echo "CMake:"
  cmake --version

  echo "Compiler:"
  g++ --version

  echo "Model:"
  sha256sum "$VLA_SMOKE_ROOT/models/smolvla-libero.gguf"

  echo "Input:"
  sha256sum "$VLA_SMOKE_ROOT/src/vla.cpp/assets/front.jpg"

  echo "Executable:"
  sha256sum "$VLA_SMOKE_ROOT/build-cuda/vla-cli"
} 2>&1 | tee \
  "$VLA_SMOKE_ROOT/evidence/environment-${VLA_EVIDENCE_TAG}.txt"

sha256sum \
  "$VLA_SMOKE_ROOT/evidence/environment-${VLA_EVIDENCE_TAG}.txt" \
  | tee "$VLA_SMOKE_ROOT/evidence/environment-${VLA_EVIDENCE_TAG}.sha256"
```

两次 smoke 的实际运行命令保存在原始日志中，并在本记录的 [`README.md`](README.md#已知构建与运行条件) 汇总。

## 当前工作区解析值

| 项目 | 2026-08-14 当前值 |
|---|---|
| 操作系统 | Ubuntu 24.04.4 LTS |
| `nproc` | `8` |
| `build-essential` | `12.10ubuntu1` |
| `cmake` | `3.28.3-1build7` |
| `ninja-build` | `1.11.1-2` |
| `git` | `1:2.43.0-1ubuntu7.3` |
| `curl` | `8.5.0-2ubuntu10.9` |
| `ca-certificates` | `20240203` |
| `time` | `1.9-0.2build1` |
| `g++` | `13.3.0-6ubuntu2~24.04.1` |
| CUDA compiler build | `cuda_12.8.r12.8/compiler.35583870_0` |

## 后续显式复现命令

正式配置应使用绝对源目录、显式 CUDA graphs 选项和已记录的并行度，避免依赖工作目录或 CMake 默认值：

```bash
export VLA_SMOKE_ROOT=/root/vla-smoke-c01

cmake \
  -S "$VLA_SMOKE_ROOT/src/vla.cpp" \
  -B "$VLA_SMOKE_ROOT/build-cuda" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DGGML_CUDA=ON \
  -DGGML_CUDA_GRAPHS=ON \
  -DCMAKE_CUDA_ARCHITECTURES=86

cmake \
  --build "$VLA_SMOKE_ROOT/build-cuda" \
  --parallel 8
```

正式 `Configuration ID` 还必须记录当次实际解析的软件包版本、`/etc/os-release`、GPU/驱动、CUDA、依赖 commit、完整命令及新生成的制品 SHA-256；不能直接继承本可行性记录的当前值。
