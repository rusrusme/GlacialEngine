# AGENTS.md

Workflow and project guidelines for BasisEngine, including build commands, execution steps, target outputs, and documentation rules.

--- 

### Documentation Guidelines
* Store all documentation in /documentation/.
* Maintain up-to-date documentation on:
    * Current features
    * Completed features (tracked using tick boxes - [x])
* Existing documentation files:
  * `Basis Shader Language.md`
  * `Rendering Architecture.md`
  * `Roadmap - Bindless Materials.md`

---

### Tracking Skills
Skills useful for future use should be saved.

---

### Build Command
Build the project using CMake:

```bash
cmake.exe --build cmake-build-debug --target Engine --config Debug -- -j 10
```

### Running Executables
After building the Engine library, execute the compiled executables directly from the output directory:

```bash
./cmake-build-debug/bin/FramebufferTest.exe
# or
./cmake-build-debug/bin/Editor.exe
```
