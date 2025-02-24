#ifndef _VULKAN_TEMPLATE_HEADER_
#define _VULKAN_TEMPLATE_HEADER_

#include <string>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>
#include <array>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_GTC_matrix_transform

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/ext.hpp>

#include "imgui/imgui.h"
#include "imgui/imconfig.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_glfw.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyObjLoader/tiny_obj_loader.h>

#ifdef _DEBUG
constexpr bool enableValidationLayers = true;
#else
constexpr bool enableValidationLayers = false;
#endif

#include "ValidationLayers.hpp"
// custom memory management module
#include "MMM.h"

static void check_vk_result(VkResult err) {
	if (err == 0)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

namespace VkApplication{

	constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		"VK_KHR_shader_clock",
		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
	VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
	VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
	VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
	VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
	VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME
	//VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_EXTERNAL_SEMAPHORE_EXTENSION_NAME
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

struct Vertex {
	glm::vec3 color;
	glm::vec3 vertexNormal;
	glm::vec3 pos;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, color);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, vertexNormal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, pos);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && vertexNormal == other.vertexNormal && texCoord == other.texCoord;
	}
};

// Per-instance data block
struct InstanceData {
	glm::vec3 pos;
	glm::vec3 rot;
	uint32_t texIndex;
	glm::vec3 instanceColor;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 1;
		bindingDescription.stride = sizeof(InstanceData);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(4);

		attributeDescriptions[0].binding = 1;
		attributeDescriptions[0].location = 4;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(InstanceData, pos);

		attributeDescriptions[1].binding = 1;
		attributeDescriptions[1].location = 5;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(InstanceData, rot);

		attributeDescriptions[2].binding = 1;
		attributeDescriptions[2].location = 6;
		attributeDescriptions[2].format = VK_FORMAT_R32_SINT;
		attributeDescriptions[2].offset = offsetof(InstanceData, texIndex);

		attributeDescriptions[3].binding = 1;
		attributeDescriptions[3].location = 7;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(InstanceData, instanceColor);

		return attributeDescriptions;
	}

	bool operator==(const InstanceData& other) const {
		return pos == other.pos && rot == other.rot && texIndex == other.texIndex && instanceColor == other.instanceColor;
	}
};

struct AccelerationStructure {
	VkAccelerationStructureKHR handle;
	uint64_t deviceAddress = 0;
	VkDeviceMemory memory;
	VkBuffer buffer;
};

struct ScratchBuffer {
	uint64_t deviceAddress = 0;
	VkBuffer handle = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
};

// mostly extends buffer class
struct ShaderBindingTable {
	VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegion{};
	VkDevice* device;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDescriptorBufferInfo descriptor;
	VkDeviceSize size = 0;
	VkDeviceSize alignment = 0;
	void* mapped = nullptr;
	/** @brief Usage flags to be filled by external source at buffer creation (to query at some later point) */
	VkBufferUsageFlags usageFlags;
	/** @brief Memory property flags to be filled by external source at buffer creation (to query at some later point) */
	VkMemoryPropertyFlags memoryPropertyFlags;

	VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
		return vkMapMemory(*device, memory, offset, size, 0, &mapped);
	}
	void unmap() {
		if (mapped){
			vkUnmapMemory(*device, memory);
			mapped = nullptr;
		}
	}
	VkResult bind(VkDeviceSize offset = 0) {
		return vkBindBufferMemory(*device, buffer, memory, offset);
	}
	void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
		descriptor.offset = offset;
		descriptor.buffer = buffer;
		descriptor.range = size;
	}
	void copyTo(void* data, VkDeviceSize size) {
		assert(mapped);
		memcpy(mapped, data, size);
	}
	VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkFlushMappedMemoryRanges(*device, 1, &mappedRange);
	}
	VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = memory;
		mappedRange.offset = offset;
		mappedRange.size = size;
		return vkInvalidateMappedMemoryRanges(*device, 1, &mappedRange);
	}
	void destroy() {
		if (buffer) vkDestroyBuffer(*device, buffer, nullptr);
		if (memory)vkFreeMemory(*device, memory, nullptr);
	}
};

