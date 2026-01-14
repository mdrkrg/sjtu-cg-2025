#import "@preview/codly:1.3.0": *
#import "@preview/codly-languages:0.1.10": *

#set text(font: "Noto Sans CJK SC")
#set heading(numbering: "1.1")

#show: codly-init.with()
#codly(languages: codly-languages)

#set document(
  author: "Bangyan Gong <crvenabombarder@sjtu.edu.cn>",
  title: "计算机图形学大作业报告",
)

#align(center)[
#text(size: 16pt)[
*计算机图形学大作业报告*
]

龚邦彦 \ 523021910130
]

= 概述

最终作业实现在作业 2
的基础上，增加了完整的游戏交互、解谜机制、粒子特效和高级渲染效果。项目还实现了模块化的架构设计，在可维护性和可扩展性方面做了相当多的工作。

= 系统架构总览

游戏系统采用分层架构模式，将游戏逻辑、输入处理和渲染逻辑分离。

== 架构模式应用

- *单例模式*：`Application`
  采用单例模式管理全局游戏循环和资源，确保线程安全的唯一实例访问
- *策略模式*：受 Unity 启发，通过 `IGameObjectBehaviour`
  接口实现可组合的对象行为
- *工厂模式*：提供统一的对象创建方法，`ModelFactory` 和
  `ParticleFactory` 封装复杂初始化逻辑
- *观察者模式*：实现松散耦合的事件系统，用于输入处理和对象选择
- *组件模式*：构建灵活的对象系统，`GameObject`
  通过可选组件扩展功能

== 游戏循环架构

游戏循环由单例 `Application` 统一管理：

```cpp
void Application::run() {
  while (not glfwWindowShouldClose(window.get())) {
    updateDeltaTime();                                    // 计算时间差
    inputHandler->update(deltaTime);                      // 处理输入
    renderer->update(deltaTime);                          // 更新场景
    renderer->render(projection, view, camera.Position);  // 渲染
    glfwSwapBuffers(window.get());                        // 交换缓冲区
    glfwPollEvents();                                     // 轮询事件
  }
}
```

== 输入处理架构

用户输入处理采用分层架构：

*`InputHandler`*：负责原始输入处理

- 注册GLFW回调函数捕获键盘、鼠标、滚轮事件
- 处理相机控制（WASD移动）和云控制模式切换
- 实现鼠标射线计算，将屏幕坐标转换为世界空间射线
- 处理特殊功能键（截图、调试模式切换等）

*输入处理流程*：

```
GLFW输入事件 → InputHandler回调 → InputHandler::update()每帧处理
     ↓
 相机控制 → Camera::ProcessKeyboard()
     ↓
 鼠标点击 → getMouseRay() → onMouseClick回调
     ↓
GraphicsRenderer::handleMouseClick() → GameManager::handleRayCast()
```

*组件职责分离*：

- `Application`：单例管理游戏循环
- `InputHandler`：处理原始输入，计算鼠标射线
- `GameManager`：管理游戏对象状态，处理对象选择
- `GraphicsRenderer`：负责渲染管线和视觉效果
- `Camera`：管理视图和投影矩阵

== 组件系统架构

组件模式构建灵活的对象系统：

- *`GameObject` 基础类*：包含位置、旋转、缩放等变换数据
- *行为组件*：通过 `IGameObjectBehaviour` 接口附加功能
- *`GameManager` 状态管理*：管理所有
  `GameObject`，统一更新和渲染逻辑，处理射线投影和对象选择

= 台灯光照系统

== 系统架构设计

台灯光照系统采用*策略模式*和*统一缓冲对象(UBO)*架构，实现了高效的光照管理和材质系统。系统核心由三个组件构成：`LightManager`
负责统一管理场景中的所有点光源，`LampLightingBehaviour`
实现台灯交互行为，`Material` 结构体提供材质支持。系统通过 `GameObject`
的 `meshMaterials` 数组实现每个网格的独立材质控制，支持精细到网格的材质管理。

== UBO 光照管理技术实现

