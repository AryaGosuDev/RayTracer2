#ifndef __MMM_H__
#define __MMM_H__

template <typename T> 
T align_up(T value, T alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

uint32_t findMemoryType(VkPhysicalDevice & physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i; break;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

struct AllocationInfo {
    VkDeviceSize offset;
    VkDeviceSize size;
};

class VulkanMemoryPool {
private:
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkDeviceMemory memoryPool;
    VkDeviceSize poolSize;
    VkDeviceSize currentOffset = 0;  // Linear offset
    std::unordered_map<VkBuffer, AllocationInfo> allocations;

public:
    VulkanMemoryPool(VkDevice device, VkDeviceSize size) : device(device), poolSize(size) {
        // Allocate large memory block
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = size;
        allocInfo.memoryTypeIndex = findMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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

    ~VulkanMemoryPool() {
        vkFreeMemory(device, memoryPool, nullptr);
    }
};

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