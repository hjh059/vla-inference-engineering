# 部署平台与资源策略

> 状态：2026-08-31 当前主机的 A10/CUDA 配置已冻结为 [`a10-cuda-smolvla-20260831-r1`](../../experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md)。本文记录正式候选设备筛选条件与当前正式环境入口，不是采购计划。

## 当前正式目标环境

当前唯一正式比较配置是阿里云 A10/CUDA 的 [`a10-cuda-smolvla-20260831-r1`](../../experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md)。设备、系统、驱动、Toolkit、工具版本和构建条件只在该配置及其[环境记录](../../experiments/a10-cuda-smolvla-20260831-r1/raw/environment-20260831.md)维护；profile 工件状态和模型结论只在[PROFILE-ANALYSIS.md](../../experiments/a10-cuda-smolvla-20260831-r1/PROFILE-ANALYSIS.md)维护。

2026-08-09 的项目负责人环境检查曾确认阿里云 A10 可运行最小 CUDA 程序，并能使用 Nsight Systems、Nsight Compute 与硬件计数器；这只支持当时的环境选择，不构成当前模型的性能、profile 或瓶颈结论。

## 正式候选设备筛选条件

当前正式候选设备采用以下筛选条件：

- NVIDIA Ampere 或更新架构；
- 单卡显存不少于 24 GB；
- 能运行 Nsight Systems 和 Nsight Compute；
- 能读取 GPU hardware performance counters 并导出原始 profile 工件。

16 GB 只作为部分模型可能达到的最低推理线，不作为当前项目的默认筛选线；具体显存需求仍须由选定模型/checkpoint 的固定版本验证。
