#ifndef __MMM_H__
#define __MMM_H__

// return first address to be multiple or alignment
template <typename T> 
T align_up(T value, T alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

// find the memory index that best fits required mem type
uint32_t findMemoryType(VkPhysicalDevice & physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

// find the memory index that best fits required mem type for the pool absent of the buffer type
uint32_t findMemoryTypePool(VkPhysicalDevice& physicalDevice, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ( (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
            return i;
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

// value pair in buffer hash LUT
struct AllocationInfo {
    VkDeviceSize offset;
    VkDeviceSize size;
};

class VulkanMemoryManagement {
protected:
    VkDevice * device;
    VkPhysicalDevice *  physicalDevice;
    VkDeviceMemory memoryPool;
    VkDeviceSize poolSize;
    VkDeviceSize currentOffset = 0;  // Linear offset
    std::unordered_map<VkBuffer, AllocationInfo> allocations;

public:
    VulkanMemoryManagement() = default;
    VulkanMemoryManagement(VkPhysicalDevice& physicalDevice, VkDevice &device, VkDeviceSize size, 
        VkMemoryPropertyFlags properties) : device(&device), poolSize(size), physicalDevice(&physicalDevice){
        // allocate large mem block
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = size;
        allocInfo.memoryTypeIndex = findMemoryTypePool( physicalDevice , properties);

        vkAllocateMemory(device, &allocInfo, nullptr, &memoryPool);
    }

    // Allocate memory for a buffer
    VkDeviceSize allocate(VkBuffer buffer, VkDeviceSize size) {
        if (currentOffset + size > poolSize) {
            return VK_WHOLE_SIZE; // Out of memory
        }

        VkDeviceSize allocatedOffset = currentOffset;
        currentOffset += size; // Move offset forward

        // Store allocation info
        allocations[buffer] = { allocatedOffset, size };

        // Bind buffer to allocated offset
        vkBindBufferMemory(device, buffer, memoryPool, allocatedOffset);

        return allocatedOffset;
    }

    // Free memory (logical removal only)
    void free(VkBuffer buffer) {
        allocations.erase(buffer);
        // No merging, just track logical deallocations
    }

    // Retrieve the offset for a given buffer
    VkDeviceSize getOffset(VkBuffer buffer) {
        return allocations.count(buffer) ? allocations[buffer].offset : VK_WHOLE_SIZE;
    }

    ~VulkanMemoryManagement() {
        vkFreeMemory(device, memoryPool, nullptr);
    }
};

class VulkanMemoryManagementGPU : public VulkanMemoryManagement {
private :

public :
    VulkanMemoryManagementGPU() = default;
    VulkanMemoryManagementGPU(VkPhysicalDevice& physicalDevice, VkDevice& device, VkDeviceSize size, VkMemoryPropertyFlags properties) {
        VulkanMemoryManagement::VulkanMemoryManagement( physicalDevice,device, size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

};

class VulkanMemoryManagementModule {

private :

public :
    VulkanMemoryManagementModule() = default;





};


/*
Common Vulkan Memory Types                 | Memory Type	Flags                   |	Best Used For
VRAM(Device Local)                        |	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT	    |   Vertex buffers, index buffers, textures, acceleration structures
CPU - Visible(Host Visible, Host Coherent)|	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |	Staging buffers, uniform buffers
CPU - Cached(Fast Reads)                  |	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT VK_MEMORY_PROPERTY_HOST_CACHED_BIT |	Readback buffers, CPU - side access
Sparse Binding Memory	                  |VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT + VK_MEMORY_PROPERTY_SPARSE_BINDING_BIT |	Sparse resources, streamed assets

*/

/*

// Create memory pool (256MB)
VulkanMemoryPool memoryPool(device, 256 * 1024 * 1024);

// Create a buffer
VkBufferCreateInfo bufferInfo{};
bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
bufferInfo.size = 64 * 1024; // 64KB buffer
bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

VkBuffer myBuffer;
vkCreateBuffer(device, &bufferInfo, nullptr, &myBuffer);

// Allocate from pool
VkDeviceSize offset = memoryPool.allocate(myBuffer, bufferInfo.size);

// Retrieve the offset of the buffer later
VkDeviceSize storedOffset = memoryPool.getOffset(myBuffer);


*/

#endif