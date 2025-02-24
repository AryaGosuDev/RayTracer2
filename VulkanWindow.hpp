#ifndef __VK_WINDOW_HPP__
#define __VK_WINDOW_HPP__

namespace VkApplication {

	void MainVulkApplication::initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Arya Wallfacer Vulkan RT", nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void MainVulkApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<MainVulkApplication*>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}

	void MainVulkApplication::createSurface() {
		// does not allocate memory, memory is allocated later when the swapchain is allocated
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}
}

#endif