struct ShaderBindingTables {
	ShaderBindingTable raygen;
	ShaderBindingTable miss;
	ShaderBindingTable hit;
	ShaderBindingTable callable;
} shaderBindingTables;

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 normalMatrix;
};

struct PushConstants {
	int useReflectionSampler;
};

struct KeyControls {
	bool kickParticle = false;
};

struct RTImageViews {
	VkImage RTColorImage;
	VkImageView RTColorImageView;
	VkDeviceMemory RTColorImageMemory;

	VkImage RTDepthImage;
	VkImageView RTDepthImageView;
	VkDeviceMemory RTDepthImageMemory;

};

class MainVulkApplication {

	friend void mainLoop(VkApplication::MainVulkApplication*);
	friend void updateUniformBuffer(MainVulkApplication*);
	friend void loadInitialVariables(MainVulkApplication*);

protected:
	MainVulkApplication() {}
	~MainVulkApplication() { cleanup(); }
	
public:

	MainVulkApplication(MainVulkApplication& other) = delete;
	void operator=(const MainVulkApplication&) = delete;

	static MainVulkApplication* GetInstance();

	void setup(std::string appName = "") {
		initWindow();
		initVulkan(appName);
	}

	void cleanupApp() {
		cleanup();
	}

private:

	static MainVulkApplication* pinstance_;

	size_t WIDTH = 1600;
	size_t HEIGHT = 1200;
	
	GLFWwindow* window;
	ImGui_ImplVulkanH_Window imgui_window;
	KeyControls keyControl;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	VkCommandPool commandPool;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	size_t dynamicAlignment;
	size_t bufferDynamicSize;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	std::vector<VkBuffer> uniformFragBuffers;
	std::vector<VkDeviceMemory> uniformFragBuffersMemory;

	std::vector<VkBuffer> uniformDynamicBuffers;
	std::vector<VkDeviceMemory> uniformDynamicBuffersMemory;

	VkDescriptorPool descriptorPool;
	VkDescriptorPool descriptorPoolIMGui;
	std::vector<VkDescriptorSet> descriptorSets;

	UniformBufferObject ubo;

	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	size_t currentFrame = 0;

	bool framebufferResized = false;

	VkBuffer asBuffer;

