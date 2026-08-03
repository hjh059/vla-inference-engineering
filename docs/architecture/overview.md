# 候选架构：VLA 推理与安全控制闭环

> 状态：Candidate Solution / Hypothesis。本文保留此前的架构思考，但不是当前 Problem Discovery 的实施依据，也不代表现有方案已经被证明缺少这些组件。
>
> 只有复现证据表明具体用户工作流需要新的运行时、动作生命周期、安全边界或控制适配时，本文中的相应部分才能重新进入架构评审。

## 适用前提

本文候选架构依赖以下尚未验证的假设：

- 现有 Runtime 无法在目标工作流中提供足够的输入、动作或生命周期语义；
- 延迟、过期结果或 Runtime 故障会造成可重复且有明确影响的问题；
- 问题不能通过现有方案、配置、文档修正或小型上游补丁解决；
- 独立进程、ROS2/DDS 和专用 RobotAdapter 是解决问题所必需的最小边界。

在这些假设成立前，以下组件、数据流和不变量只用于形成调查问题与对照候选。

## 候选系统上下文

候选 RoboEdge VLA Runtime 接收固定相机图像、版本化语言指令和机器人状态，构造 Observation，经一个逻辑上的 VLA 生成 Proposed Action Chunk。候选设计要求模型动作经过独立 SafetySupervisor 检查，只有 Validated Action Chunk 可以进入 RobotAdapter。

```text
Camera + Language Instruction + Robot State
                    │
                    ▼
        Observation Synchronization
                    │
                    ▼
        one logical VLA policy
           (GPU vla_runtime)
                    │
                    ▼
         ProposedActionChunk
                    │ ROS2/DDS
                    ▼
           SafetySupervisor
          (non-GPU safety_control)
             │             │
          reject        approve
             │             ▼
      SafetyDecision  ValidatedActionChunk
                           │
                           ▼
                      RobotAdapter
                           │
                           ▼
                 selected controller / robot
```

## 候选组件

### Observation Synchronization

- 对齐图像、机器人状态和指令的时间、序列和版本；
- 拒绝过期、乱序、不完整或 embodiment 不一致的输入；
- 只构造完整、最新且语义明确的 Observation。

### `vla_runtime`

- 加载唯一生产模型及其可信制品；
- 维护模型上下文、Cache、generation 和 deadline；
- 以有界单执行槽和 latest-pending Observation 处理背压；
- 只产生 ProposedActionChunk，不直接控制机器人；
- fatal CUDA 状态时退出，由外部 supervisor 有界重启或回滚。

### `SafetySupervisor`

- 位于非 GPU 的 `safety_control` 故障域；
- 独立订阅机器人状态并维护动作有效期；
- 在反归一化后的实际物理量上验证动作；
- 拒绝时产生 SafetyDecision，通过时产生 ValidatedActionChunk。

### `RobotAdapter`

- 只接受 ValidatedActionChunk；
- 把选定 action space 转换为唯一机器人控制接口；
- 不推断或修补未经定义的模型动作语义；
- 由控制器和机器人安全系统承担最终轨迹执行、保护停和急停。

## 候选故障边界

`vla_runtime` 与 `safety_control` 是两个进程。CUDA 进程崩溃、重启或回滚时，后者仍独立维护机器人状态、动作有效期和停止策略。

首版不把 VLA 放入硬实时控制线程，也不依赖公网或云端控制闭环。推理超时、动作过期、版本不匹配、控制器异常或通信中断必须进入显式拒绝或停止路径。

## 候选不变量

1. 未经 SafetySupervisor 验证的模型输出不能进入 RobotAdapter；
2. 缺少单位、坐标系、关节顺序、时钟或版本语义的裸张量不能跨越运行时边界；
3. 旧 observation、instruction revision、context epoch 或 generation 的结果不能执行；
4. SafetySupervisor 不依赖 GPU 进程保持机器人受控停止；
5. 若选择纯 C++ 数据面，Python 只用于参考、导出、评测和仿真适配；
6. 首版只有一个生产模型、一个后端、一个硬件 profile 和一个控制路径。

## 候选契约

以下内容只是候选语义要求。只有已验证问题确实需要正式跨边界契约时，后续阶段才决定是否写入版本化 IDL/Schema：

- 图像、机器人状态和指令的时钟、序列、有效期与所有权；
- action space、物理单位、坐标系、关节顺序和 Chunk horizon；
- 模型、Processor、Tokenizer、归一化、后端和 Engine 身份；
- observation、instruction revision、context epoch 和 generation 的失效规则；
- 拒绝原因、追踪 ID 和稳定错误码。

Proposed/Validated Action Chunk 的候选运行时和安全职责分别记录在 Draft [ADR-0010](../adr/0010-runtime-concurrency-lifecycle.md)与 [ADR-0012](../adr/0012-safety-process-boundary.md)中。

## 候选运行时和控制原则

- 同一时刻只有一个执行中的 VLA 请求和一个 latest-pending Observation；
- 新观测覆盖尚未执行的旧观测，过期 GPU 结果按 epoch/generation 丢弃；
- 队列等待、推理 deadline 和 Chunk 有效期分别记录；
- Token、embedding、KV Cache、Action Head 状态和随机状态具有显式 owner、容量与 epoch；
- 控制接口在目标机器人和 action space 确定后只选择一种；
- VLA 的低频策略更新与控制器的高频执行明确解耦。

## 候选安全检查边界

SafetySupervisor 至少检查：

- finite、shape、action type、单位、坐标系和关节顺序；
- Observation/Chunk age、epoch、generation、instruction revision 和 expiry；
- 关节位置、速度、加速度、jerk、接续连续性和工作空间；
- 控制器生命周期、机器人模式、保护停和急停状态；
- 已有可靠几何输入时的自碰撞与环境碰撞。

这些是应用级安全检查，不构成功能安全认证。最终急停和硬限制仍由机器人安全控制系统承担。
