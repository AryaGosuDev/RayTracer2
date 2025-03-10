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
#include <glm/gtc/packing.hpp>
#include <glm/gtx/type_aligned.hpp>
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

#define ASSIMPORT_INCLUDES
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

	const std::string MODEL_PATH = "models/LPRoom.glb";

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
	glm::vec3 normal;
	glm::vec3 pos;
	glm::vec2 texCoord;
	glm::vec2 texCoord1;
	glm::vec4 tangent;
	uint32_t materialIndx;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(7);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, color);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, pos);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[4].offset = offsetof(Vertex, texCoord1);

		attributeDescriptions[5].binding = 0;
		attributeDescriptions[5].location = 5;
		attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[5].offset = offsetof(Vertex, tangent);

		attributeDescriptions[6].binding = 0;
		attributeDescriptions[6].location = 6;
		attributeDescriptions[6].format = VK_FORMAT_R32_UINT;
		attributeDescriptions[6].offset = offsetof(Vertex, materialIndx);

		return attributeDescriptions;
	}

	void applyTransform(const glm::mat4& m) {
		auto newp = m * glm::vec4(pos, 1.0);
		pos = glm::vec3(newp.x, newp.y, newp.z);
		glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(m));
		normal = normalMatrix * normal;
		tangent = glm::inverseTranspose(m) * tangent;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && normal == other.normal && texCoord == other.texCoord;
	}
};

struct stbImageData {
	stbImageData(const std::vector<uint8_t>& imageData, bool useFloat = false);
	stbImageData(const std::vector<char>& imageData, bool useFloat = false);
	~stbImageData();
	void* data = nullptr;
	int width = 0;
	int height = 0;
	int channels = 0;
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
// used mostly for addressable gpu memory bit
struct ExtendedvKBuffer {
	VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegion{};
	VkDevice* device;
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDescriptorBufferInfo descriptor;
	VkDeviceSize size = 0;
	VkDeviceSize alignment = 0;
	void* mapped = nullptr;
	VkObjectType objectType; 
	std::string name;
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
	ExtendedvKBuffer raygen;
	ExtendedvKBuffer miss;
	ExtendedvKBuffer hit;
	ExtendedvKBuffer callable;
} shaderBindingTables;

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 normalMatrix;
};

struct RTImageViews {
	VkImage RTColorImage;
	VkImageView RTColorImageView;
	VkDeviceMemory RTColorImageMemory;

	VkImage RTDepthImage;
	VkImageView RTDepthImageView;
	VkDeviceMemory RTDepthImageMemory;
};

struct Material {
	int basecolorTextureId = -1;
	int basecolorSamplerId = -1;
	int metallicRoughnessTextureId = -1;
	int metallicRoughnessSamplerId = -1;
	int normalTextureTextureId = -1;
	int normalTextureSamplerId = -1;
	int emissiveTextureId = -1;
	int emissiveSamplerId = -1;
	float metallicFactor = 1.0;
	float roughnessFactor = 1.0;
	glm::vec4 basecolor;
	// maybe more properties to add like occlusion etc....
	glm::vec2 padding;  // needed to make sure its aligned since materials will be passed as
	// this makes it 64 bytes
};

struct SceneObject {
	std::string name;
	uint64_t id; // hash key

	SceneObject() {
		id = std::hash<std::string>{}(name);
	}

	ExtendedvKBuffer vertexBuffer;
	ExtendedvKBuffer indexBuffer;
	AccelerationStructure blas; 

	Material material;
	glm::mat4 transform; 
};

struct Light {
	glm::vec3 position;
	glm::vec3 color;
	float intensity;
};

class Scene {
public:
	std::unordered_map<uint64_t, SceneObject> objects;
	std::vector<Light> lights;

	void addObject(const SceneObject& obj) {
		objects[obj.id] = obj;
	}

	SceneObject* getObjectByName(const std::string& name) {
		uint64_t id = std::hash<std::string>{}(name);
		if (objects.find(id) != objects.end()) {
			return &objects[id];
		}
		return nullptr;
	}
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

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkRenderPass renderPass;

	VkDescriptorSetLayout globalDescriptorSetLayout;
	VkDescriptorSetLayout materialDescriptorSetLayout;
	VkDescriptorSetLayout frameDescriptorSetLayout;
	std::vector< VkDescriptorSet> globalDescriptorSet;
	std::vector< VkDescriptorSet> materialDescriptorSet;
	std::vector< VkDescriptorSet> frameDescriptorSet;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	RTImageViews rtImageViews;

	VkCommandPool commandPool;

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
	void SetObjectName(VkDevice, uint64_t, VkObjectType, const std::string&);
	//Create the viewport
	void initWindow();
	//Call back to re-buffer the viewport window
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	void createSurface();
	//choose the correct GPU render device
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();
	void getEnabledFeatures();
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

	void createVertexBuffer();
	void createBuffer(VkDeviceSize, VkBufferUsageFlags,
		VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&, bool );
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
	void setupAS();
	void createBLAS();
	void createTLAS();
	void createSBT();
	void createShaderBindingTable(ExtendedvKBuffer&, uint32_t);

	void initVulkan(std::string appName ) {

		createInstance(appName);

		setupDebugMessenger();

		createSurface();
		pickPhysicalDevice();
		getEnabledFeatures();
		
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createCommandPool();
		//createTextureImage();
		//createTextureImageView();
		//createTextureSampler();

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
#include "VulkanAS.hpp"
#include "VulkanSBT.hpp"

#endif