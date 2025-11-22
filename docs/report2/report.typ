#import "@preview/codly:1.3.0": *
#import "@preview/codly-languages:0.1.10": *
#import "@preview/oxdraw:0.1.0": *

#set text(font: "Noto Sans CJK SC")
#set heading(numbering: "1.1")

#show: codly-init.with()
#codly(languages: codly-languages)

#set document(
  author: "Bangyan Gong <crvenabombarder@sjtu.edu.cn>",
  title: "作业2：建模与动画特效",
)

#align(center)[
#text(size: 16pt)[
*建模与动画特效*
]

龚邦彦 \ 523021910130
]

#figure(
  caption: "系统整体架构图",
)[
  #image("resources/1_arch.svg")
]


= 项目概述

本项目在作业1的基础上，实现了一个包含地形沙盘、雨云控制、雨雪粒子系统的交互式3D场景。系统采用现代OpenGL技术，实现了地形生成、体积云渲染、粒子系统、镜头控制等核心功能。

= 地形模型生成算法

== 地形生成实现

地形生成位于 `include/terrain_mesh.hpp` 和 `src/graphics/terrain_mesh.cpp` 中，主要特点如下：

*核心算法：*

- 使用Perlin噪声生成高度图
- 采用八层分形噪声叠加，增强地形细节
- 实现平滑边缘处理，避免边界突变

*技术原理详解：*

地形生成采用基于Perlin噪声的分形布朗运动（Fractal Brownian Motion）技术。Perlin噪声是一种梯度噪声，通过插值网格点上的随机梯度向量生成平滑的连续噪声。八层分形叠加意味着在多个频率尺度上叠加噪声，低频噪声决定地形的大致轮廓，高频噪声添加细节纹理。

平滑边缘处理采用距离衰减函数，在沙盘边缘80%区域内保持平坦地形，外缘20%区域通过smoothstep函数实现平滑过渡到边界。这种设计既保证了沙盘中心区域的可用性，又避免了边界的视觉突变。

```cpp
// 平滑过渡函数
float smoothstep(float low, float high, float x);

void TerrainMesh::generateHeightMap(float frequency, size_t octaves, size_t seed) {
  const siv::PerlinNoise perlin{seed};
  // 80%区域为平坦地形，边缘进行圆滑过渡
  const float innerBoxSize = 0.8f;
}
```

*地形网格生成：*

- 根据高度图数据生成顶点网格
- 自动计算法线向量用于光照
- 采用三角形带索引优化渲染性能

网格生成采用规则网格拓扑结构，每个顶点包含位置、法线、纹理坐标等信息。法线计算基于相邻三角形面的加权平均，确保光照计算的准确性。三角形带索引通过重用顶点数据，显著减少了内存占用和渲染调用次数。

#figure(
  caption: "地形生成效果示意图",
)[
  #image("resources/2_terrain.png")
]

*地形生成数学公式：*

1. *Perlin噪声函数*（二维）：
   $
   P(x,y) = sum_(i=0)^(n-1) (N (2^i x, 2^i y)) / (2^i)
   $
   其中 $N(x,y)$ 为基本噪声函数，$n$ 为八层数

2. *平滑过渡函数*：
   $
   "smoothstep"(t) = t^2 (3 - 2t) \
   "falloff" = 1 - "smoothstep"(d / r)
   $
   其中 $d$ 为到中心的距离，$r$ 为过渡范围

3. *法线计算*：
   $
   bold(n) = ((p_2 - p_1) times (p_3 - p_1)) / abs((p_2 - p_1) times (p_3 - p_1))
   $

= 雨云模型构建

== 体积云实现

雨云模型位于 `include/cloud.h` 和 `src/graphics/cloud.cpp` 中：

*渲染技术：*

- 使用16层切片渲染实现体积效果
- 生成3D噪声纹理模拟云层细节
- 支持透明度和光照计算

