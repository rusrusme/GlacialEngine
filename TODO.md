# Todo List

- [ ] Light Entities (ECS Integration)
- Auslagern des hardcoded Lichts in eine DirectionalLightComponent und PointLightComponent. Der Renderer liest im
  Render-Pass einfach alle Licht-Komponenten aus dem EnTT-Registry aus.

<p>

- [ ] Serialisierung (Scene Save / Load)
- Ein universelles Dateiformat (z. B. YAML oder JSON via nlohmann/json), das den EnTT-Registry-Zustand, Entitäten und
  Komponenten in eine .scene-Datei schreibt und wieder einliest.

<p>

- [ ] Asset Pipeline & Management
- Ein zentraler AssetManager, der Texturen, Meshes und Shaders über eindeutige UUIDs oder Handles lädt, im Speicher hält
  und automatisiertes Ref-Counting übernimmt.

<p>

- [ ] Collision & Physics Engine
- Generische AABB-/OBB-Kollisionsprüfungen, Ray-Collider-Intersections und Trigger-Events direkt auf Engine-Ebene.

<p>

- [ ] In-Game UI System
- Ein eigenes UI-Canvas (oder die Integration einer Library wie RmlUi), da ImGui nur als Editor- und Debug-Overlay
  dient.

<p>

- [ ] Audio Subsystem
- Integration einer leichtgewichtigen Audio-Library (z. B. miniaudio oder SoLoud) für 2D-Sound und 3D-Spatial-Audio
  basierend auf der Kamera-Transformation.

<p>

- [ ] Instanced Rendering mit glDrawElementsInstanced