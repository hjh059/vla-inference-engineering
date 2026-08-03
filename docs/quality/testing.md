# 候选测试与证据目录

> 状态：Deferred Candidate Design。当前只保存可能复用的验证维度，不代表测试已经执行，也不表示最终方案会包含 contracts、SafetySupervisor、ROS2 或 RobotAdapter。

Problem Discovery 当前只实施被选 Issue 所需的最小复现、诊断、回归和影响验证；固定输入比较或异常观察仅在问题相关时执行。以下完整测试体系只有在问题成立且对应组件进入最小解法后才启用。

## 原则

- 核心 contracts、runtime 和 safety 可以脱离 ROS2 单元测试；
- 仿真证据与真实机器人证据分开报告；
- 失败样本、原始配置、日志和制品哈希必须留存；
- 数值门禁和统计方法只由[验收原则](../adr/0014-acceptance-principles.md)维护。

## 单元测试

- Observation、ProposedActionChunk、SafetyDecision 和 ValidatedActionChunk；
- Tokenizer、图像/状态预处理和动作反归一化；
- ContextEpoch、Cache owner 和清理；
- 安全边界、单位、坐标系和关节顺序；
- manifest、哈希、签名和兼容拒绝。

## 属性与模糊测试

- timestamp、乱序、重复和时钟跳变；
- action shape、非有限值、单位和 joint order；
- generation 与 instruction revision 交错时旧结果不得发布；
- RobotAdapter 不得接收 ProposedActionChunk；
- Chunk 切换、耗尽、过期和控制器异常；
- 制品 parser 和外部消息边界。

## 模型与后端正确性

- Python Processor 与 C++ Processor 逐元素对齐；
- 视觉编码器、多模态主干、Cache、状态编码器和 Action Head；
- 不同精度和后端之间的动作误差；
- 生成式模型的固定 seed/noise 定位；
- 原始张量摘要、配置和哈希归档。

## 集成测试

- 虚拟时钟下的有界队列、deadline、取消和 stale result；
- ROS2 Lifecycle、QoS、断连和重连；
- 控制 goal 的接受、取消、替换和 abort；
- OOM、坏 Engine、错误 artifact、进程重启和回滚；
- 日志、指标、请求 ID 和故障分类。

## 仿真与真机

仿真固定场景、资产、任务、episode、初始状态、seed、相机、控制频率、成功判定和制品版本。LIBERO 可以用于通用模型基准，但不能自动替代与参考工位匹配的场景验证。

真机从无动作回放、低速和受限工作空间逐级扩大，覆盖输入中断、推理超时、控制器异常、GPU 进程退出、重启和受控停止。没有真机时，报告必须明确只证明仿真和推理系统能力。

## 工具

- C++：GTest、CTest、clang-tidy 或等价工具；
- Python：pytest；
- ROS2：`launch_testing`；
- 内存与并发：ASan、UBSan、TSan；
- GPU：Compute Sanitizer；
- 性能：Google Benchmark、Nsight、`trtexec`；
- 系统追踪：LTTng、ros2_tracing。

具体工具和版本在首次落地时写入锁文件。新增运行时依赖必须单独说明收益、替代方案、许可证和维护成本。

## CI 分层

- 每次变更：格式、静态分析、编译、单元测试和轻量仿真 smoke；
- 定期任务：sanitizer、完整 episode、制品可复建和目标平台性能；
- 发布门禁：故障注入、长稳、制品校验以及当前能力声明所需的真机验证。
