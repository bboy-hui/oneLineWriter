# 架构规划

## 模块边界

- `core`：应用生命周期、配置、日志、任务编排
- `gcode`：路径数据、工艺参数、GCode 编译
- `device`：串口通信、GRBL 状态机、发送节流与回执处理
- `render`：统一场景数据与后端抽象
- `app/desktop`：ImGui 界面编排与各模块装配

## 渲染抽象

- 统一接口：`IRenderBackend`
- 统一输入：`CameraState`、`Polyline3D`
- 后端扩展：`BackendType::Vulkan`、`BackendType::OpenGL`
- 原则：业务层不依赖具体图形 API

## 近期里程碑

- M1：跑通文本写字 GCode 生成与串口发送
- M2：实现 ImGui 主界面、任务面板、2D 预览
- M3：加入 3D 仿真视图和执行进度回放
- M4：补全 Vulkan 后端实现
- M5：补全 OpenGL 后端适配层
