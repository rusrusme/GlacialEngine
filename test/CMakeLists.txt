# test/CMakeLists.txt
# ----------------------------------------------------------------------------
add_executable(FramebufferTest rhi/FramebufferTest.cpp rhi/FramebufferTest.hpp)
target_link_libraries(FramebufferTest PRIVATE Engine glfw glad)

add_executable(ShaderSystemTest shader/ShaderSystemTest.cpp shader/ShaderSystemTest.hpp)
target_link_libraries(ShaderSystemTest PRIVATE Engine glfw glad)

add_executable(RHITest rhi/RHITest.cpp rhi/RHITest.hpp)
target_link_libraries(RHITest PRIVATE Engine glfw glad)
