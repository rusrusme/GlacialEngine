#### Rendering Architectural Migration

---

* With `(?)` marked todos need revision / fact-checks
* With `(!)` marked todos require major effort
---
* Track progress with:
    * **DONE**
    * **PENDING**
    * **POSTPONED**
---

**Not in implementation order:**

* RenderQueue
    * Sorting Phase
      * Opaque: front to back
      * Transparent: back to front 
* Frustum + Occlusion Cull (GPU) `(!)`
* GPU-driven computation (!?)
* Offscreen HDR Framebuffer & Tone-mapping `(?)`
    * Post-Processing Pass
    * Tonemap-Pass `(?)`
* Shadow Pass (CSM & Point Light Cube-maps)
    * Directional Light 
    * Point Lights
* RenderGraph (Pass-Pipeline)
    * Shadow Pass   
    * Depth-Pre Pass 
    * Opaque Pass
    * Skybox (? order)
    * Transparent Pass
    * Post-Process Pass
    * UI Pass

---

#### Old todos, which are partially complete and need a revision:

Future features:

- Frustum culling
    - Object
    - Camera **DONE**
    - Light-space
- LODs
- Shadow mapping **DONE**
- Post-processing pipeline **PENDING**
- Asset manager **PENDING**
    - Drag and drop textures on objects
- More precise scene picking **PENDING**
- Simulation state management **DONE**
    - Edit, Play, Pause **DONE**
- Environment lighting
- Audio-subsystem **DONE**
    - Wrap miniaudio to handle audio **DONE**
- Low-level RHI abstraction **PENDING**
- Undo / Redo command buffer
- File I/O **DONE**
    - Proper menus **DONE**

By splitting your rendering code into dynamic link libraries (DLLs), your Game.exe remains completely decoupled from the specific graphics API.
Here is how you can achieve this without ruining your clean, compile-time architecture:

1. The Architecture

Instead of using your compile-time switch (#if defined) inside the Game.exe source code, you move that switch to your build system (like CMake).
You compile your rendering source files into two entirely separate DLL targets:
• GLRendering.dll (Compiled with -DRENDER_API_OPENGL)
• VulkanRendering.dll (Compiled with -DRENDER_API_VULKAN)
Inside these DLLs, your classes (like GLFramebuffer or VulkanFramebuffer) still use your ultra-fast, zero-overhead compile-time aliases. They do not use virtual functions internally.

2. The Bridge: A Minimal Dynamic Interface

To connect Game.exe to these DLLs, you only need one single abstract interface at the very top level of your rendering library—usually a RenderDriver or IRenderer interface.
Your Game.exe only talks to this one interface:
cpp
// Inside Game.exe / Common Header
class IRenderer {
public:
virtual ~IRenderer() = default;
virtual void Initialize() = 0;
virtual void BeginFrame() = 0;
virtual void EndFrame() = 0;
// Note: You do NOT put individual Framebuffer or Buffer virtual calls here!
};
Verwende Code mit Vorsicht.

3. Loading the API on Startup

When your Game.exe boots up, it reads a config file (e.g., api=vulkan). It then uses standard OS calls to load the specific DLL and grab the entry point function:
cpp
// Windows API Example inside Game.exe startup
typedef IRenderer* (*CreateRendererFunc)();

IRenderer* load_render_backend(const std::string& api_type) {
std::string dll_name = (api_type == "vulkan") ? "VulkanRendering.dll" : "GLRendering.dll";

    HMODULE hModule = LoadLibraryA(dll_name.c_str());
    if (!hModule) return nullptr;

    // Grab the factory function exported by the DLL
    CreateRendererFunc create_fn = (CreateRendererFunc)GetProcAddress(hModule, "create_renderer");
    return create_fn();
}
Verwende Code mit Vorsicht.

Why this is the ultimate solution for your engine:

1. Best of Both Worlds: Inside VulkanRendering.dll, everything is perfectly optimized at compile time. There is zero virtual function overhead for binding framebuffers, updating uniform buffers, or drawing meshes. The only virtual overhead happens 2 or 3 times per frame when Game.exe calls BeginFrame() or EndFrame().
2. Single Executable: You ship one single Game.exe alongside your two tiny DLLs. If a user wants to switch APIs, your game edits the config file and prompts a restart. Upon restart, Game.exe simply loads the other DLL.
3. Failsafe Backends: If LoadLibraryA("VulkanRendering.dll") fails because the user doesn't have Vulkan drivers, your C++ code can instantly fallback to loading GLRendering.dll instead of crashing.
   Are you using CMake or a similar build system for your engine? If you want, I can show you how to structure the build target so it compiles the exact same rendering source code into two separate DLLs automatically.
