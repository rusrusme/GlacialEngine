project(GlazeScript
        LANGUAGES CXX
        DESCRIPTION "Scripting language and compiler for Glacial Engine")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(glaze_core STATIC
            # Public headers
            include/glaze/AST.hpp
            include/glaze/Compiler.hpp
            include/glaze/Config.hpp
            include/glaze/Runtime.hpp

            # Private
            src/GLexer.cpp
            src/GLexer.hpp
            src/Token.hpp
            src/GParser.cpp
            src/GParser.hpp
            src/GChecker.cpp
            src/GChecker.hpp
            src/SymbolTable.cpp
            src/SymbolTable.hpp
            src/GTranspiler.cpp
            src/GTranspiler.hpp
            src/Compiler.cpp
)

target_include_directories(glaze_core
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(glazec src/main.cpp)
target_link_libraries(glazec PRIVATE glaze_core)

add_library(INTERNAL::GLAZESCRIPT ALIAS glaze_core)
