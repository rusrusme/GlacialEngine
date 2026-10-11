# Basis Engine - Milestone Specification: Bindless Material & Global Mesh Management

## 1. Context & Architecture Overview

The **Basis Engine** GPU-driven rendering pipeline has successfully verified the following components:
* **Indirect Draw Pipeline**: `glMultiDrawElementsIndirect` / `GPUDrawCommand` with strict 20-byte unpadded `std430` layout.
* **GPU Compute Culling**: Combined Sphere Frustum Culling (`frustum_cull.bsh`) and 2-Pass HZB Occlusion Culling (`hzb_initial.bsh`, `hzb_reduce.bsh`, `occlusion_cull.bsh`).
* **RHI Abstraction**: Platform-agnostic `DescriptorSet`, `ComputePipeline`, `UniformBuffer`, `ShaderStorageBuffer`, and `Texture2D`.

---

## 2. Milestone Objective

Transition the forward/deferred main rendering pass from traditional CPU-side per-object texture/material binding to a **GPU-Driven Bindless Material & Mesh System**.

The main pass shader must fetch per-instance material properties, texture indices, and vertex attributes directly using `instance.material_id` and `instance.mesh_id` provided by `GPUInstanceData`.

---

## 3. Detailed Technical Requirements

### A. Material Data Layout (`render/gpu/GPUData.hpp`)
Add the GPU material struct with strict alignment matching GLSL `std430` layout:

```cpp
/**
 * @brief Per-material data stored in global Material SSBO (std430 layout).
 */
struct alignas(16) GPUMaterialData {
    glm::vec4 albedo_factor{1.0f, 1.0f, 1.0f, 1.0f};
    
    float metallic_factor{1.0f};
    float roughness_factor{1.0f};
    float normal_scale{1.0f};
    float ao_strength{1.0f};

    uint32_t albedo_texture_index{0};
    uint32_t normal_texture_index{0};
    uint32_t metallic_roughness_index{0};
    uint32_t ao_texture_index{0};

    uint32_t flags{0};
    uint32_t padding[3]{0, 0, 0}; // Pad to 16-byte boundary (64 bytes total) (If it doesn't cause the alignment bug)
};
```

### B. Global Material Manager (render/gpu/MaterialSystem.hpp / .cpp)
Implement a centralized manager for materials:

Maintain a host-side std::vector<GPUMaterialData> and backing ShaderStorageBuffer (material_ssbo_).

Provide methods to register/update materials and assign unique uint32_t material_ids.

Manage bindless texture handles or global texture array descriptors.

Bind the Material SSBO to slot 2 (or DescriptorSet index) during scene rendering passes.

### C. Mesh Buffer & Submesh Data Structure
Extend mesh management to support GPU-driven indirect drawing:

Global Vertex & Index Buffers: Consolidate mesh geometry into large global mega-buffers or bindless vertex buffers.

Submesh Descriptor Buffer: Store mesh bounding boxes (bounding_box_min, bounding_box_max) and submesh offsets (first_index, base_vertex) in SSBO for compute cull passes.

### D. Main Pass Shaders Update (assets/shaders/engine/mesh.vert & mesh.frag)
Update vertex and fragment shaders to utilize the global buffers:

Vertex Shader (mesh.vert):

Read GPUInstanceData via SSBO binding or gl_BaseInstance / gl_DrawID.

Pass material_id and interpolated vertex outputs to the Fragment Shader.

Fragment Shader (mesh.frag):

Read GPUMaterialData from MaterialBuffer using fs_in.material_id.

Fetch PBR textures using texture array / bindless handles based on albedo_texture_index, etc.

Execute standard Metallic-Roughness PBR lighting.

## 4. Implementation Steps & Checklist
[ ] Task 1: Add GPUMaterialData to render/gpu/GPUData.hpp and verify std430 alignment (64 bytes).

[ ] Task 2: Create MaterialSystem class to manage materials and host the global Material SSBO.

[ ] Task 3: Update GLDescriptorSet / RHI to support global Material SSBO binding for graphics pipelines.

[ ] Task 4: Create/Update main PBR shaders (mesh.vert / mesh.frag or BSH equivalents) to sample materials bindlessly using material_id.

[ ] Task 5: Create a validation layer or test in GPUTest verifying multi-material rendering without per-draw CPU binding.

## 5. Strict Guidelines for Hermes Agent
RHI Abstraction Constraint: Absolutely NO raw OpenGL calls (gl*) outside the render/rhi/opengl/ directory. All GPU operations must go through RHI interfaces (Buffer, Texture, DescriptorSet, CommandBuffer).

Memory Layout Discipline:

GPUDrawCommand must remain exactly 20 bytes (unpadded).

SSBO structs must strictly respect GLSL std430 alignment rules.

Engine Coding Standards:

Use C++20 features.

Use CORE_INFO, CORE_WARN, CORE_ERROR for logging.

Maintain clean headers with #pragma once and proper namespace wrapping (namespace engine).