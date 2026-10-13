# test/CMakeLists.txt
# ----------------------------------------------------------------------------
add_executable(FramebufferTest FramebufferTest.cpp FramebufferTest.hpp)
target_link_libraries(FramebufferTest PRIVATE Engine glfw glad)

add_executable(ShaderSystemTest ShaderSystemTest.cpp ShaderSystemTest.hpp)
target_link_libraries(ShaderSystemTest PRIVATE Engine glfw glad)
