# test/CMakeLists.txt
# ----------------------------------------------------------------------------
add_executable(FramebufferTest FramebufferTest.cpp FramebufferTest.hpp)
target_link_libraries(FramebufferTest PRIVATE Engine glfw glad)

