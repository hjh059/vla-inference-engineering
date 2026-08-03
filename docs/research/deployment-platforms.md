# 部署平台与资源策略调研

> 状态：Problem Discovery 输入，不是生产平台或采购决策。
>
> 调研基准日期：2026-08-02。版本、价格和支持矩阵必须在实际复现或采购前重新核对。

Problem Discovery 默认使用当前本地设备开发和执行最小复现，不以选出“最优生产平台”为目标。Jetson、x86_64 整机升级和临时云 GPU 都是候选资源；只有出现稳定、可归因的资源阻塞后，才比较并确认具体方案。

## 当前本地基线

以下为 2026-08-02 对当前宿主机的只读检查结果，分类为 `Fact`。它只证明设备清单，不证明目标 Runtime 已经可用：

| 项目 | 当前值 | 调查含义 |
|---|---|---|
| 设备 | TR 911K 笔记本 | 当前零新增采购成本的开发基线 |
| CPU | Intel Core i7-7700HQ，4 核 8 线程，支持 AVX2 | 可进行代码检查、构建和小规模 CPU 路径验证 |
| GPU | NVIDIA GeForce GTX 1060，6 GiB，Compute Capability 6.1 | 可调查小模型或量化路径，但现代 CUDA/TensorRT 支持需要单独核验 |
| 内存 | 32 GiB DDR4-2400 | 可支持开发、转换和有限规模本地实验 |
| 存储 | 128 GB SSD + 1 TB HDD | SSD 空间紧张；模型、缓存和日志可放 HDD，但 I/O 测量需排除盘速影响 |
| 当前系统 | Windows 11 宿主机 + Ubuntu 24.04 WSL | 当前会话无法访问 GPU，不等同于宿主机没有 GPU |

后续计划切换到原生 Ubuntu 桌面版。Ubuntu 24.04 LTS 是当前首个兼容性验证候选，不是冻结决策；最终版本必须同时满足 GTX 1060 驱动、CUDA、被选 Issue 路径及相关实现/Runtime 和必要开发工具的兼容性。原生系统安装完成前，GPU 可用性仍为 `Unknown`。

GTX 1060 属于 Pascal 架构。它不能被默认视为当前 TensorRT 路径的有效基线；实际调查必须固定 CUDA、驱动和 Runtime 版本，并区分“旧版本仍可运行”和“当前上游仍受支持”。

## 已确认的资源策略

1. **本地优先**：先使用当前设备完成代码检查、构建、固定输入、CPU/小模型 smoke test 和能够解释的失败复现；
2. **问题触发**：只有遇到可重复的显存、算力、GPU 架构、驱动、平台 I/O 或软件栈阻塞，才进入资源选型；
3. **同题比较**：Jetson、整机升级和云 GPU 必须使用同一最小复现与同一成功条件比较；
4. **最小投入**：先验证临时资源能否消除阻塞，再决定长期采购；
5. **不以采购代替问题发现**：新硬件使某条路径可运行，不自动证明存在值得独立立项的技术问题。

## 评估约束

如果后续问题涉及完整边缘部署，候选平台可能需要同时容纳模型、Cache、后端工作区和机器人侧进程。以下条件目前都是候选评估维度：

- 无 swap 运行；
- 被选实现、Runtime 或后端及其所需算子可用；
- 策略更新、首动作、任务效果、功耗和温度门禁；
- 相机、机器人控制、调试与交付条件；
- 可维护的软件栈和可验证的升级、回滚路径。

硬件与模型、后端和精度是联合选择，不能先独立选出“最优硬件”再让其他部分迁就。

## 候选资源路径

### 当前设备继续开发

优势：

- 无新增硬件成本，可立即用于非 GPU 工作和小规模路径；
- 能暴露旧 GPU 架构、有限显存和本地部署兼容性问题。

边界：

- 6 GiB 显存和 Pascal 架构可能无法运行现代 TensorRT 或较大 VLA；
- 老 CPU、SATA SSD/HDD 和笔记本散热会混入性能结果；
- 不能将本机跑不动直接归类为现有 Runtime 缺陷。

继续使用条件：目标最小路径能够运行，或当前工作仍是与 GPU 无关的调查。

### 临时云 GPU

优势：

- 可按需验证更大显存、更新 GPU 架构或官方推荐环境能否消除本地阻塞；
- 在整机采购前获得模型显存、延迟和软件兼容性证据。

边界：

- 费用会随镜像准备、模型下载和重复实验累积；
- 不能替代 Jetson 的功耗、热稳态、摄像头、GPIO、机器人网络和设备侧故障验证；
- 云端结果不能直接代表本地闭环端到端延迟。

使用条件：已有固定复现和明确的 GPU 规格假设，并设置时间与费用上限。

### Jetson

Jetson 是可选的第二平台，适合验证 ARM64、JetPack、统一内存、功耗和机器人侧 I/O，不是当前开发主机的默认替代品。

#### Jetson Orin Nano / Orin NX

优势：

