# VLA 推理部署与优化

本项目通过可复现的实验，帮助学习和理解 VLA 模型从部署运行到性能优化的过程，包括验证推理输出、测量请求与 GPU 开销、定位瓶颈，以及评估优化收益和数值精度取舍。代码、操作步骤与原始实验记录相互对应，便于复现和分析结果。

当前实验采用 SmolVLA-LIBERO GGUF、`vla.cpp` C++/CUDA 推理引擎和单张 NVIDIA A10。

## 当前进展

- [x] **部署跑通**：选定 SmolVLA-LIBERO GGUF + `vla.cpp`，在 A10 上完成固定请求推理。
- [x] **基线建立**：冻结环境与输入，完成输出结构和运行内数值可重复性检查，归档 30 次稳态性能样本。
- [x] **时间线分析**：加入 NVTX 请求标记，采集并归档 Nsight Systems 稳态 trace，分析 GPU 执行、同步等待和 kernel 热点。
- [x] **Kernel 指标采集**：使用 Nsight Compute 采集类型转换和 GEMM 两个代表性 kernel 的硬件指标。
- [ ] **优化 1**：已实现原生 BF16 GEMM 默认路径及 F32 回退，源码已固定为可追溯 commit；已完成固定输入正确性回归、F32 回退一致性检查和同条件 30 样本性能比较：客户端端到端均值 **70.751 → 57.476 ms（降低 18.76%）**，服务端 inference 均值 **48.735 → 35.707 ms（降低 26.73%）**。优化后大型 profile 工件仍待归档，并需更新校验清单与验证记录。

## 仓库结构

```text
README.md                                 # 项目介绍与统一导航
concepts/                                 # 理解 VLA 推理与性能优化所需的基础知识
├── foundations.md                        # 训练、参数、张量、GEMM 与数值精度
├── vla-inference-and-correctness.md      # 推理路径、动作生成、输入输出契约与正确性
└── measurement-and-optimization.md       # 基线、计时、profiling、优化验证与证据等级
experiments/                              # 部署、基线测量、性能分析与优化的实验记录
├── SELECTION.md                          # 模型、运行时与设备选择
├── README.md                             # 实验流程、比较与工件保存规则
├── 01-baseline/                          # 建立固定配置下的正确性与性能基线
│   ├── README.md                         # 冻结配置、复现步骤与结果
│   ├── scripts/                          # 固定请求客户端及构建脚本
│   ├── raw/                              # 环境、日志与校验清单
│   └── results/                          # 基线样本、摘要与参考输出
├── 02-profiling/                         # 分析 GPU 开销并定位性能瓶颈
│   ├── BUILD.md                          # 观测构建
│   ├── NSYS-RUNBOOK.md                   # 时间线采集
│   ├── NCU-RUNBOOK.md                    # kernel 硬件指标采集
│   ├── ANALYSIS.md                       # 分析与证据边界
│   └── raw/                              # 报告、日志与校验清单
└── 03-optimization-01/                   # 原生 BF16 GEMM 优化及效果验证
    ├── OPTIMIZATION.md                   # 优化实现、复现与验证
    ├── raw/                              # 优化工件校验清单
    └── results/                          # 优化样本、参考输出与回退验证
```

实验的执行顺序、阶段文档和共用规则见[实验流程与阶段导航](experiments/README.md)。

推理引擎源码由 [vla.cpp](https://github.com/hjh059/vla.cpp) 仓库管理，实验记录引用其固定 revision。

模型本体与构建产物不提交到本仓库，其来源、版本、构建条件和字节身份记录在对应实验文档中。

## 适用范围

当前结果来自固定配置下的单请求实验，尚未进行任务成功率或真实机器人控制验证。