光照系统采用统一缓冲对象(UBO)技术，将每个点光源包含的位置、环境光、漫反射光等参数，打包到
GPU 的连续内存区域。`LightManager` 负责管理 UBO
的创建、更新和绑定，每帧仅调用一次 `updateUBO()`
方法更新所有光照数据，减少了 CPU 到 GPU 的数据传输开销。

== 台灯交互机制实现

台灯交互通过 `LampLightingBehaviour` 实现，它继承自
`IGameObjectBehaviour` 接口。当用户点击台灯时，`onSelect()`
方法被调用，切换灯光状态并更新 UBO
中的光照数据。灯光开启时，在台灯上方添加一个点光源到
`LightManager`；关闭时则移除该光源。

灯罩发光效果通过 `Material` 结构体的 `emissive`
属性实现。系统遍历台灯模型的所有网格，通过 `Mesh::getName()`
方法查找名称为 `"Lampshade"`
的网格，动态调整其发射强度和颜色。材质名称在模型加载时从 Assimp 的 MTL
文件中提取，通过 `aiMaterial::Get(AI_MATKEY_NAME)` 获取并存储在 `Mesh`
对象中。其他网格的发射属性被禁用，确保只有灯罩部分发光。

== 光晕效果实现细节

台灯光晕采用屏幕空间后处理效果实现，通过 `LampAuraEffect` 继承自
`ScreenSpaceGlowEffect`
基类。系统基于对象在屏幕空间的位置计算光晕强度，光晕半径、颜色、强度和衰减曲线均可配置，支持不同强度的灯光效果。

光晕计算在屏幕空间进行，首先将对象的世界坐标转换为屏幕坐标，然后基于距离计算每个像素的光晕强度。系统使用
`glowRadius` 控制光晕范围，`glowIntensity`
控制强度，`glowFalloffMultiplier` 控制衰减速度。光晕颜色默认为暖黄色
`(1.0, 0.8, 0.4)`，与台灯光源颜色匹配。

动态脉冲效果通过 `pulseBaseIntensity` 和 `pulseFrequency`
参数控制，计算公式为
`currentStrength = base + sin(time * frequency) * 0.1`，使光晕呈现脉冲效果。

#figure([#image("resources/lamp-lighting.png")],
  caption: [
    台灯光照效果截图
  ]
)

= 解谜游戏实现

== 游戏管理架构设计

`GameManager` 负责游戏对象的状态管理：

*核心职责*：

- 维护所有 `GameObject` 实例的容器
- 处理射线投射和对象选择
- 管理 `PuzzleManager` 和 `TrapManager` 子管理器
- 更新游戏对象的动画和行为状态

== 暗格谜题实现机制

暗格谜题通过 `PuzzleManager` 和 `PuzzleMovementBehaviour`
行为组件协同实现。`PuzzleManager`
负责管理谜题的整体状态，包括点击计数和完成条件，而
`PuzzleMovementBehaviour`
管理游戏对象的移动动画，实现谜题的移动效果。

`PuzzleManager`
负责跟踪解谜进度。该系统记录玩家对特定机关对象的点击次数，当点击次数达到预设值时，谜题完成，触发暗格的打开动画。这种设计支持多次交互，同时也可以进一步支持更复杂的游戏逻辑，如记录
`GameObject` 的名字实现点击顺序检查。

`PuzzleMovementBehaviour` 继承自 `IGameObjectBehaviour` 接口，管理
`GameObject`
的移动和解谜游戏状态更新。该行为组件管理起始位置、目标位置，调用
`GameObject` 的动画函数实现移动。动画完成后，行为组件触发后续事件。

暗格的具体实现通过 `HiddenCellBehaviour` 和 `FloorCompartmentBehaviour`
等专用行为组件完成。这些组件与 `PuzzleManager`
协同工作，当谜题完成时激活相应的移动动画。`HiddenCellBehaviour`
处理墙壁隔间的打开，`FloorCompartmentBehaviour` 管理地板隔间的升降。

== 灵珠特效实现

灵珠特效结合了粒子效果和后处理发光。

粒子系统使用 `SimulationSpace::LOCAL`
模式，确保粒子跟随灵珠移动。每个粒子在灵珠表面生成，然后沿椭圆轨道运动，轨道参数（半径、速度）随机生成以增加视觉多样性。

