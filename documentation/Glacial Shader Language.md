# Glacial Shader Language (GShader) - Language Specification

---

## 1. Overview & Architecture

The **Glacial Shader Language (GShader)** is a unified, single-file shader language created for the **Glacial Engine**. GShader shaders are preprocessed and compiled by the **glacialc (Glacial Shader Compiler)** tool into binary shader assets (`.gsb`) targeting OpenGL, Vulkan, and DirectX 12 backends.

Key features include:
* Single-file multi-stage shader definitions (`#vert`, `#frag`, `#geom`, `#comp`).
* Built-in preprocessor directives (`#include`, `#pragma once`, `#define`, etc.).
* C/GLSL-compliant expression hierarchy, layout qualifiers, interface blocks (`uniform`, `buffer`), and custom `struct` declarations.

---

## 2. Preprocessor & Stage Directives

A `.gshader` file consists of top-level preprocessor directives, interface blocks, struct declarations, global variables, and function definitions.

### Stage Directives
* `#vert` : Marks Vertex Shader stage code.
* `#frag` : Marks Fragment Shader stage code.
* `#geom` : Marks Geometry Shader stage code.
* `#comp` : Marks Compute Shader stage code.

### Preprocessor Directives
* `#include "path/to/shader.glsl"` : Includes external shader header files.
* `#pragma once` : Guarantees single-inclusion for shader header files.
* `#version <number> [profile]` : Declares GLSL compatibility version.
* `#define <IDENTIFIER> [expr]` : Defines preprocessor macros.
* `#ifdef <IDENTIFIER>` / `#ifndef <IDENTIFIER>` / `#endif` : Conditional compilation blocks.

---

## 3. Tokens & Lexical Rules

### Identifiers & Literals
* **Identifier**: `[a-zA-Z_][a-zA-Z0-9_]*`
* **Numeric Literal**: `[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?[fF]?` (Integers, Floats, Scientific Exponents, Float Suffixes)
* **Boolean Literals**: `true`, `false`
* **String Literal**: `"..."`
* **Comments**: Single-line (`// ...`) and multi-line block comments (`/* ... */`).

### Keywords & Modifiers
* **Control Flow**: `if`, `else`, `while`, `for`, `do`, `switch`, `case`, `default`, `break`, `continue`, `return`, `discard`
* **Interface & Storage**: `layout`, `uniform`, `buffer`, `in`, `out`, `const`, `struct`
* **Function Parameter Modifiers**: `in`, `out`, `in out`
* **Pragma Modifiers**: `once`

---

## 4. Type System

### Scalar Types
`void`, `bool`, `int`, `uint`, `float`, `double`

### Vector Types
* **Floating-Point**: `vec2`, `vec3`, `vec4`
* **Integer**: `ivec2`, `ivec3`, `ivec4`
* **Unsigned Integer**: `uvec2`, `uvec3`, `uvec4`
* **Boolean**: `bvec2`, `bvec3`, `bvec4`
* **Double-Precision**: `dvec2`, `dvec3`, `dvec4`

### Matrix Types
* **Single-Precision**: `mat2`, `mat3`, `mat4`
* **Double-Precision**: `dmat2`, `dmat3`, `dmat4`

### Texture Samplers & Storage Images
* **Samplers**: `sampler1D`, `sampler2D`, `sampler3D`, `samplerCube`, `sampler2DArray`, `sampler2DShadow`, `isampler2D`, `usampler2D`
* **Storage Images**: `image2D`

### Custom Structs
```gshader
struct GPUInstanceData {
    mat4 model_matrix;
    vec4 bounding_center_radius;
    vec4 bounding_box_min;
    vec4 bounding_box_max;
    uint material_id;
    uint mesh_id;
    uint flags;
    uint padding;
};
```

---

## 5. Memory & Interface Declarations

### Layout Qualifiers
Variables and interface blocks accept explicit layout parameters:

```gshader
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
layout(location = 0) uniform int u_IsFirstPass;
layout(r32f, binding = 0) readonly uniform image2D u_InputHZB;
```

### Interface Blocks (UBOs & SSBOs)

**Uniform Buffer Block (`uniform`):**
```gshader
layout(std140, binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 projection;
    mat4 view_projection;
    vec4 camera_position;
    vec4 frustum_planes[6];
    vec2 viewport_size;
    float near_plane;
    float far_plane;
} camera;
```

**Shader Storage Buffer Block (`buffer`):**
```gshader
layout(std430, binding = 0) buffer InstanceBuffer {
    GPUInstanceData instances[];
};
```

---

## 6. Operator Precedence & Expression Syntax

1. **Primary Expressions**: Identifiers, Numeric/Bool/String Literals, Type Constructors (e.g., `vec4(0.0)`), Parenthesized expressions `(expr)`.
2. **Postfix Operators**: Member Access (`.`), Array Access (`[]`), Function Calls (`()`), Post-Increment (`++`), Post-Decrement (`--`).
3. **Unary Operators**: Prefix `++`, `--`, `+`, `-`, Logical Not (`!`), Bitwise Not (`~`).
4. **Multiplicative Operators**: `*`, `/`, `%`.
5. **Additive Operators**: `+`, `-`.
6. **Bitwise Shift**: `<<`, `>>`.
7. **Relational Operators**: `<`, `>`, `<=`, `>=`.
8. **Equality Operators**: `==`, `!=`.
9. **Bitwise Operators**: `&`, `^`, `|`.
10. **Logical Operators**: `&&`, `||`.
11. **Ternary Operator**: `cond ? expr1 : expr2`.
12. **Assignment Operators**: `=`, `+=`, `-=`, `*=`, `/=`, `%=`, `<<=`, `>>=`, `&=`, `|=`, `^=`.

---

## 7. Concrete GShader Code Example

```gshader
#comp
#pragma once

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(r32f, binding = 0) readonly  uniform image2D u_InputHZB;
layout(r32f, binding = 1) writeonly uniform image2D u_OutputHZB;

void main() {
    ivec2 output_pos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 output_size = imageSize(u_OutputHZB);

    if (output_pos.x >= output_size.x || output_pos.y >= output_size.y) {
        return;
    }

    ivec2 input_pos = output_pos * 2;

    float d0 = imageLoad(u_InputHZB, input_pos + ivec2(0, 0)).r;
    float d1 = imageLoad(u_InputHZB, input_pos + ivec2(1, 0)).r;
    float d2 = imageLoad(u_InputHZB, input_pos + ivec2(0, 1)).r;
    float d3 = imageLoad(u_InputHZB, input_pos + ivec2(1, 1)).r;

    float max_depth = max(max(d0, d1), max(d2, d3));
    imageStore(u_OutputHZB, output_pos, vec4(max_depth, 0.0, 0.0, 0.0));
}
```