# RoboEdge 文档导航

> 当前阶段：Problem Discovery。活跃文档只维护当前使用的规则、事实和调查记录。

## 阅读顺序

1. [仓库入口](../README.md)：当前状态、项目动机和退出条件；
2. [项目定位](project/positioning.md)：项目动机和能力声明边界；
3. [Problem Discovery 范围](project/scope.md)：当前做什么、不做什么；
4. [Problem Discovery 计划](plans/phase-0.md)：执行流程、证据规则和退出决策；
5. [候选 Issue 筛选表](research/issue-candidates.md)：不超过 10 个 Issue 的动态台账；
6. [Runtime 与后端生态事实](research/backend-candidates.md)：按需查询的外部项目事实；
7. [部署平台与资源策略](research/deployment-platforms.md)：当前硬件事实和资源门禁。

## 声明分类

| 分类 | 含义 |
|---|---|
| `Fact` | 有明确来源，并标注证据等级、日期和适用范围 |
| `Motivation` | 项目发起人的能力目标，不是用户问题 |
| `Constraint` | 时间、资源和调查边界，不证明问题不存在或某个方案必要 |
| `Issue report` | 真实用户问题的线索，不等于问题已经成立 |
| `Hypothesis` | 等待代码检查、复现或测量验证的判断 |
| `Decision` | 基于当前证据作出的 shortlist、上游贡献、停止或转向结论 |

## 维护规则

1. 同一规则或事实只维护一处，其他文档使用链接引用；
2. 官方声明、代码检查、受控复现、受控测量和 `Unknown` 必须区分；
3. “未找到”不能自动写成“不支持”，当前设备无法复现也不能证明问题不存在；
4. Issue 热度、技术趣味、易修复性或适合某种语言不能证明问题值得立项；
5. 模型、Runtime、仿真、硬件和实现语言只由被选 Issue 决定；
6. 不适用字段标记 `N/A`，不能为了填满模板扩大调查；
7. 指标必须由问题影响推导，证据不足不能判定成功；
8. 没有有价值缺口时，允许上游贡献、停止或更换主题；
9. 未被真实问题触发的架构、构建、发布和未来阶段设计不进入活跃文档树。
