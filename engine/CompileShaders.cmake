file(GLOB_RECURSE SHADER_SOURCES RELATIVE "${SOURCE_ASSETS}/shaders" CONFIGURE_DEPENDS "${SOURCE_ASSETS}/shaders/*.gshader")
set(COMPILED_SHADERS "")

foreach (SHADER_REL IN LISTS SHADER_SOURCES)
    set(SHADER_IN "${SOURCE_ASSETS}/shaders/${SHADER_REL}")
    cmake_path(REPLACE_EXTENSION SHADER_REL "gsb" OUTPUT_VARIABLE SHADER_OUT_REL)
    set(SHADER_OUT "${RUNTIME_ASSETS}/shaders/compiled/${SHADER_OUT_REL}")

    add_custom_command(
            OUTPUT "${SHADER_OUT}"
            COMMAND $<TARGET_FILE:gshaderc> "${SHADER_IN}" -o "${SHADER_OUT}" -b ${GSHADER_BACKEND}
            DEPENDS "${SHADER_IN}" $<TARGET_FILE:gshaderc>
            COMMENT "Compiling engine shader: ${SHADER_REL} [${GSHADER_BACKEND}]"
            VERBATIM
    )
    list(APPEND COMPILED_SHADERS "${SHADER_OUT}")
endforeach ()

add_custom_target(CompileShaders ALL DEPENDS ${COMPILED_SHADERS})
add_dependencies(Engine CompileShaders)
set_target_properties(CompileShaders PROPERTIES FOLDER "GlacialEngine")