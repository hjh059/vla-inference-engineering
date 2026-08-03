# Architecture Decision Records

> 当前阶段：Problem Discovery。问题成立前不接受最终项目架构 ADR；原有架构选择已重新分类为 Draft 候选方案。

ADR 只记录已经提出或接受、并且会长期约束实现的架构选择。候选调研、问题假设、执行计划、测试清单和易变化的版本号不属于 Accepted ADR。

## 状态

- `Draft`：候选设计或内容尚未完整，不作为实施依据；
- `Proposed`：问题已经成立，决策提案已经形成，等待确认或实测；
- `Accepted`：问题、约束和证据已经支持该决策，实施必须遵循；
- `Superseded`：曾经采用，现已被后续 ADR 替代；
- `Rejected`：经过评估后不采用。

Problem Discovery 中的 `Hypothesis` 和 `Candidate Solution` 是内容分类，不是 Accepted 决策。它们优先保存在调研、计划或 Draft ADR 中。

## 现行记录

| ADR | 状态 | 当前分类 | 重新进入评审所需证据 |
|---|---|---|---|
| [0010](0010-runtime-concurrency-lifecycle.md) | Draft | Candidate Solution：有界推理、epoch/generation 和过期结果 | 固定负载下的并发、阻塞或 stale-result 复现及 SLO |
| [0012](0012-safety-process-boundary.md) | Draft | Candidate Solution：独立安全进程和 Validated Action 边界 | 目标 Runtime/控制路径的故障与影响证据 |
| [0013](0013-build-system.md) | Draft | Conditional Constraint：CMake、ament_cmake 和 colcon | 已成立问题的实现边界确实需要自建 C++/ROS2 工程 |
| [0014](0014-acceptance-principles.md) | Proposed | Methodology：正确性、统计和证据原则；具体数字仍是 Hypothesis | 问题、任务、基线和影响确定后注册最终指标 |

当前没有 Accepted 架构 ADR。

## 下一项预期决策

Problem Discovery 不预设下一篇 ADR。只有退出结论为 `Continue` 时，才根据已验证问题决定是否需要：

- 问题与最小解决边界 ADR；
- Runtime、并发或故障边界 ADR；
- 构建和集成 ADR；
- 最终验收与生产基线 ADR。

不能直接把当前候选架构恢复为 Accepted，也不能因为完成模型、后端或硬件选型而跳过问题证据。

## 内容归属

| 内容 | 唯一维护位置 |
|---|---|
| 项目动机、候选用户和场景 | `docs/project/positioning.md` |
| Problem Discovery 步骤与退出条件 | `docs/plans/phase-0.md` |
| Runtime、模型、后端和平台事实 | `docs/research/` |
| 候选推理并发与状态失效 | Draft ADR-0010 |
| 候选安全与控制故障边界 | Draft ADR-0012 |
| 候选构建系统 | Draft ADR-0013 |
| 通用测量方法和候选门禁 | ADR-0014 |
| 测试维度和工具候选 | `docs/quality/testing.md` |
| 候选制品和发布边界 | `docs/delivery/artifacts.md` |
| 实际版本和复现证据 | 实验执行后产生的锁文件、manifest 和原始报告 |

## 维护规则

1. ADR 必须引用已经成立的问题、约束或实测证据；
2. 发现问题前的方案思考只能是 Draft/Candidate Solution；
3. 每篇 ADR 只记录一个可以明确表述的决策；
4. 实验结果通过报告 ID 和制品哈希引用，不复制原始报告；
5. 未执行指标写“未执行”，不得表达为实测结果；
6. 普通代码调整、候选列表变化和日常任务不新增 ADR。

本仓库尚无提交和实现，因此本次状态纠正属于建立初始可信基线，而不是改写已经实施的历史决策。
