
#include <iostream>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

import Core;

int main() {
	gfx::Application app{};
	app.run();
	#if 0
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	auto* window = glfwCreateWindow(800, 600, "vulkan tutorial", nullptr, nullptr);

	glm::mat4 matrix;
	glm::vec4 vec;
	auto test = matrix * vec;
	
	while(!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	#endif
	return 0;
}