*技术原理详解：*

体积云渲染采用切片渲染技术（Slice-based Volume Rendering），将3D体积数据分解为16个平行于视平面的2D切片。每个切片都是一个面向相机的公告板（Billboard），通过透明度混合实现体积效果。

3D噪声纹理使用 Perlin 噪声算法生成，在32×32×32分辨率的3D纹理中存储云层密度信息。球形基础形状通过距离函数生成，Perlin 噪声添加不规则细节，模拟真实云层的蓬松感和层次感。

```cpp
bool Cloud::generateVolumeTexture() {
  // 创建球形云层形状
  float distance = sqrt(cx * cx + cy * cy + cz * cz);
  float shape = 1.0f - (distance * 2.0f);
  // 添加 Perlin 噪声增强细节
  const siv::PerlinNoise perlin{42};
  float noiseValue = perlin.octave3D_01(nx * 4.0f, ny * 4.0f, nz * 4.0f, 3);
}
```

#figure(
  caption: "体积云切片渲染示意图",
)[
  #image("resources/3_cloud.png")
]

*公告板算法公式：*

云层采用公告板技术实现始终面向相机，其数学原理如下：

1. *视图方向计算：*
   $
   bold(v) = (bold(c)_"camera" - bold(c)_"billboard") / abs(bold(c)_"camera" - bold(c)_"billboard")
   $
   其中 $bold(c)_"camera", bold(c)_"billboard"$ 分别为相机和公告板的世界坐标位置

2. *正交基向量计算：*
   $
   bold(r)  &= (bold(u) times bold(v)) / abs(bold(u) times bold(v)) \
   bold(u)' &= (bold(v) times bold(r)) / abs(bold(v) times bold(r))
   $
   其中 $bold(u)$ 为向上的单位向量，$bold(r)$ 为视图方向向右的辅助向量，$bold(u)'$ 为重新计算的公告板的向上单位向量

3. *切片位置计算：*
   $
   bold(p)_"slice" = bold(c)_"billboard" + d dot.c bold(v)
   $
   其中 $d$ 为切片的深度

4. *最终顶点位置：*
   $
   bold(p)_"world" = bold(p)_"slice" + s_x dot.c a_x dot.c bold(r) + s_y dot.c a_y dot.c bold(u)'
   $
   其中 $a_x, a_y$ 为顶点的本地二维坐标位置，$s_x, s_y$ 为缩放系数

公告板技术的关键在于动态构建一个始终面向相机的局部坐标系。通过叉积运算构建正交基向量，确保无论相机如何移动，云层切片始终正对观察者，同时保持正确的透视关系。

== 云层控制

*键盘控制：*

- M键：切换雨云显示/隐藏
- WAXD键：控制云层在沙盘上的移动
- 云层移动速度通过 `CLOUD_SPEED` 常量控制

*控制机制详解：*

云层控制采用状态机模式，通过M键在显示/隐藏状态间切换。移动控制基于时间增量，确保在不同帧率下保持一致的移动速度。云层位置限制在沙盘范围内，避免超出可见区域。

控制逻辑与粒子系统联动，当云层移动时，雨雪粒子的发射位置同步更新，确保粒子始终从云层下方发射，保持视觉一致性。

= 雨雪粒子系统实现

== 粒子系统架构

粒子系统采用模块化设计，位于 `include/particles/` 目录下：

*核心组件：*

- `ParticleSystem`: 粒子系统管理器
- `ParticleEmitter`: 粒子发射器基类
- `ParticleBehaviour`: 粒子行为接口
- `Particle`: 粒子数据结构

*架构设计详解：*

粒子系统采用策略模式实现高度解耦。`ParticleBehaviour` 接口定义了粒子的生命周期管理，包括初始化、更新、存活检测等操作。不同的粒子类型（雨、雪）通过实现不同的行为类来定义其独特特性。

