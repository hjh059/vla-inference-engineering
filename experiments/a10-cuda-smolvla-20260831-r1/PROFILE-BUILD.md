# Profile 观测构建

本文只记录 Nsight Systems 和 Nsight Compute 共用的观测构建。正式性能基线使用 `vla.cpp@e9e57ea734692bbcb34528fee16318e230ba1100`；该 revision 不包含请求级 NVTX 范围，不能直接用于当前 profile runbook。

Profile 使用派生 revision：

```text
vla.cpp@4e46904efbc57ef4ccff613c2a23cfd1cc335fed
```

该 revision 相对基线只在 CUDA `vla-server` 中增加 `vla.profile` domain 的 `request rid=<id>` 和嵌套 `predict rid=<id>` 范围，并在 CUDA 构建时定义 `VLA_NVTX_PROFILE`。正式采集使用的二进制身份由 [`raw/profile-artifacts.sha256`](raw/profile-artifacts.sha256) 维护。

## 构建命令

使用干净的独立 checkout，并在满足 [`CONFIGURATION.md`](CONFIGURATION.md#冻结条件) 中工具链和依赖条件后执行：

```bash
cd /root/vla-smoke-c01

git -C src/vla.cpp switch --detach 4e46904efbc57ef4ccff613c2a23cfd1cc335fed
git -C src/vla.cpp rev-parse HEAD
# 必须输出：4e46904efbc57ef4ccff613c2a23cfd1cc335fed

cmake -S src/vla.cpp \
  -B build-formal-a10-20260831 \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DGGML_CUDA=ON \
  -DGGML_CUDA_GRAPHS=ON \
  -DCMAKE_CUDA_ARCHITECTURES=86

cmake --build build-formal-a10-20260831 \
  --target vla-server \
  --parallel 16
```

上述目录与绝对路径属于已采集二进制的构建条件。换用其他构建目录时，ELF `RUNPATH` 会变化，不能期待得到相同 SHA-256；此时只能声明相同源码和构建配置下的重新构建，不能冒充原二进制。

验证当前正式 profile 二进制：

```bash
cd /root/vla-smoke-c01
sha256sum -c src/vla-inference-engineering/experiments/a10-cuda-smolvla-20260831-r1/raw/profile-artifacts.sha256
```

该清单还包含 profile 工件，因此命令会同时验证当前目录下的观测构建和已归档报告。基线构建快照与 profile 构建不是同一制品；前者结果见 [`RESULTS.md`](RESULTS.md)，后者的适用范围见 [`PROFILE-ANALYSIS.md`](PROFILE-ANALYSIS.md)。
