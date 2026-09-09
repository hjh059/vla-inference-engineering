# VLA 推理部署与优化

面向求职的 VLA 推理部署与优化项目（VLA Inference Deployment & Optimization）。完成可行性选择后，项目将在一组冻结的模型、设备和任务配置上跑通现有部署路径，建立可复现的正确性与性能基线，用 profiling 定位一个主要瓶颈，完成有因果依据的优化，并以相同条件下的前后指标验证结果。

> 当前状态：正式配置 [`a10-cuda-smolvla-20260831-r1`](experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md) 已完成正确性与性能基线、隔离稳态 Nsight Systems profile、两个代表性 kernel 的 Nsight Compute 采集，以及首个最小优化的同条件正确性回归和 30 样本性能比较。原生 BF16 GEMM 路径使客户端端到端均值从 70.751 ms 降至 57.476 ms、服务端 inference 均值从 48.735 ms 降至 35.707 ms；完整条件、风险和证据缺口见 [OPTIMIZATION-01.md](experiments/a10-cuda-smolvla-20260831-r1/OPTIMIZATION-01.md)。优化尚未形成可提交 revision，且优化后的大型 profile 工件尚未归档，因此项目尚未完成最终交付。
>
> “固定”指正式基线开始前冻结一组实验配置，不表示在项目启动时预设最终 Runtime、通用架构或生产控制链路。

## 项目动机

项目的主交付是一份可运行、可复现、可解释的 VLA 推理部署与优化工程证据。它面向 C++ AI 推理、GPU 性能工程和机器人模型部署岗位，展示部署、正确性分析、GPU profiling、性能优化和工程取舍能力。

C++、Python、CUDA/TensorRT、Processor、协议和控制端都是候选工具，只在选定路径和已验证瓶颈需要时使用；项目不以新建通用 Runtime、完整机器人闭环或复杂架构作为目标。

## 项目目标

项目分为可行性选择与基线优化两个阶段：先以有限 smoke test 选择一条可运行的模型—设备—任务路径，再在冻结配置下完成端到端基线、profiling、单一主瓶颈优化及前后验证。规范性的两阶段范围、冻结字段和交付要求只在[部署与性能优化范围](docs/project/scope.md)维护。

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

正式完成标准和必须产出见[项目范围的“必须产出”](docs/project/scope.md#必须产出)；本页只维护仓库入口、状态摘要和导航。

## 文档

- [文档导航](docs/README.md)
- [推理路径与输出正确性（可选入门）](docs/concepts/inference-path-and-correctness.md)
- [可复现实验、测量与 profiling（可选入门）](docs/concepts/reproducible-measurement-and-profiling.md)
- [项目定位](docs/project/positioning.md)
- [部署与性能优化范围](docs/project/scope.md)
- [部署基线与优化计划](docs/plans/phase-0.md)
- [实验记录与工件管理](experiments/README.md)
- [优化 01：原生 BF16 GEMM](experiments/a10-cuda-smolvla-20260831-r1/OPTIMIZATION-01.md)
- [路径选择与辅助证据记录](docs/research/issue-candidates.md)
- [部署平台与资源策略](docs/research/deployment-platforms.md)

## 当前限制

- 项目工件直接保存在本仓库，并以 SHA-256 索引校验；模型本体、构建产物和项目外临时证据不归档。2026-08-09 的历史 profiling smoke 报告尚未恢复，不构成当前模型结论；
- 模型与运行时代码的许可证文字差异仍需在发布或分发前按冻结 revision 复核；
- 当前性能结论仅限冻结配置下的单请求稳态基线与优化 01 的同条件比较；优化输出并非与基线跨实现数值等价。动作语义、LIBERO 任务成功率、跨设备或跨输入泛化、可靠性、机器人控制和生产可用性结论均未成立；优化版 source commit 与大型 profile 工件也尚待归档。
