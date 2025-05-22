cmake_minimum_required(VERSION 3.31)

project(vk-tutorial)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions("$<$<NOT:$<CONFIG:Debug>>:NDEBUG>")

add_executable(vk-tutorial)
target_compile_options(vk-tutorial PRIVATE -Wall -Wextra -Wpedantic)
target_compile_options(vk-tutorial PRIVATE -g0)

target_sources(vk-tutorial
	PRIVATE
	main.cpp
#	PRIVATE
#	FILE_SET CXX_MODULES
#	FILES
#	Applicaion.cppm
)
#LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi
add_subdirectory("external/glfw" EXCLUDE_FROM_ALL)
find_library(VULKAN vulkan REQUIRED)

target_link_libraries(vk-tutorial
	PUBLIC
	glfw
	${VULKAN}
)
