# VLA 推理部署与优化

面向求职的 VLA 推理部署与优化项目（VLA Inference Deployment & Optimization）。完成可行性选择后，项目将在一组冻结的模型、设备和任务配置上跑通现有部署路径，建立可复现的正确性与性能基线，用 profiling 定位一个主要瓶颈，完成有因果依据的优化，并以相同条件下的前后指标验证结果。

> 当前状态：正式配置 [`a10-cuda-smolvla-20260831-r1`](experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md) 已完成固定-noise 的正确性与 30 样本稳态性能基线；结果见 [RESULTS.md](experiments/a10-cuda-smolvla-20260831-r1/RESULTS.md)。已归档含 NVTX 请求范围的隔离稳态 Nsight Systems profile，确认 `vla::predict()` 内 GPU kernel 执行是主要瓶颈层级；尚未形成有 Nsight Compute 硬件计数器支持的细粒度根因或优化结论。
>
> “固定”指正式基线开始前冻结一组实验配置，不表示在项目启动时预设最终 Runtime、通用架构或生产控制链路。

## 项目动机

项目的主交付是一份可运行、可复现、可解释的 VLA 推理部署与优化工程证据。它面向 C++ AI 推理、GPU 性能工程和机器人模型部署岗位，展示部署、正确性分析、GPU profiling、性能优化和工程取舍能力。

C++、Python、CUDA/TensorRT、Processor、协议和控制端都是候选工具，只在选定路径和已验证瓶颈需要时使用；项目不以新建通用 Runtime、完整机器人闭环或复杂架构作为目标。

## 项目目标

项目分为两个阶段：

1. **可行性选择**：在资源与时间边界内核对现有路径，以有限的 smoke test 选择一条可运行的模型—设备—任务路径；这一阶段不形成正式性能结论。
2. **基线与优化**：按[正式基线配置](docs/project/scope.md#正式基线配置)冻结可比性条件，完成端到端部署、基线测量、profiling、单一主瓶颈优化以及前后验证。

正式交付必须说明：

- 选择该场景、主指标和现有部署路径的依据；
- 固定环境、输入、运行命令和正确性标准；
- 优化前后的原始测量、汇总方法和 profile 工件；
- 主瓶颈、优化机制及其因果证据；
- 改进范围、未验证项和适用边界。

## 实施闭环

```text
候选场景与可用资源
        │
        ▼
核对官方/现有部署路径并完成有限 smoke test
        │
        ▼
选择并冻结一组基线实验配置
        │
        ▼
跑通端到端路径，建立正确性与性能基线
        │
        ▼
采集 profile，定位一个主要瓶颈
        │
        ▼
实施与该瓶颈对应的最小优化
        │
        ▼
以相同配置验证正确性和前后性能
        │
        ▼
交付可复现结果、取舍与限制
```

外部资料核查可辅助说明场景约束、指标、瓶颈和部署路径选择，但不替代本项目的基线、profile 与前后验证。

## 证据规则

官方文档、代码检查、smoke test、受控测量和 profile 结论必须区分。性能结论只适用于固定的模型、版本、设备、输入和测量方法；没有原始结果、正确性验证或足够证据时，结论保持为 `Unknown` 或 `Insufficient evidence`。

外部 Issue、公开 benchmark 和替代方案可作为路径选择或解释限制的参考，不能替代本项目的基线、profile 和前后验证。

## 完成标准

- 现有路径能在固定环境与输入下重复运行；
- 正确性基线明确，优化后没有回归；
- 性能指标、测量方法、原始结果和汇总方法可复现；
- profile 支持对一个主要瓶颈的判断；
- 优化与该瓶颈具有可说明的因果关系；
- 优化前后在同一实验配置下测量，并记录改进与限制。

## 文档

- [文档导航](docs/README.md)
- [项目定位](docs/project/positioning.md)
- [部署与性能优化范围](docs/project/scope.md)
- [部署基线与优化计划](docs/plans/phase-0.md)
- [实验记录与工件管理](experiments/README.md)
- [路径选择与辅助证据记录](docs/research/issue-candidates.md)
- [Runtime 与后端历史参考](docs/research/backend-candidates.md)
- [部署平台与资源策略](docs/research/deployment-platforms.md)

## 当前限制

- 新正式配置的制品、输入、随机性、正确性标准、预热和测量方法已冻结；正确性、30 次性能基线与隔离稳态 Nsight Systems 原始工件已归档，但尚无成功的模型 Nsight Compute 报告或任何优化前后结果；
- 项目工件直接保存在本仓库，并以 SHA-256 索引校验；模型本体、构建产物和项目外临时证据不归档。2026-08-09 的历史 profiling smoke 报告尚未恢复，不构成当前模型结论；
- 模型与运行时代码的许可证文字差异仍需在发布或分发前按冻结 revision 复核；
- 动作语义、LIBERO 任务成功率以及任何正式性能、可靠性、机器人控制或生产可用性结论均未成立。