	/* This struct holds hardware properties related to the ray tracing pipeline, such as shader recursion depth and performance limits.
	shaderGroupHandleSize: Size(in bytes) of each shader group handle.
	maxRayRecursionDepth : Maximum ray recursion depth supported(e.g., 31 or higher).
	maxRayDispatchInvocationCount : Max invocations for a single vkCmdTraceRaysKHR() call.
	shaderGroupBaseAlignment : Memory alignment requirement for shader groups.
	*/
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR  rayTracingPipelineProperties{};
	/*This struct enables support for acceleration structures(BVH trees) used in Vulkan Ray Tracing.

	accelerationStructure: Indicates whether acceleration structures(AS) are supported.
	accelerationStructureCaptureReplay : Enables AS capture& replay(e.g., for debugging).
	accelerationStructureIndirectBuild : Supports indirect AS builds.
	accelerationStructureHostCommands : Allows AS builds from the CPU instead of GPU.
	*/
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};

	// Enabled features and properties
	/*
	This struct enables buffer device addresses, which allow accessing GPU memory directly via 64 - bit pointers.This is crucial for ray tracing acceleration structures.
	 bufferDeviceAddress: Enables vkGetBufferDeviceAddress(), allowing buffers to act as raw pointers.
	 bufferDeviceAddressCaptureReplay: Allows capturing/replaying buffer addresses.
	 bufferDeviceAddressMultiDevice: Enables buffer addresses in multi-GPU environments.
	*/
	VkPhysicalDeviceBufferDeviceAddressFeatures enabledBufferDeviceAddresFeatures{};
	/*This struct enables the ray tracing pipeline itself.
	rayTracingPipeline: Enables VkPipeline objects that contain ray tracing shaders.
	rayTracingPipelineShaderGroupHandleCaptureReplay: Supports capturing shader group handles for replay.
	rayTracingPipelineTraceRaysIndirect: Allows indirect dispatch of vkCmdTraceRaysKHR().
	*/
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR enabledRayTracingPipelineFeatures{};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR enabledAccelerationStructureFeatures{};

	std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups = {};

	AccelerationStructure bottomLevelAS;
	AccelerationStructure topLevelAS;
	ScratchBuffer scratchBuffer;

	// Function pointers for ray tracing related stuff
	PFN_vkGetBufferDeviceAddressKHR vkGetBufferDeviceAddressKHR;
	PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
	PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
	PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
	PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
	PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
	PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
	PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
	PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
	PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;

	void* deviceCreatepNextChain = nullptr;

	//FUNCTIONS
	void createInstance(std::string appName);
	//Create the viewport
	void initWindow();
	//Call back to re-buffer the viewport window
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	void createSurface();
	//choose the correct GPU render device
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice );
	bool checkDeviceExtensionSupport(VkPhysicalDevice );

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& );
	void createSwapChain();
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR&);
	void createImageViews();
	VkImageView createImageView(VkImage, VkFormat, VkImageAspectFlags);
	void createImage(uint32_t, uint32_t,
		VkFormat, VkImageTiling, VkImageUsageFlags, VkMemoryPropertyFlags, VkImage&, VkDeviceMemory&);
	uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags);

	VkFormat findDepthFormat();
	VkFormat findSupportedFormat(const std::vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags);

	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>&);
	void createCommandPool();
	void createDepthResources();
	void createFramebuffers();

	void createVertexBuffer();
	void createBuffer(VkDeviceSize, VkBufferUsageFlags,
		VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);
	void createIndexBuffer();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	
	void createImguiContext();
	void render_gui();
	void drawImgFrame(VkCommandBuffer& );

	void copyBuffer(VkBuffer, VkBuffer, VkDeviceSize);
	void createCommandBuffers();
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer);

	void createSyncObjects();
	void drawFrame();
	void updateUniformBuffer(uint32_t );
	void recreateSwapChain();
	void cleanupSwapChain();

	void loadModel();
	void createTextureImage();
	void copyBufferToImage(VkBuffer , VkImage , uint32_t , uint32_t );
	void transitionImageLayout(VkImage, VkFormat, VkImageLayout, VkImageLayout);
	void createTextureImageView();
	void createTextureSampler();


	void initVulkan(std::string appName ) {

		createInstance(appName);

		setupDebugMessenger();

		createSurface();
		pickPhysicalDevice();
		
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createCommandPool();
		createDepthResources();
		createFramebuffers();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();

		loadModel();
		createVertexBuffer();
		createIndexBuffer();
		createUniformBuffers();
		createDescriptorPool();
		//createDescriptorSets();
		createImguiContext();
		createCommandBuffers();
		createSyncObjects();
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(device);
	}

	void cleanup() {
		cleanupSwapChain();

		vkDestroySampler(device, textureSampler, nullptr);
		vkDestroyImageView(device, textureImageView, nullptr);

		vkDestroyImage(device, textureImage, nullptr);
		vkFreeMemory(device, textureImageMemory, nullptr);

		vkDestroyDescriptorPool(device, descriptorPool, nullptr);

		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}

		vkDestroyBuffer(device, indexBuffer, nullptr);
		vkFreeMemory(device, indexBufferMemory, nullptr);

		vkDestroyBuffer(device, vertexBuffer, nullptr);
		vkFreeMemory(device, vertexBufferMemory, nullptr);

		
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(device, commandPool, nullptr);

		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(window);

		glfwTerminate();
	}

	void setupDebugMessenger() {
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}
};
}

namespace std {
	template<> struct hash<VkApplication::Vertex> {
		size_t operator()(VkApplication::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

#include "VulkanDescriptor.hpp"
#include "VulkanInstance.hpp"
#include "VulkanDevice.hpp"
#include "VulkanWindow.hpp"
#include "VulkanSwapchain.hpp"
#include "VulkanDraw.hpp"
#include "VulkanRenderSettings.hpp"
#include "VulkanSync.hpp"
#include "VulkanGeometry.hpp"
#include "VulkanTexture.hpp"
#include "VulkanRTDraw.hpp"
#include "VulkanImgui.hpp"

#endif