cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
cmake_policy(VERSION 3.7)

add_executable(clearSample 
    main.cpp)

find_package(Vulkan REQUIRED)

target_link_libraries(clearSample glfw ${GLFW_LIBRARIES} ${Vulkan_LIBRARIES})