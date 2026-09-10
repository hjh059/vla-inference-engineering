# 路径选择与辅助证据记录

> 状态：路径选择已完成。本文保留选定路径、候选回退路径与相关辅助证据；文件名沿用历史链接。

本文只记录正式基线路径的选择理由、候选回退路径和辅助外部资料。公开 Issue、README、benchmark 或维护者讨论可以帮助解释场景约束、已有路径或已知限制，但不是本项目的性能或正确性证据。选定路径的冻结条件与结果只链接到对应的正式实验记录，不在本文重复维护。

## 决策准则

选择首个正式基线时，仅评估能否在资源边界内完成一条离线、单进程、单客户端顺序请求的 VLA 推理路径；不连接真实机器人，也不形成控制闭环。候选必须：

1. 有可核对的模型、checkpoint、许可证和运行说明；
2. 能加载候选 checkpoint，接收观测与语言指令，并产出其接口定义的动作张量或动作序列；
3. 与候选设备、任务场景和资源边界兼容，并能在授权资源内完成有限 smoke test 与后续正式测量；
4. 能定义接口/结构正确性标准和单请求端到端主指标；
5. 能取得足以定位一个主要瓶颈的端到端测量与 GPU profile 工件。

候选设备还须满足[正式候选设备筛选条件](deployment-platforms.md#正式候选设备筛选条件)。训练、真实机器人动作、在线控制闭环、多请求吞吐和跨设备性能比较不属于首次选路范围。已选路径的实际设备、模型、输入、正确性标准、测量协议与结果以正式 [Configuration ID](../../experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md) 及其 [RESULTS.md](../../experiments/a10-cuda-smolvla-20260831-r1/RESULTS.md) 为准。

## 候选目录

| ID | 当前决策 | 详情 |
|---|---|---|
| C-01 | `Selected baseline` | [SmolVLA-LIBERO GGUF + vla.cpp](#c-01smolvla-libero-gguf--vlacpp) |
| C-02 | `Candidate`（P2） | [π0.5-LIBERO GGUF + Embodied.cpp](#c-02π05-libero-gguf--embodiedcpp) |
| C-03 | `Candidate`（P3） | [GR00T N1.7 LIBERO + NVIDIA Isaac-GR00T](#c-03groot-n17-libero--nvidia-isaac-groot) |

## 候选组合详情

### C-01：SmolVLA-LIBERO GGUF + vla.cpp

- **正式配置与结果**：模型制品、运行时版本、构建选项、输入、固定 noise 及正确性和测量协议以 [`a10-cuda-smolvla-20260831-r1`](../../experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md) 为准；可复现结果见 [RESULTS.md](../../experiments/a10-cuda-smolvla-20260831-r1/RESULTS.md)。
- **选路期外部入口**：[vla.cpp 仓库](https://github.com/VinRobotics/vla.cpp)、[架构说明](https://github.com/VinRobotics/vla.cpp/blob/main/docs/ARCHITECTURE.md)和[项目页面](https://fai-modelopt-tech.github.io/vla-cpp.github.io/)。
- **许可与边界**：模型与运行时代码的许可证文字仍须在发布或分发前按冻结 revision 复核。固定输入下的数值可重复性不代表动作语义、LIBERO 任务成功率或机器人控制正确性。

### C-02：π0.5-LIBERO GGUF + Embodied.cpp

- **模型与固定制品**：`SEU-PAISys/Embodied.cpp@db7fb09cfcdd90cd12389f290c582e34bb483331` 中的 `pi05_libero_finetuned_v044/pi05.gguf` 与 `pi05-mmproj.gguf`，上游为 `lerobot/pi05_libero_finetuned_v044@8e174154ef5f6c60a8da12ae99c303d8963138c1`，模型卡标注约 4B 参数。
- **固定部署路径**：`SEU-PAISys/Embodied.cpp@87bafdcdda485efb4e0552ee9c979a009970ad6b`，计划采用 `MODEL_BUILD_VLA_PI05`、CUDA 和 `CMAKE_CUDA_ARCHITECTURES=86` 构建 `vla-server`；输入由其 LIBERO π0.5 配置对应的固定观测提供。
- **固定任务边界**：从一个固定 LIBERO 样本冻结多视角图像、机器人状态和文本指令，保留服务端原始连续动作输出；不运行完整 episode，也不以任务成功率作为首次 smoke 的门槛。
- **适配依据**：运行时文档声明支持 CUDA 12.x、π0.5、LIBERO 客户端和 Ampere 构建。其公开相对性能声明缺少本机绝对显存数据，因此不视为 A10 已适配的证据。
- **许可与未知项**：运行时代码为 Apache-2.0；转换制品页标注 `other`，上游 checkpoint 标注 Gemma 许可，必须在下载前确认组合使用条款。A10 24 GB 峰值显存、转换制品完整性以及服务端与固定输入的最小依赖集合尚未验证；仓库仍在快速演进。
- **当前决策**：第二顺位。只有当前正式路径的 GPU 工作负载过轻，或其输出契约无法满足测量目标时，才优先转向该路径。

### C-03：GR00T N1.7 LIBERO + NVIDIA Isaac-GR00T

- **模型与固定制品**：`nvidia/GR00T-N1.7-LIBERO` 的 `libero_10` 子目录，固定到该目录模型上传 revision `94a57dda8a41d792233ef415459864719d10691d`。后续只取官方部署文档列出的推理文件，不取 optimizer 等训练状态。
- **固定部署路径**：`NVIDIA/Isaac-GR00T@b9955401d50c92a29258732e3ad6ccd579f1bdc0` 的官方 PyTorch standalone inference；固定 `LIBERO_PANDA`、`demo_data/libero_demo`、trajectory 0 和首个测量步。
- **固定任务边界**：单进程加载后，对一份固定离线 demo 观测执行有限推理，保存动作、相对真值的 MSE/MAE 与原始时间记录；不启动模拟器，不执行完整轨迹评测。
- **适配依据**：官方硬件说明将推理最低显存列为 16 GB+、CUDA 12.6+，模型卡列出 Ampere 支持；当前 A10 24 GB、CUDA 12.8 满足这些公开下限，但不等于本地峰值显存已验证。
- **许可与未知项**：代码为 Apache-2.0，模型使用 NVIDIA Open Model License；下载前需复核固定 revision 的实际文件及适用条款。Python/PyTorch 依赖安装、A10 峰值显存和端到端延迟均未验证。
- **当前决策**：第三顺位，作为第一方参考与回退路径。它的准确性检查接口最完整，但部署依赖面和环境准备成本高于前两条 C++ 路径。

## 路径选择结果与回退规则

1. 当前正式比较唯一依据为 [`a10-cuda-smolvla-20260831-r1`](../../experiments/a10-cuda-smolvla-20260831-r1/CONFIGURATION.md)。
2. 仅当当前正式路径的 profile 证明确实缺少可分析的主要 GPU 工作负载时，才重新评估 C-02。
3. C-03 继续保留为第一方参考/回退路径；当前不继续扩展或 smoke 其他候选。

路径选择不是性能结论。性能、正确性和 profile 结论均以正式配置中冻结的输入、随机性、环境和测量方法为准。

## 决策值

- `Candidate`：信息待核对或可进行有限可行性验证；
- `Selected path`：有限 smoke 已通过并选为正式基线路径，但 Configuration ID 尚未冻结；
- `Selected baseline`：已冻结为正式实验配置；
- `Deferred`：当前资源、兼容性或场景不合适；
- `Reference only`：仅保留为场景、指标或限制的辅助资料。

当前进度与下一步见[端到端部署与性能优化计划](../plans/phase-0.md)；证据强度和测量规则见[文档导航](../README.md#证据强度)，交付要求见[部署与性能优化范围](../project/scope.md#必须产出)。
