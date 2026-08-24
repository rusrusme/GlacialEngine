# CoreEngine

### C++ 3D simulation engine built with OpenGL 4.6.

---

## Tech Stack & Dependencies

* **Language:** C++20
* **Graphics API:** OpenGL 4.6 (via GLAD)
* **Windowing & Input:** GLFW
* **Mathematics:** GLM
* **Build System:** CMake

---

## Getting Started

### Prerequisites

* C++20 compatible compiler (MSVC 2022 / GCC 11+ / Clang 13+)
* CMake 3.20 or higher

### Building the Project

1. Clone the repository:
    ```bash
    git clone https://github.com/rusrusme/CoreEngine.git
    ```

2. Standard Build (Engine + Editor)
    ```bash
    cmake -B build
    cmake --build build
    ```
3. Packaging the engine distribution (engine-lib/)
    ```bash
    .\Deploy.cmd
    ```