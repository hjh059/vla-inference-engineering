# 候选模型制品与交付边界

> 状态：Deferred Candidate Design。本文保留此前的制品思考，不是当前 Problem Discovery 的实施范围。
>
> 只有稳定问题证明 checkpoint、Processor、Engine、Schema 或配置混用会造成明确影响，或者存在真实交付需求时，才重新评审原子制品、签名、激活和回滚设计。

## 原子制品

只有该候选方案被真实交付问题重新启用时，才定义原子制品集合；以下条目仅按实际使用的资产纳入，无关字段不强制。纳入集合的资产必须作为一个版本安装、验证、激活和回滚：

- 模型、Tokenizer、Processor 和归一化统计；
- 导出图、Engine、分图描述和模块精度；
- robot embodiment、Action Schema 和 safety config；
- calibration manifest 与黄金向量；
- 运行时库版本、SBOM、checksums 和签名。

禁止混用不同版本的模型、Processor、Engine、Schema 或安全配置。

## Manifest

Manifest 至少记录：

- 模型仓库、commit、权重和配置哈希；
- OS、driver、CUDA、cuDNN、TensorRT 和其他后端；
- GPU 架构、Engine compatibility、Plugin；
- 编译器、C++ ABI 和构建容器 digest；
- ROS2 distro 和精确包版本；
- 数据与许可证证据。

具体运行版本以锁文件和 manifest 为事实源，不在 ADR 或调研文档重复维护。

## 加载检查

运行时按以下顺序执行：

1. 验证签名和文件哈希；
2. 验证软件、硬件和 Schema 兼容性；
3. 运行黄金向量 smoke test；
4. 全部通过后进入 inactive；
5. 不兼容时拒绝激活，并保留上一已知良好版本。

不在机器人目标机现场重新导出 Engine。

## 发布流程

1. 在固定环境导出模型或子图；
2. 在对应平台 profile 构建 Engine；
3. 运行黄金向量、模块、集成和仿真回归；
4. 生成 manifest、SBOM、checksums 和签名；
5. 保存不可变制品；
6. 目标机执行离线预检和无动作 smoke test；
7. 显式激活；
8. 失败时原子回滚。

## 延后能力

设备级信任根、release counter、密钥轮换、撤销列表和 anti-rollback 只有在存在远程分发、非可信网络或正式设备运营需求时立项。它们不是 Phase 0 或首个垂直切片的阻断项。
