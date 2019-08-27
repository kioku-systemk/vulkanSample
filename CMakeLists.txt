cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
cmake_policy(VERSION 3.7)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(NAME VulkanSample)

project(${NAME})

add_subdirectory(third_party/glfw)

include_directories(third_party/vulkan/include)
include_directories(third_party/glfw/include)

add_subdirectory(src)