粒子生命周期管理采用渐变消失效果，alpha
值随剩余寿命线性减少。粒子颜色与灵珠发光颜色匹配，但添加随机变化。轨道运动通过
`updateOrbitalPosition()`
方法计算，考虑三个维度的运动，使粒子轨迹更加立体。

后处理发光在屏幕空间实现，基于像素亮度和深度信息计算发光强度。系统首先将场景渲染到浮点帧缓冲，提取亮度超过阈值的区域，然后应用高斯模糊创建光晕效果。颜色空间转换正确处理
sRGB
到线性空间的转换，确保颜色准确性。多重效果叠加通过效果链实现，基础发光、光晕和颜色校正按顺序应用。

动态脉冲强度通过
`emission = base + sin (time * 2.0)`
公式计算，使发光强度周期性变化。浮动动画为垂直方向的简谐运动，`position.y = originalY + sin(time) * amplitude`。

#figure([#image("resources/orb-glow.png");],
  caption: [
    灵珠特效截图
  ]
)

= 陷阱机关

== 箭矢发射器架构设计

箭矢发射器系统采用 `EmitterGroup`
协调多个发射器，支持复杂的发射模式。`EmitterGroup`
维护发射器列表和共享行为，提供
`launch()`、`startContinuousFire()`、`stopContinuousFire()`
等协调发射方法。`launch()`
方法从所有发射器同时发射指定数量的箭矢，`startContinuousFire()`
启动连续发射模式，按指定速率发射箭矢。这种设计允许单个陷阱包含多个发射点，创造更复杂的发射模式。

每个 `ArrowEmitter`
负责特定位置的箭矢发射，包含位置、方向、发射速率等配置参数。发射器与
`ParticleSystem` 解耦，通过 `EmitterGroup`
协调工作。

发射模式支持单次发射和连续发射两种主要配置。`launch()`
方法实现单次齐射，从所有发射器同时发射指定数量的箭矢，参数
`arrowsPerEmitter` 控制每个发射器的发射数量。`startContinuousFire()`
方法启动连续发射模式，参数 `arrowsPerSecond`
控制所有发射器的总发射速率。

== 箭矢粒子系统组件架构

箭矢粒子系统采用组件化架构。受
Unity 启发，粒子系统将粒子行为分离为独立的 `Initializer` 和
`Updater` 组件，支持灵活的组件组合和重用。这种设计解决了早期设计中
`ParticleBehaviour` 职责过重的问题。

*`ArrowInitializer` 初始化组件*

负责箭矢的初始状态设置，包括设置箭矢的生命周期、颜色、大小以及初始的
`stuck` 状态标记。

*`ArrowPhysicsUpdater` 物理更新组件*

实现箭矢的物理模拟，包括重力、空气阻力和旋转更新。旋转更新确保箭矢在重力影响下逐渐指向运动方向。组件解决了箭矢旋转问题，通过正确的
GLM 欧拉角约定（ZYX 旋转顺序）确保箭矢在不同方向上的正确朝向。

*`ArrowCollisionUpdater` 碰撞更新组件*

集成两阶段碰撞检测系统，首先进行快速的 AABB
相交测试，如果通过则执行精确的射线-网格相交检测。组件维护碰撞目标列表，每帧检查所有活跃箭矢的碰撞状态。当检测到碰撞时，组件标记箭矢为
`stuck` 状态，停止物理更新，表示箭矢插入了物体表面。

*组件组合策略*

允许箭矢系统灵活配置不同的行为组合。例如，可以同时添加
`ArrowPhysicsUpdater` 和 `ArrowCollisionUpdater`
实现物理和碰撞模拟，或者添加 `WindUpdater`
实现风力影响。这种架构支持未来扩展新的行为组件而不修改现有代码。

== 碰撞检测系统实现

碰撞检测系统采用两阶段优化架构，结合轴对齐包围盒(AABB)的快速筛选和精确的网格相交检测，在保证准确性的同时大幅提升性能。为了保证游戏帧率稳定不卡顿，引入了粗筛和细筛的分层检测策略。