`ParticleEmitter` 负责粒子的生成策略，支持连续发射和爆发式发射两种模式。发射器与行为解耦，同一个发射器可以发射具有不同行为的粒子。

`ParticleSystem` 作为管理器，负责粒子的渲染、更新和资源管理。采用对象池重用粒子对象，避免频繁的内存分配和释放，显著提升性能。

#figure(
  caption: "粒子系统架构图",
)[
  #image("resources/4_particle.svg")
]

== 雨粒子实现

*雨粒子特性：*

- 从云层位置向下发射
- 受重力影响加速下落
- 与地形碰撞检测后消失
- 支持粒子池重用优化性能

*技术实现详解：*

雨粒子模拟真实雨滴的物理特性。初始速度向下，受重力加速度影响逐渐加速。碰撞检测基于地形高度函数，当粒子位置低于地形高度时触发碰撞事件。

粒子池机制预先分配一定数量的粒子对象，在粒子死亡时将其回收到池中，新粒子发射时从池中获取，避免了频繁的对象创建和销毁开销。

== 雪粒子实现

*雪粒子特性：*

- 缓慢飘落，受轻微风力影响
- 与地面碰撞后逐渐堆积
- 支持积雪视觉效果
- 粒子生命周期管理

*技术实现详解：*

雪粒子相比雨粒子具有更复杂的运动特性。除了重力外，还受到随机风力的影响，产生飘落效果。碰撞后不是立即消失，而是逐渐堆积，通过调整粒子生命周期与透明度模拟积雪效果。

生命周期管理采用时间衰减机制，粒子从生成到消失经历完整的生命周期，确保粒子数量的稳定控制。

*粒子更新逻辑：*

```cpp
void ParticleSystem::update(float deltaTime, const glm::mat4 &model) {
  // 更新粒子生命值
  p.life -= deltaTime;
  // 检查粒子是否应该移除
  bool shouldRemove = emitter->getBehaviour()->isAlive(p, model);
  // 更新粒子位置和状态
  p.position += p.velocity * deltaTime;
}
```

#figure(
  caption: "雨雪粒子效果对比图",
)[
  #image("resources/5_rain.png")
  #image("resources/5_snow.png")
]

*粒子物理模拟公式：*

1. *雨粒子运动方程*：
   $
   bold(v)(t) &= bold(v)_0 + bold(g) dot.c t \
   bold(p)(t) &= bold(p)_0 + bold(v)_0 dot.c t + 1 / 2 bold(g) dot.c t^2
   $
   其中 $bold(g) = (0, -9.8, 0)$ 为重力加速度

2. *雪粒子运动方程*：
   $
   bold(v)(t) &= bold(v)_0 + (bold(g) + bold(w)(t)) dot.c t \
   bold(p)(t) &= bold(p)_0 + bold(v)_0 dot.c t + 1 / 2 (bold(g) + bold(w)(t)) dot.c t^2
   $
   其中 $bold(w)(t)$ 为风力场函数

3. *碰撞检测*：
   $
   h_"terrain" &= T(x,z) \
   "collision" &= p_y lt.eq h_"terrain"
   $
   其中 $T(x,z)$ 为地形高度函数

4. *生命周期管理*：
   $
   "life"_"new" &= "life"_"current" - Delta t \
   "death" &= "life"_"new" lt.eq 0
   $

物理模拟采用欧拉积分方法，虽然精度有限但计算效率高，适合实时图形应用。碰撞检测通过查询地形高度函数实现，确保粒子与地形的精确交互。

= 输入控制系统

== 键盘输入处理

输入处理位于 `src/core/input_handler.cpp` 中：

*控制模式切换：*

- 默认模式：WASD控制镜头移动
- 云层模式：WAXD控制云层位置，同时控制粒子发射位置

*快捷键功能：*

- M键：切换雨云显示
- R键：开始/停止下雨
- S键：开始/停止下雪
- ESC键：退出程序

