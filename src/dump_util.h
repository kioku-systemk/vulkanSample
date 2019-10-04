
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

void dumpDeviceStatus(VkPhysicalDevice &gpu)
{
	VkPhysicalDeviceProperties gpu_properties;
	VkPhysicalDeviceFeatures features_properties;
	VkPhysicalDeviceMemoryProperties memory_properties;

	uint32_t queue_families_count = 0;
	vkGetPhysicalDeviceProperties(gpu, &gpu_properties);
	uint32_t apiVer = gpu_properties.apiVersion;

	std::cout << "gpu_Properties" << std::endl;
	std::cout << std::endl;
	std::cout << "Name:\t" << gpu_properties.deviceName << std::endl;
	std::cout << "API Version:\t" << VK_VERSION_MAJOR(apiVer) << "." << VK_VERSION_MINOR(apiVer) << "." << VK_VERSION_PATCH(apiVer) << std::endl;
	std::cout << "Driver Version:\t" << gpu_properties.driverVersion << std::endl;
	std::cout << "Vendor ID:\t" << gpu_properties.vendorID << std::endl;
	std::cout << "Device ID:\t" << gpu_properties.deviceID << std::endl;
	std::cout << "Device Type:\t" << gpu_properties.deviceType << std::endl;

	std::cout << std::endl;

	vkGetPhysicalDeviceFeatures(gpu, &features_properties);
	vkGetPhysicalDeviceMemoryProperties(gpu, &memory_properties);



	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_families_count, nullptr);
	std::vector<VkQueueFamilyProperties> family_property(queue_families_count);
	std::cout << "QueufamilyCount:\t" << queue_families_count << std::endl;
	std::cout << std::endl;

	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_families_count, family_property.data());
	std::cout << "QueueFamilyProperties" << std::endl;
	std::cout << std::endl;
	for (uint32_t i = 0; i <queue_families_count; ++i) {
		std::cout << std::endl;
		std::cout << "Queue Family #" << i << std::endl;
		std::cout << "VK_QUEUE_GRAPHICS_BIT      :\t" << ((family_property[i].queueFlags &VK_QUEUE_GRAPHICS_BIT) != 0) << std::endl;
		std::cout << "VK_QUEUE_COMPUTE_BIT       :\t" << ((family_property[i].queueFlags &VK_QUEUE_COMPUTE_BIT) != 0) << std::endl;
		std::cout << "VK_QUEUE_TRANSFER_BIT      :\t" << ((family_property[i].queueFlags &VK_QUEUE_TRANSFER_BIT) != 0) << std::endl;
		std::cout << "VK_QUEUE_SPARSE_BINDING_BIT:\t" << ((family_property[i].queueFlags &VK_QUEUE_SPARSE_BINDING_BIT) != 0) << std::endl;
		std::cout << "QUEUE COUNT:\t" << family_property[i].queueCount << std::endl;
		std::cout << "timestampValidBits:\t" << family_property[i].timestampValidBits << std::endl;
		uint32_t width = family_property[i].minImageTransferGranularity.width;
		uint32_t height = family_property[i].minImageTransferGranularity.height;
		uint32_t depth = family_property[i].minImageTransferGranularity.depth;
		std::cout << "minImageTransferGranularity:\t" << width << ", " << height << ", " << depth << std::endl;

	}

	uint32_t extensions_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr);
	std::vector<VkExtensionProperties> extensions(extensions_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, extensions.data());

	std::cout << std::endl;
	std::cout << "Amount of Extensions " << extensions_count << std::endl;
	std::cout << std::endl;
	std::cout << "ExtensionProperties" << std::endl;
	std::cout << std::endl;

	for (int i = 0; i < extensions_count; i++) {
		std::cout << std::endl;
		std::cout << "Name:        " << extensions[i].extensionName << std::endl;
		std::cout << "specVersion: " << extensions[i].specVersion << std::endl;

	}
	std::cout << std::endl;

	uint32_t layer_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

	std::vector<VkLayerProperties> layers(layer_count);

	vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

	std::cout << "Amount of Layers " << layer_count << std::endl;
	std::cout << std::endl;
	for (int i = 0; i < layer_count; i++) {
		std::cout << std::endl;
		std::cout << "layerName:            " << layers[i].layerName << std::endl;
		std::cout << "specVersion:          " << layers[i].specVersion << std::endl;
		std::cout << "implementationVersion:" << layers[i].implementationVersion << std::endl;
		std::cout << "description:          " << layers[i].description << std::endl;
	}

	std::cout << std::endl;
}