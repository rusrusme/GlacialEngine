# templates/default//CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Direct build outputs to bin/
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_subdirectory(basis)

file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.hpp" "src/*.h")

add_executable(${PROJECT_NAME} ${GAME_SOURCES})

# Include src/ directory for clean header paths
target_include_directories(${PROJECT_NAME} PRIVATE src)

# Link against engine core
target_link_libraries(${PROJECT_NAME} PRIVATE BasisEngine::Engine)

# Copy game assets post-build
link_assets(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/assets")