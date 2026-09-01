# 端到端部署与性能优化计划

> 状态：进行中。当前主机的正式配置已完成端到端、正确性和性能基线；接下来采集隔离稳态 profile、确认主瓶颈并完成优化验证，不建设通用生产系统。

## 目标

本计划以[部署与性能优化范围](../project/scope.md#项目目标)为准。核心结果是在固定实验配置上以可复现证据说明：现有路径如何运行、正确性和性能基线是什么、主要瓶颈在哪里、优化为何有效，以及结果的适用边界。

## 当前状态

已完成：

- [x] 定义项目的证据等级与测量原则；
- [x] 整理待重新验证的历史设备信息；
- [x] 整理截至 2026-08-02 的外部路径资料摘录。
- [x] 定义候选任务场景、可用设备范围和主指标类型；
- [x] 核验本地设备的适用边界；
- [x] 验证阿里云 A10 候选环境的 CUDA、Nsight Systems、Nsight Compute 和硬件计数器访问能力。
- [x] 形成 3 组固定 revision 的模型/checkpoint—部署路径候选，记录兼容性依据、未知项和验证顺序。
- [x] 选择 SmolVLA-LIBERO GGUF + vla.cpp 作为正式基线路径；早期 smoke 未固定 initial noise，因此正式配置将 noise 纳入冻结输入。
- [x] 创建并冻结当前主机的正式 `Configuration ID`：[`a10-cuda-smolvla-20260831-r1`](../../experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md)。

尚未完成：

- [x] 在冻结配置下跑通并归档端到端部署路径；
- [x] 建立正确性与性能基线；
- [x] 采集范围隔离的 Nsight Systems profile；`vla-steady-r2` 已对 10 次稳定请求建立 NVTX 范围，见[结果记录](../../experiments/a10-cuda-smolvla-20260831-r1/RESULTS.md)；
- [ ] 仅依据 `vla-steady-r2` 的 `predict rid=3–12` 范围确认一个主要瓶颈；
- [ ] 实施最小优化；
- [ ] 完成优化后的正确性回归、前后测量和交付记录。

## 当前工作

后续执行以[正式配置](../../experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md)和[基线结果](../../experiments/a10-cuda-smolvla-20260831-r1/RESULTS.md)为权威入口。范围隔离 profile 已采集；下一步是只在 `predict rid=3–12` 中汇总 kernel 与 CUDA API，必要时以 Nsight Compute 验证候选硬件限制。确认主要瓶颈后，再实施最小优化并执行同条件正确性回归与性能比较。

### 隔离稳态 profile 采集方案

现有 `nsys-baseline` 覆盖加载、warm-up 和一次固定请求，且没有请求范围标记。对其 SQLite 的只读审查只能识别尾部的重复 CUDA Graph 调用模式，不能把候选时间段可靠地证明为某个完整请求；因此不得用其全局或候选区间汇总确认稳态主瓶颈。

#### 1. 在 vla.cpp 增加观测标记

CUDA 构建的 `vla-server` 已在 `vla.cpp` 中加入 NVTX 观测标记。每个已解析的有效请求都会在 `vla.profile` domain 中产生外层 `request rid=<id>` 范围；其中的 `vla::predict()` 产生嵌套 `predict rid=<id>` 范围。外层范围覆盖服务端验证、图像解码、推理、响应序列化和 REP reply，嵌套范围只覆盖模型预测。

NVTX 的原理是由应用进程在 CPU 时间线上写入带名称的 begin/end 事件。Nsight Systems 同时记录这些事件、CUDA Runtime API、CUDA Graph 与 GPU kernel/node 活动；同一请求范围内的活动因此可被精确筛选和汇总。`rid` 能将范围与服务端日志、固定客户端请求顺序对应起来。标记不会改变模型、输入、推理计算、响应内容或既有计时；但 Nsight 的追踪本身可能引入观测开销，所以 profile 延迟只能用于检查范围和稳定性，不能替代冻结协议下的 30 次性能基线。

#### 2. 总体流程

后续 profile 始终使用冻结的模型、输入、noise、线程数和 `vla-server` 路径：

1. 启动服务并等待模型加载完成。
2. 在未开启 Nsight collection 的条件下发送 5 次 warm-up。
3. 开启 `cuda,nvtx` trace，启用 CUDA Graph node 追踪，并导出 SQLite。
4. 发送 10 次无额外 warm-up 的顺序固定请求；这 10 次请求都带有 `request`/`predict` 观测范围。
5. 立即停止 collection，保存 profile、SQLite、10 样本 CSV、工具版本、完整命令和 SHA-256。

#### 3. 具体执行命令

以下命令在 `/root/vla-smoke-c01` 中执行。终端 A 用于启动服务，终端 B 用于请求，终端 C 用于控制 Nsight 会话。

终端 A：启动由 Nsight 会话管理、但尚未开始 collection 的服务。

```bash
cd /root/vla-smoke-c01

VLA_N_THREADS=16 nsys launch \
  --session-new vla-steady-r1 \
  --trace=cuda,nvtx \
  --cuda-graph-trace=node \
  --wait=primary \
  ./build-formal-a10-20260831/vla-server \
  --bind tcp://127.0.0.1:5555 \
  models/smolvla-libero.gguf
```

等待日志出现 `vla-server: bound to tcp://127.0.0.1:5555. ready.`。

终端 B：执行正式采集前的 5 次 warm-up。固定客户端要求 `--reps >= 1`，因此此命令额外发送的 1 次请求不进入 profile 或性能结论。

```bash
cd /root/vla-smoke-c01

./build-formal-a10-20260831/fixed_request_client \
  --addr tcp://127.0.0.1:5555 \
  --image src/vla.cpp/assets/front.jpg \
  --out /tmp/vla-steady-r1-warmup \
  --warmup 5 \
  --reps 1
```

终端 C：启动范围隔离的 CUDA/NVTX collection，并导出 SQLite。

```bash
cd /root/vla-smoke-c01

nsys start \
  --session vla-steady-r1 \
  --sample=none \
  --cpuctxsw=none \
  --export=sqlite \
  --output /tmp/vla-steady-r1-nsys
```

终端 B：collection 开始后立即发送 10 次稳定请求。

```bash
cd /root/vla-smoke-c01

./build-formal-a10-20260831/fixed_request_client \
  --addr tcp://127.0.0.1:5555 \
  --image src/vla.cpp/assets/front.jpg \
  --out /tmp/vla-steady-r1-profile-samples \
  --warmup 0 \
  --reps 10
```

终端 C：客户端结束后立即停止、关闭会话并记录校验和。

```bash
nsys stop --session vla-steady-r1
nsys shutdown --session vla-steady-r1

sha256sum \
  /tmp/vla-steady-r1-nsys.nsys-rep \
  /tmp/vla-steady-r1-nsys.sqlite \
  /tmp/vla-steady-r1-profile-samples/samples.csv
```

最后在终端 A 使用 `Ctrl-C` 停止服务。profile 输出先保留在 `/tmp`；归档时按实验工件规则使用 Git LFS 或外部存储，并在 Git 中保留索引与校验和。

验收条件为：SQLite 中恰有 10 个 `request` 与 10 个嵌套 `predict` 范围；每个 `predict` 范围均包含 CUDA Graph、Runtime 与 GPU kernel/node 活动；仅在这些范围内按请求和汇总两个层级统计 GPU busy、CUDA API 和 kernel/node 时间。profile 中的延迟只用于检查范围和稳定性，不能替代冻结协议下的 30 次性能基线。

若范围内的时间集中于少量 CUDA Graph node 或 kernel，再使用同一范围隔离方式做一次 Nsight Compute 采集，确认硬件限制后才选择最小优化；若 GPU busy 显著低于 `inference`，先检查 CPU 同步、CUDA API、图更新与数据搬运。

## 可行性范围控制

可行性验证默认从可用本地资源和最有希望的路径开始，不要求在 smoke test 前预设固定时长、费用上限或路径更换次数。为避免无限扩展候选范围，出现以下任一情况时，应在继续前单独评估新增投入是否合理，并记录继续、延期或停止的决定：

- 需要使用付费资源；
- 已因不兼容而反复切换候选路径；
- 预计后续工作将明显超出个人项目的合理投入。

未触发上述情况时，可在现有范围内直接执行有限 smoke test。

## 1. 可行性选择

可行性阶段的目的是选择正式基线，不进行正式性能调优，也不预设通用或生产架构。执行顺序：

1. 写明候选场景的可观察目标、约束和主指标类型；
2. 核对模型/checkpoint、现有部署路径、许可证、硬件与软件兼容性；
3. 只执行选择所需的有限 smoke test；
4. 记录路径选择依据、已知限制和未选择方案的直接原因；
5. 创建首个 `Configuration ID`，冻结后进入正式基线。

如果候选路径不兼容或无法在授权资源内运行，应记录兼容性事实并优先验证下一条最有希望的路径；反复切换路径时，按上述范围控制规则评估是否继续。

## 2. 建立正式基线

基线前必须按[范围文档的正式基线配置](../project/scope.md#正式基线配置)冻结可比性条件，并记录基线实现 revision。

执行顺序：

1. 运行现有路径并保存环境设置、命令、日志和输入资产；
2. 按任务定义建立正确性基线；
3. 预先固定主指标、预热、重复次数、样本汇总方式和异常处理；
4. 采集端到端性能与相关资源数据；
5. 保存原始测量结果和 profile 工件。

正式基线是优化前后比较的唯一参照；优化后的实现 revision 必须与基线实现 revision 成对记录。更换冻结条件时必须创建新的 `Configuration ID`。

## 3. 定位和优化主要瓶颈

1. 根据 profile、端到端分解和资源数据提出可证伪的主要瓶颈假设；
2. 选择仅覆盖该假设所需的最小优化；
3. 记录优化改变的路径、预期机制、潜在正确性/资源取舍；
4. 不以单次结果或未经测量的推测认定根因；
5. 不因优化需要扩展为通用 Runtime、跨模型框架或生产控制链路。

## 4. 验证与交付

优化完成后，在与基线相同的冻结条件、输入和测量方法下，比较已记录的基线与优化实现 revision：

1. 执行正确性回归；
2. 重新采集性能、资源和必要的 profile；
3. 对比原始结果和汇总指标，说明改进、波动及取舍；
4. 记录可复现命令、配置、实现范围、限制和未验证项。

## 证据规则

所有能力声明标记为以下等级之一：

| 等级 | 含义 | 可以支持的结论 |
|---|---|---|
| `Official claim` | 官方仓库、文档或论文明确声明 | 候选路径或公开限制，不代表本地可用 |
| `Code inspected` | 在固定 commit 和文件范围内检查代码 | 说明已检查实现，不代表整条路径已运行 |
| `Locally reproduced` | 固定输入和受控环境下重复观察到预期行为 | 该行为在声明环境中可重复 |
| `Locally measured` | 保存原始样本、日志、配置和测量方法 | 该配置下的定量结果 |
| `Profiled` | 保存分析工具、版本、采样/追踪范围和原始工件 | 已观测的热点或等待关系 |
| `Unknown` | 证据不足 | 不得表达为支持或缺失 |

`Locally` 表示本项目控制输入、执行和证据，不限定物理位置。单次构建或运行失败首先分类为配置、环境、资产、实现或资源问题，不能直接包装为模型或 Runtime 缺陷。

## 测量与判定原则

- 指标和正确性标准由选定场景推导，并在查看优化结果前固定；
- 保存原始日志、测量结果、profile 工件和配置；
- 性能报告明确端到端边界，按需报告延迟、吞吐、显存、CPU/GPU 利用率或其他资源指标；
- 在报告中写明预热、重复次数、汇总统计和异常样本处理；
- 优化后必须以相同配置执行正确性回归；
- 证据不足只能得出 `Insufficient evidence`，不能判定优化成功；
- 外部 Issue、公开 benchmark 和替代方案只在选择路径或解释限制需要时核查，不形成强制的全生态反证任务。

## 完成标准

计划完成时必须具备：

- 一份冻结的正式实验配置；
- 可重复的端到端运行命令和正确性基线；
- 优化前后的原始性能/资源结果及汇总方法；
- 支持主瓶颈判断的 profile 工件；
- 与该瓶颈对应的最小优化及因果说明；
- 优化后的正确性回归；
- 结果、取舍、适用范围和未验证项。

## 内容归属

| 内容 | 维护位置 |
|---|---|
| 项目动机和能力声明边界 | `docs/project/positioning.md` |
| 部署与优化边界、配置字段和交付物 | `docs/project/scope.md` |
| 执行步骤、证据和完成标准 | 本文 |
| 路径选择与外部辅助证据 | `docs/research/issue-candidates.md` |
| Runtime 与后端历史参考 | `docs/research/backend-candidates.md` |
| 历史设备信息与环境冻结规则 | `docs/research/deployment-platforms.md` |
| 实验记录、脚本、小型原始日志、指标、校验和及大型工件索引 | `experiments/`；规则见[实验记录与工件管理](../../experiments/README.md) |
| vla.cpp 基线与优化源码 | 个人 fork；基线 `e9e57ea734692bbcb34528fee16318e230ba1100`，优化使用新的 commit |
| 模型本体与构建产物 | 不提交；只在对应实验记录中保存来源、版本、构建配置、大小和 SHA-256 |
