Basis Engine - Rendering Architecture
├── GPU Pipeline
│   ├── Indirect Command Buffer (GPU)
│   ├── Instance Buffer (GPU)
│   ├── Cull Pass (GPU)
│   ├── Frustum & AABB (GPU)
│   ├── Image-Based Lighting (GPU)
│   ├── HZB & Occlusion Culling (GPU)
│   ├── PSO & Memory Barriers (GPU)
│   └── LOD Management (GPU)
├── High-Level API
│   ├── SceneRenderer (CPU)
│   ├── Render Graph (CPU)
│   ├── RenderPass Interface (CPU)
│   ├── CSM (Cascaded Shadow Maps)
│   └── Render Pass Execution Queue
│       ├── Shadow Pass
│       ├── Depth Prepass
│       ├── Light Cull Pass
│       ├── Main Opaque Pass
│       ├── AlphaTest Pass
│       ├── SkyBox Pass
│       ├── Transparent Pass
│       ├── Post Process Pass
│       └── UI Render Pass
├── Low-Level RHI
│   ├── FrameData UBO (CPU)
│   ├── RenderCommand & Command Buffers (CPU/GPU)
│   ├── Command Pool (CPU)
│   ├── RendererAPI Backends (DX12, Vulkan, OpenGL 4.6)
│   ├── Framebuffer / Swapchain (CPU/GPU)
│   ├── Descriptors & Resource Binding Sets (Set 0, Set 1, Set 2)
│   ├── BufferLayout & VAO (OpenGL) / Pipeline Layout (Vulkan/DX12)
│   ├── Shader & ShaderLibrary (CPU)
│   ├── Sampler & TextureView (CPU/GPU)
│   └── MemoryBarrier & CPU-GPU Fence (CPU/GPU)
├── Assets
│   ├── AssetManager (CPU)
│   ├── UUID Mapping (CPU)
│   ├── Mesh Asset (CPU)
│   └── Material Asset (CPU)
└── Data Containers
├── Vertex Struct (CPU)
├── Texture2D Raw Bytes (CPU)
└── Buffer Staging Wrappers (CPU)

