# VLA 推理部署与优化：文档导航

> 当前阶段以仓库入口的[状态摘要](../README.md)为准。正式配置、基线结果、profile 分析和下一步分别见 [`CONFIGURATION.md`](../experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md)、[`RESULTS.md`](../experiments/a10-cuda-smolvla-20260831-r1/RESULTS.md)、[`PROFILE-ANALYSIS.md`](../experiments/a10-cuda-smolvla-20260831-r1/PROFILE-ANALYSIS.md) 和[执行计划](plans/phase-0.md)；本页只维护文档导航以及项目统一的内容分类、证据强度和测量规则，不重复维护阶段结论。

## 可选入门


- [推理路径与输出正确性](concepts/inference-path-and-correctness.md)：VLA 推理路径、动作生成与正确性层级；
- [可复现实验、测量与 profiling](concepts/reproducible-measurement-and-profiling.md)：基线、性能测量、profile 和优化验证。

## 阅读顺序

1. [仓库入口](../README.md)：项目状态、主交付和文档入口；
2. [项目定位](project/positioning.md)：项目动机、能力声明和对外叙述边界；
3. [部署与性能优化范围](project/scope.md)：唯一的两阶段范围、配置字段和交付要求；
4. [部署基线与优化计划](plans/phase-0.md)：当前进度与下一步；
5. [实验记录与工件管理](../experiments/README.md)：仓库职责、证据目录和大工件保存规则；
6. [路径选择与辅助证据记录](research/issue-candidates.md)：候选的决策准则、依据及回退规则；
7. [部署平台与资源策略](research/deployment-platforms.md)：设备筛选条件、资源决策及历史环境的适用边界。

## 历史原始记录（按需）

- [本地设备只读核验原始输出](research/environment-check-2026-08-09.txt)：2026-08-09 的本机检查命令与输出。

## 文档内容类型

下表用于区分一段内容承担的职责，不表示证据强弱；具体实验结论必须在其权威记录中同时说明证据来源和适用范围。

| 分类 | 含义 |
|---|---|
| `Fact` | 有明确来源，并标注证据等级、日期和适用范围 |
| `Motivation` | 项目发起人的能力目标，不是实验结果 |
| `Constraint` | 时间、资源、场景和实验边界 |
| `Configuration` | 正式性能比较前冻结的非实现条件；完整定义见项目范围 |
| `Measurement` | 固定配置下保存原始结果和方法的本地测量 |
| `Hypothesis` | 等待 profile、代码检查或实验验证的判断 |
| `Decision` | 路径选择、主要瓶颈或优化取舍结论 |

## 证据强度

下表是项目使用证据时的唯一等级定义，与上节“文档内容类型”是两个独立维度。

| 等级 | 含义 | 可以支持的结论 |
|---|---|---|
| `Official claim` | 官方仓库、文档或论文明确声明 | 候选路径或公开限制，不代表本地可用 |
| `Code inspected` | 在固定 commit 和文件范围内检查代码 | 说明已检查实现，不代表整条路径已运行 |
| `Locally reproduced` | 固定输入和受控环境下重复观察到预期行为 | 该行为在声明环境中可重复 |
| `Locally measured` | 保存原始样本、日志、配置和测量方法 | 该配置下的定量结果 |
| `Profiled` | 保存分析工具、版本、采集范围和原始工件 | 已观测的热点、等待关系或硬件计数器，不自动证明优化根因 |
| `Not run` | 尚未执行本地运行 | 只能保留为候选，不能形成任何本地行为结论 |
| `Unknown` | 证据不足或适用条件未确认 | 不得表达为支持或缺失 |

`Locally` 表示本项目控制输入、执行和证据，不限定物理位置。单次失败首先分类为配置、环境、资产、实现或资源问题，不能直接包装为模型或 Runtime 缺陷。

## 测量与判定原则

- 指标和正确性标准由选定场景推导，并在查看优化结果前冻结；
- 保存原始日志、逐次测量、profile 工件、配置和汇总方法；
- 性能报告必须明确端到端边界、预热、重复次数和异常样本处理；
- profile 运行受到 trace 或 replay 扰动，除非协议另有验证，不作为正式性能结果；
- 优化后必须在同一冻结配置下执行正确性回归和性能比较；
- 证据不足只能得出 `Unknown` 或 `Insufficient evidence`，不能判定优化成功；
- 外部 Issue、公开 benchmark 和替代方案只用于选择路径或解释限制，不能替代本地实验。

## 维护规则

1. 每项会随阶段变化的规范性规则或详细事实只有一个权威维护位置；导航页、入口页可提供简短摘要，但不得复制完整定义或独立更新结论；
2. 官方声明、代码检查、smoke test、受控测量、profile 与 `Unknown` 必须区分；
3. 正式性能实验前固定实验配置；关键配置变化时创建新的 `Configuration ID`；
4. 指标和正确性标准由选定场景推导，并在查看优化结果前固定；
5. Issue、公开 benchmark 和替代方案仅作为辅助证据，不能替代本地基线与前后验证；
6. 不适用字段标记 `N/A`，不能为了填满模板扩大范围；
7. 不因单次成功、单次失败或性能提升外推为所有模型、硬件或部署路径均适用；
8. 未经 profile 证据支持的架构扩展、通用框架和生产化设计不进入活跃文档树。
9. 实验工件的仓库归属、目录和大文件保存方式以[实验记录与工件管理](../experiments/README.md)为准。
