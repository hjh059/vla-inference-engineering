# VLA Runtime 与推理后端生态调研

> 状态：Problem Discovery 输入，不是生产 Runtime 或后端决策。
>
> 调研更新日期：2026-08-02。生态变化很快，所有能力必须在执行前固定仓库 commit 并重新核对。

## 调研目的

本文保存已经存在的 VLA 部署 Runtime 和官方部署路径事实。只有被选 Issue 的根因、修复或替代方案反证涉及 Runtime、跨实现正确性、C++ 部署或底层后端时，才从本文选择相关工具；Problem Discovery 不要求每个问题经过 C++ Runtime 对照。

当前不能假设：

- 市场缺少 C++ VLA Runtime；
- 官方方案只有 Python eager inference；
- 现有方案缺少 Processor、动作生命周期、ROS2 或故障恢复；
- 现有方案无法完成仿真或机器人闭环；
- 必须自行组合 TensorRT、ONNX Runtime 或 LibTorch 才能形成部署路径。

未在 README 中找到某项能力时，证据等级为 `Unknown`，不记录为“不支持”。

## 证据等级

| 等级 | 含义 |
|---|---|
| `Official claim` | 官方仓库、文档或论文明确声明 |
| `Code inspected` | 已固定 commit 并检查相关代码和调用路径 |
| `Locally reproduced` | 已在固定本地环境运行成功 |
| `Locally measured` | 已保存原始输入、日志和测量结果 |
| `Unknown` | 尚无足够证据 |

当前表格主要是 `Official claim`。仓库内尚无本地复现结果。

## 现有 Runtime 与官方路径

### vla.cpp