*AABB快速筛选（粗筛）*

使用 slab 方法进行射线-AABB相交测试，算法复杂度为
O(1)，能够快速排除大多数不相交的情况。AABB 结构体包含 `min` 和 `max`
两个向量定义包围盒边界，检测时需要遍历 `GameManager` 每个 `GameObject` 计算世界空间下的
AABB。当射线与目标的 AABB
不相交时，系统立即返回，避免慢得多的网格测试。

*精细网格测试（细筛）*

仅在粗筛通过后执行，使用 PMP
库生成的简化碰撞网格进行精确的射线-三角形相交测试。`Mesh`
拥有并缓存简化的碰撞网格，为所有使用该网格的 `GameObject`
提供共享，减少内存占用和生成开销。

碰撞检测流程首先进行 AABB 射线相交测试，计算相交区间
`[tMin, tMax]`。如果射线与 AABB
不相交，立即返回空值。接着进行距离检查，如果最近交点超过最大命中距离，同样提前退出。最后执行射线-网格相交测试，使用缓存的碰撞网格进行精确碰撞检测。

碰撞响应系统处理箭矢与游戏对象的交互逻辑。当检测到碰撞时，系统标记粒子为
`stuck`
状态，停止其物理更新和渲染。系统仅对移动或新发射的箭矢进行碰撞检测，跳过静止或已完成碰撞处理的粒子。

#figure([#image("resources/trap-trigger.png");],
  caption: [
    陷阱触发机关
  ]
)

#figure([#image("resources/arrow-collision-1.png");],
  caption: [
    箭矢碰撞效果截图
  ]
)

#figure([#image("resources/arrow-collision-2.png");],
  caption: [
    箭矢与窗户碰撞效果截图
  ]
)

= 粒子系统架构演进

相较于作业 2，最终的粒子系统提供了更加完善的功能和更解耦的架构。

== 模拟空间概念设计

模拟空间概念解决了粒子应该独立运动还是跟随发射器的核心问题。系统引入
`SimulationSpace` 枚举，定义 WORLD、LOCAL、CUSTOM 三种空间模式。WORLD
空间用于雨、雪等独立粒子，LOCAL 空间用于灵珠光环等跟随粒子，CUSTOM
空间用于高级特效。

空间选择影响粒子的整个生命周期。在 WORLD
空间，粒子发射时从局部坐标转换到世界坐标，存储为世界坐标，更新时直接应用世界力，渲染时使用单位矩阵。在
LOCAL
空间，粒子存储局部坐标，更新时将世界力转换到局部空间，渲染时使用发射器的模型矩阵。

坐标变换管道确保空间转换的一致性。发射阶段根据空间模式选择坐标转换方式，更新阶段根据空间模式选择力转换方式，渲染阶段根据空间模式选择模型矩阵。这种明确的分工使系统易于理解和维护。

== 组件化粒子系统架构

粒子系统经过架构重构，将原有的 `ParticleBehaviour` 拆分为独立的
`Initializer` 和 `Updater`
组件，实现了更清晰的关注点分离和更灵活的组件组合。这种架构遵循”one
system, one emitter, one initializer, multiple updaters”的最佳实践。

*架构核心原则*：

- *一个系统对应一个发射器*：每个 `ParticleSystem` 实例拥有一个
  `Emitter`，负责粒子的生成时机和位置
- *一个发射器对应一个初始化器*：每个 `Emitter` 拥有一个
  `Initializer`，负责粒子的初始状态设置
- *一个系统对应多个更新器*：每个 `ParticleSystem` 可以拥有多个
  `Updater`，实现不同的物理行为和效果

*组件职责分离*：

- *`Initializer`
  接口*：负责粒子初始化，包括位置、速度、旋转、颜色、生命周期等初始属性的设置
- *`Updater`
  接口*：负责粒子更新，包括物理模拟（重力、风力）、碰撞检测、生命周期管理等
- *`DeathCheck`
  接口*：负责粒子死亡条件检查，与更新逻辑分离，支持灵活的粒子生命周期管理

*架构优势*：