- 适合调查紧凑或量化 VLA 的边缘部署；
- 功耗、体积和设备侧接口更接近机器人边缘端。

风险：

- 统一内存需由模型、Cache、工作区和其他进程共享；
- ARM64、JetPack/L4T 版本绑定和第三方包支持会引入与 x86_64 不同的变量；
- 开发套件、载板、存储、电源和散热共同构成实际成本；
- 低端型号不能替代大显存 dGPU 的完整参考或训练/转换环境。

适用条件：候选问题明确涉及设备侧部署、功耗、I/O 或 JetPack，且目标模型在可用内存与软件栈内有可验证路径。

#### Jetson AGX Orin / AGX Thor

优势：

- 内存、带宽和边缘算力余量更大；
- 可用于较大模型以及功耗、热稳态和设备侧集成验证。

风险：

- 采购成本不一定低于 x86_64 + dGPU 整机；
- 平台较新或特定 JetPack 路径的驱动、外设和部署经验需要实测；
- 如果候选问题不涉及边缘功耗、I/O 或 ARM64，购买依据不足。

适用条件：云 GPU 或借用设备已确认模型资源需求，且项目问题明确要求在 Jetson 上闭环验证。

### x86_64 整机升级

优势：

- 调试、构建、转换和 GPU 后端支持通常更完整；
- GPU、内存和存储可按已测需求选择并后续升级；
- 可同时承担日常开发、正确性参考和本地性能实验。

风险：

- 前期投入高，容易在问题和模型未确定前过度配置；
- 功耗、体积和设备侧 I/O 不代表最终机器人边缘平台；
- 仅升级 GPU 可能受现有整机电源、散热、机箱和平台代际限制，需按整机核算。

适用条件：固定复现证明现有设备受 GPU 架构、显存、CPU、内存或存储的组合限制，且本地高频使用比临时云 GPU 更合理。

## 升级或采购门禁

提交任何具体采购建议前必须具备：

- 固定仓库 commit、模型、输入、环境和复现命令；
- 至少三次一致失败或资源不足记录；
- GPU 显存、系统内存、延迟、磁盘或兼容性中的明确阻塞指标；
- 对配置错误、旧版本路径、小模型/量化路径和合理替代 Runtime 的检查；
- 云 GPU 或借用设备上的一次对照，或说明无法对照的原因；
- Jetson、整机升级、继续使用本机和云 GPU 的成本—证据收益比较；
- 采购后能够判定“阻塞已解除”的验收命令和指标。

若没有满足上述条件，结论保持为 `Insufficient evidence`，不确认具体设备。

## 软件栈候选

### Orin 候选路线

JetPack 6.2、Ubuntu 22.04 和 ROS2 Humble 的部署积累较多，但对 Thor 和较新的 VLA 优化路径覆盖有限。

### JetPack 7 候选路线

以 JetPack 7、Ubuntu 24.04 和 ROS2 Jazzy 组成统一候选。实际 CUDA、cuDNN 和 TensorRT 版本由 JetPack 绑定，不在 Jetson 上独立追新。

Isaac ROS/NITROS 不能预设为所有 Orin/Thor 组合的统一依赖。只有目标平台明确支持且基准证明收益时才评估。

### 独立组件追新

在 Jetson 上绕过 JetPack 单独替换 CUDA 或 TensorRT 会破坏平台验证矩阵和回滚边界，当前不进入首轮。

## 评测输入

平台候选使用同一任务、模型制品和输入，记录：

- 冷启动、模型加载和 Engine 反序列化；
- 峰值系统内存、GPU 共享内存和工作区；
- 完整 Chunk、首动作和端到端延迟；
- CPU/GPU 利用率、功耗、温度和降频；
- 任务结果、安全拒绝和非法动作；
- 热稳态与长稳结果。

具体统计方法和通过标准只由[验收原则](../adr/0014-acceptance-principles.md)维护。评测顺序和写回项见 [Phase 0 实施计划](../plans/phase-0.md)。

## 资料

- [JetPack SDK](https://developer.nvidia.com/embedded/jetpack/downloads)
- [Jetson Orin Nano Super Developer Kit](https://www.nvidia.com/en-us/autonomous-machines/embedded-systems/jetson-orin/nano-super-developer-kit/)
- [Jetson Orin](https://www.nvidia.com/en-us/autonomous-machines/embedded-systems/jetson-orin/)
- [Jetson Thor](https://www.nvidia.com/en-gb/autonomous-machines/embedded-systems/jetson-thor/)
- [GR00T 硬件建议](https://github.com/NVIDIA/Isaac-GR00T/blob/main/getting_started/hardware_recommendation.md)
- [TensorRT 支持矩阵](https://docs.nvidia.com/deeplearning/tensorrt/latest/getting-started/support-matrix.html)
- [CUDA Toolkit 12.9 Release Notes](https://docs.nvidia.com/cuda/archive/12.9.0/cuda-toolkit-release-notes/index.html)
- [Torch-TensorRT Releases](https://github.com/pytorch/TensorRT/releases)
