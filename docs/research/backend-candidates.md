# VLA Runtime 与推理后端历史参考

> 状态：按需外部参考，不是候选后端清单、兼容性结论或生产决策。
>
> 资料摘录日期：2026-08-02。当时尚未选定部署路径；当前 C-01 已按固定 revision 核对、通过重复 smoke 并选为正式基线路径，选择证据见[路径选择记录](issue-candidates.md#c-01smolvlalibero-gguf--vlacpp)。

本文只保留已收集的外部入口。选定模型、设备和任务后，只核对与该路径直接相关的运行说明、许可证、输入/输出边界和兼容性；正式性能、正确性和瓶颈结论只来自本项目的固定配置实验。

| 路径 | 外部来源 |
|---|---|
| `vla.cpp` | [官方仓库](https://github.com/VinRobotics/vla.cpp)、[架构说明](https://github.com/VinRobotics/vla.cpp/blob/main/docs/ARCHITECTURE.md)、[项目页面](https://fai-modelopt-tech.github.io/vla-cpp.github.io/) |
| `Embodied.cpp` | [官方仓库](https://github.com/SEU-PAISys/Embodied.cpp)、[论文](https://arxiv.org/abs/2607.02501)、[模型制品](https://huggingface.co/SEU-PAISys/Embodied.cpp) |
| NVIDIA Isaac GR00T | [官方仓库](https://github.com/NVIDIA/Isaac-GR00T)、[部署与推理指南](https://github.com/NVIDIA/Isaac-GR00T/blob/main/scripts/deployment/README.md) |

未核对到的能力、限制或兼容性记为 `Unknown`，不得从该路径的 README、路线图或公开 benchmark 推导本地结论。选定路径后的具体事实与 `Configuration ID` 记录按[部署与性能优化范围](../project/scope.md#正式基线配置)维护。
