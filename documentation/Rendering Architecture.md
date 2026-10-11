### GPU Pipeline

1. [x] Indirect Command Buffer (GPU)
2. [x] Instance Buffer (GPU)
3. [ ] Cull Pass (GPU)
4. [x] Frustum & AABB (GPU)
5. [ ] Image-Based Lighting (GPU)
6. [x] HZB & Occlusion Culling (GPU)
7. [ ] PSO & Memory Barriers (GPU)
8. [ ] LOD Management (GPU)

### High-Level API

1. [ ] SceneRenderer (CPU)
2. [ ] Render Graph (CPU)
3. [ ] RenderPass Interface (CPU)
4. [ ] CSM (Cascaded Shadow Maps)
5. [ ] Render Pass Execution Queue
6. [ ] Shadow Pass
7. [ ] Depth Prepass
8. [ ] Light Cull Pass
9. [ ] Main Opaque Pass
10. [ ] AlphaTest Pass
11. [ ] SkyBox Pass
12. [ ] Transparent Pass
13. [ ] Post Process Pass
14. [ ] UI Render Pass

### Low-Level RHI

1. [x] FrameData UBO (CPU)
2. [x] RenderCommand & Command Buffers (CPU/GPU)
3. [x] Command Pool (CPU)
4. [x] RendererAPI Backends (DX12, Vulkan, OpenGL 4.6)
5. [x] Framebuffer / Swapchain (CPU/GPU)
6. [x] Descriptors & Resource Binding Sets (Set 0, Set 1, Set 2)
7. [x] BufferLayout & VAO (OpenGL) / Pipeline Layout (Vulkan/DX12)
8. [x] Shader & ShaderLibrary (CPU)
9. [x] Sampler & TextureView (CPU/GPU)
10. [x] MemoryBarrier & CPU-GPU Fence (CPU/GPU)

### Assets

1. [ ] AssetManager (CPU)
2. [ ] UUID Mapping (CPU)
3. [ ] Mesh Asset (CPU)
4. [ ] Material Asset (CPU)

### Data Containers

1. [x] Vertex Struct (CPU)
2. [x] Texture2D Raw Bytes (CPU)
3. [x] Buffer Staging Wrappers (CPU)
