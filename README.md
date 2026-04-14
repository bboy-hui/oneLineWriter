# OneLineWriter

基于 C++20 + CMake 的跨平台 GRBL 上位机写字/激光项目骨架，目标能力包括：

- 文本/图形到轨迹与 GCode 生成
- GCode 流式发送到 GRBL 控制器执行
- ImGui 桌面界面
- 二维与三维预览渲染
- 渲染后端抽象，支持 Vulkan 与 OpenGL

## 构建

```bash
cmake -S . -B build
cmake --build build --config Release
```

## 运行

```bash
./build/apps/desktop/Release/owl_desktop.exe
```

默认会根据文本 `HELLO GRBL` 生成写字路径并导出到 `output/demo.gcode`，同时通过模拟控制器发送队列输出发送内容。

可使用命令行参数选择渲染后端：

```bash
./build/apps/desktop/Release/owl_desktop.exe --backend=opengl
./build/apps/desktop/Release/owl_desktop.exe --backend=vulkan
```

程序会输出渲染帧统计信息，包括加工折线数、空走折线数、顶点数量与预览包围盒（AABB）。
Windows 下会弹出 ImGui 图形界面窗口，包含写字参数、二维路径预览与渲染统计面板。

## 当前工程结构

- `apps/desktop`：桌面应用入口
- `src/core`：应用内核与基础服务
- `src/render`：渲染抽象接口与渲染服务
- `src/gcode`：工艺到 GCode 生成
- `src/device`：GRBL 通信与发送队列
- `docs`：架构与里程碑文档
