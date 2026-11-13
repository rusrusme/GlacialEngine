project(GlazeScript
        LANGUAGES CXX
        DESCRIPTION "Scripting language and compiler for Glacial Engine")

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
                           PUBLIC include
                           PRIVATE src
)

target_compile_features(glaze_core PUBLIC cxx_std_20)

add_executable(glazec src/main.cpp)
target_link_libraries(glazec PRIVATE glaze_core)

add_library(INTERNAL::GLAZESCRIPT ALIAS glaze_core)
