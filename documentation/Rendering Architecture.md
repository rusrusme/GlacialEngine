### GPU Pipeline

1. [x] Indirect Command Buffer (GPU)
2. [x] Instance Buffer (GPU)
3. [ ] Cull Pass (GPU)
4. [x] Frustum & AABB (GPU)
5. [ ] Image-Based Lighting (GPU)
6. [x] HZB & Occlusion Culling (GPU)
7. [x] PSO & Memory Barriers (GPU)
8. [ ] LOD Management (GPU)

### High-Level API

1. [ ] SceneRenderer (CPU)
2. [ ] Render Graph (CPU)
3. [ ] RenderPass Interface (CPU)
4. [ ] CSM (Cascaded Shadow Maps)
5. [ ] Render Pass Execution Queue
   1. [ ] Shadow Pass
   2. [ ] Depth Prepass
   3. [ ] Light Cull Pass
   4. [ ] Main Opaque Pass
   5. [ ] AlphaTest Pass
   6. [ ] SkyBox Pass
   7. [ ] Transparent Pass
   8. [ ] Post Process Pass
   9. [ ] UI Render Pass

### Low-Level RHI

1. [x] FrameData UBO (CPU)
2. [x] RenderCommand & Command Buffers (CPU/GPU)
3. [x] Command Pool (CPU)
4. [x] RenderCommandQueue (CPU Thread-Submission)
5. [x] RendererAPI Backends (DX12, Vulkan, OpenGL 4.6)
6. [x] Framebuffer / Swapchain (CPU/GPU)
7. [x] Descriptors & Resource Binding Sets (Set 0, Set 1, Set 2)
8. [x] BufferLayout & VAO (OpenGL) / Pipeline Layout (Vulkan/DX12)
9. [x] Shader & ShaderLibrary (CPU)
10. [x] Sampler & TextureView (CPU/GPU)
11. [x] MemoryBarrier & CPU-GPU Fence (CPU/GPU)

### Assets

1. [ ] AssetManager (CPU)
2. [ ] UUID Mapping (CPU)
3. [ ] Mesh Asset (CPU)
4. [ ] Material Asset (CPU)

### Data Containers

1. [x] Vertex Struct (CPU)
2. [x] Texture2D Raw Bytes (CPU)
3. [x] Buffer Staging Wrappers (CPU)
