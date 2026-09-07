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
- [ ] 选择并实施一个与现有证据对应的最小优化；
- [ ] 完成优化后的正确性回归、同条件性能比较和交付记录。

## 下一步

后续优化必须直接引用[profile 分析](../../experiments/a10-cuda-smolvla-20260831-r1/PROFILE-ANALYSIS.md)中已经成立的证据，不得把热点占比、代表性 kernel 的局部结论或受 profiler 扰动的延迟扩大解释。必要的复现入口为 [Nsight Systems](../../experiments/a10-cuda-smolvla-20260831-r1/NSYS-RUNBOOK.md)、[Nsight Compute](../../experiments/a10-cuda-smolvla-20260831-r1/NCU-RUNBOOK.md) 与[原始 profile 工件索引](../../experiments/a10-cuda-smolvla-20260831-r1/raw/vla-steady-r2/README.md)。

1. 根据现有 NCU 证据提出一个可证伪、改动范围明确的优化假设；
2. 在修改前记录预期机制、受影响路径、正确性风险和潜在资源取舍；
3. 只修改验证该假设所需的实现，并记录新的 vla.cpp revision；
4. 按冻结配置先执行正确性回归，再采集同样的 30 个稳态性能样本；
5. 从原始样本计算相同统计量，必要时补充范围受控的 profile，以判断机制是否成立；
6. 若正确性回归失败或主指标没有可重复改善，将结果记录为未通过，不扩大优化范围来追逐单次数字。

变更任何模型、设备、驱动、构建选项、输入、noise、线程数、warm-up、重复次数或统计方法时，必须创建新的 `Configuration ID`，不得与当前基线组成优化前后比较。
