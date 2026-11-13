file(GLOB_RECURSE ASSET_FILES RELATIVE "${SOURCE_ASSETS}" CONFIGURE_DEPENDS "${SOURCE_ASSETS}/*")
list(FILTER ASSET_FILES EXCLUDE REGEX "\\.gshader$")

set(SYNCED_ASSETS "")
foreach (ASSET_REL IN LISTS ASSET_FILES)
    set(ASSET_IN "${SOURCE_ASSETS}/${ASSET_REL}")
    set(ASSET_OUT "${RUNTIME_ASSETS}/${ASSET_REL}")

    add_custom_command(
            OUTPUT "${ASSET_OUT}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ASSET_IN}" "${ASSET_OUT}"
            DEPENDS "${ASSET_IN}"
            COMMENT "Syncing asset: ${ASSET_REL}"
            VERBATIM
    )
    list(APPEND SYNCED_ASSETS "${ASSET_OUT}")
endforeach ()

add_custom_target(SyncAssets ALL DEPENDS ${SYNCED_ASSETS})
add_dependencies(Engine SyncAssets)
set_target_properties(SyncAssets PROPERTIES FOLDER "GlacialEngine")