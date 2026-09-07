# 部署平台与资源策略

> 状态：历史本地设备与云端检查仍保留其日期和证据边界；2026-08-31 当前主机的 A10/CUDA 配置已冻结为 [`a10-cuda-smolvla-20260831-r1`](../../experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md)，本文记录设备适用边界与正式环境冻结规则，不是采购计划。

## 本地设备核验

以下事实来自 2026-08-09 的本机只读检查，原始命令与输出保存在 [`environment-check-2026-08-09.txt`](environment-check-2026-08-09.txt)：

| 项目 | 当前事实 | 可行性影响 |
|---|---|---|
| CPU | Intel Core i7-7700HQ，4 核 8 线程，支持 AVX2 | 可用于构建、预处理和 CPU 路径检查 |
| GPU | NVIDIA GeForce GTX 1060，6144 MiB，Compute Capability 6.1；Windows 驱动 572.60 | 候选路径必须核对 Pascal、6 GiB 显存和所需 CUDA/框架版本的兼容性 |
| 物理内存 | Windows 报告 33,984,040,960 bytes（约 31.65 GiB） | WSL2 当前只可见约 11.68 GiB，不能按全部物理内存规划进程 |
| 物理存储 | 128 GB SATA SSD + 1 TB SATA HDD | `C:` 当前仅约 10.43 GB 可用；WSL 根文件系统约 1.01 TB 可用，但其物理磁盘映射未核对 |
| 宿主系统 | Windows 11，版本 10.0.26100 | 保留为宿主环境，不作为 Linux 原生性能数据 |
| 当前 Linux 环境 | WSL2，Ubuntu 24.04.3 LTS，内核 `6.6.87.2-microsoft-standard-WSL2` | 已满足 Ubuntu 24.04 用户态候选，无需仅为版本号迁移系统 |
| WSL GPU 状态 | `/dev/dxg` 不存在；`nvidia-smi` 返回 `GPU access blocked by the operating system` | 当前不能在该 WSL 实例执行 GPU smoke test 或 GPU 性能基线 |
| 当前工具 | CMake 3.28.3、GCC 13.3.0、Python 3.12.3；未找到 `nvcc` 和 Docker | 具体依赖只在选定路径后核对和安装；`nvcc` 缺失不单独证明路径不可运行 |

### 本地设备适用边界

- `Hypothesis`：GTX 1060 可用于固定版本下的小模型、量化模型或 CPU/CUDA smoke test，具体 `vla.cpp` 模型兼容性仍为 `Unknown`；
- `Decision`：本机只作为轻量可行性和开发辅助环境，不作为正式 VLA GPU 基线与 profiling 的首选设备；6 GiB 显存、Pascal 架构及当前 WSL GPU 不可用都会限制候选路径；
- `Official claim`：[CUDA Toolkit 13.0 Release Notes](https://docs.nvidia.com/cuda/archive/13.0.0/cuda-toolkit-release-notes/index.html#deprecated-architectures)说明 CUDA 13.0 已移除 Maxwell、Pascal 和 Volta 的离线编译与库支持，CUDA 12.x 仍可为这些架构构建；
- `Decision`：不为本项目优先迁移本机到原生 Ubuntu 24.04；现有 WSL 已是 Ubuntu 24.04，迁移不能改变 GPU 架构或显存容量。

## 当前正式目标环境

当前唯一正式比较配置是阿里云 A10/CUDA 的 [`a10-cuda-smolvla-20260831-r1`](../../experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md)。设备、系统、驱动、Toolkit、工具版本和构建条件只在该配置及其[环境记录](../../experiments/a10-cuda-smolvla-20260831-r1/raw/environment-20260831.md)维护；profile 工件状态和模型结论只在[PROFILE-ANALYSIS.md](../../experiments/a10-cuda-smolvla-20260831-r1/PROFILE-ANALYSIS.md)维护。

2026-08-09 的项目负责人环境检查曾确认阿里云 A10 可运行最小 CUDA 程序，并能使用 Nsight Systems、Nsight Compute 与硬件计数器；原始报告、源码和完整命令未入库且不再恢复。这只支持当时的环境选择，不构成当前模型的性能、profile 或瓶颈结论。

## 正式候选设备筛选条件

当前正式候选设备采用以下筛选条件：

- NVIDIA Ampere 或更新架构；
- 单卡显存不少于 24 GB；
- 能运行 Nsight Systems 和 Nsight Compute；
- 能读取 GPU hardware performance counters 并导出原始 profile 工件。

16 GB 只作为部分模型可能达到的最低推理线，不作为当前项目的默认筛选线；具体显存需求仍须由选定模型/checkpoint 的固定版本验证。

## 环境选择与冻结

- 当前 A10/CUDA 以 `a10-cuda-smolvla-20260831-r1` 为唯一正式比较配置；其详细事实由对应 `CONFIGURATION.md` 维护；
- 历史设备、构建产物、随机 smoke 输出及其时间不进入当前配置的性能、profile 或优化前后比较；
- 当前不购买 GPU，也不迁移本机操作系统；
- 国产加速卡迁移不进入当前 NVIDIA/CUDA 主闭环；如果后续启动，必须建立新的 `Configuration ID`，不得与 NVIDIA 设备结果组成同一优化前后对比；
- 只有需要付费资源、反复切换路径或预计投入明显超出合理范围时，才单独评估是否继续；
- 进入正式测量前，目标设备、操作系统、驱动、CUDA 与 Runtime 版本必须按[正式基线配置](../project/scope.md#正式基线配置)冻结；
- 优化前后必须使用相同环境与输入；关键环境变化时建立新的 `Configuration ID`；
- 借用设备或付费资源仅在获得单独授权后使用，并只验证明确的兼容性或资源假设；
- 本文不预设操作系统迁移、平台选择或硬件采购路线。
