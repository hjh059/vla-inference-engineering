# ADR-0012：候选安全进程与机器人控制边界

- 状态：Draft
- 分类：Candidate Solution / Safety Constraint Hypothesis
- 日期：2026-07-27

> 本 ADR 保留安全和故障包含思考，但不作为当前实施依据。独立进程、Validated Action 和 RobotAdapter 是否必要，必须由目标 Runtime、动作消费者和故障实验决定。

## 背景

如果后续问题涉及真实机器人，VLA 输出可能进入动作消费路径。模型进程可能包含 CUDA、动态内存和复杂推理状态；它是否会成为停止能力的唯一故障域尚未在目标方案中核查。

此前候选设计希望保证：

- 模型输出不能直接成为机器人命令；
- 推理进程崩溃时仍能拒绝过期动作并停止机器人；
- 安全检查作用于反归一化后的实际物理量；
- 仿真与真机使用同一内部动作边界。

## 候选

### 推理进程直接控制机器人

边界少，但模型、GPU、安全和控制可能共享故障域。此前判断为不优先；需根据被选控制路径、现有控制器能力和故障实验重新比较。

### 安全检查作为推理进程内模块

能够复用内存，但 CUDA fatal error 可能同时带走检查和停止能力。此前判断为不优先；需根据实际故障域和停止能力重新比较。

### 独立安全与控制进程

GPU `vla_runtime` 只提出动作，非 GPU `safety_control` 独立维护机器人状态、动作有效期和停止策略。此前将其选为方案；当前重新分类为待验证候选。

## 候选方案

```text
vla_runtime
    │
    ▼
ProposedActionChunk
    │ ROS2/DDS
    ▼
SafetySupervisor
├── reject ──► SafetyDecision
└── approve ─► ValidatedActionChunk
                    │
                    ▼
               RobotAdapter
                    │
                    ▼
           selected controller
```

候选方案包括：

1. `vla_runtime` 与 `safety_control` 分进程部署；
2. SafetySupervisor 与 RobotAdapter 位于非 GPU 的 `safety_control`；
3. RobotAdapter 只接受 ValidatedActionChunk；
4. 推理工作不进入 subscription callback 或硬实时控制线程；
5. 首版只实现一个控制路径，不在线切换；
6. 最终急停和硬限制仍由机器人安全控制系统承担。

## ROS2 边界

如果问题成立且最小解法选择 ROS2，候选系统可使用 ROS2 Lifecycle 表达配置、预检、激活、错误和清理。具体 distro、RMW 和包版本届时按证据决定。

QoS 不依赖默认值。接口必须明确：

- 相机的 sensor-data 语义、history 和 depth；
- 机器人状态的 reliability、deadline 和 liveliness；
- 指令的 session、revision、TTL 和可靠性；
- ProposedActionChunk 与 SafetyDecision 的可靠性和有效期；
- 诊断通道不能阻塞控制数据面。

共享内存、NITROS、自定义 executor 和同进程 composition 不是首版默认能力，只有兼容性和 Profiling 证明收益时才重新决策。

## SafetySupervisor

SafetySupervisor 在动作反归一化后至少检查：

- finite、shape、action type、单位、坐标系和关节顺序；
- Observation 与 Chunk age、sequence、epoch、generation 和 expiry；
- 关节 position、velocity、acceleration 和 jerk；
- 单周期变化和新旧 Chunk 接续；
- 末端工作空间与奇异区；
- 控制器 lifecycle、robot mode、保护停和急停状态；
- goal/path tolerance；
- 仅在存在可靠几何输入和独立失效定义时检查 self/world collision。

任一关键检查失败时：

- 拒绝整个 ProposedActionChunk；
- 产生带稳定原因码和追踪 ID 的 SafetyDecision；
- 不产生 ValidatedActionChunk；
- 按机器人配置执行 hold、decelerate 或 stop。

除非存在独立的正确性和稳定性证据，否则不允许静默裁剪动作后继续执行。

## RobotAdapter

如果问题成立且需要 RobotAdapter，候选实现应根据模型 action space 和目标机器人只选择一种路径：

- 模型动作可以无语义损失地转换为时间参数化关节轨迹时，使用 `FollowJointTrajectory`；
- 必须使用速度、笛卡尔增量或高频 setpoint 时，使用 robot-specific adapter。

候选 RobotAdapter 需要定义接受、拒绝、替换、generation、取消、超时和停止语义。具体接口、控制频率和映射只有在对应问题成立后才固定。

## 故障与恢复

推理进程退出、通信中断或 Chunk 过期时，`safety_control` 继续独立维护 deadline 并执行停止策略。

恢复动作前必须：

1. 清除旧 Chunk 并递增 context epoch；
2. 重新同步实测机器人状态；
3. 验证控制器 active；
4. 生成并验证新的 Chunk；
5. 再允许 RobotAdapter 执行动作。

## 安全声明边界

SafetySupervisor 是应用级防护和故障包含机制，不是经过 IEC 61508、ISO 10218 或 ISO 13849 认证的功能安全组件。人员检测、硬件急停、扭矩或速度硬限制仍由经过适当设计的机器人安全系统承担。

## 预期后果与验证要求

该候选边界会增加进程间传输和独立状态管理，预期目标是避免 GPU 故障同时带走应用级停止能力。必须先观察现有方案在退出、断连、超时和动作过期时的实际行为，再比较同进程防护、外部控制器能力和独立进程方案；没有证据时不实施。
