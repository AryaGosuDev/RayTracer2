#ifndef __VK_INSTANCE_HPP__
#define __VK_INSTANCE_HPP__

namespace VkApplication {

	void MainVulkApplication::createInstance( std::string appName ) {
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName.c_str();
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "RT 2 Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		auto extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0; createInfo.pNext = nullptr;
		}
		
		VkResult errcode = vkCreateInstance(&createInfo, nullptr, &instance);
		if (errcode != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
		SetObjectName(device, reinterpret_cast<uint64_t>(instance), VK_OBJECT_TYPE_INSTANCE, "Main Instance");
	}


	bool checkExtensions() {
		uint32_t extensions_count = 0;
		VkResult result = VK_SUCCESS;

		result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, nullptr);
		if ((result != VK_SUCCESS) || (extensions_count == 0)) {
			std::cout << "Could not get the number of Instance extensions." << std::endl;
			return false;
		}
		std::vector< VkExtensionProperties> available_extensions;
		available_extensions.resize(extensions_count);
		result = vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, &available_extensions[0]);
		if ((result != VK_SUCCESS) || (extensions_count == 0)) {
			std::cout << "Could not enumerate Instance extensions." << std::endl;
			return false;
		}
		for (auto& xVK : available_extensions) 
			std::cout << xVK.extensionName << std::endl;
		
		return true;
	}

	void MainVulkApplication::SetObjectName(VkDevice device, uint64_t objectHandle, VkObjectType objectType, const std::string& name) {
		if (vkSetDebugUtilsObjectNameEXT) {
			VkDebugUtilsObjectNameInfoEXT nameInfo{};
			nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
			nameInfo.objectType = objectType;
			nameInfo.objectHandle = objectHandle;
			nameInfo.pObjectName = name.c_str();

			vkSetDebugUtilsObjectNameEXT(device, &nameInfo);
		}
	}

}


#endif