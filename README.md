# Basis Engine

---

## Getting Started

### Prerequisites

* C++20 compatible compiler
* CMake 4.3 or higher

---

### Building the Project

1. Clone the repository:
    ```bash
    git clone https://github.com/rusrusme/BasisEngine.git
    ```

2. Standard Build (Engine + Editor)
    ```bash
    cmake -B build
    cmake --build build
    ```

---

### Creating the Entry Point

This repository contains an `engine/` and `editor/` directory. Create an additional directory and set up a
`CMakeLists.txt` with the following:
```cmake
# Set the binary output directory if you wish
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(Sandbox
               Sandbox.cpp
               Sandbox.hpp)

# Link executable with the engine
target_link_libraries(Sandbox PRIVATE Engine)
```

---

Next add a class inheriting the application. Here you see an example with Sandbox:

**Sandbox.hpp:**
```c++
#pragma once

#include "core/Application.hpp"

class Sandbox : public engine::Application {
public:
    Sandbox();
    ~Sandbox() override = default;
};
```

**Sandbox.cpp:**
```c++
#include "Sandbox.hpp"

Sandbox::Sandbox(): Application("Sandbox") {}

int main() {
    Sandbox sandbox;
    sandbox.run();
}
```

---

Try to run it. If you have done everything correctly you will see a dark gray screen. With that, the entry point is done! Everything else is in the documentation.

---

## Basis Engine Documentation