<img width="1200" height="720" alt="图片2" src="https://github.com/user-attachments/assets/7847becc-45e5-4f6e-8c21-e65961bd60e3" />


[English](#english) | [中文](#chinese)

---

<a name="english"></a>
## Physically Based Renderer with Global Illumination

A from-scratch, CPU-based offline renderer that supports Global Illumination (GI) and Physically Based Rendering (PBR), designed to produce highly realistic images.

### ✨ Core Features

-   **Dual Rendering Engines**: Integrates both **Path Tracing** and **Photon Mapping** core rendering engines.
-   **Physical Accuracy**: Implements a complete **Bidirectional Reflectance Distribution Function (BRDF)** model to ensure physically accurate material and light interactions.
-   **High-Performance Task Scheduling**: Features a highly reliable and thread-safe **rendering task queue** and **thread pool** for efficient management of CPU computational resources.
-   **High-Quality Output**: Supports saving rendered images in lossless formats like **PPM** and **TGA**.

### 🖥️ Interactive GUI

-   **Immediate Mode GUI**: Developed using an **Immediate mode GUI (IMGUI)** framework for a smooth and user-friendly experience.
-   **Scene Construction**: Allows real-time adjustment of camera parameters, render settings, and dynamic addition/modification of models and light sources.
-   **Model Import**: Enables quick import of **OBJ format** model files and assignment of physical materials.
-   **Real-Time Preview**: Provides a real-time preview of the render directly within the application window during the rendering process.

### 🛠️ Technical Implementation

-   **Pure C++ Implementation**: All core rendering algorithms (ray tracing, geometric calculations, shading, etc.) are implemented in pure C++ without relying on any graphics APIs (e.g., OpenGL, DirectX) or ray tracing acceleration libraries (e.g., Intel® Embree).
-   **OpenGL for Display Only**: The OpenGL API is used solely for creating and maintaining the IMGUI application window and is not involved in rendering computations.

---

<a name="chinese"></a>
## 基于物理的全局光照渲染器

一个从零实现的、基于CPU的离线渲染器，支持全局光照（GI）与基于物理的渲染（PBR），旨在生成高度真实的画面。

### ✨ 核心特性

-   **双渲染引擎**：集成了**路径追踪（Path Tracing）** 与**光子映射（Photon Mapping）** 两大核心渲染引擎。
-   **物理真实性**：实现了完整的**双向反射分布函数（BRDF）** 模型，确保材质与光照交互的物理准确性。
-   **高性能任务调度**：内置高可靠性、线程安全的**渲染任务队列**与**线程池**，高效管理CPU计算资源。
-   **高质量输出**：支持以 **PPM**、**TGA** 等无损图像格式保存渲染成品。

### 🖥️ 交互式图形界面

-   **即时GUI**：基于 **Immediate mode GUI (IMGUI)** 框架开发，提供流畅便捷的操作体验。
-   **场景构建**：支持实时调整摄像机参数、渲染设置、动态添加/修改模型与光源。
-   **模型导入**：可快速导入 **OBJ格式** 模型文件，并为模型指定物理材质。
-   **实时预览**：在渲染过程中，可直接在应用窗口内实时预览画面效果。

### 🛠️ 技术实现

-   **纯C++实现**：所有渲染核心算法（光线追踪、几何计算、着色处理等）均使用纯C++编写，不依赖任何图形API（如OpenGL、DirectX）或光线追踪加速库（如Intel® Embree）。
-   **OpenGL仅用于显示**：代码中调用的OpenGL接口仅用于创建和维护IMGUI交互窗口，与渲染计算无关。
