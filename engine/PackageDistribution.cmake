# engine/PackageDistribution.cmake
if (ENGINE_DEBUG_DIST OR ENGINE_RELEASE_DIST)
    if (ENGINE_DEBUG_DIST)
        set(DIST_CONFIG_DIR "debug")
    else ()
        set(DIST_CONFIG_DIR "release")
    endif ()

    add_custom_command(TARGET Engine POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_BINARY_DIR}/lib"
                       "${CMAKE_SOURCE_DIR}/engine-lib/lib/${DIST_CONFIG_DIR}"

                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_CURRENT_SOURCE_DIR}/include"
                       "${CMAKE_SOURCE_DIR}/engine-lib/include"

                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_SOURCE_DIR}/assets"
                       "${CMAKE_SOURCE_DIR}/engine-lib/assets"

                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_SOURCE_DIR}/lib/glad/include"
                       "${CMAKE_SOURCE_DIR}/engine-lib/include"
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                       "${CMAKE_SOURCE_DIR}/lib/stb_image"
                       "${CMAKE_SOURCE_DIR}/engine-lib/include"

                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${glm_SOURCE_DIR}/glm" "${CMAKE_SOURCE_DIR}/engine-lib/include/glm"
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${entt_SOURCE_DIR}/src/entt" "${CMAKE_SOURCE_DIR}/engine-lib/include/entt"
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${json_SOURCE_DIR}/include/nlohmann" "${CMAKE_SOURCE_DIR}/engine-lib/include/nlohmann"
                       COMMAND ${CMAKE_COMMAND} -E copy "${cgltf_SOURCE_DIR}/cgltf.h" "${CMAKE_SOURCE_DIR}/engine-lib/include/cgltf.h"
                       COMMAND ${CMAKE_COMMAND} -E copy "${miniaudio_SOURCE_DIR}/miniaudio.h" "${CMAKE_SOURCE_DIR}/engine-lib/include/miniaudio.h"
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${glfw_SOURCE_DIR}/include/GLFW" "${CMAKE_SOURCE_DIR}/engine-lib/include/GLFW"
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${rmlui_SOURCE_DIR}/Include/RmlUi" "${CMAKE_SOURCE_DIR}/engine-lib/include/RmlUi"
                       COMMAND ${CMAKE_COMMAND} -E copy_directory "${freetype_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/engine-lib/include"

                       COMMENT "Packaging ${DIST_CONFIG_DIR} distribution into engine-lib/..."
    )
endif ()