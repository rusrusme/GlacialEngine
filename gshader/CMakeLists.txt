cmake_minimum_required(VERSION 3.20)

project(GShader
        LANGUAGES CXX
        DESCRIPTION "Multi-backend shader compiler for Glacial Engine"
)

option(GSHADER_ENABLE_OPENGL "Enable OpenGL GLSL shader backend" ON)
option(GSHADER_ENABLE_VULKAN "Enable Vulkan SPIR-V shader backend" ON)
option(GSHADER_ENABLE_DX12 "Enable DirectX 12 DXIL shader backend" ${WIN32})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(RENDER_BACKEND_SOURCES)
if (GSHADER_ENABLE_OPENGL)
    list(APPEND RENDER_BACKEND_SOURCES
         src/platform/GLCompiler.hpp
         src/platform/GLCompiler.cpp
    )
endif ()
if (GSHADER_ENABLE_VULKAN)
    list(APPEND RENDER_BACKEND_SOURCES
         src/platform/VulkanCompiler.hpp
         src/platform/VulkanCompiler.cpp
    )
endif ()
if (GSHADER_ENABLE_DX12)
    list(APPEND RENDER_BACKEND_SOURCES
         src/platform/DX12Compiler.hpp
         src/platform/DX12Compiler.cpp
    )
endif ()

add_library(gshader_core STATIC
            # Public headers
            include/gshader/GShader.hpp
            include/gshader/GShaderCompiler.hpp
            include/gshader/Utility.hpp

            # Private
            src/Lexer.hpp
            src/Lexer.cpp

            src/Preprocessor.hpp
            src/Preprocessor.cpp

            src/Compiler.cpp
            src/Compiler.hpp

            src/BinaryPackager.hpp
            src/BinaryPackager.cpp

            src/GShaderCompiler.cpp

            src/Log.hpp
            src/Internal.hpp

            ${RENDER_BACKEND_SOURCES}
)

target_include_directories(gshader_core
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(gshader_core PUBLIC cxx_std_20)
include(${CMAKE_CURRENT_SOURCE_DIR}/RenderBackend.cmake)

add_executable(gshaderc src/main.cpp)
target_link_libraries(gshaderc PRIVATE gshader_core)

add_library(INTERNAL::GSHADER ALIAS gshader_core)
