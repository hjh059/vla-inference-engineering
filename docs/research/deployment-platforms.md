# 部署平台与资源策略调研

> 状态：Problem Discovery 输入，不是生产平台或采购决策。
>
> 调研基准日期：2026-08-02。版本、价格和支持矩阵必须在实际复现或采购前重新核对。

Problem Discovery 默认使用当前本地设备开发和执行最小复现，不以选出“最优生产平台”为目标。稳定问题成立前，可以使用借用设备、Jetson 或有费用上限的临时云 GPU 验证资源假设，但不进行长期硬件采购；只有稳定问题成立并出现可归因的资源阻塞后，才比较长期采购方案。

## 当前本地基线

以下为 2026-08-02 对当前宿主机的只读检查结果，分类为 `Fact`。它只证明设备清单，不证明任何被选 Issue 路径已经可用：

| 项目 | 当前值 | 调查含义 |
|---|---|---|
| 设备 | TR 911K 笔记本 | 当前零新增采购成本的开发基线 |
| CPU | Intel Core i7-7700HQ，4 核 8 线程，支持 AVX2 | 可进行代码检查、构建和小规模 CPU 路径验证 |
| GPU | NVIDIA GeForce GTX 1060，6 GiB，Compute Capability 6.1 | 可调查小模型或量化路径，但现代 CUDA/TensorRT 支持需要单独核验 |
| 内存 | 32 GiB DDR4-2400 | 可支持开发、转换和有限规模本地实验 |
| 存储 | 128 GB SSD + 1 TB HDD | SSD 空间紧张；模型、缓存和日志可放 HDD，但 I/O 测量需排除盘速影响 |
| 当前系统 | Windows 11 宿主机 + Ubuntu 24.04 WSL | 当前会话无法访问 GPU，不等同于宿主机没有 GPU |

仓库当前未保存这次设备检查的原始命令输出。相关规格进入正式资源决策前，必须重新执行只读检查并保存原始结果。

后续计划切换到原生 Ubuntu 桌面版。Ubuntu 24.04 LTS 是当前首个兼容性验证候选，不是冻结决策；最终版本必须同时满足 GTX 1060 驱动、CUDA、被选 Issue 路径及相关实现/Runtime 和必要开发工具的兼容性。原生系统安装完成前，GPU 可用性仍为 `Unknown`。

GTX 1060 属于 Pascal 架构。它不能被默认视为当前 TensorRT 路径的有效基线；实际调查必须固定 CUDA、驱动和 Runtime 版本，并区分“旧版本仍可运行”和“当前上游仍受支持”。

## 已确认的资源策略

1. **本地优先**：先使用当前设备完成能够执行的问题调查；
2. **优先级而非真实性**：当前设备不能复现只影响调查顺序，不证明问题不存在；
3. **分级门禁**：可执行复现暴露资源阻塞后，可以启用临时验证资源；只有稳定问题和长期需求均成立后，才定义采购规格；
4. **临时验证优先**：在长期采购前，优先使用有费用上限的云 GPU 或借用设备验证资源假设；
5. **不以采购代替问题发现**：新硬件使路径可运行，不自动证明问题值得独立立项。

## 可用资源选项

| 选项 | 当前角色 | 启用条件 |
|---|---|---|
| 当前设备 | 默认开发与调查环境 | 能执行被选 Issue 的全部或部分复现 |
| 借用设备或有限额云 GPU | 临时验证显存、GPU 架构或软件兼容性假设 | 已有固定复现、明确资源假设和费用/时间上限 |
| Jetson | ARM64、JetPack、功耗或设备 I/O 的条件性验证资源 | 被选 Issue 明确涉及这些平台属性 |
| x86_64 整机升级 | 稳定问题成立后的长期本地开发和较大资源负载 | 临时资源已确认长期需求，且总体成本优于继续租用 |

以上选项不是生产平台候选排序。无法获得某种资源时，候选 Issue 可以进入 `Backlog`，不能因此标记为虚假或已解决。

## 升级或采购门禁

提交任何具体采购建议前必须具备：

- 已成立的稳定问题及其直接影响；
- 固定 Issue、受影响版本、输入、环境和复现命令；
- 可重复或有充分证据支持的资源阻塞；
- 与问题相关的显存、系统内存、延迟、磁盘、架构或兼容性指标；
- 对配置错误、合理替代路径和临时资源的检查；
- 云 GPU 或借用设备上的一次对照，或说明无法对照的原因；
- Jetson、整机升级、继续使用本机和云 GPU 的成本—证据收益比较；
- 采购后能够判定“阻塞已解除”的验收命令和指标。

若没有满足上述条件，结论保持为 `Insufficient evidence`，不确认具体设备。

## 资料

- [JetPack SDK](https://developer.nvidia.com/embedded/jetpack/downloads)
- [Jetson Orin Nano Super Developer Kit](https://www.nvidia.com/en-us/autonomous-machines/embedded-systems/jetson-orin/nano-super-developer-kit/)
- [Jetson Orin](https://www.nvidia.com/en-us/autonomous-machines/embedded-systems/jetson-orin/)
- [Jetson Thor](https://www.nvidia.com/en-gb/autonomous-machines/embedded-systems/jetson-thor/)
- [GR00T 硬件建议](https://github.com/NVIDIA/Isaac-GR00T/blob/main/getting_started/hardware_recommendation.md)
- [TensorRT 支持矩阵](https://docs.nvidia.com/deeplearning/tensorrt/latest/getting-started/support-matrix.html)
- [CUDA Toolkit 12.9 Release Notes](https://docs.nvidia.com/cuda/archive/12.9.0/cuda-toolkit-release-notes/index.html)
- [Torch-TensorRT Releases](https://github.com/pytorch/TensorRT/releases)
