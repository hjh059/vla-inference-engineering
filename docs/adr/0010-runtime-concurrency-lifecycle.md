# ADR-0010：候选运行时并发与状态失效

- 状态：Draft
- 分类：Candidate Solution / Hypothesis
- 日期：2026-07-27

> 本 ADR 不作为当前实施依据。重新评审需要先在固定 Runtime、负载、控制频率和 SLO 下复现并发、阻塞、资源失控或 stale-result 问题。

## 背景

VLA 推理可能包含图像、语言、机器人状态、Cache 和生成式 Action Head。此前设计假设同步 ROS2 回调难以表达背压、取消、过期结果和上下文失效，而通用线程池会扩大资源和状态空间；这些影响尚未在目标工作流中复现。

如果 Problem Discovery 证明旧结果可能被消费或资源无界增长造成明确影响，运行时可能需要优先保证输入新鲜度、有界资源和旧结果不可执行，而不是最大化请求吞吐。

## 候选

### ROS2 回调内同步推理

实现简单，但可能阻塞 executor，长尾、deadline 和取消行为需要实测。此前判断为不优先；需由被选问题和测量重新比较。

### 多级线程池和无锁流水线

可能提高并行度，但也可能放大共享 Cache、过期工作和关闭路径的复杂度。当前没有 Profiling 证据支持；此前判断为不优先，需由被选问题和测量重新比较。

### 有界单执行槽 + 最新观测槽

同一时间只运行一个 VLA 请求，待处理区只保存最新完整 Observation。GPU 工作不能安全抢占时允许完成，但结果必须再次校验 epoch 和 generation。

## 候选方案

候选方案是“一个执行中请求 + 一个 latest pending Observation”的有界运行时：

```text
ROS2 inputs
    │
    ▼
ObservationAssembler
    │
    ▼
latest_pending[1]
    │
    ▼
InferenceSession
├── ModelProcessor
├── SelectedBackend
├── ContextCache
└── ActionDecoder
    │
    ▼
ProposedActionChunk
```

对应的候选安全与控制边界记录在 Draft [ADR-0012](0012-safety-process-boundary.md)中。只有 Problem Discovery 得出 `Continue` 后，才决定是否需要生产后端或生产基线 ADR。

## Observation 进入条件

进入推理的 Observation 必须具有：

- request、sequence、session 和 embodiment 身份；
- 图像、指令和机器人状态的时间与版本语义；
- 单调递增的 instruction revision；
- 明确的单位、坐标系、关节顺序和所有权。

ObservationAssembler 拒绝不完整、过旧、乱序或跨 epoch 输入。最终字段由版本化 IDL/Schema 定义，不在本 ADR 复制。

## Context 与 Cache

每个机器人 session 拥有 `ContextEpoch`。以下事件使旧上下文失效：

- instruction revision 变化；
- 模型制品或归一化统计变化；
- robot embodiment 变化；
- 控制器重新激活；
- 需要重新同步的模型、通信或控制故障。

Token、embedding、KV Cache、Action Head 状态和随机状态必须具有显式 owner、容量和 epoch，禁止使用全局隐藏可变 Cache。

## 背压、超时和取消

- 新完整 Observation 覆盖尚未执行的旧观测，并记录覆盖计数；
- 排队 deadline、推理 deadline 和 Chunk 有效期分别记录；
- CPU 取消在模块边界检查；
- GPU 工作完成后重新校验 epoch、generation 和 instruction revision；
- 过期结果只计入遥测，不发布；
- fatal CUDA 状态使当前 Chunk 失效，并由独立安全进程接管停止；
- 同一污染进程内不无限重试 CUDA fatal error。

外部重启次数、时间窗和回滚策略属于部署配置与验证计划，不在运行时 ADR 固定。

## Action Chunk 新鲜度

ProposedActionChunk 必须携带：

- Observation sequence；
- context epoch 和 generation；
- instruction revision；
- artifact 和 Schema 身份；
- `valid_from`、`expires_at`、action period 和 horizon；
- action space、单位、坐标系和关节顺序。

运行时只保证提议动作的身份和新鲜度，不声明动作已经安全。未经安全监督的动作不能进入 RobotAdapter。

## Chunk 切换原则

1. 只接受更大的 generation、相同 epoch 且未过期的候选；
2. 指令更新立即使旧 Chunk 失效；
3. 新 Chunk 生效后旧 Chunk 不可重新激活；
4. 输入中断或推理超时不能无限重复最后动作；
5. 恢复后先重新同步机器人状态，再生成新 Chunk；
6. 推理频率与控制频率解耦，控制线程不等待 VLA。

候选接续、拒绝和停止行为记录在 Draft ADR-0012 中；只有对应问题成立并选择 RobotAdapter 后，才形成最终契约。

## 生命周期

启动时依次校验 Schema、制品、黄金向量和运行资源；停止时：

1. 停止接收新 Observation；
2. 递增 epoch 并失效所有 Chunk；
3. 请求取消并等待有界时间；
4. 通知安全与控制侧停止当前命令；
5. 逆序销毁后端、CUDA、ROS2 和遥测资源。

## 预期后果与验证要求

该候选设计预计牺牲吞吐，换取有界资源和较清晰的状态空间。是否真的改善延迟、内存或 stale-result 行为必须与现有 Runtime 基线比较；没有问题复现和测量时，不实施单执行槽，也不预设需要新的调度框架。

验证用例统一维护在[测试策略](../quality/testing.md)，数值门禁统一维护在[验收原则](0014-acceptance-principles.md)。