*输入处理机制详解：*

输入系统根据当前激活的系统（相机、云层、粒子）动态切换控制模式。键盘事件通过GLFW回调捕获，存储在按键集合中，在每帧更新时统一处理。

防抖动机制确保单次按键触发不会产生多次状态切换。模式切换时自动同步相关系统的状态，如云层显示时自动切换到云层控制模式。

== 鼠标输入处理

*镜头控制：*

- 鼠标移动控制镜头方向
- 滚轮控制镜头缩放

*鼠标控制技术详解：*

鼠标移动通过差值计算实现平滑的镜头旋转。滚轮缩放采用非线性映射，在近距离时提供更精细的控制，远距离时提供更大的缩放范围。首次鼠标移动检测机制避免初始位置的跳跃。

= 镜头控制系统

== 相机实现

相机系统位于 `include/camera.h` 中：

*核心功能：*

- 欧拉角相机模型
- 基于四元数的旋转计算
- 平滑的移动和视角控制

*相机技术详解：*

相机采用第一人称视角设计，基于欧拉角（偏航角 Yaw、俯仰角 Pitch）表示方向。虽然欧拉角存在万向锁问题，但在本应用的限制角度范围内不会出现此问题。

相机向量通过球面坐标转换计算，前向量由偏航角和俯仰角决定，右向量和上向量通过叉积运算获得。这种计算方式确保了相机坐标系的正确正交性。

```cpp
void Camera::ProcessKeyboard(Movement direction, float deltaTime) {
  float velocity = MovementSpeed * deltaTime;
  // 根据方向移动相机位置
  Position += Front * velocity; // 前进
  Position -= Right * velocity; // 左移
}
```

*MVP变换矩阵：*

1. *模型矩阵（Model Matrix）*：
   $
   bold(M) = bold(T) dot.c bold(R) dot.c bold(S)
   $
   其中 $bold(T)$ 为平移矩阵，$bold(R)$ 为旋转矩阵，$bold(S)$ 为缩放矩阵

2. *视图矩阵（View Matrix）*：
   $
   bold(V) = mat(
   r_x, r_y, r_z, -bold(r) dot.c bold(p);
   u_x, u_y, u_z, -bold(u) dot.c bold(p);
   f_x, f_y, f_z, -bold(f) dot.c bold(p);
   0, 0, 0, 1;
   )
   $
   其中 $bold(r)$ 为右向量，$bold(u)$ 为上向量，$bold(f)$ 为前向量，$bold(p)$ 为相机位置

3. *投影矩阵（Projection Matrix）*：
   $
   bold(P) &= mat(
   (2n) / (r-l), 0, (r+l) / (r-l), 0;
   0, (2n) / (t-b), (t+b) / (t-b), 0;
   0, 0, -(f+n) / (f-n), -(2 f n) / (f-n);
   0, 0, -1, 0;
   ) \
   &= bold(M)_#[persp $->$ ortho] dot.c bold(M)_"ortho" \
   &= mat(
   n, 0, 0, 0;
   0, n, 0, 0;
   0, 0, f + n, -n f;
   0, 0, 1, 0;
   ) dot.c mat(
   2 / (r-l), 0, 0, 0;
   0, 2 / (t-b), 0, 0;
   0, 0, 2 / (n-f), 0;
   0, 0, 0, 1;
   ) dot.c mat(
   1, 0, 0, - (r+l) / 2;
   0, 1, 0, - (t+b) / 2;
   0, 0, 1, - (n+f) / 2;
   0, 0, 0, 1;
   )
   $
   其中 $l, r, b, t, f, n$ 分别为包含所有渲染物体的立方体的左、右，顶、底，和远、近的世界坐标，$bold(M)_"ortho"$ 为正交变换矩阵，$bold(M)_#[persp $->$ ortho]$ 为正交变换转为投影变化的变换矩阵

