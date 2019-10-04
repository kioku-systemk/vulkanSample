#ifdef _WIN32
	#define NOMINMAX
	#include <windows.h>
	#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

GLFWwindow *window;
VkSurfaceKHR _surface = VK_NULL_HANDLE;
VkSwapchainKHR _swapchain = VK_NULL_HANDLE;

#include <assert.h>
#include <vector>
#include <iostream>
#include <algorithm>

#include "dump_util.h"

VkInstance createInstance(const char* appName)
{
	VkInstance instance = nullptr;

	std::vector<const char*> instance_layers;
	std::vector<const char*> instance_extensions;
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

VkPhysicalDevice pickPhysicalDevice(VkInstance instance)
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


void findGraphicsQueueIndex(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t& graphic_index, uint32_t& present_index) {
	graphic_index = static_cast<uint32_t>(-1);
	present_index = static_cast<uint32_t>(-1);

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphic_index = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			present_index = i;
		}

		i++;
	}
}

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface,
	uint32_t& graphics_queue_index, uint32_t& present_queue_index)
{
	VkDevice device = nullptr;
	findGraphicsQueueIndex(physicalDevice, surface, graphics_queue_index, present_queue_index);

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = graphics_queue_index;
	queueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {};
	VkDeviceCreateInfo createInfo = {};
	std::vector<const char*> deviceExt;
	deviceExt.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = deviceExt.size();
	createInfo.ppEnabledExtensionNames = deviceExt.data();
	createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	return device;
}

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice phyDevice, VkSurfaceKHR surface) {
	
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phyDevice, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(phyDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(phyDevice, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) { // the performace is better than FIFO
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}


VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { 512, 512 };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

VkSwapchainKHR createSwapChainAndImages(VkPhysicalDevice phyDevice, VkDevice dev, VkSurfaceKHR surface,
	uint32_t graphics_queue_index, uint32_t present_queue_index,
	std::vector<VkImage> swapChainImages, std::vector<VkImageView> swapChainImageViews
) {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(phyDevice, surface);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);


	VkSwapchainCreateInfoKHR swapchain_ci = {};
	swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_ci.pNext = NULL;
	swapchain_ci.surface = surface;
	swapchain_ci.minImageCount = swapChainSupport.capabilities.minImageCount;
	swapchain_ci.imageFormat = surfaceFormat.format;
	swapchain_ci.imageExtent.width = extent.width;
	swapchain_ci.imageExtent.height = extent.height;
	swapchain_ci.preTransform = swapChainSupport.capabilities.currentTransform;;
	swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_ci.imageArrayLayers = 1;
	swapchain_ci.presentMode = presentMode;
	swapchain_ci.oldSwapchain = VK_NULL_HANDLE;
	swapchain_ci.clipped = VK_TRUE;
	swapchain_ci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_ci.queueFamilyIndexCount = 0;
	swapchain_ci.pQueueFamilyIndices = nullptr;
	uint32_t queueFamilyIndices[2] = { graphics_queue_index, present_queue_index };
	if (graphics_queue_index != present_queue_index) {
		// If the graphics and present queues are from different queue families,
		// we either have to explicitly transfer ownership of images between
		// the queues, or we have to create the swapchain with imageSharingMode
		// as VK_SHARING_MODE_CONCURRENT
		swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_ci.queueFamilyIndexCount = 2;
		swapchain_ci.pQueueFamilyIndices = queueFamilyIndices;
	}

	VkSwapchainKHR swapChain;
	if (vkCreateSwapchainKHR(dev, &swapchain_ci, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	// -----------------------------------s
	// create swapChain images and bufferView
	
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(dev, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	swapChainImageViews.resize(imageCount);
	vkGetSwapchainImagesKHR(dev, swapChain, &imageCount, swapChainImages.data());

	for (uint32_t i = 0; i < imageCount; i++) {
		VkImageViewCreateInfo color_image_view = {};
		color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		color_image_view.pNext = NULL;
		color_image_view.flags = 0;
		color_image_view.image = swapChainImages[i];
		color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		color_image_view.format = surfaceFormat.format;
		color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
		color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
		color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
		color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
		color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		color_image_view.subresourceRange.baseMipLevel = 0;
		color_image_view.subresourceRange.levelCount = 1;
		color_image_view.subresourceRange.baseArrayLayer = 0;
		color_image_view.subresourceRange.layerCount = 1;

		VkResult res = vkCreateImageView(dev, &color_image_view, NULL, &swapChainImageViews[i]);
		assert(res == VK_SUCCESS);
	}

	return swapChain;
}

VkPipelineLayout createPipelineLayout(VkDevice dev) {
	VkPipelineLayout pipelineLayout;
	
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(dev, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	return pipelineLayout;
}

VkPipeline createGraphicsPipeline() {
	VkPipeline graphicsPipeline;


	return graphicsPipeline;
}

VkRenderPass createRenderPass() {
	VkRenderPass renderPass;

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}

	return renderPass;
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

	// create Physical Device
	VkPhysicalDevice physicalDevice = pickPhysicalDevice(vkInst);
	dumpDeviceStatus(physicalDevice);

	// create LogicalDevice
	uint32_t gfxq_index;
	uint32_t present_index;
	VkDevice dev = createLogicalDevice(physicalDevice, surface, gfxq_index, present_index);

	// get DeviceQueue
	VkQueue graphicsQueue;
	vkGetDeviceQueue(dev, gfxq_index, 0, &graphicsQueue);
	if (!glfwGetPhysicalDevicePresentationSupport(vkInst, physicalDevice, gfxq_index)) // vkGetPhysicalDeviceSurfaceSupportKHR
	{
		assert(0 && "Vulkan ERROR: Can't get device presentation support!!");
		std::exit(-1);
		return;
	}

	// create swapchain and Images, ImageViews
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkSwapchainKHR swapchain = createSwapChainAndImages(physicalDevice, dev, surface,
		gfxq_index, present_index,
		swapChainImages, swapChainImageViews);

	// create Pipeline
	VkPipelineLayout pipeline = createPipelineLayout(dev);

	// create RenderPass

}

/*
void cleanupSwapChain(VkDevice device) {
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
	}

	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);
}*/

void vulkanCleanup(VkInstance instance, VkSurfaceKHR surface, VkDevice device, VkSwapchainKHR swapchain) {
	// cleanupSwapChain();
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

	//vulkanCleanup();
	glfwDestroyWindow(window);
}