# ADR-0014：Problem Discovery 的测量与验收原则

- 状态：Proposed（方法论可用于调查；具体任务和数值门禁均为 Hypothesis）
- 分类：Methodology + Example Thresholds
- 日期：2026-07-27

> 本文中的成功率、延迟、频率、长稳和优化比例不是当前有效门禁。只有真实问题、用户影响、任务和基线固定后，才能预注册对应指标。

## 背景

如果 Problem Discovery 得出 `Continue` 且最小解法涉及 VLA 部署，其成功不能由单一 FPS、层延迟或张量相似度证明。最终验收可能需要同时覆盖：

1. 模型与跨后端正确性；
2. 推理性能、资源和能效；
3. 机器人任务效果、安全和故障恢复。

三个测量层级是候选模块，不要求每个 Issue 全部覆盖。只选择与问题声明、根因、修复和声称影响直接相关的模块。

## 测量原则

- 先冻结任务、输入、版本、硬件和基线，再 Profiling；
- 保存原始样本和汇总，不只报告平均数；
- 每项优化只改变一个主要变量；
- 正确性、任务效果、安全和可靠性拥有否决权；
- 未执行指标明确写“未执行”；
- 复杂优化不达保留门槛时回滚；
- 问题成立后的数值门禁只在本文或未来的验收记录中维护，其他 ADR 只引用。

## 实验可复现信息

每次实验通用必填：

- 实验 ID、假设和判定规则；
- 仓库、受影响版本或 Git commit；
- 固定输入、环境、复现命令和候选改动；
- 原始日志、失败样本和结果位置。

以下字段按问题适用性记录；不适用时标记 `N/A` 并说明原因：

- 模型、数据、Tokenizer、Processor 和制品哈希；
- OS、driver、CUDA、后端、ROS2 和编译器；
- GPU、功耗模式、时钟、散热和环境温度；
- 输入 shape、instruction、robot state、horizon 和 seed/noise；
- warmup、样本数、并发、控制频率和 background load；
- 遥测和 Profiler 原始报告。

测量延迟时报告 p50、p95、p99、min、max 和样本数；测量功耗时区分 idle、steady 和 peak；测量 `J/action` 时只按最终被控制器接受的有效动作计算，并同时报告 J/chunk、horizon、拒绝和过期数量。

## 第一层：模型正确性

### Processor

- Token ID 完全一致；
- 图像 resize、crop、color 和 normalize；
- robot state 顺序、单位和归一化；
- action 反归一化、关节顺序和坐标系。

### 模块

- 视觉编码器；
- 语言或多模态主干；
- state encoder；
- KV、视觉和上下文 Cache；
- Action Head 的关键中间态。

### 动作与任务

- 连续动作报告逐维 MAE、RMSE、max error 和越阈次数；
- 离散动作报告逐 Token 或逐动作一致率；
- 固定 seed/noise 只用于定位数值漂移，不要求跨后端随机 kernel 逐位一致；
- 闭环报告成功次数、总次数和置信区间；
- 后端和精度候选与参考后端进行配对比较。

动作误差阈值由最终 action space 的物理单位和控制敏感性预注册。

## 旧架构留存的仿真阈值示例

以下数字来自旧架构的仿真筛选提案，不是当前 Phase 0 的任务、默认路线或有效门禁：

- 全部注册 episode 的成功率，其 95% 单侧 Wilson 置信下界不低于 70%；
- 每个必需 task suite 的成功率，其 95% 单侧 Wilson 置信下界不低于 50%；
- 碰撞、越界或非法动作次数为 0；
- 不使用总体平均值掩盖必需 suite 失败。

这些数字不是行业标准。只有被选问题确实需要相应仿真任务时，任务拥有者才根据问题影响重新注册或替换阈值，并在看到候选结果前固定；episode 数通过预先的功效或精度分析确定，样本不足时结论是“证据不足”。

真实机器人门禁在目标硬件确定后注册，包括绝对成功率、分任务下限、最大完成时间、人工干预率和零危险动作要求。

## 候选后端与量化非劣

候选与参考实现使用相同 episode 和 seed 配对评测。初始提案为：

- 预注册方法所得 95% 单侧置信区间下界不低于 -3 个百分点；
- 不增加碰撞、越界或非法动作；
- p95 Chunk 延迟或 J/action 至少改善 15%。

实验前必须按预期成功率、非劣界限和检验功效计算样本量。证据不足不能判定通过。

## 第二层：推理与系统性能

分段测量：

- Tokenizer、图像和状态预处理；
- 图像编码、多模态主干和 Action Head；
- 首动作和完整 ProposedActionChunk；
- SafetySupervisor、ValidatedActionChunk 和端到端。

系统测量：

- p50、p95、p99 和 deadline miss；
- CPU、GPU 和共享内存峰值；
- 利用率、功耗、温度、降频和 J/action；
- coalesced、cancelled、stale 和 expired 数量；
- 冷启动、模型加载和 Engine 反序列化。

以下是旧 Phase 0 留存的示例性能提案，不是 Problem Discovery 的通过标准：

- 有效 Chunk 稳态发布率不低于 10 Hz；
- 单请求完整 Chunk p95 不高于 100 ms；
- 新指令首个有效动作 p95 不高于 200 ms；
- deadline miss、stale result 和 expired Chunk 比例分别不高于 1%；
- 不使用 swap；
- 持续负载不因热降频失守；
- 24 小时无持续资源增长。

任务、控制频率、horizon 和延迟预算确定后，必须在运行候选前确认或替换这些数字；不能因候选性能不足而事后降低。

## 第三层：机器人系统

- 任务成功率、置信区间和完成时间；
- 重试、人工干预和任务放弃；
- 动作 timeout、expiry 和安全拒绝；
- 碰撞、越界和非法动作；
- 输入、时钟、模型、进程、ROS2、控制器和硬件异常；
- 恢复时间、最终状态、升级失败和回滚；
- 仿真与真机结果分别报告。

只有目标真机通过声明范围内的正确性、安全、恢复和长稳门禁，才能声明完整机器人 VLA 案例。

## 优化触发

多 Stream、CUDA Graph、自定义 Plugin、pinned memory、零拷贝和无锁队列只在系统时间线证明其对应瓶颈时评估。

默认保留条件为 p95 或 J/action 至少改善 10%，且不造成正确性、任务、安全、温度和可靠性退化。复杂 Plugin、Graph 或多 Stream 应设置更高专项门槛并新增 ADR。

## 结果模板

模板字段同样按问题适用；不适用项填写 `N/A`，不得为了填满模板扩展实验范围。

```text
Experiment ID:
Hypothesis:
Fixed environment:
Baseline artifact:
Candidate artifact:
Correctness/task result:
p50/p95/p99:
Effective chunk rate/deadline miss:
Memory/utilization:
Power/temperature/J-action:
Fault/soak result:
Raw report path:
Decision: Keep / Revert / Insufficient evidence
Known limitation:
```

## 后果

Problem Discovery 期间只使用本文的可复现、原始数据、配对比较和“证据不足”原则。具体阈值必须由已复现问题的影响推导；若退出结论为 `Continue`，再与问题陈述和实验报告一并冻结。

测试执行层次见[测试策略](../quality/testing.md)。