4. *最终变换*：
   $
   bold("MVP") = bold(P) dot.c bold(V) dot.c bold(M) \
   bold(p)_("clip") = bold("MVP") dot.c bold(p)_("world")
   $

MVP变换是计算机图形学的核心概念。模型矩阵将对象从局部坐标系变换到世界坐标系，视图矩阵将世界坐标系变换到相机坐标系，投影矩阵将3D场景投影到2D屏幕空间。

*镜头移动公式：*

1. *基于时间的速度计算*：
   $
   v = s dot.c Delta t
   $
   其中 $s$ 为移动速度，$Delta t$ 为时间增量

2. *位置更新*：
   $
   bold(p)_"new" = bold(p)_"current" + bold(d) dot.c v
   $
   其中 $bold(d)$ 为移动方向向量

基于时间的移动确保在不同帧率下获得一致的移动体验。方向向量基于相机坐标系，确保移动方向与观察方向一致。

= 渲染管线

== 着色器系统

*云层着色器：*

- 体积渲染技术
- 光照计算

*着色器技术详解：*

云层着色器采用体积渲染技术，通过16个切片模拟3D体积效果。每个切片使用相同的顶点着色器但不同的深度偏移，在片段着色器中采样3D纹理获取密度信息。

*云层光照计算公式：*

1. *漫反射光照*：
   $
   I_"diffuse" = max(bold(n) dot.c bold(l), 0)
   $
   其中 $bold(n)$ 为法线向量，$bold(l)$ 为光照方向

2. *边缘光照*：
   $
   I_"rim" = (1 - max(bold(v) dot.c bold(n), 0))^p
   $
   其中 $bold(v)$ 为视图方向，$p$ 为幂次参数

3. *最终颜色*：
   $
   C_"final" = C_"base" dot.c (k_a + k_d dot.c I_"diffuse") + k_r dot.c I_"rim"
   $
   其中 $k_a$ 为环境光系数，$k_d$ 为漫反射系数，$k_r$ 为边缘光系数

光照计算采用简化的Phong模型，边缘光照增强云层的体积感和轮廓感。

*粒子着色器：*

- 点精灵渲染

*粒子着色器技术详解：*

粒子系统使用点精灵（Point Sprite）技术，每个粒子渲染为一个始终面向相机的四边形，使用 `GL_POINTS` 渲染为点。顶点着色器输出粒子位置和大小，片段着色器根据粒子属性计算最终颜色。

== 性能优化

*优化措施：*

- 粒子池重用机制
- 批量渲染
- 层次细节管理

*性能优化技术详解：*

粒子池机制显著减少了内存分配开销，通过预分配和重用粒子对象避免了频繁的 `new` / `delete` 操作。

批量渲染将多个粒子数据打包到单个绘制调用中，减少了OpenGL状态切换和API调用开销。

层次细节管理根据距离调整粒子数量和细节级别，距离过远的粒子不渲染，在保证视觉效果的同时优化性能。

= 实现特点

== 技术特色

1. *模块化设计*：各系统独立开发，便于维护和扩展
2. *物理模拟*：粒子系统包含重力、碰撞等物理效果
3. *视觉效果*：体积云、粒子效果等增强视觉体验
4. *交互性*：完整的键盘鼠标控制系统

*技术特色详解：*

模块化设计使得各个系统（地形、云层、粒子、相机）可以独立开发和测试，降低了代码复杂度。物理模拟基于真实物理定律，提供了逼真的视觉效果。完整的交互系统提供了直观的用户体验。

== 代码质量

- 使用现代C++特性
- 遵循RAII原则管理资源
- 异常安全设计
- 清晰的接口分离

*代码质量详解：*

采用现代C++17特性，包括智能指针、lambda表达式等，提高了代码的可读性和安全性。RAII原则确保资源的正确管理，避免内存泄漏。异常安全设计保证了程序的健壮性。清晰的接口分离便于代码维护和扩展。
