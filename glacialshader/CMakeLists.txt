cmake_minimum_required(VERSION 3.20)

project(glacialc
        LANGUAGES CXX
        DESCRIPTION "Multi-backend shader compiler for Glacial Engine"
)

option(GLACIALC_ENABLE_OPENGL "Enable OpenGL GLSL shader backend" ON)
option(GLACIALC_ENABLE_VULKAN "Enable Vulkan SPIR-V shader backend" ON)
option(GLACIALC_ENABLE_DX12 "Enable DirectX 12 DXIL shader backend" ${WIN32})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(RENDER_BACKEND_SOURCES)
if (GLACIALC_ENABLE_OPENGL)
    list(APPEND RENDER_BACKEND_SOURCES src/platform/opengl/GLCompiler.cpp)
endif ()
if (GLACIALC_ENABLE_VULKAN)
    list(APPEND RENDER_BACKEND_SOURCES src/platform/vulkan/VulkanCompiler.cpp)
endif ()
if (GLACIALC_ENABLE_DX12)
    list(APPEND RENDER_BACKEND_SOURCES src/platform/dx12/DX12Compiler.cpp)
endif ()

add_library(glacialc_core STATIC
            # Public headers
            include/glacialc/BinaryPackager.hpp
            include/glacialc/Compiler.hpp
            include/glacialc/GShader.hpp
            include/glacialc/GShaderCompiler.hpp
            include/glacialc/Lexer.hpp
            include/glacialc/Log.hpp
            include/glacialc/Preprocessor.hpp
            include/glacialc/Utility.hpp
            include/glacialc/platform/dx12/DX12Compiler.hpp
            include/glacialc/platform/opengl/GLCompiler.hpp
            include/glacialc/platform/vulkan/VulkanCompiler.hpp

            # Private
            src/Lexer.cpp
            src/Preprocessor.cpp
            src/Compiler.cpp
            src/BinaryPackager.cpp
            src/GShaderCompiler.cpp
            ${RENDER_BACKEND_SOURCES}
)

target_include_directories(glacialc_core
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(glacialc_core PUBLIC cxx_std_20)
include(${CMAKE_CURRENT_SOURCE_DIR}/RenderBackend.cmake)

add_executable(glacialc src/main.cpp)
target_link_libraries(glacialc PRIVATE glacialc_core)

add_library(INTERNAL::GLACIALSHADER ALIAS glacialc_core)
