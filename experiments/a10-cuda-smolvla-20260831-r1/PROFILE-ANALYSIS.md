# Profile 分析：`a10-cuda-smolvla-20260831-r1`

> 采集日期：2026-09-01。本文只维护 profile 的采集范围、观察、硬件计数器分析和结论边界。正式正确性与无 profiler 的 30 样本性能基线见 [RESULTS.md](RESULTS.md)。所有观察仅适用于各节记录的派生 profile 构建、冻结模型、输入、固定 noise、A10、驱动、单客户端和测量协议。

## 已归档 profile 工件

已生成 [`raw/nsys-baseline.nsys-rep`](raw/nsys-baseline.nsys-rep)（1.6 MiB）和由 Nsight Systems 导出的 [`raw/nsys-baseline.sqlite`](raw/nsys-baseline.sqlite)（4.2 MiB）。字节身份统一由 [`raw/profile-artifacts.sha256`](raw/profile-artifacts.sha256) 维护。

该 trace 覆盖模型加载、5 次 warm-up 和 1 次固定请求；因而 CUDA API 与 kernel 汇总混入初始化、CUDA graph 建立和稳态请求，**不能单独用于确认稳态主瓶颈**。它只证明当前环境可生成并解析模型 trace。

## 隔离稳态 profile：`vla-steady-r2`

此 profile 保持冻结的模型、固定 JPEG、token、32 维零状态、固定 noise、单客户端顺序 REQ/REP 与 `VLA_N_THREADS=16`。为建立可切片的请求边界，profile 使用加入 NVTX 观测标记的派生 CUDA `vla-server`；源码 revision 和构建方法见 [PROFILE-BUILD.md](PROFILE-BUILD.md)，二进制身份由 [`raw/profile-artifacts.sha256`](raw/profile-artifacts.sha256) 维护。该构建仅用于 profile，不能替代正式 30 样本性能基线的冻结构建产物。

服务加载完成后，在 collection 外执行 5 次 warm-up；开始 Nsight Systems collection 后，额外发送 2 个丢弃请求以消除 collection 启动阶段的扰动，再发送 10 个正式固定请求。`vla.profile` domain 为每个请求记录 `request rid=<id>`，并为模型预测记录嵌套 `predict rid=<id>`。

| 检查 | 结果 |
|---|---|
| NVTX 范围完整性 | 12 个 `request` 与 12 个 `predict` 范围均闭合，且 12 个 `predict` 均完全嵌套于同 ID 的 `request` |
| 丢弃范围 | `rid=1–2`；request 时长分别为 111.406 ms、178.478 ms，不参与稳态汇总 |
| 正式范围 | `rid=3–12`，共 10 次 |
| 正确性 | 10 次均为 50×32、无 `NaN`/`Inf`，并与本次 reference action 按位一致 |
| 正式 request 时长 | 76.628–85.040 ms；这是 profile 观测值，不是性能基线 |
| CUDA Runtime 与 kernel 覆盖 | `rid=3–12` 每次均有 36 个 CUDA Runtime 调用与 11,384 个 kernel；范围内 kernel 时间为 64.603–66.780 ms |

该 profile 已满足“稳定请求可精确切片”的证据要求。后续分析只使用 `predict rid=3–12`，不得混入 collection 启动阶段的 `rid=1–2`。

### 范围内汇总与瓶颈假设

下表来自 `nsys.sqlite` 中与 `predict rid=3–12` 相交的 CUDA Runtime 和 kernel 活动；GPU busy 是同一请求内 GPU kernel 时间区间的并集，不重复累计重叠 stream 的时间。

| 指标 | 10 次正式范围的结果 | 解释 |
|---|---:|---|
| `predict` 时长 | 均值 77.960 ms；范围 74.022–82.493 ms | profile 观测值，不能替代正式性能基线 |
| GPU unique busy | 均值 65.261 ms，占 `predict` 的 83.87% | 主要时间在 GPU kernel 执行，而不是请求协议或 CPU 空闲 |
| `cudaGraphLaunch` | 每请求 3 次；CPU API 时间均值 11.787 ms | 与 GPU 执行重叠，不能同 GPU busy 相加 |
| `cudaStreamSynchronize` | 每请求 18 次；CPU API 时间均值 61.739 ms | 主要表示主机等待已提交的 GPU 工作，不是独立 CPU 计算热点 |

按 kernel `shortName` 聚合的范围内 GPU 时间前六项如下。百分比以所有 kernel 累积时间为分母；它不等同于 wall time，stream 并发时不能与上表 GPU busy 直接相加。

