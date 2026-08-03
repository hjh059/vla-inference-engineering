# 候选 Issue 筛选表

> 状态：Problem Discovery 调查台账。当前尚未收集候选 Issue，也未选定首个复现问题。

本文只记录真实用户问题的线索和筛选依据，不把 Issue 描述、评论中的归因或建议方案当成已验证事实。首批候选总数不超过 10 个；进入本地复现准备的候选不超过两个，同时只执行一个主复现路径。

## 问题池顺序

1. **第一问题池：LeRobot Issues**。优先关注 SmolVLA/LIBERO、异步推理、动作语义、Processor、GPU 内存和控制端消费；
2. **第二问题池：Isaac-GR00T Issues**。重点关注 ONNX/TensorRT、Jetson、跨硬件正确性和性能，不作为默认首个复现路径。

Issue 是问题来源，不是项目场景。只有固定版本、本地复现、影响验证和替代方案反证完成后，才能形成项目问题证据。

真实性、本地可复现性、技术深度、硬件成本和已有修复只用于安排调查优先级。Issue 热度、技术趣味、容易修复或适合 C++、CUDA/TensorRT 等工具不增加真实性；当前设备无法复现也不能证明问题不存在。

## 候选列表

| ID | 问题池 | Issue permalink | 报告用户/工作流 | 真实性 | 本地可复现性 | 技术深度 | 硬件成本 | 已有修复/PR | 决策 |
|---|---|---|---|---:|---:|---:|---:|---:|---|

当前记录数：0 / 10。

## 评分规则

每个维度使用 0—2 分。分数用于排序，不代替工程判断。

| 维度 | 0 | 1 | 2 |
|---|---|---|---|
| 真实性 | 只有结论或功能请求 | 有版本、现象或维护者确认中的一部分 | 用户、工作流、版本、预期/实际、日志或最小示例较完整 |
| 本地可复现性 | 必须先购买不可获得的设备或资产 | 需要有费用上限的云 GPU、借用设备或较多准备 | 当前设备、CPU、小模型或现有公开资产可复现 |
| 技术深度 | 单纯安装、用法或配置问答 | 需要跨组件定位但边界较浅 | 涉及模型、Processor、Runtime、动作语义、异步调度、GPU 内存、协议、正确性或控制消费 |
| 硬件成本 | 必须先采购高成本硬件 | 需要临时云资源或可借用设备 | 无新增硬件成本 |
| 已有修复/PR | 最新版本或已合并 PR 已解决 | 存在 workaround 或活跃 PR，仍需验证 | 没有已知修复，或现有修复不能解决且有证据 |

进入主动复现准备至少要求真实性和本地可复现性均不为 0；本地可复现性为 0 的线索进入 `Backlog`，不判定为虚假。总分相同时，优先选择固定输入更容易、根因更可证伪且影响更可观察的候选。最小修复、实现语言和对照工具不参与 Issue 准入评分。

## 单条记录要求

候选进入前两名时，除表格摘要外必须补充：

```text
Issue ID and permalink:
Repository and affected version/commit:
Reporter role or candidate user (mark inference):
Workflow and expected outcome:
Observed behavior and direct impact:
Environment and hardware:
Logs, reproducer or attached assets:
Maintainer response and duplicates:
Workaround, related PR and latest-version status:
Local reproduction hypothesis:
Required model, data, simulator and hardware:
Expected-behavior or correctness baseline:
Planned reproduction and diagnostic tools:
Hypothesized affected layer and implementation language:
Root-cause questions to test:
Planned impact validation (numeric / regression / resource / simulation / consumer; mark N/A with reason):
Triage score and decision:
```

信息缺失时填写 `Unknown`，不得推断为“不支持”或“现有方案无法解决”。

## 决策值

- `Shortlist`：进入最多两个候选之一，允许准备复现；
- `Backlog`：问题线索有效，但当前优先级或资源不合适；
- `Use as counterexample`：已有修复或替代方案，作为反证材料；
- `Reject`：缺少真实工作流、已被当前版本或合并修复解决、与调查范围无关、属于普通支持问题或证据表明报告归因不成立。

具体执行顺序、证据链和退出条件见 [Phase 0：Problem Discovery 计划](../plans/phase-0.md)。