- *清晰的职责分离*：初始化、更新、死亡检查各自负责自己的一部分，代码更易理解和维护
- *组件灵活组合*：可以通过组合不同的 `Updater`
  实现复杂的行为，如同时应用重力和风力
- *代码复用*：`Initializer` 和 `Updater`
  可以在不同的粒子系统间共享
- *易于扩展*：添加新的行为只需实现新的 `Updater`，无需修改现有架构

*箭矢系统示例*：

```cpp
// 创建箭矢粒子系统
auto arrowSystem = std::make_shared();

// 设置发射器和初始化器（one emitter, one initializer）
auto arrowInitializer = std::make_shared();
auto emitter = std::make_shared(arrowInitializer);
arrowSystem->setEmitter(emitter);

// 添加多个更新器（multiple updaters）
arrowSystem->addUpdater(std::make_shared());
arrowSystem->addUpdater(std::make_shared());
arrowSystem->addUpdater(std::make_shared());

// 添加死亡检查
arrowSystem->addDeathCheck(std::make_shared());
arrowSystem->addDeathCheck(std::make_shared());
```

这种架构设计使得粒子系统更加模块化、可配置和可扩展，为复杂的游戏特效提供了坚实的基础架构支持。

#figure([#image("resources/arrow-emitter.png")], caption: [
  粒子系统架构灵活，支持右键插入新的发射器，按 `T` 键开启或关闭发射
])

= 后处理管线

== 帧缓冲架构设计

后处理管线采用多阶段渲染架构，由 `PostProcessingManager`
统一管理。系统使用 `FrameBuffer` 封装 OpenGL
帧缓冲对象，包括一个主帧缓冲用于场景渲染和两个 ping-pong
帧缓冲用于效果链处理。场景首先渲染到主帧缓冲，然后按顺序应用后处理效果。

`PostProcessingManager` 协调三个核心效果：`BasicPostProcessEffect`
提供基础后处理（色调映射、伽马校正），`OrbGlowEffect`
实现灵珠发光效果，`LampAuraEffect`
实现台灯光晕效果。效果应用顺序为：基础效果 → 灵珠发光 → 台灯光晕，通过
ping-pong 帧缓冲实现效果链的衔接。

屏幕空间发光效果基于对象在屏幕上的位置计算光晕强度，使用高斯衰减函数生成柔和的光晕。效果参数可动态配置，包括光晕半径、颜色、强度、衰减速度和脉冲频率，支持丰富的视觉效果定制。

== 基础后处理效果实现

`BasicPostProcessEffect` 提供完整的颜色空间管理和色调映射功能。在
`shaders/post-process.fs.glsl` 着色器中实现以下处理流程：

+ *sRGB 到线性空间转换*：使用 `sRGBToLinear()` 函数将输入颜色从
  sRGB 空间转换到线性空间，确保物理正确的光照计算
+ *曝光调整*：通过 `exposure` 参数控制场景亮度
+ *色调映射*：使用 Reinhard 算子
  `color = color / (color + vec3(1.0))` 将 HDR
  值映射到显示范围，防止颜色过曝
+ *伽马校正*：使用 `linearToSRGB()` 函数将线性空间颜色转换回
  sRGB 空间，补偿显示设备的非线性响应

== 屏幕空间发光效果实现

屏幕空间发光效果通过 `ScreenSpaceGlowEffect`
基类提供统一接口，`OrbGlowEffect` 和 `LampAuraEffect`
继承并实现特定参数配置。效果实现基于以下核心参数：

+ *光晕颜色* (`glowColor`)：通过 `setGlowColor()`
  设置，灵珠效果使用青色 `(0.2, 0.8, 1.0)`，台灯效果使用暖黄色
  `(1.0, 0.8, 0.4)`
+ *光晕半径* (`glowRadius`)：通过 `setGlowRadius()`
  设置，控制光晕在屏幕空间的影响范围
+ *光晕强度* (`glowIntensity`)：通过 `setGlowIntensity()`
  设置，控制光晕的亮度
+ *衰减乘数* (`glowFalloffMultiplier`)：通过
  `setGlowFalloffMultiplier()` 设置，控制光晕从中心到边缘的衰减速度
