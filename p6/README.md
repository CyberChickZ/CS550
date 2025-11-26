# Project 6（Pig-in-the-Python）信息总览

本 README 汇总了在我的 Project 5 代码基础上完成 Project 6 所需的全部已知信息，仅做信息整理，不包含实现建议或计划。

---

## 1. Project 6 官方要求（原文信息整理）

### 1.1 目标与描述

使用 GLSL vertex shader 制作 “pig-in-the-python” 效果：  
在模型（如 `snakeH.obj`）沿 X 方向产生一个随时间移动的 bulge。  

使用 GLSL fragment shader 实现 per-fragment lighting。  

bulge 的中心位置和高度需要能够根据 Time 动态变化。  
bulge 的宽度可以写死。  

### 1.2 输入模型

- 推荐模型：`snakeH.obj`（36,720 triangles）  
- snake 的 X 范围约为 -13 到 +9。  

bulge 中心位置公式（给定）：

- 尾到头移动：`x = -13.f + Time*( 9.f + 13.f );`
- 头到尾移动：`x = 9.f - Time*( 9.f + 13.f );`

### 1.3 ShaderProgram 要求

- 使用课程提供的 `GLSLProgram` C++ 类（`glslprogram.cpp`）。  
- 在 C++ 侧至少调用：

  ```cpp
  SetUniformVariable("uPigD", floatVal);
  SetUniformVariable("uPigH", floatVal);
  ```

- 须提供两个 uniform：

| uniform 名 | 类型 | 含义                         |
|-----------|------|------------------------------|
| `uPigD`   | float| bulge 在 X 轴上的中心位置    |
| `uPigH`   | float| bulge 的高度（幅度）         |

### 1.4 Vertex Shader 要做的事

- 计算 bulge 的 `pulse` 值（通常用 `smoothstep`）。  
- 根据 `pulse` 修改 `yz`：

  ```glsl
  MCvertex.yz *= (1. + something);
  ```

- 同时计算 per-fragment lighting 需要的：
  - `vN = gl_NormalMatrix * gl_Normal`
  - `vL = LightPosition - ECposition`
  - `vE = Eye vector`
- 位置输出：

  ```glsl
  gl_Position = gl_ModelViewProjectionMatrix * MCvertex;
  ```

### 1.5 Fragment Shader 要做的事

- 实现 per-fragment lighting：ambient + diffuse + specular。  
- 常量包括：`Color`, `SpecularColor`, `Ka`, `Kd`, `Ks`, `Shininess`。  
- 输入：`in vec3 vN, vL, vE;`（在 Mac 上用 `varying` 关键字）。  
- 输出：`gl_FragColor`。  

### 1.6 smoothstep pulse（官方课件内容）

来自 `SmoothStep.pdf`：  

构造 smoothpulse：

```glsl
pulse = smoothstep(PigD - PigW/2., PigD, x)
      - smoothstep(PigD, PigD + PigW/2., x);
```

- 示例宽度：`PigW = 6`。  
- 尺度控制：`yzScale = 1. + pulse * uPigH`。  

### 1.7 动画控制（Time）

来自 `SinesAndCosines.pdf`：  

- 构造 `Time ∈ [0,1)`：

  ```cpp
  ms   = glutGet(GLUT_ELAPSED_TIME);
  Time = (ms % Period) / Period;
  ```

- `sin(2π*Time)` 对应平滑动画。  
- `uPigH` 可以用 `sin`，也可以通过 toggle 等方式控制。  

### 1.8 Turn-in

需要交付：

- C++ 主程序（.cpp）  
- 顶点 shader（`.vert`）  
- 片段 shader（`.frag`）  
- PDF 报告  
- Kaltura 视频（unlisted）  

报告中需要说明动画是否符合要求。  

### 1.9 Grading

| 项目                         | 分值 |
|------------------------------|------|
| bulge 出现                  | 20   |
| bulge 随时间移动（`uPigD`） | 30   |
| bulge 高度随时间变化（`uPigH`） | 30 |
| per-fragment lighting        | 20   |
| **总计**                     | 100  |

---

## 2. 从 P5 代码中需要知道的结构性信息

