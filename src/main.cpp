#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

GLFWwindow *window;
VkSurfaceKHR _surface = VK_NULL_HANDLE;
VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

#include <assert.h>
#include <vector>
#include <iostream>

std::vector<const char*> instance_layers;
std::vector<const char*> instance_extensions;

VkInstance initInstance()
{
	VkInstance instance = nullptr;
	instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	
	VkApplicationInfo application_info{};
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.apiVersion = VK_MAKE_VERSION(1, 0, 3);
	application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	application_info.pApplicationName = "MyVulkanApp";

	VkInstanceCreateInfo instance_create_info{};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pNext = nullptr;
	instance_create_info.flags = 0;
	instance_create_info.pApplicationInfo = &application_info;
	instance_create_info.enabledExtensionCount = instance_layers.size();
	instance_create_info.ppEnabledLayerNames = instance_layers.data();
	instance_create_info.enabledExtensionCount = instance_extensions.size();
	instance_create_info.ppEnabledExtensionNames = instance_extensions.data();

	auto err = vkCreateInstance(&instance_create_info, nullptr, &instance);
	if (VK_SUCCESS != err) {
		assert(0 && "Vulkan ERROR: Create instance failed!!");
		std::exit(-1);
		return nullptr;
	}
	return instance;
}

void vulkanInit() {
	// Extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::cout << extensionCount << " extensions supported" << std::endl;
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	for (const auto& extension : extensions) {
		std::cout << "\t" << extension.extensionName << std::endl;
	}

	// create Instance
	VkInstance	vkInst = initInstance();

	// create Device

	// ...
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(300,400, "vukan tutorial",nullptr,nullptr );

	vulkanInit();
	
    while (!glfwWindowShouldClose(window));
    {
        glfwPollEvents();
    }

	
	glfwDestroyWindow(window);
}