# VLA 模型候选调研

> 状态：Problem Discovery 输入，不是生产模型决策。
>
> 原调研基准日期：2026-07-28。模型版本、许可证、部署路径和维护状态必须在进入复现前按固定 commit 重新核对。

Problem Discovery 不以选出 primary model 为目标。只有为复现一条现有工作流所必需时，才选择当前资源可运行的模型；选择结果不能表达为生产选型。

## 评估约束

用于最小复现的模型应尽量满足：

- 接收图像、语言指令和机器人状态；
- 生成与目标机器人匹配的 Action Chunk；
- 具有可复现的官方参考实现；
- 能为被选 Issue 固定受影响版本、问题原生路径和可解释的正确性基线；
- 能在当前资源边界内复现，或满足为被选 Issue 预注册的性能、内存和能效指标；
- 具有可审计的代码、权重、底座、Tokenizer 和数据许可证。

## 问题调查优先级

该顺序用于控制 Issue 调查和复现成本，不是模型排名或生产选型。第一问题池以 LeRobot/SmolVLA/LIBERO 为默认起点；Isaac-GR00T 是第二问题池；其他模型只在替代方案反证或第一、第二问题池无法形成可执行路径时进入。

### 第一问题池：SmolVLA 450M

LeRobot Issues 是第一问题来源，SmolVLA/LIBERO 是默认调查基线，优先用于异步推理、动作语义、Processor、GPU 内存和控制端消费问题。具体 Issue 未筛选前，不把这些维度写成已确认缺陷。

优势：

- 规模紧凑，适合验证低成本边缘部署；
- LeRobot 提供训练、评测和异步推理入口；
- 结构覆盖图像、语言、状态和 Action Chunk。

风险：

- 代码许可证不自动覆盖权重、底座和数据；
- Tokenizer、Processor、导出、采样和任务效果需要按被选 Issue 验证；
- 模型规模可能限制任务泛化。

### 第二问题池：NVIDIA GR00T N1.7 3B

优势：

- 官方提供 ONNX/TensorRT 全量或分段部署路径；
- 视觉语言骨干与 Action Head 可以分图；
- 与 NVIDIA VLA 工具链和高性能边缘平台路线一致。

风险：

- 内存、算力、功耗和平台成本较高；
- 官方部署路径仍可能保留 PyTorch 算子；
- 上游验证平台和本项目目标平台可能不同；
- gated backbone、补丁和传递依赖需要分别留证。

GR00T 重点用于调查 ONNX/TensorRT、Jetson、跨硬件正确性和性能。考虑当前硬件成本，它不作为默认首个复现路径。

### 替代与反证候选

#### X-VLA 0.9B

优势：

- 公开代码和权重标注 Apache-2.0；
- 面向跨 embodiment，规模仍有边缘部署可能。

风险：

- 生态和生产部署积累较少；
- action space、算子导出、任务复现和维护活跃度需要验证。

#### OpenVLA-OFT 7B

优势：

- Action Chunk 和 LIBERO 评测路径成熟；
- 可作为较大模型的参考上限。

风险：

- 推理内存和边缘部署余量有限；
- 较大模型的本地复现、部署和低延迟验证成本高；
- 代码许可证不自动覆盖底座、Tokenizer、数据和派生权重。

## 暂不进入首轮

- π0/π0.5：当前不属于第一或第二问题池；只有被选 Issue、相关 PR 或替代方案反证需要时才进入；
- 世界模型：不属于首版 VLA 主链路；
- 其他研究型 VLA：只有当前问题池和替代候选均无法形成可执行路径时才扩展。

## 许可证检查

每个候选分别记录：

- 仓库代码；
- 模型权重和底座模型；
- Tokenizer、Processor 和配置；
- 训练、微调、校准和评测数据；
- 仿真场景、机器人资产和演示素材；
- Plugin、第三方算子和容器。

证据至少包含 SPDX 标识、原始链接、版本或 commit、许可证文本哈希、Notice/署名要求、再分发条件和商业使用限制。关键资产来源未知、限制研究用途、禁止必要再分发或条款冲突时，不进入产品评测。

## Problem Discovery 使用方式

本文只维护候选事实和许可证风险，不维护 primary/fallback 流程、生产门禁或最终结论。复现顺序和证据要求见 [Problem Discovery 计划](../plans/phase-0.md)。

## 资料

- [LeRobot](https://github.com/huggingface/lerobot)
- [SmolVLA](https://huggingface.co/blog/smolvla)
- [X-VLA](https://github.com/2toinf/X-VLA)
- [NVIDIA Isaac GR00T](https://github.com/NVIDIA/Isaac-GR00T)
- [OpenVLA-OFT](https://github.com/moojink/openvla-oft)
