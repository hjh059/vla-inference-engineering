# 端到端部署与性能优化计划

## 目标

本计划以[部署与性能优化范围](../project/scope.md#项目目标)为准。当前主闭环只处理正式配置 [`a10-cuda-smolvla-20260831-r1`](../../experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md)，不扩展为通用 Runtime、跨模型框架或生产控制系统。

## 当前状态

- [x] 定义项目范围、证据等级和测量原则；
- [x] 核验本地设备、阿里云 A10、CUDA、Nsight Systems、Nsight Compute 和硬件计数器访问能力；
- [x] 比较候选路径并选择 SmolVLA-LIBERO GGUF + vla.cpp；
- [x] 冻结模型、设备、输入、initial noise、正确性标准和测量协议；
- [x] 跑通并归档端到端部署路径；
- [x] 建立固定输入下的正确性基线和 30 样本稳态性能基线；
- [x] 归档带 NVTX 请求范围的隔离稳态 Nsight Systems profile；
- [x] 确认主要瓶颈层级为 `vla::predict()` 内 GPU kernel 执行；
- [x] 对 `convert_unary` 和代表性 CUTLASS `Kernel2` GEMM 采集 Nsight Compute 硬件计数器；
- [x] 选择并实施一个与现有证据对应的最小优化（SmolVLA 原生 BF16 GEMM）；
- [x] 完成优化后的固定输入正确性回归和同条件 30 样本性能比较；
- [ ] 归档优化后的大型 profile 工件、将优化固化为可追溯的 vla.cpp commit，并完成最终交付记录。

## 下一步

优化 01 的机制、实现、固定输入回归、性能比较和未验证边界见 [OPTIMIZATION-01.md](../../experiments/a10-cuda-smolvla-20260831-r1/OPTIMIZATION-01.md)。其性能结论不能扩大到其他模型、输入或设备，也不能据此主张动作语义保持不变。必要的复现入口仍为 [Nsight Systems](../../experiments/a10-cuda-smolvla-20260831-r1/NSYS-RUNBOOK.md)、[Nsight Compute](../../experiments/a10-cuda-smolvla-20260831-r1/NCU-RUNBOOK.md) 与[原始 profile 工件索引](../../experiments/a10-cuda-smolvla-20260831-r1/raw/vla-steady-r2/README.md)。

1. 将当前最小 diff 固化为 `vla.cpp` commit，并在优化记录中以该 commit 替换工作树 diff 身份；
2. 将已采集的优化后 Nsight Systems `.nsys-rep`、SQLite、日志和校验和移入当前 Configuration ID 的工件目录，或按同一条件重采；
3. 复核归档后的原始结果、checksum 与文档链接，再完成最终交付记录；
4. 如需扩大正确性主张，新增独立的动作语义或 LIBERO 任务级验证；不得把当前固定输入数值可重复性外推为该结论。

变更任何模型、设备、驱动、构建选项、输入、noise、线程数、warm-up、重复次数或统计方法时，必须创建新的 `Configuration ID`，不得与当前基线组成优化前后比较。
