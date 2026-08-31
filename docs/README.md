# VLA 推理部署与优化：文档导航

> 当前阶段：历史 C-01 smoke 已完成可行性选择；当前主机的正式配置 [`a10-cuda-smolvla-20260831-r1`](../experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md) 已完成正确性与性能基线，结果见 [RESULTS.md](../experiments/a10-cuda-smolvla-20260831-r1/RESULTS.md)。仍需隔离稳态 profile、主瓶颈确认与优化验证。活跃文档维护后续优化所需的规则、辅助资料和实验记录。

## 阅读顺序

1. [仓库入口](../README.md)：项目状态、主交付和完成标准；
2. [项目定位](project/positioning.md)：项目动机、能力声明和边界；
3. [部署与性能优化范围](project/scope.md)：两阶段范围、配置字段和产出；
4. [部署基线与优化计划](plans/phase-0.md)：执行流程、证据规则和完成标准；
5. [实验记录与工件管理](../experiments/README.md)：仓库职责、证据目录和大工件保存规则；
6. [路径选择与辅助证据记录](research/issue-candidates.md)：候选路径与选择依据；
7. [Runtime 与后端历史参考](research/backend-candidates.md)：选定路径时按需核对的外部来源；
8. [部署平台与资源策略](research/deployment-platforms.md)：本地设备边界、已验证的云端候选环境与正式环境冻结规则。

## 声明分类

| 分类 | 含义 |
|---|---|
| `Fact` | 有明确来源，并标注证据等级、日期和适用范围 |
| `Motivation` | 项目发起人的能力目标，不是实验结果 |
| `Constraint` | 时间、资源、场景和实验边界 |
| `Configuration` | 正式性能比较前冻结的非实现条件；完整定义见项目范围 |
| `Measurement` | 固定配置下保存原始结果和方法的本地测量 |
| `Hypothesis` | 等待 profile、代码检查或实验验证的判断 |
| `Decision` | 路径选择、主要瓶颈或优化取舍结论 |

## 维护规则

1. 同一规则或事实只维护一处，其他文档使用链接引用；
2. 官方声明、代码检查、smoke test、受控测量、profile 与 `Unknown` 必须区分；
3. 正式性能实验前固定实验配置；关键配置变化时创建新的 `Configuration ID`；
4. 指标和正确性标准由选定场景推导，并在查看优化结果前固定；
5. Issue、公开 benchmark 和替代方案仅作为辅助证据，不能替代本地基线与前后验证；
6. 不适用字段标记 `N/A`，不能为了填满模板扩大范围；
7. 不因单次成功、单次失败或性能提升外推为所有模型、硬件或部署路径均适用；
8. 未经 profile 证据支持的架构扩展、通用框架和生产化设计不进入活跃文档树。
9. 实验工件的仓库归属、目录和大文件保存方式以[实验记录与工件管理](../experiments/README.md)为准。
