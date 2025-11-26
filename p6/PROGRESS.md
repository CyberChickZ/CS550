# Project 6 Work Log (step-by-step)

- 初始化仓库：从 P5 基线拷贝到 p6；确认 `snakeH.obj` 资源存在并重命名去掉空格。
- 场景精简：删除多行星表，只保留单个 `ObjectInfo` 指向 `snakeH.obj`，默认纹理关闭（纯材质）。
- 输入控制调整：`1` 跟随蛇，`0` 返回自由视角；空格/`;` 暂停/恢复物体运动；保持相机交互。
- 光源与可视化：双光源保留，轨迹中心加入微小正弦抖动；Spot 光圈缩到 14°；`b` 独立控制光束显示，默认关闭。
- 时间驱动：在 `Animate()` 增加 `PigTime01`，周期初始 10s 后改为 5s。
- 引入 GLSL：取消注释 `glslprogram.cpp`，创建 `PigInPython.vert/.frag`；降级到 `#version 120` 并用 `varying` 以兼容 Mac，解决“cannot handle shaders”错误。
- Vertex shader：实现 pig-in-the-python；使用 smoothstep(pulse) 缩放 Y/Z；uniform `uPigD/uPigH/uPigW`；LightPosition 常量。宽度默认 8。
- Fragment shader：per-fragment Phong，颜色改为亮橙色，常量 Ka/Kd/Ks/Shininess。
- CPU 侧：计算 `uPigD` 依据 Time 在 [-13,+9] 往返；`uPigH` 用正弦动画（中心 1.0，幅度 ±0.8，范围约 0.2–1.8），可关闭动画时固定 1.0；`uPigW` 传入。
- 默认状态：纹理 OFF；物体运动 OFF；光源运动 OFF；光束 OFF；Point 光；鼓包动画开启、5s 一周。
- HUD 精简：第一行显示光模式/运动/光束/物体运动；第二行单独显示 Pig 方向（p）、高度动画状态（h）。颜色保持清晰。
- 交互整理：`p` 切换尾→头 / 头→尾；`h` 切换高度动画；`k` Point/Spot；`b` 光束；`;`/空格 物体运动；`1` 跟随蛇、`0` 轨道视角。
