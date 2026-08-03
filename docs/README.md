# RoboEdge 文档导航

> 当前阶段：Problem Discovery。文档中的动机、事实、假设、候选方案和已接受决策必须明确区分。

## 初次了解

建议按以下顺序阅读：

1. [仓库入口](../README.md)：当前状态、项目动机和退出条件；
2. [项目定位](project/positioning.md)：动机、候选用户、工作流和调查约束；
3. [Problem Discovery 范围](project/scope.md)：本阶段做什么、不做什么；
4. [Problem Discovery 计划](plans/phase-0.md)：证据等级、执行顺序和退出决策；
5. [条件性路线图](project/roadmap.md)：问题成立前后的阶段边界。

## 当前调查

- [候选 Issue 筛选表](research/issue-candidates.md)
- [VLA Runtime 与推理后端生态](research/backend-candidates.md)
- [模型候选资料](research/model-candidates.md)
- [部署平台候选资料](research/deployment-platforms.md)

Issue report 是候选问题来源，不等于问题已验证。能力调研只记录 `Official claim`、`Code inspected`、`Locally reproduced`、`Locally measured` 或 `Unknown`，不表达生产选型。

## 声明分类

| 分类 | 含义 |
|---|---|
| `Fact` | 有明确来源或本地证据，并同时标注证据等级和适用范围 |
| `Motivation` | 项目发起人的求职或能力目标，不是用户问题 |
| `Constraint` | 时间、资源、安全或选题边界，不证明某个方案必要 |
| `Hypothesis` | 需要代码检查或实验验证的问题猜想 |
| `Candidate Solution` | 可能解决已发现问题的设计，不是实施依据 |
| `Accepted Decision` | 问题、约束和证据已经支持，并由 Accepted ADR 记录 |

一项声明可以同时具有来源和分类，例如 `Official claim + Fact` 或 `Unknown + Hypothesis`。当前没有 `Accepted Decision` 级别的项目架构。

## 候选设计与方法

- [候选架构](architecture/overview.md)：保留的 C++ 推理与安全控制设计，不是实施依据；
- [ADR 索引](adr/README.md)：Draft 候选方案和测量方法；
- [候选测试与证据目录](quality/testing.md)；
- [候选制品与交付边界](delivery/artifacts.md)。

## 文档类型

| 类型 | 目录 | 当前作用 |
|---|---|---|
| 项目定义 | `project/` | 区分动机、调查范围和条件性未来阶段 |
| 阶段计划 | `plans/` | 执行 Problem Discovery 并形成退出结论 |
| 候选调研 | `research/` | 保存按证据等级标记的生态事实和 Unknown |
| 候选架构 | `architecture/` | 保存设计假设，不作为当前实施依据 |
| 决策记录 | `adr/` | 保存 Draft 候选方案和 Proposed 方法论 |
| 质量与交付 | `quality/`、`delivery/` | 保存可能复用的验证维度，暂不实施生产体系 |

## 维护规则

1. 根 README 只说明当前阶段、动机、调查闭环和退出条件；
2. 同一事实只维护一处，其他文档使用链接引用；
3. 官方文档声明不能写成本地复现结果；
4. “未找到”不能自动写成“不支持”；
5. 问题假设和候选解决方案不得标为 Accepted；
6. 未执行指标明确标记“未执行”或“示例”，不得表达为门禁或实测结果；
7. Problem Discovery 得出 `Continue` 前，不创建最终项目架构 ADR；
8. 没有有价值缺口时，允许上游贡献、停止或更换主题。