本节是为了让另一位开发者在当前 Project 5 代码骨架上完成 Project 6。

### 2.1 主程序结构

- 入口文件：`sample.cpp`。  
- 主要函数：
  - `InitGraphics()`
  - `InitLists()`
  - `Reset()`
  - `Animate()`
  - `Display()`
- 所有 GL / GLUT 初始化都已经存在，不需要新建窗口逻辑。  

### 2.2 P5 的绘制过程

- 所有物体（太阳、9 个行星）都是类似形式：

  ```cpp
  glPushMatrix();
      glTranslatef(...);
      glRotatef(...);
      glScalef(...);
      glCallList(displayList);
  glPopMatrix();
  ```

- 所有 object 的绘制都在 `Display()` 内部完成。  
- 对 P6：绑定 / 解绑 shader 的位置需要插在 draw 之前或之后。  

### 2.3 P5 的材质与光照

- 使用固定功能管线 `GL_LIGHT0`, `GL_LIGHT1`。  
- 使用 `SetMaterial() / SetPointLight() / SetSpotLight()`。  
- P6 不需要固定管线光照，可以保留或关闭固定光照，因为 fragment shader 会做自己的 lighting。  

### 2.4 使用 OBJ 的方式

- OBJ 使用 display list：
  - `objSourceDL = LoadObjMtlFiles(...)`
  - `glCallList(objSourceDL)`  
- 对 P6：需要依赖 OpenGL compatibility pipeline 自动提供的 `gl_Vertex` 与 `gl_Normal`，从而将 OBJ 顶点位置 / 法线传入 vertex shader。  

### 2.5 `glslprogram.cpp` 在 P5 中尚未启用

- `glslprogram.cpp` 文件已存在于工程目录，但在 `sample.cpp` 里被注释掉：  

  ```cpp
  // #include "glslprogram.cpp"
  ```

- 开启方式：只需取消注释上述 `#include`。  
- 使用示例结构：

  ```cpp
  GLSLProgram shader;
  shader.Create("PigInPython.vert", "PigInPython.frag");
  shader.Use();
  shader.SetUniformVariable("uPigD", value);
  shader.SetUniformVariable("uPigH", value);
  ```

> 上面代码段仅用于说明接口和文件名，不代表具体实现方案。  

### 2.6 GlobalTime 在 P5 中已经存在

- 在 `Animate()` 中已经有类似代码：

  ```cpp
  int ms = glutGet(GLUT_ELAPSED_TIME);
  GlobalTime = 0.001f * ms;
  ```

- 如需构造 `Time ∈ [0,1)`，可以基于 `GlobalTime` 按 `SinesAndCosines` 规范重新定义。  

### 2.7 键盘、鼠标、相机控制

- 键盘 / 鼠标 / 相机控制逻辑已经在 P5 中实现。  
- `Display()` 每帧根据当前相机状态更新 view 矩阵（轨道相机 + LookSlot）。  
- 对 P6：shader 中的光照方向需要与 eye vector 一致。  

### 2.8 displayList 基本结构

- 每个 `ObjectInfo` 拥有一个 `displayList` 用于绘制，构建函数类似：

  ```cpp
  BakeDisplayListForObject(i);
  ```

- 对 P6：最终可以只绘制蛇（`snakeH.obj`），而不是 P5 的 10 个行星；也可以暂时保留行星系统，仅额外加载蛇模型。  

---

## 3. 完成 P6 所需的额外资料

### 3.1 SmoothStep 资料

- 来自 `SmoothStep.pdf`（用于 pulse 的计算）。  
- 需要掌握：
  - `step()` 与 `smoothstep()` 的含义。  
  - smoothpulse 公式：

    ```glsl
    smoothstep(PigD - PigW/2., PigD, x)
    smoothstep(PigD, PigD + PigW/2., x)
    ```

  - 示例宽度：`PigW = 6`。  
  - `yzScale = 1 + pulse * uPigH`。  

### 3.2 Sines & Cosines 动画资料

- 来自 `SinesAndCosines.pdf`，用于 `uPigH` 的动态变化。  
- 需要掌握：
  - 角度与弧度关系。  
  - `sin` 与 `cos` 在动画中的行为。  
  - `Time` 的构造方法。  
  - 振幅与频率控制方式，如 `A * sin(F * (2π * Time))`。  

