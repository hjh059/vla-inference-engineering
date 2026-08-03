# ADR-0013：候选构建系统

- 状态：Draft
- 分类：Conditional Constraint / Candidate Solution
- 日期：2026-07-27

> 解决真实 VLA 推理与机器人部署问题是项目动机；C++ 是已有能力和候选解决工具。CMake、ament_cmake、colcon、CUDA、ROS2 和 Jetson 是否属于最终实现，取决于被选问题、根因和最小修复。

## 背景

如果已验证问题需要自建 C++20、CUDA、ROS2 和 Python 参考环境，构建系统可能需要：

- 与 ROS2 和 C++/CUDA 生态兼容；
- 允许核心模块脱离 ROS2 单元测试；
- 支持 host、Jetson、sanitizer 和 release profile；
- 避免在起步阶段维护第二套构建系统或重复依赖来源。

## 候选

### CMake + ament_cmake + colcon

与 ROS2 和 C++/CUDA 生态原生兼容，目标机调试和团队理解成本低。

### Bazel

依赖图和远程缓存能力强，但会增加 ROS2、CUDA 和 Jetson 的适配成本。当前项目规模无法证明收益。

### CMake + Conan/vcpkg

能够管理部分 C++ 依赖，但不能替代 JetPack 和 ROS2 的系统依赖矩阵，会增加第二个包来源。

## 候选方案

- 使用 CMake 构建普通 C++/CUDA target；
- ROS2 包使用 `ament_cmake` 和 `colcon`；
- 使用 CMake Presets 管理 host、Jetson、sanitizer、coverage 和 release profile；
- contracts、runtime 和 safety 核心模块保持普通 CMake target，可脱离 ROS2 测试；
- Python 仅用于参考、导出、评测和仿真适配；
- Jetson 使用与 JetPack 绑定的 CUDA 和 TensorRT；
- 外部依赖固定 exact version、commit 或 container digest；
- CI 和发布构建禁止依赖浮动 branch。

如果该候选方案重新进入评审，编译器、CMake、ROS2 和系统库的实际版本应由锁文件和 system manifest 维护，不在本 ADR 固定易变化的版本号。

## 环境组织原则

实现阶段按需建立：

```text
CMakePresets.json
cmake/
containers/
locks/
```

只有实际使用的模型参考环境和目标平台 profile 进入主线维护。fallback 失败环境只归档 lock 和报告，不继续维护其生产依赖。

## 预期后果与重新评审条件

当前 Problem Discovery 不采用任何生产构建体系。若退出结论为 `Continue`，应首先遵循被选上游项目和最小解法的现有工具链；只有确需自建 C++/ROS2 工程时，才重新比较 CMake、Bazel 和依赖管理方案。

测试分层维护在[测试策略](../quality/testing.md)，模型制品和发布边界维护在[制品文档](../delivery/artifacts.md)。
