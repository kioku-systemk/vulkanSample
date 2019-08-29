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

void dumpExtensions() {
	// Extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::cout << extensionCount << " extensions supported" << std::endl;
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
	for (const auto& extension : extensions) {
		std::cout << "\t" << extension.extensionName << std::endl;
	}
}

VkInstance createInstance(const char* appName)
{
	VkInstance instance = nullptr;
	instance_layers.push_back("VK_LAYER_LUNARG_standard_validation"); // for Debug
	instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef _WIN32
	instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME); // need for Windows
#endif

	VkApplicationInfo application_info{};
	application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application_info.apiVersion = VK_MAKE_VERSION(1, 0, 3);
	application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	application_info.pApplicationName = appName;

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

bool isGPUCheck(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		deviceFeatures.geometryShader;
}

VkPhysicalDevice createDevice(VkInstance instance)
{
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		assert(0 && "failed to find GPUs with Vulkan support!");
		std::exit(-1);
		return nullptr;
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (isGPUCheck(device)) {
			physicalDevice = device;
			break;
		}
	}
	if (physicalDevice == VK_NULL_HANDLE) {
		assert(0 && "failed to find a suitable GPU!");
	}

	return physicalDevice;
}

class QueueFamilyIndices {
public:
	uint32_t graphicsFamily;
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		i++;
	}
	return indices;
}

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice)
{
	VkDevice device = nullptr;
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
	queueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {};
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = 0;
	createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	return device;
}

void vulkanInit(GLFWwindow* window) {
	
	dumpExtensions();

	// create Instance
	VkInstance	vkInst = createInstance("MyApp");
	VkSurfaceKHR surface;

	// create Surface
	VkResult err = glfwCreateWindowSurface(vkInst, window, NULL, &surface);
	if (err) {
		assert(0 && "Vulkan ERROR: Create WindowSurface failed!!");
		std::exit(-1);
		return;
	}

	// create Device
	VkPhysicalDevice physicalDevice = createDevice(vkInst);

	// create LogicalDevice
	VkDevice dev = createLogicalDevice(physicalDevice);

	// ...
}

void vulkanCleanup(VkInstance instance, VkSurfaceKHR surface) {
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(300,400, "vukan tutorial",nullptr,nullptr );

	vulkanInit(window);
	
    while (!glfwWindowShouldClose(window));
    {
        glfwPollEvents();
    }

	
	glfwDestroyWindow(window);
}