---

## 4. 开发者必须具备的所有“输入参数与文件名”

| 类型           | 名称                | 说明                            |
|----------------|---------------------|---------------------------------|
| OBJ 模型       | `snakeH.obj`        | 作业提供的蛇模型                |
| 顶点 shader    | `PigInPython.vert`  | 作业 skeleton                   |
| 片段 shader    | `PigInPython.frag`  | 作业 skeleton                   |
| GLSLProgram 类 | `glslprogram.cpp`   | P5 已有，需启用                 |
| uniform        | `uPigD`             | bulge center                    |
| uniform        | `uPigH`             | bulge height                    |
| 常量示例       | `PigW = 6`          | bulge width                     |
| fragment 常量  | `Color`, `SpecularColor`, `Ka`, `Kd`, `Ks`, `Shininess` | 可硬编码 |
| PDF            | `SmoothStep.pdf`    | smoothstep 教程                 |
| PDF            | `SinesAndCosines.pdf` | 动画控制教程                  |

---

## 5. 完整的任务清单（纯信息版）

以下为完成 Project 6 需要完成的事项列表，仅列出任务名称本身，不包含如何实现的建议：

1. 在 P5 框架中启用 `GLSLProgram` C++ 类。  
2. 将 `PigInPython.vert` 与 `PigInPython.frag` 加入工程。  
3. 加载 `snakeH.obj`（可以替换 P5 的行星体系）。  
4. 在 C++ 中执行 `shaderProgram.Use()` 以绑定 shaders。  
5. 在每帧根据 `Time` 计算 `uPigD`，并传给 shader。  
6. 在每帧根据 `Time` 计算 `uPigH`，并传给 shader。  
7. Vertex shader 读取 `gl_Vertex.x`，使用 `smoothstep` 组合成 `pulse`。  
8. Vertex shader 根据 `pulse` 调整 `gl_Vertex.yz`。  
9. Vertex shader 输出 `vN`, `vL`, `vE`。  
10. Fragment shader 实现 per-fragment lighting。  
11. 在 PDF 报告中加入姓名、邮箱、描述、截图、Kaltura 链接等。  

---

## 6. 项目范围声明

- 本项目不需要 texture。  
- 本项目不需要固定管线光照。  
- 本项目只关注：bulge（位置 + 高度） + fragment lighting。  
- Project 6 不依赖 P5 的 obj array，只需保留最小绘制逻辑即可。  

---

## 7. 课程网页原文（英文拷贝）

以下内容是从课程网页复制的 Project #6 原文，方便离线查看：