| kernel 类别 | GPU 时间占比 |
|---|---:|
| CUTLASS `Kernel2` GEMM | 28.55% |
| `convert_unary` | 27.52% |
| `soft_max_f32` | 8.03% |
| `splitKreduce_kernel` | 7.38% |
| `k_bin_bcast` | 6.96% |
| `flash_attn_ext_f16` | 6.67% |

因此，本轮 **确认的主要瓶颈层级** 是 `vla::predict()` 内的 GPU kernel 执行；CPU 端同步是在等待这些 GPU 工作完成。

### `convert_unary` 的类型归因更正

原始 `nsys.sqlite` 可按完整 demangled kernel name 重新聚合。对同一 `predict rid=3–12` 范围，`convert_unary<__nv_bfloat16, float>` 有 21,710 次 launch、累计 157.329 ms（约 15.733 ms/请求），是 `convert_unary` 聚合时间的主体；`convert_unary<float, __half>` 为 7,030 次、12.465 ms，`convert_unary<float, __nv_bfloat16>` 为 730 次、10.016 ms，`convert_unary<__half, float>` 为 10 次、0.069 ms。

代码检查显示 SmolVLA 权重默认常驻 BF16，但 `mm_w()` 默认写入 `GGML_PREC_F32`；这与主要的 BF16→F32 转换相符，形成“原生 BF16 GEMM 可减少重复权重扩展”的可证伪假设。该假设随后由优化 01 的同条件性能比较支持，具体结果和精度风险见 [OPTIMIZATION-01.md](OPTIMIZATION-01.md)。

### Nsight Compute 细粒度验证

使用 `--target-processes all`、`--nvtx-include 'vla.profile@predict rid=6/'` 和 `--nvtx-push-pop-scope process`，分别对 `convert_unary` 与代表性 CUTLASS `Kernel2` GEMM 成功采集 18-pass `detailed` 报告。两个报告均记录了目标 kernel，且范围元数据显示其位于 `vla.profile` 的 `request rid=6` / `predict rid=6`。

`ncu-convert-client/` 是本次 NCU 环境下唯一保留的固定客户端正确性证据：它执行 5 次 warm-up 与 1 次计时请求，输出为 50×32、全有限并与 reference action 按位一致。NCU replay 显著扰动计时，因此该客户端时间只用于采集与正确性记录，**不得**用于正式性能基线或优化前后比较。GEMM 的验收结论只依据其权威 NCU 硬件报告，不主张存在可与该报告严格关联的客户端正确性工件。

| 目标 kernel | 关键 NCU 指标 | 验收结论 |
|---|---|---|
| `convert_unary<float, __half>` | DRAM 吞吐 `268.92 GB/s`，为峰值的 `45.30%`；计算吞吐 `41.05%`；实现占用率 `78.21%` | 不支持“这个 F32→F16 kernel 已饱和全局内存带宽”的假设。它不是上节主要的 BF16→F32 转换类型，不能将其局部计数器直接外推为后者的根因。 |
| `cutlass_80_tensorop_s1688gemm_64x64_16x6_tn_align4` `Kernel2` | DRAM 吞吐 `535.68 GB/s`，为峰值的 `89.42%`；计算吞吐 `38.67%`；理论/实现占用率为 `16.67%` / `15.64%` | 非计算受限。动态 shared memory 将理论占用率限制为 `16.67%`；192 个 block 形成一整 wave 加 48-block 尾 wave，NCU 估计该尾部在均匀 block 时最多可占 kernel 时间的 50%。因此该实例同时存在低占用、形状/launch 与尾 wave 约束，并具有高 DRAM 利用率。 |

细粒度结论只适用于这两个代表性 kernel 和冻结配置；它不等价于整个 `vla::predict()` 都由某一单项资源限制，也不构成已实施优化的证据。下一步应只选择与上述证据相符的最小优化，并在同一冻结条件下完成正确性回归和前后性能测量。

`CUDA_GRAPH_NODE_EVENTS` 已导出，但其事件时长均为 0，不能用于 node 时间汇总；本轮以 NVTX 范围内的 CUDA Runtime 和 `CUPTI_ACTIVITY_KIND_KERNEL` 时间为准。

## 工件索引

原始 profile 工件、用途和大小见 [`raw/vla-steady-r2/README.md`](raw/vla-steady-r2/README.md)。字节身份只在 [`raw/profile-artifacts.sha256`](raw/profile-artifacts.sha256) 中维护。
