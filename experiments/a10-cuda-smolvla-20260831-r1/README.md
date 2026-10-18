# 实验总览：`a10-cuda-smolvla-20260831-r1`

本页维护本实验的流程导航和目录结构；配置、复现步骤、测试结果与分析保存在对应阶段目录。

- [实验流程](#实验流程)
- [基线配置与结果](baseline/README.md)

## 实验流程

本实验使用基线阶段记录的[冻结配置](baseline/README.md#冻结配置)。下表按实验主线串联操作流程与结果记录；具体命令、指标和结论在对应文档维护。

| 顺序 | 阶段与目的 | 操作或配置文档 | 结果与分析 |
|---|---|---|---|
| 1 | 冻结配置：确定模型、设备、软件版本、固定输入和比较条件 | [基线配置](baseline/README.md#冻结配置) | 后续基线与优化比较共同遵循的实验条件 |
| 2 | 建立基线：验证固定请求的正确性，测量端到端及服务端阶段耗时 | [基线复现流程](baseline/README.md#复现步骤) | [基线结果](baseline/README.md#实验结果) |
| 3 | 准备观测构建：加入 NVTX 请求与预测范围，固定 profiling 二进制身份 | [观测构建](profiling/BUILD.md) | 为后续采集提供可定位的请求边界 |
| 4 | 定位整体热点：用 Nsight Systems 采集稳态 CUDA 时间线，分析 GPU busy、同步等待和 kernel 耗时分布 | [NSYS-RUNBOOK.md](profiling/NSYS-RUNBOOK.md) | [Profile 分析](profiling/ANALYSIS.md) |
| 5 | 深入检查代表性 kernel：用 Nsight Compute 采集选定 kernel 的硬件指标 | [NCU-RUNBOOK.md](profiling/NCU-RUNBOOK.md) | [Profile 分析](profiling/ANALYSIS.md)，同时说明采样对象与结论边界 |
| 6 | 实施并验证优化：根据热点和代码检查形成假设，实施最小改动，进行正确性回归及同条件性能比较 | [优化 01](optimization-01/OPTIMIZATION.md)，通用测量流程仍复用基线 runbook | [性能比较](optimization-01/OPTIMIZATION.md#同条件性能比较)与[正确性回归](optimization-01/OPTIMIZATION.md#正确性回归)，机制、取舍及证据缺口见优化记录 |

Nsight Compute 检查按分析需要开展，并非每次优化的必经步骤；单个 kernel 的指标不能直接外推到整类 kernel。Profile 采集用于定位热点与解释机制，正式性能比较使用独立的基线测量流程。

目录按阶段组织：

```text
README.md                 # 实验流程与目录导航
baseline/
├── README.md             # 冻结配置、复现步骤、正确性与性能结果
├── scripts/              # 固定请求客户端及构建脚本，各阶段复用
├── raw/                  # 基线环境记录、日志和校验清单
└── results/              # 基线样本、摘要和参考输出
profiling/
├── BUILD.md              # 观测构建
├── NSYS-RUNBOOK.md       # 时间线采集
├── NCU-RUNBOOK.md        # kernel 硬件指标采集
├── ANALYSIS.md           # 分析与证据边界
└── raw/                  # 原始报告、采集日志及校验清单
optimization-01/
├── OPTIMIZATION.md       # 优化假设、实现、复现及验证
├── raw/                  # 优化工件校验清单
└── results/              # 优化样本、参考输出和回退验证
```

工件保存规则见上级[实验记录与工件管理](../ARTIFACT-POLICY.md)。原始日志和报告中的历史路径保持不变；当前归档位置以本页导航及各阶段校验清单为准。