+ *脉冲基础强度* (`pulseBaseIntensity`)：通过
  `setPulseBaseIntensity()` 设置，控制脉冲效果的基础亮度
+ *脉冲频率* (`pulseFrequency`)：通过 `setPulseFrequency()`
  设置，控制脉冲效果的频率

效果计算在屏幕空间进行，首先将对象的世界坐标通过视图和投影矩阵转换为屏幕坐标，然后基于像素到光晕中心的距离计算光晕强度。强度计算公式使用高斯衰减函数：`strength = intensity * exp(-distance^2 / (radius^2 * falloff))`，其中
`distance` 是像素到光晕中心的距离。

动态脉冲效果通过 `currentStrength = base + sin(time * frequency) * 0.1`
公式实现，使光晕呈现呼吸效果。`PostProcessingManager`
负责协调多个发光效果的叠加，确保效果链的正确顺序和性能优化。

= 设计模式应用

== 策略模式实现对象行为

策略模式通过 `IGameObjectBehaviour` 接口实现可组合的对象行为。该接口定义
`onSelect`、`onUpdate`、`onPreRender`、`onPostRender`、`onHover`
等生命周期方法，以及 `isActive`、`setActive`、`getName` 等状态管理方法。

行为可组合性允许一个对象附加多个行为，每个行为处理特定方面的功能。例如，台灯可以同时具有
`LampLightingBehaviour`（灯光控制）、`PuzzleMovementBehaviour`（解密游戏的一部分）。这种设计提高代码复用性，减少重复代码。

行为与对象解耦，通过 `addBehaviour()`
方法动态附加。行为可以独立开发、测试和调试，提高开发效率。接口设计支持未来扩展，可以轻松添加新的行为类型。

== 工厂模式统一对象创建

工厂模式通过 `ModelFactory`、`ParticleFactory`
等提供统一的对象创建接口。`ModelFactory` 提供
`createCube`、`createSphere`、`createArrow`
等方法，封装复杂模型创建逻辑。`ParticleFactory` 提供
`createRainSystem`、`createOrbAuraSystem` 等方法，简化粒子系统配置。

== 观察者模式实现事件系统

观察者模式通过回调函数实现松散耦合的事件系统。`GameObject` 包含
`onSelect`、`onHover`、`onUpdate`
等回调成员，允许外部代码注册事件处理函数。当事件发生时，对象调用注册的回调函数。这样可以减少对象间的直接依赖，使代码更加模块化。

= 与作业 2 的主要差异

== 架构升级对比

作业 2
采用简单的渲染循环和有限的对象管理，主要关注基础渲染功能。本次作业实现演进为完整的游戏架构，包含
`GameManager`
协调游戏对象、行为系统组件化、事件系统观察者模式。这种架构升级支持复杂的游戏逻辑和交互系统。

渲染与逻辑分离是架构升级的核心。作业 2
中渲染和逻辑代码相互耦合，难以维护和扩展。最终实现通过 MVC
模式明确分离职责，`GraphicsRenderer` 专注于渲染，`GameManager`
专注于逻辑，实现关注点分离。

组件化设计支持功能扩展。作业 2
使用继承实现对象功能，导致类层次复杂。最终实现使用组件组合，通过附加行为组件为对象添加功能，让代码逻辑更清晰。

== 粒子系统技术演进

作业 2
的粒子系统基于点精灵，功能有限且性能一般。最终作业实现的粒子系统支持模型粒子、实例化渲染、模拟空间等特性，提升了视觉效果和性能表现。

*架构重构*是粒子系统的主要更新。作业 2 使用单一的
`ParticleBehaviour`
处理所有粒子逻辑，导致职责过重和代码耦合。最终实现将
`ParticleBehaviour` 拆分为独立的 `Initializer` 和 `Updater`
组件，遵循“一个系统，一个发射器，一次初始化，多个更新器”的最佳实践。这种组件化架构支持灵活的组件组合，允许多个更新器协同工作，实现复杂的粒子行为。

