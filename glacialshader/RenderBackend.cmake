# shaderc is required as the GLSL front-end for all backends
find_package(Vulkan REQUIRED)

if (NOT TARGET Vulkan::Shaderc)
    if (TARGET Vulkan::shaderc_combined)
        add_library(Vulkan::Shaderc ALIAS Vulkan::shaderc_combined)
    elseif (TARGET Vulkan::shaderc_shared)
        add_library(Vulkan::Shaderc ALIAS Vulkan::shaderc_shared)
    else ()
        find_library(SHADERC_LIB
                     NAMES shaderc_shared shaderc_combined
                     HINTS $ENV{VULKAN_SDK}/Lib
        )
        add_library(Vulkan::Shaderc UNKNOWN IMPORTED)
        set_target_properties(Vulkan::Shaderc PROPERTIES
                              IMPORTED_LOCATION "${SHADERC_LIB}"
                              INTERFACE_INCLUDE_DIRECTORIES "${Vulkan_INCLUDE_DIRS}"
        )
    endif ()
endif ()

target_link_libraries(glacialc_core PUBLIC Vulkan::Shaderc)

if (GLACIALC_ENABLE_OPENGL)
    target_compile_definitions(glacialc_core PUBLIC GLACIALC_BACKEND_OPENGL)
endif ()

if (GLACIALC_ENABLE_VULKAN)
    target_compile_definitions(glacialc_core PUBLIC GLACIALC_BACKEND_VULKAN)
endif ()

if (GLACIALC_ENABLE_DX12)
    if (NOT WIN32)
        message(FATAL_ERROR "GLACIALC_ENABLE_DX12 requires Windows (dxcompiler.lib)")
    endif ()

    if (NOT TARGET spirv-cross-hlsl)
        include(FetchContent)
        set(SPIRV_CROSS_ENABLE_HLSL ON CACHE BOOL "" FORCE)
        set(SPIRV_CROSS_ENABLE_GLSL OFF CACHE BOOL "" FORCE)
        set(SPIRV_CROSS_ENABLE_CPP OFF CACHE BOOL "" FORCE)
        set(SPIRV_CROSS_ENABLE_REFLECT OFF CACHE BOOL "" FORCE)
        set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
        set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(
                spirv_cross
                GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
                GIT_TAG vulkan-sdk-1.4.363.0
        )
        FetchContent_MakeAvailable(spirv_cross)
    endif ()

    target_compile_definitions(glacialc_core PUBLIC GLACIALC_BACKEND_DX12)
    target_link_libraries(glacialc_core PRIVATE spirv-cross-hlsl dxcompiler.lib)
endif ()