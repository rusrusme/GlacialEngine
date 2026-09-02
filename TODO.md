Future features:

- Frustum culling DONE
- Shadow mapping
    - Consider adding light-space frustum culling 
    - Consider optimizing two-pass rendering 
        - Consider refactoring into a CastsShadowComponent (Reason: 2 draw calls per object)
- Post-processing pipeline
- Refined asset manager
    - Drag and drop textures on objects
- More precise scene picking
- Simulation state management
    - Edit, Play, Pause
- Environment lighting
- Audio-subsystem
    - Wrap miniaudio to handle audio
- Low-level RHI abstraction
- Undo / Redo command buffer
- File I/O
    - Proper menus