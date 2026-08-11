# 部署平台与资源策略

> 状态：本地设备和阿里云 A10 候选环境已于 2026-08-09 完成可行性核验；本文记录设备适用边界与正式环境冻结规则，不是采购计划。

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

## 当前云端候选环境

阿里云 `ecs.gn7i-c8g1.2xlarge` 已通过 CUDA 与 profiling 可行性验证，详细记录见[阿里云 A10 环境与 profiling 可行性记录](cloud-environment-check-2026-08-09.md)。

| 项目 | 当前事实 | 证据范围 |
|---|---|---|
| 计算资源 | 8 vCPU、30 GiB、完整 NVIDIA A10 × 1；实测显存 `23028 MiB`，Ampere 架构 | 官方实例规格、NVIDIA 数据表及项目负责人提供的实例检查结果 |
| 软件环境 | Ubuntu 24.04、驱动 `580.126.09`、CUDA Toolkit `12.8`、Python `3.12.3`、Docker `29.1.3` | 项目负责人提供的环境检查结果 |
| Profiling | Nsight Systems `2024.6.2` 与 Nsight Compute `2025.1.1.0` 均成功生成报告；硬件计数器访问未出现 `ERR_NVGPUCTRPERM` | 只证明工具、权限和报告导出可用，不构成模型性能结论 |
| 原始工件 | `.ncu-rep` 与 `.nsys-rep` 当前保存在云实例 `/root` 下 | 尚未持久化到本工作区，实例释放前必须导出 |

## 正式候选设备筛选条件

当前正式候选设备采用以下筛选条件：

- NVIDIA Ampere 或更新架构；
- 单卡显存不少于 24 GB；
- 能运行 Nsight Systems 和 Nsight Compute；
- 能读取 GPU hardware performance counters 并导出原始 profile 工件。

16 GB 只作为部分模型可能达到的最低推理线，不作为当前项目的默认筛选线；具体显存需求仍须由选定模型/checkpoint 的固定版本验证。

## 环境选择与冻结

- C-01 已在阿里云 A10 环境通过重复 smoke 并选为正式基线路径；本机仅用于轻量检查和开发辅助；
- 阿里云 A10 已作为所选路径的正式设备候选，但还不是已冻结的正式基线；创建 `Configuration ID` 后才冻结为正式环境；
- 当前不购买 GPU，也不迁移本机操作系统；
- 国产加速卡迁移不进入当前 NVIDIA/CUDA 主闭环；如果后续启动，必须建立新的 `Configuration ID`，不得与 NVIDIA 设备结果组成同一优化前后对比；
- 只有需要付费资源、反复切换路径或预计投入明显超出合理范围时，才单独评估是否继续；
- 进入正式测量前，目标设备、操作系统、驱动、CUDA 与 Runtime 版本必须按[正式基线配置](../project/scope.md#正式基线配置)冻结；
- 优化前后必须使用相同环境与输入；关键环境变化时建立新的 `Configuration ID`；
- 借用设备或付费资源仅在获得单独授权后使用，并只验证明确的兼容性或资源假设；
- 本文不预设操作系统迁移、平台选择或硬件采购路线。
