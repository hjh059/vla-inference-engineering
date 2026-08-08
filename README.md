# RoboEdge VLA Deployment Problem Discovery

面向机器人 VLA 部署生态的问题发现项目。当前不预设需要新建 C++ Runtime、安全进程、ROS2 控制链路或完整生产系统，而是通过现有方案复现、代码检查和最小实验，识别一个真实存在、可重复、尚未被现有方案充分解决且值得投入的工程问题。

> 当前状态：Problem Discovery。尚未形成经过证据支持的问题陈述、最终项目目标或生产架构。
>
> 已知存在 `vla.cpp`、`Embodied.cpp` 和 GR00T 官方 ONNX/TensorRT 等部署路径。它们的能力、限制和适用边界需要按固定版本核查；“官方未说明”不等于“不支持”，“本地未复现”也不等于“方案不可用”。

## 项目动机

项目以真实 Issue 的完整证据链形成可验证的 VLA 部署工程成果；具体动机和能力声明边界见[项目定位](docs/project/positioning.md)。

C++、Python、CUDA/TensorRT、Processor、协议和控制端工程都是已有能力或候选解决工具，具体使用什么由问题决定。面向中高级 C++ AI 推理、GPU 性能工程和机器人部署岗位形成工程证据是预期收益，不是问题准入条件，也不能证明应当建设某一种 Runtime 或机器人闭环。

## 当前目标

Problem Discovery 只回答：

1. 谁在使用现有 VLA 部署方案完成什么工作；
2. 当前方案在固定输入和环境下实际发生什么失败或困难；
3. 问题造成什么明确影响；
4. 其他现有方案是否已经解决；
5. 问题是否值得形成独立项目，还是更适合作为上游 Issue/PR；
6. 应以什么证据判断问题已经解决。

当前不会冻结生产模型、Runtime、硬件、ROS2、控制路径或安全架构，也不会把候选设计描述为已确认需求。

## 调查闭环

```text
LeRobot / Isaac-GR00T Issue 问题线索
        │
        ▼
线索完整度、用户影响、复现可行性与已有修复筛选
        │
        ▼
固定候选用户、工作流、版本、输入和环境
        │
        ▼
使用问题原生路径完成最小复现
        │
        ▼
建立预期行为或官方正确性基线
        │
        ▼
定位根因并在适当层级完成最小修复
        │
        ▼
执行与问题声明匹配的影响验证
        │
        ▼
用替代方案和上游版本反证
        │
        ▼
Continue / Contribute upstream / Stop/Pivot
```

固定工位拣选、C++ 推理、安全监督和机器人控制可以作为调查场景或候选方案，但在问题证据出现前都不是项目既定目标。详细边界见[项目范围](docs/project/scope.md)，当前执行步骤见 [Problem Discovery 计划](docs/plans/phase-0.md)。

## 证据规则

所有能力和问题声明都按 [Problem Discovery 计划中的证据规则](docs/plans/phase-0.md#证据规则)分类。GitHub Issue、公开 benchmark 和 README 只作为调查输入；未经受控复现、测量和替代方案反证，不能形成当前项目的问题结论。

## Problem Discovery 退出条件

只有满足 [Problem Discovery 计划中的 `Continue` 条件](docs/plans/phase-0.md#continue)，才能基于已验证问题重新定义项目目标并进入后续架构或产品化设计；`Contribute upstream` 和 `Stop/Pivot` 也是合法退出结果。

## 文档

- [文档导航](docs/README.md)
- [项目定位](docs/project/positioning.md)
- [Problem Discovery 范围](docs/project/scope.md)
- [Problem Discovery 计划](docs/plans/phase-0.md)
- [候选 Issue 筛选表](docs/research/issue-candidates.md)
- [Runtime 与后端生态事实](docs/research/backend-candidates.md)
- [部署平台与资源策略](docs/research/deployment-platforms.md)

## 当前限制

- 仓库尚无实现、实验报告或本地复现证据；
- 首批候选 Issue 尚未收集和筛选；
- 候选用户、工作流和目标资源尚未固定；
- 被选问题相关的上游实现、参考基线和替代方案尚未完成代码级或实验级核查；
- 任何性能、安全、可靠性或生产可用性结论均未成立。
