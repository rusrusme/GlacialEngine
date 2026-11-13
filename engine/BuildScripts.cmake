if (WIN32)
    set(SCRIPT_NAME "BuildScripts.cmd")
    set(SCRIPT_PERMS "")
else ()
    set(SCRIPT_NAME "BuildScripts.sh")
    set(SCRIPT_PERMS
        FILE_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    )
endif ()

configure_file(
        "${PROJECT_SOURCE_DIR}/${SCRIPT_NAME}.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT_NAME}"
        @ONLY
        ${SCRIPT_PERMS}
)

add_custom_target(SetupBuildScripts ALL
                  COMMAND ${CMAKE_COMMAND} -E copy_if_different
                  "${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT_NAME}"
                  "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SCRIPT_NAME}"
                  COMMENT "Syncing ${SCRIPT_NAME} to runtime directory"
                  VERBATIM
)

set(SCRIPT_OUT_DIR "${RUNTIME_ASSETS}/scripts/compiled")
set(SCRIPT_BUNDLE_CPP "${SCRIPT_OUT_DIR}/GlazeScriptsBundle.cpp")

file(GLOB_RECURSE GSCRIPT_SOURCES RELATIVE "${SOURCE_ASSETS}/scripts" CONFIGURE_DEPENDS "${SOURCE_ASSETS}/scripts/*.gscript")

set(COMPILED_SCRIPTS "")
foreach (SCRIPT_REL IN LISTS GSCRIPT_SOURCES)
    set(SCRIPT_IN "${SOURCE_ASSETS}/scripts/${SCRIPT_REL}")
    cmake_path(REPLACE_EXTENSION SCRIPT_REL "hpp" OUTPUT_VARIABLE SCRIPT_OUT_REL)
    set(SCRIPT_OUT "${SCRIPT_OUT_DIR}/${SCRIPT_OUT_REL}")

    add_custom_command(
            OUTPUT "${SCRIPT_OUT}"
            COMMAND $<TARGET_FILE:glazec> "${SCRIPT_IN}" -o "${SCRIPT_OUT_DIR}" -e "${PROJECT_SOURCE_DIR}"
            DEPENDS "${SCRIPT_IN}" $<TARGET_FILE:glazec>
            COMMENT "Transpiling GlazeScript: ${SCRIPT_REL}"
            VERBATIM
    )
    list(APPEND COMPILED_SCRIPTS "${SCRIPT_OUT}")
endforeach ()

if (COMPILED_SCRIPTS)
    add_custom_command(
            OUTPUT "${SCRIPT_BUNDLE_CPP}"
            COMMAND ${CMAKE_COMMAND} -E touch_nocreate "${SCRIPT_BUNDLE_CPP}"
            DEPENDS ${COMPILED_SCRIPTS}
            VERBATIM
    )

    add_library(Scripts SHARED "${SCRIPT_BUNDLE_CPP}")
    target_precompile_headers(Scripts REUSE_FROM Engine)
    target_include_directories(Scripts PRIVATE "${SCRIPT_OUT_DIR}")
    target_link_libraries(Scripts PRIVATE Engine)

    set_target_properties(Scripts PROPERTIES
                          RUNTIME_OUTPUT_DIRECTORY "${RUNTIME_ASSETS}/scripts"
                          LIBRARY_OUTPUT_DIRECTORY "${RUNTIME_ASSETS}/scripts"
                          PREFIX ""
                          FOLDER "GlacialEngine"
    )
    target_include_directories(Engine PUBLIC "${SCRIPT_OUT_DIR}")
endif ()

add_dependencies(Engine SetupBuildScripts)
set_target_properties(SetupBuildScripts PROPERTIES FOLDER "GlacialEngine")