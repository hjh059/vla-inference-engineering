# 路径选择与辅助证据记录

> 状态：可行性选择台账。本文文件名沿用历史链接。

本文只记录选择首个正式部署基线所需的候选路径、公开资料和直接证据。公开 Issue、README、benchmark 或维护者讨论可以帮助解释场景约束、已有路径或已知限制，但不是本项目的性能或正确性证据。

## 首个候选任务场景

当前先验证固定离线输入的单请求 VLA 推理，不连接真实机器人，也不形成控制闭环。

| 字段 | 当前定义 |
|---|---|
| 可观察目标 | 现有部署路径能够加载固定 checkpoint，接收一组固定观测与语言指令，并产出该路径定义的动作张量或动作序列 |
| 执行边界 | 单进程、单请求、离线推理；优先使用已通过 profiling 可行性验证的阿里云 A10 24 GB、Ubuntu 24.04 候选环境 |
| 固定输入 | 一组模型所需的固定图像、语言指令、必要的机器人状态及 initial noise/随机策略；具体资产和随机输入边界在选择模型路径时确定并记录来源与许可证 |
| Smoke 正确性 | 进程完成模型加载和一次推理；输出类型与形状符合固定版本接口；数值不存在非预期的 `NaN`/`Inf`；有官方参考输出时按其方法比较 |
| 正式正确性 | 在选择路径后、查看优化结果前，根据上游参考和任务语义冻结；当前为 `Unknown` |
| 主指标类型 | 单请求端到端延迟，从固定原始输入进入现有部署路径到得到可消费的动作输出 |
| 资源约束 | 以[部署平台与资源策略](deployment-platforms.md#当前云端候选环境)为准；正式候选路径需在单张 A10 的实测 `23028 MiB` 显存内运行，并兼容 CUDA Toolkit 12.8；本机 GTX 1060 和 CPU 路径只用于辅助检查 |
| 当前排除 | 训练、真实机器人动作、在线控制闭环、多请求吞吐和跨设备性能比较 |

该定义只用于筛选首条可运行路径；精确输入资产、正确性阈值、预热、重复次数和测量方法在创建正式 `Configuration ID` 时冻结。

## 候选路径记录

| ID | 模型/checkpoint | 现有部署路径 | 候选设备与任务 | 公开资料/固定版本 | 兼容性或 smoke 证据 | 选择依据 | 决策 |
|---|---|---|---|---|---|---|---|
| C-01 | SmolVLA-LIBERO 0.6B；GGUF | vla.cpp；C++/CUDA CLI | 历史 A10 24 GB；单图像、固定 token、零状态，生成 action chunk | [vla.cpp `7710596`](https://github.com/VinRobotics/vla.cpp/tree/7710596ecf2349a7dcbb41b4f9ce042025cbd435)；[GGUF `349851c`](https://huggingface.co/vrfai/smolvla-libero-gguf/tree/349851cd4e872d199c75e1802f037c8026767190)；[上游 `6721902`](https://huggingface.co/HuggingFaceVLA/smolvla_libero/tree/6721902bc4d61e50a3bfdb11dfb4cb626f05d102) | `Locally reproduced`：A10/CUDA 上独立执行两次，均为 `rows=50 bad_width=0 nonfinite=0`；因 CLI 未固定 initial noise，两次动作数值不同 | 路径可行性历史证据；当前正式实验见 `a10-cuda-smolvla-20260831-r1` | Historical feasibility |
| C-02 | π0.5 LIBERO 4B；GGUF | Embodied.cpp；C++/CUDA `vla-server` | 阿里云 A10 24 GB；固定 LIBERO 观测与指令，生成连续动作 | [Embodied.cpp `87bafdc`](https://github.com/SEU-PAISys/Embodied.cpp/tree/87bafdcdda485efb4e0552ee9c979a009970ad6b)；[GGUF `db7fb09`](https://huggingface.co/SEU-PAISys/Embodied.cpp/tree/db7fb09cfcdd90cd12389f290c582e34bb483331/pi05_libero_finetuned_v044)；[上游 `8e17415`](https://huggingface.co/lerobot/pi05_libero_finetuned_v044/tree/8e174154ef5f6c60a8da12ae99c303d8963138c1) | `Official claim`：支持 CUDA 12.x、Ampere 和 π0.5；A10 峰值显存 `Unknown`；`Not run` | 较重模型可能提供更有价值的 GPU profiling 面，但制品许可与显存风险更高 | Candidate（P2） |
| C-03 | GR00T N1.7 LIBERO `libero_10`；约 3B BF16 | NVIDIA Isaac-GR00T；官方 PyTorch standalone inference | 阿里云 A10 24 GB；`LIBERO_PANDA`、固定 demo trajectory 0/首步，输出动作并与真值比较 | [Isaac-GR00T `b995540`](https://github.com/NVIDIA/Isaac-GR00T/tree/b9955401d50c92a29258732e3ad6ccd579f1bdc0)；[`libero_10` `94a57dd`](https://huggingface.co/nvidia/GR00T-N1.7-LIBERO/tree/94a57dda8a41d792233ef415459864719d10691d/libero_10) | `Official claim`：最低 16 GB+、CUDA 12.6+，并列出 Ampere 支持；`Not run` | 第一方路径和误差输出完整，可作参考/回退；Python 依赖面大于前两条 C++ 路径 | Candidate（P3） |

当前记录数：3。

## 候选组合详情

### C-01：SmolVLA-LIBERO GGUF + vla.cpp

- **模型与固定制品**：`vrfai/smolvla-libero-gguf@349851cd4e872d199c75e1802f037c8026767190`，上游为 `HuggingFaceVLA/smolvla_libero@6721902bc4d61e50a3bfdb11dfb4cb626f05d102`；模型卡标注约 0.6B 参数；当前固定 revision 已删除独立 `mmproj`，唯一必需的 `smolvla-libero.gguf` 是约 1.13 GB 的单一自包含制品。
- **固定部署路径**：`VinRobotics/vla.cpp@7710596ecf2349a7dcbb41b4f9ce042025cbd435`；已使用 Release、`GGML_CUDA=ON`、`GGML_CUDA_GRAPHS=ON` 和 `CMAKE_CUDA_ARCHITECTURES=86` 构建。CMake 通过 `FetchContent` 自动获取 `llama.cpp` 标签 `b9866`，对应完整 commit `75a48a90559abf65df3f3616a53bb16e5afb9d07`；不需要 `patches/patch.sh` 或 Git submodule。
- **固定任务边界**：仓库 `assets/front.jpg`、token `1,100,200,2` 和默认零状态；通过 `vla-cli` 执行单进程、单请求推理并记录原始 action chunk，不接机器人或模拟器。该 CLI 未提供 initial noise，固定实现会为每个模型进程从随机初始化的 RNG 采样 `N(0,1)` noise，因此本次 smoke 的动作数值不是固定输入下的确定性输出。
- **适配依据**：项目文档声明支持 CUDA 12.x、Ampere 和 SmolVLA，公开示例直接提供单次 CLI 路径。公开性能数字只作为资源可行性线索，不作为本项目基线。
- **许可与未知项**：运行时代码仓库的 `LICENSE.md` 为 Apache-2.0，上游及 GGUF 模型卡也标注 Apache-2.0；GGUF 模型卡中关于运行时许可的文字与仓库不一致，发布或分发前仍须按固定 revision 复核。当前示例 token 未证明动作语义或 LIBERO 任务成功率，模型是否能形成足以支持后续优化的主要 GPU 热点也尚未验证。
- **当前决策**：该 smoke 仅保留为历史可行性证据；当前主机上的正式实验已迁移到 [`a10-cuda-smolvla-20260831-r1`](../../experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md)，不得混用两者的时间、输出或制品。

#### 2026-08-11 有限 smoke 证据

| 项目 | 观察结果 |
|---|---|
| 证据等级 | `Locally reproduced`；相同命令独立执行两次 |
| 运行设备与后端 | NVIDIA A10，Compute Capability 8.6；实际后端为 CUDA device 0 |
| 模型制品 | 单一自包含 `smolvla-libero.gguf`，约 1.13 GB，无独立 `mmproj` |
| 模型 SHA-256 | `6fb2d475c98b4c2cef3e27c4eff4e67b483740cbf983fff320a3b8a5e5f74fe8` |
| 输入图像 SHA-256 | `e6f0738ce9184be421dbade5a73900fb33e44c288c475d3985054fbbc4dfb7cf`（`assets/front.jpg`） |
| 可执行文件 SHA-256 | `126c401f92252e5f3e688c70815ccf2da2346a59423eabdf284ff6dddd3e4579`（`build-cuda/vla-cli`） |
| 原始记录 | [环境、完整日志、工件索引与复核摘要](../../experiments/feasibility/c-01-smoke-20260811/README.md) |
| 模型结构 | state 维度 8、action 维度 7、action chunk 长度 50 |
| CLI 输出契约 | 每行 32 个值，其中后 25 个为对齐填充；共 50 行 |
| 自动检查 | 两次均为 `rows=50 bad_width=0 nonfinite=0`；350 个有效动作值全部不同，平均绝对差约 `0.144618051`、最大绝对差约 `2.021502` |
| 资源观察 | 权重缓冲区约 1069.0 MiB；日志内最高已报告 GPU 显存约 1346.2 MiB |
| 非基线时间 | 首次进程总耗时约 1.79 s，包含进程启动、模型加载和推理 |

该证据支持的结论仅为：声明环境能够重复加载单文件 GGUF，通过 CUDA 完成相同观测条件的单请求推理，并产生结构正确且无 `NaN`/`Inf` 的动作输出。由于 initial noise 未固定，证据不支持动作数值可重复。1.79 s 和 1.76 s 均不代表稳态推理性能；smoke 也不支持动作语义正确或 LIBERO 任务成功率结论。

### C-02：π0.5-LIBERO GGUF + Embodied.cpp

- **模型与固定制品**：`SEU-PAISys/Embodied.cpp@db7fb09cfcdd90cd12389f290c582e34bb483331` 中的 `pi05_libero_finetuned_v044/pi05.gguf` 与 `pi05-mmproj.gguf`，上游为 `lerobot/pi05_libero_finetuned_v044@8e174154ef5f6c60a8da12ae99c303d8963138c1`，模型卡标注约 4B 参数。
- **固定部署路径**：`SEU-PAISys/Embodied.cpp@87bafdcdda485efb4e0552ee9c979a009970ad6b`，计划采用 `MODEL_BUILD_VLA_PI05`、CUDA 和 `CMAKE_CUDA_ARCHITECTURES=86` 构建 `vla-server`；输入由其 LIBERO π0.5 配置对应的固定观测提供。
- **固定任务边界**：从一个固定 LIBERO 样本冻结多视角图像、机器人状态和文本指令，保留服务端原始连续动作输出；不运行完整 episode，也不以任务成功率作为首次 smoke 的门槛。
- **适配依据**：运行时文档声明支持 CUDA 12.x、π0.5、LIBERO 客户端和 Ampere 构建。其公开相对性能声明缺少本机绝对显存数据，因此不视为 A10 已适配的证据。
- **许可与未知项**：运行时代码为 Apache-2.0；转换制品页标注 `other`，上游 checkpoint 标注 Gemma 许可，必须在下载前确认组合使用条款。A10 24 GB 峰值显存、转换制品完整性以及服务端与固定输入的最小依赖集合尚未验证；仓库仍在快速演进。
- **当前决策**：第二顺位。只有 C-01 的 GPU 工作负载过轻，或其输出契约无法满足测量目标时，才优先转向该路径。

### C-03：GR00T N1.7 LIBERO + NVIDIA Isaac-GR00T

- **模型与固定制品**：`nvidia/GR00T-N1.7-LIBERO` 的 `libero_10` 子目录，固定到该目录模型上传 revision `94a57dda8a41d792233ef415459864719d10691d`。后续只取官方部署文档列出的推理文件，不取 optimizer 等训练状态。
- **固定部署路径**：`NVIDIA/Isaac-GR00T@b9955401d50c92a29258732e3ad6ccd579f1bdc0` 的官方 PyTorch standalone inference；固定 `LIBERO_PANDA`、`demo_data/libero_demo`、trajectory 0 和首个测量步。
- **固定任务边界**：单进程加载后，对一份固定离线 demo 观测执行有限推理，保存动作、相对真值的 MSE/MAE 与原始时间记录；不启动模拟器，不执行完整轨迹评测。
- **适配依据**：官方硬件说明将推理最低显存列为 16 GB+、CUDA 12.6+，模型卡列出 Ampere 支持；当前 A10 24 GB、CUDA 12.8 满足这些公开下限，但不等于本地峰值显存已验证。
- **许可与未知项**：代码为 Apache-2.0，模型使用 NVIDIA Open Model License；下载前需复核固定 revision 的实际文件及适用条款。Python/PyTorch 依赖安装、A10 峰值显存和端到端延迟均未验证。
- **当前决策**：第三顺位，作为第一方参考与回退路径。它的准确性检查接口最完整，但部署依赖面和环境准备成本高于前两条 C++ 路径。

## 路径选择结果与回退规则

1. C-01 的历史 smoke 已完成路径可行性选择；当前正式比较唯一依据为 [`a10-cuda-smolvla-20260831-r1`](../../experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md)。
2. 仅当 C-01 无法形成可重复的正式正确性/性能基线，或后续 profile 证明确实缺少可分析的主要 GPU 工作负载时，才重新评估 C-02。
3. C-03 继续保留为第一方参考/回退路径；当前不继续扩展或 smoke 其他候选。

路径选择不是性能结论。C-01 的代码、模型、观测输入、当前构建校验和和原始 smoke 日志已可核对，但 initial noise/随机策略、正式正确性标准、预热和测量方法仍待与这些校验和一起写入同一 Configuration ID；在此之前不得将 smoke 时间作为正式基线。

## 选择规则

选择首个正式基线时，只判断与本项目交付直接相关的事项：

1. 路径是否有可核对的模型、checkpoint、许可证和运行说明；
2. 是否与候选设备、任务场景和资源边界兼容；
3. 是否能定义可观察的正确性标准和主性能指标；
4. 是否能在授权资源内完成有限 smoke test 与后续正式测量；
5. 是否能得到足以定位一个主要瓶颈的端到端测量与 profile 工件。

候选表只维护选择或延期的直接依据。选定路径后，模型/checkpoint、软件版本、输入、正确性标准和测量方法只在[范围文档的正式基线配置](../project/scope.md#正式基线配置)中记录；信息不足时填写 `Unknown`。

## 决策值

- `Candidate`：信息待核对或可进行有限可行性验证；
- `Selected path`：有限 smoke 已通过并选为正式基线路径，但 Configuration ID 尚未冻结；
- `Selected baseline`：已冻结为正式实验配置；
- `Deferred`：当前资源、兼容性或场景不合适；
- `Reference only`：仅保留为场景、指标或限制的辅助资料。

正式执行步骤、证据规则和完成标准见[端到端部署与性能优化计划](../plans/phase-0.md)。