> CS 450/550 -- Fall Quarter 2025  
> Project #6  
> 100 Points  
> Due: November 26  
>  
> Shaders, I  
>  
> This page was last updated: November 16, 2025  
>  
> I created a summary document describing the steps in using Shaders. It is called Shader Steps. Check it out. It might help you!  
>  
> Introduction:  
>    
> The goal of this project is to use a GLSL vertex shader to enlarge a shape's Y-Z cross-section as you go down it. This effect is sometimes called "the pig-in-the-python". You can key off of x, y, or z coordinates, or from s and t. (x is probably easiest.) Your fragment shader will implement per-fragment lighting.  
>  
> Learning Objective:  
> When you are done with this assignment, you will understand how to create a GPU-program, known as a shader. Your vertex shader will let you alter the shape of the object. Your fragment shader will let you control the coloring and lighting pattern on the object. Today's games and movies employ hundreds of shader programs to get the variety of special effects they need.  
>  
> Instructions:  
>  
> Since this called a pig-in-the-python, I am giving you a snake OBJ file, but you are free to use any object that shows the effect. Here is an OBJ file you can use:  
> File Triangles  
> snakeH.obj 36,720  
> Left-click to look at it. Right-click to download it.  
>  
> The snake's X coordinates go from around -13. to +9. That means that at a given value of Time, the x coordinate of the center of the bulge will be at:  
> Bulge moving tail-to-head: float x = -13.f + Time*( 9.f + 13.f );  
> Bulge moving head-to-tail: float x = 9.f - Time*( 9.f + 13.f );  
>  
> Use our GLSLProgram C++ class to create a shader program from your vert and frag files.  
>  
> Write a vertex shader that creates a smooth bulge that is able to travel the length of the snake. The beginning and ending X coordinates of the pulse need to be computed as a function of Time. You can build the bulge any way you want, but one way to do this is by using the GLSL built-in smoothstep( ) function. The vertex shader should also setup for per-fragment lighting. When lighting, don't worry about the fact that you are changing part of the snake's coordinates. Just setup the per-fragment lighting for the original snake surface normal vectors.  
>  
> You need to be able to change the height of the bulge under program control. This could be continuous, such as using a sine function. Or it could be discontinuous, such as toggling between a low value and a high value. You can do this any way you want, but one way to do this is by using the the animation information from our SinesAndCosines handout.  
>  
> You can hardcode the bulge's width. This does not need to be animated unless you want to.  
>  
> In your C++ code, use the SetUniformVariable( ) method to set the uPigD and uPigH uniform variables.  
> Variable Type Purpose  
> uPigD float Where the bulge is centered along the length of the snake  
> uPigH float How high the bulge is at uPigD  
>  
> Write a fragment shader that performs per-fragment lighting.  
>  
> To help you get started, here are some program skeletons to work from:  
> PigInPython.vert  
> PigInPython.frag  
>  
> The fragment shader also needs lighting parameters: Color, SpecularColor, Ka, Kd, Ks, and Shininess. These can be hard-coded. They can also be animated if you so choose.  
>  
> In this project, the fragment shader's sole job is to apply per-fragment lighting to the object. Ignore the fact that we are displacing the object and just use the original un-displaced surface normals.  
>  
> Use the in and out keywords to pass the vL, vN, and vE variables from the vertex shader to the fragment shader. Those 3 variables will be interpolated in the rasterizer so that each fragment gets its own copy of them. (On a Mac, use the word varying instead of in and out.)  
>  
> The GLSLProgram C++ class  
> The glslprogram.cpp file is already in your Sample folder. You just need to un-comment its #include.  
>  
> See the class Shader notes for how to use the GLSLProgram C++ class.  
>  
> Turn-in:  
>  
> Use Canvas to turn in:  
>  
> Your .cpp, .vert, and .frag files  
> A short PDF report containing:  
> Your name  
> Your email address  
> Project number and title  
> A description of what you did to get the display you got  
> A couple of cool-looking screen shots from your program.  
> Tell us what convinces you that your animation is indeed doing what you set it up to do.  
> The link to the Kaltura video demonstrating that your project does what the requirements ask for. If you can, we'd appreciate it if you'd narrate your video so that you can tell us what it is doing.  
> To see how to turn these files in to Canvas, go to our Project Notes noteset, and go the the slide labeled How to Turn In a Project on Canvas.  
> Be sure that your video's permissions are set to unlisted.. The best place to set this is on the OSU Media Server.  
> A good way to test your video's permissions is to ask a friend to try to open the same video link that you are giving us.  
> The video doesn't have to be made with Kaltura. Any similar tool will do.  
>  
> Grading:  
> Item Points  
> A smooth bulge of some sort appears somewhere on the snake 20  
> The smooth bulge moves down the snake as a function of time (uPigD) 30  
> The smooth bulge has its height animated (uPigH) 30  
> Per-fragment lighting works 20  
> Potential Total 100  

---

## 8. 当前 p6 代码骨架状态概览

- 本目录 `p6` 是在 `p5` 代码基础上直接拷贝得到的初始骨架。  
- 目前 `sample.cpp` 顶部注释中的课程信息和项目编号已更新为 Project #6 / Shaders, I，并添加了截止日期与网页更新时间说明；其余 C++ 逻辑仍保持与 Project 5 完成版本一致。  
- `glslprogram.cpp` 和 `glslprogram.h` 已在目录中存在，但在 `sample.cpp` 中的 `#include "glslprogram.cpp"` 仍保持注释状态，尚未启用。  
- OBJ / BMP 资源（如 `ducky.obj`, `*.bmp` 行星纹理）与 P5 相同，`snakeH.obj` 需从课程网页单独下载放入本目录后方可使用。  