*模拟空间概念*是另一个重要更新。作业 2
粒子固定在世界空间或局部空间，难以适应不同需求。受 Unity
的粒子系统启发，最终实现引入 `SimulationSpace`
，允许每个粒子系统独立配置空间模式，支持更灵活的粒子行为。所有组件（`Initializer`、`Updater`）都遵循模拟空间配置，确保坐标转换的一致性。

*性能优化*方面，最终实现引入了实例化渲染技术，通过单次绘制调用渲染大量模型粒子，大幅降低
GPU 开销。碰撞检测采用两阶段优化（AABB粗筛 + 网格细筛），减少 CPU 开销。

== 网格系统现代化与PMP库集成

作业 2
使用传统的顶点/索引数组表示网格，内存管理和网格操作效率有限。最终实现集成了
Polygon Mesh Processing (PMP) 库，将 `Mesh` 重构为使用 PMP 的
`SurfaceMesh`
作为主要数据表示。这种半边结构提供了更高效的网格操作和内存管理，同时保持了与现有渲染系统的兼容性。

网格系统的主要优化包括：顶点属性（位置、法线、纹理坐标、切线和副切线）全部存储为
PMP 顶点属性，消除了对缓存渲染数据的依赖；`Mesh`
现在拥有并缓存简化的碰撞网格，为所有使用该网格的 `GameObject`
提供共享，减少内存占用和生成开销。

== 场景与视觉效果改进

本次作业还进行了窗口框架和房间几何的改进：从基于纹理的窗户过渡到基于模型的方法，将
`RoomGeometry` 转换为单独的 `GameObject`
墙壁以支持碰撞检测。`ModelFactory` 增加了 `createWall`
方法，支持程序化墙壁生成和正确的纹理坐标缩放。

== OpenGL 加载器替换

最终实现使用 epoxy 库替换了作业 2 中的 glad 库作为 OpenGL
函数加载器。epoxy 提供了更简洁的 API 和更好的跨平台支持，简化了 OpenGL
扩展的管理和函数指针加载过程。

= 技术债务与未来架构优化方向

当前 `GameObject`
设计存在过度耦合、内存浪费和灵活性受限的问题，直接包含
mesh、shader、material
等渲染资源，使轻量级对象如箭矢粒子不得不携带不必要的渲染数据。理想的轻量化方案是将
`GameObject`
简化为仅包含坐标、旋转、缩放和动画数据的核心类，而将渲染资源、物理组件等通过组合方式放入新的重型对象中。这种分离使轻量级
`GameObject`
适合大量实例（如粒子系统），重型对象集中管理渲染资源优化状态。

作为补充方向，ECS（实体-组件-系统）架构提供了更彻底的解耦方案，实体仅包含唯一标识符，组件是纯数据结构，系统处理特定组件组合的逻辑，适合大规模复杂游戏的性能优化和数据导向设计。

= 感想

本次作业很大一部分工作都是在帮写前两次作业的自己擦屁股，有很多不合理、耦合的设计需要重构，再加上我有些代码洁癖，很喜欢把架构弄得很解耦，这样一来代码量一下就膨胀上去了。好在现在有大模型，在我设计整体架构的时候不仅提供了各种选项的对比，还帮忙做掉了很多重复性的劳动。

不过话说回来，即使课程工程量比较大，图形学本身还是很有意思的。从图形渲染及其物理原理到图形窗口的工程实现，再到游戏引擎的架构和机制设计，可以感受到它是一门跨度十分广泛的学科。顺带一提，学创的 VR/AR 课上收获的 Unity 经验也为大作业最后各组件的设计提供了很多灵感，不仅是图形学和游戏，也帮助我提升了一定软件工程的思想。

从这里说开去，在一个集中的项目里集成图形学的方方面面固然是一种相当综合的训练，但是从工程的角度来说，是否接触了游戏引擎的使用之后，再利用这些知识去做工程化的项目，会更好一些呢？学期中我便有一些迷思，就是这门课变成 2 学分，带大家通过一系列小作业体会图形学的方方面面；在 VR/AR 游戏开发课程上，接触了 Unity 之后再去做工程化的项目。不过也许课程安排并不允许，只是我的一些个人的想法。