官方仓库：[VinRobotics/vla.cpp](https://github.com/VinRobotics/vla.cpp)

官方文档声明：

- 基于 `llama.cpp`/ggml 的 C++ VLA 推理引擎；
- 推理数据面不需要 Python 或 PyTorch；
- 支持 SmolVLA、π0/π0.5、GR00T N1.5/1.6/1.7、OpenVLA-OFT 等多种模型；
- 提供 checkpoint 到 GGUF 的转换、CPU/CUDA/Metal、CLI 和 ZeroMQ/Protobuf server；
- 提供 LIBERO 和 SimplerEnv 的端到端评测客户端；
- 公布多个设备上的 client-side 延迟结果。

公开文档同时显示：

- one-shot CLI 接收客户端生成的 language token ID；
- `predict` 输出仍是按训练统计量归一化的动作，由调用方反归一化；
- `vla-server` 使用同步 ZeroMQ REQ/REP；
- 仿真客户端和环境仍使用 Python。

这些边界可能形成 Processor 一致性、动作语义或闭环调度的调查线索，但尚未证明会造成实际问题。ROS2、动作有效期、进程退出后的消费端行为和生产恢复能力当前均为 `Unknown`。

相关资料：

- [架构说明](https://github.com/VinRobotics/vla.cpp/blob/main/docs/ARCHITECTURE.md)
- [项目页面](https://fai-modelopt-tech.github.io/vla-cpp.github.io/)

### Embodied.cpp

官方仓库：[SEU-PAISys/Embodied.cpp](https://github.com/SEU-PAISys/Embodied.cpp)

官方文档声明：

- 面向 VLA 和 World-Action Model 的 C++ 推理 Runtime；
- 使用 GGUF，在 CPU、CUDA GPU 和其他异构硬件上运行；
- 当前支持 pi0.5、HY-VLA、GR00T N1.7 和 LingBot-VA 等路径；
- 提供模型转换脚本、ZeroMQ/Protobuf server、LIBERO 和 RoboTwin 评测；
- `adapter/` 用于把传感器或仿真输入转换为模型输入。

官方 Roadmap 同时明确：

- Runtime 架构仍在 active construction；
- 更模块化的 Runtime、real-time chunking 和 VLA caching 仍是计划项；
- 更多模型和硬件后端仍在扩展。

这些公开状态适合用于选择最小复现问题，但不能直接证明现有实现存在用户可感知缺陷。Processor 完整性、ROS2、动作生命周期、timeout 和退出恢复行为当前均为 `Unknown`。

相关资料：

- [Embodied.cpp 论文](https://arxiv.org/abs/2607.02501)
- [模型制品](https://huggingface.co/SEU-PAISys/Embodied.cpp)

### NVIDIA Isaac GR00T 官方部署

官方仓库：[NVIDIA/Isaac-GR00T](https://github.com/NVIDIA/Isaac-GR00T)

GR00T N1.7 官方部署指南声明：

- 提供 PyTorch、`torch.compile` 和 TensorRT 推理模式；
- 提供 ONNX 导出、TensorRT Engine 构建、PyTorch/TRT 正确性比较和 benchmark；
- 支持 dGPU、Jetson Thor、DGX Spark 和 Jetson Orin 的特定路径；
- 提供 LIBERO 闭环评测命令和官方样例结果。

公开限制包括：

- `full_pipeline` 仍保留 `embed_tokens`、`masked_scatter`、`get_rope_index`、VLLN 等轻量 PyTorch glue ops；
- Orin 当前只支持 DiT-only TensorRT，backbone 仍运行于 PyTorch；
- Engine 与 GPU 架构和静态 batch 绑定；
- 官方部署脚本和编排主要是 Python。

因此，“GR00T 没有优化部署路径”不成立；“GR00T 官方路径已经提供纯 C++ 完整数据面”也不能由当前资料支持。是否对候选用户造成集成、性能或交付影响必须本地复现。

相关资料：

- [GR00T Deployment & Inference Guide](https://github.com/NVIDIA/Isaac-GR00T/blob/main/scripts/deployment/README.md)

## 已调查事实对照

以下矩阵只总结当前官方资料，不代表代码审计或本地验证：

| 维度 | vla.cpp | Embodied.cpp | GR00T 官方部署 |
|---|---|---|---|
| 主要模型 | 多个公开 VLA | pi0.5、HY-VLA、GR00T、LingBot-VA 等 | GR00T N1.7 |
| C++ 模型数据面 | Official claim：是 | Official claim：是 | Unknown；官方编排主要为 Python/Torch |
| Processor/Tokenizer | 客户端提供 token；完整一致性 Unknown | adapter 存在；完整一致性 Unknown | 官方 Processor 路径存在；纯 C++ 边界 Unknown |
| checkpoint 转换 | GGUF converters | GGUF converters | ONNX export + TRT build |
| 正确性验证 | benchmark/eval scaffold；跨参考细节待核查 | eval clients；跨参考细节待核查 | 官方 PyTorch/TRT verify |
| 动作语义 | 输出归一化动作；消费端语义待核查 | Unknown | Processor/statistics 路径存在；控制语义待核查 |
| 仿真 | LIBERO、SimplerEnv | LIBERO、RoboTwin | LIBERO |
| 真实机器人接入 | Unknown | 官方声称 deployment adapters；具体路径待核查 | 存在 GR00T 生态路径；当前选定控制链路 Unknown |
| 并发和过期结果 | 同步 REQ/REP；其他 Unknown | real-time chunking/caching 在 Roadmap | 服务与异步路径待固定版本核查 |
| Runtime 退出/恢复 | Unknown | Unknown | Unknown |
| ROS2 | Unknown | Unknown | Unknown |
| 性能 | 官方多设备延迟表 | 论文/官方结果；需固定配置 | 官方多平台 PyTorch/TRT 表 |
| 测试与 CI | 仓库存在 tests/CI；覆盖度待核查 | 待代码核查 | 待限定部署路径核查 |
| 制品 | GGUF | GGUF + HF artifacts | checkpoint、ONNX、TRT engines |
| 许可证 | Runtime Apache-2.0；模型等分别核查 | Runtime Apache-2.0；模型等分别核查 | 代码、模型和传递资产分别核查 |
| 成熟度 | 新兴项目，需按 commit 和复现评估 | 官方明确仍在 active construction | 官方路径持续演进，平台差异明显 |

本文只维护已经调查得到的外部项目事实和 `Unknown`。只有被选 Issue 需要时才继续代码检查或复现；不在本文维护候选后端列表、选型流程、生产门禁或架构结论。
