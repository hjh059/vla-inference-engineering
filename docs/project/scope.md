# 部署与性能优化范围

> 状态：可行性验证。当前先选择可运行路径；正式基线开始前冻结实验配置，不把候选技术选择描述为最终生产架构。

## 项目目标

完成可行性选择后，在一个固定的 VLA 模型/checkpoint、设备和任务场景上跑通现有部署路径，建立可复现的正确性与性能基线，用 profiling 定位一个主要瓶颈，实施有因果依据的优化，并在相同配置下验证前后结果。

## 两阶段范围

### 可行性选择

本阶段只做选择所需的资料核查和有限 smoke test：

1. 定义候选任务场景、可用设备范围和主指标类型，优先验证最有希望的路径，避免无限扩展候选范围；
2. 核对相关模型、checkpoint、官方/现有部署路径、许可证和兼容性；
3. 选择一条可运行路径，记录选择依据和已知限制；
4. 在路径可运行后，按下文建立正式基线配置记录。

可行性测试不与正式性能结果混用，也不进行广泛 Runtime 横向比较。

### 基线与优化

进入正式实验后，必须：

1. 在固定环境和输入下跑通端到端路径；
2. 建立任务相关的正确性基线；
3. 用预先定义的方法采集性能与资源基线；
4. 保存 profile 工件，选择一个主要瓶颈；
5. 实施与该瓶颈对应的最小优化；
6. 在同一配置和测量方法下验证正确性与前后指标；
7. 记录结果、限制和未验证范围。

## 正式基线配置

每个正式实验配置必须有一份唯一记录，至少包含：

```text
Configuration ID:
Frozen experimental conditions:
Model and checkpoint:
Target device and operating system:
Driver, CUDA and runtime versions:
Task scenario and fixed input/assets:
Initial noise or random-seed policy:
Expected behavior and correctness criteria:
Primary and secondary metrics:
Measurement method, warm-up and repeat count:
Path context:
Deployment path and upstream repository/commit:
Compared implementations:
Baseline implementation revision and reproduction command:
Optimized implementation revision and reproduction command:
Optimization change reference:
```

模型/checkpoint、设备、关键软件版本、输入、initial noise 或随机种子策略、正确性标准和测量方法是冻结的可比性条件；基线与优化后的实现 revision 是记录在同一 `Configuration ID` 下的对比变量。变更任何冻结条件时，必须建立新的 `Configuration ID`；不得把不同配置的结果作为同一优化前后对比。

冻结首个配置时，必须在 `experiments/<Configuration ID>/` 建立该 `Configuration ID` 的唯一实例记录；字段定义只在本节维护，工件保存规则见[实验记录与工件管理](../../experiments/README.md)。

## 必须产出

- 路径选择依据与正式基线配置；
- 可复现的环境设置、运行命令和固定输入；
- 正确性和性能基线的原始结果及汇总方法；
- profile 工件、主要瓶颈判断及支持证据；
- 最小优化的实现、取舍与因果说明；
- 优化后的正确性回归和前后指标对比；
- 适用范围、限制和未验证项。

## 当前不实施

- 未经 profile 证据支持的泛化优化或架构重写；
- 通用 VLA Runtime、多模型在线切换、插件框架或云端多租户 Serving；
- 与任务无关的完整机器人控制链路、安全架构或生产发布系统；
- VLA 预训练、训练算法、视觉算法或世界模型研究；
- 真实机器人高风险动作或功能安全能力声明；
- 与已选路径和主瓶颈无关的开放式生态调研。

## 资源与证据边界

- 可行性阶段可在已有或经授权的临时资源上验证兼容性；正式性能基线必须固定目标设备与软件环境；
- 硬件选择受任务场景、路径兼容性、可获得资源和测量需求约束，不因“更像生产环境”或岗位展示价值单独决定；
- 外部 Issue、官方文档和公开 benchmark 仅用于辅助选择或解释，不构成本地正确性、性能或优化证据；
- 仿真、公开数据和真机证据必须分别记录来源、许可证和适用范围；证据不足时结论为 `Unknown` 或 `Insufficient evidence`。
