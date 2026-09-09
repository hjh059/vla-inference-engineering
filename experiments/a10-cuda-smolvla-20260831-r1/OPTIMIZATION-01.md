# 优化 01：SmolVLA 原生 BF16 GEMM

> 状态：已完成一次受控实现、正确性回归和 30 样本性能比较；尚未形成可提交的 `vla.cpp` revision，也尚未归档优化后 Nsight Systems 的大型原始工件。因此本文的性能结果可复核，但项目交付仍缺少上述两项归档工作。

## 假设与取舍

基线的隔离稳态 `nsys.sqlite` 按完整 demangled kernel name 重新聚合 `predict rid=3–12` 后，`convert_unary<__nv_bfloat16, float>` 有 21,710 次 launch、累计 157.329 ms（约 15.733 ms/请求）。代码检查显示 SmolVLA 的 VLM 和 action-expert GEMM 即使权重常驻 BF16，也会默认写入 `GGML_PREC_F32`，使该路径在每次矩阵乘前扩展权重。

假设是：不再强制 F32 precision，让 CUDA 以 BF16 权重的原生路径执行这些 GEMM，可将重复的 BF16→F32 转换替换为输入侧 F32→BF16 转换，并降低总推理时间。代价是矩阵乘输入精度和输出数值可能变化；因此不声称与基线按位等价，也不宣称动作语义或任务成功率不变。

此前 NCU 采集的 `convert_unary<float, __half>` 不是上述主要转换类型，不能用于断言 BF16→F32 kernel 的微观瓶颈；其结论边界已在 [PROFILE-ANALYSIS.md](PROFILE-ANALYSIS.md) 更正。

## 实现与回退

[`smolvla.cpp`](../../../vla.cpp/src/models/smolvla.cpp) 中 `tower_mm_f32_prec()` 改为默认不设置 `GGML_PREC_F32`。在 BF16 权重的冻结路径中，这会选择原生 BF16 GEMM。设置 `VLA_MM_PREC=f32` 可恢复改动前的 F32 precision 行为。

除这一默认选择外，模型、设备、驱动、CUDA Toolkit、CMake 选项、线程数、输入、noise、warm-up、样本数及统计方法均保持正式配置不变。

优化构建使用 `vla.cpp@4e46904efbc57ef4ccff613c2a23cfd1cc335fed` 加当前未提交的最小 diff；该 diff 的 SHA-256 为 `319510377cdab30c6d9e8539d4de692bdbb7e315628839b50e8d32b09d1c5c47`。构建目录为 `/root/vla-smoke-c01/build-opt-a10-20260909`，选项为 Release、`GGML_CUDA=ON`、`GGML_CUDA_GRAPHS=ON`、`CMAKE_CUDA_ARCHITECTURES=86`；FetchContent 的 `llama.cpp` 为 `7ba604f1cb61cd14898138e9abc0b4ff2601f180`。

## 正确性回归

`ctest --test-dir build-opt-a10-20260909 --output-on-failure -R '^bf16_cuda_ops$'` 通过。随后使用冻结的 fixed-request client 执行 5 次预热和 30 次计时请求；全部输出为 50×32、有限，且同次运行内后续 29 次输出均与首个计时请求按位一致。原始样本和摘要位于 [`results/optimization-01-native-bf16/`](results/optimization-01-native-bf16/)，字节校验见 [`raw/optimization-01-artifacts.sha256`](raw/optimization-01-artifacts.sha256)。独立的 reference capture 已保存于该目录的 `reference-capture/`。

另以 `VLA_MM_PREC=f32` 运行同一冻结请求；其 reference action 与归档基线逐字节一致。该回退检查的样本、摘要和 reference action 也已列入上述工件清单。

与基线的文本 reference 输出比较时，1,600 个值中有 350 个在打印精度下不同，最大绝对差为约 `0.002875342`。这不违反当前冻结的“同一已加载服务内按位可重复”标准，但说明它不是跨实现数值等价验证；动作语义、LIBERO 成功率与真实控制效果仍未验证。

## 同条件性能比较

所有统计均使用未排除的 30 个样本；percentile 使用基线相同的线性插值方法。单位为 ms。

| 指标（均值） | 基线 | 优化后 | 变化 |
|---|---:|---:|---:|
| 客户端端到端 wall time | 70.751 | 57.476 | -18.76% |
| 服务端 total | 68.027 | 54.789 | -19.46% |
| 服务端 vision | 18.793 | 18.625 | -0.89% |
| 服务端 inference | 48.735 | 35.707 | -26.73% |

性能改善集中在 inference，而 vision 基本不变，符合该修改只作用于 VLM/action-expert GEMM 路径的预期。完整分位数见 [RESULTS.md](RESULTS.md)。

## 机制核查与后续

优化后额外采集了一份带完整 NVTX 嵌套范围的 trace：12 个 `request` 和 12 个 `predict` 范围均闭合，10 个正式请求内不再出现 `convert_unary<__nv_bfloat16, float>`，而 `convert_unary<float, __nv_bfloat16>` 为 22,440 次、43.928 ms（约 4.393 ms/请求）。该观察与性能变化方向一致。

该 trace 的 `.nsys-rep` 与 SQLite 仍仅在项目外临时目录，未进入本仓库的工件清单；按[工件管理规则](../README.md)它不能作为长期有效证据。发布或宣称项目闭环完成前，仍需：归档该 trace 及校验和、将优化以可追溯 commit 固化、并在适当场景补充动作语义或任务级评估。
