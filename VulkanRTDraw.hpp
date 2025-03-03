#ifndef __VK_RT_DRAW_HPP__
#define __VK_RT_DRAW_HPP__

namespace VkApplication {

	void MainVulkApplication::DrawRT() {

		vkWaitForFences(device, 1, &renderFenceRT, VK_TRUE, UINT64_MAX);

		PushConstants pushConstants;
		pushConstants.avatarPos = glm::vec3(0.0, 0.5, 0.0);
		pushConstants.timeStamp = static_cast<float>(secondCount);
		/*
		vkCmdPushConstants(commandBufferRT, pipelineLayoutRT,
			VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 0, sizeof(PushConstants), &pushConstants);
		*/

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		check_vk_result(vkResetCommandBuffer(commandBufferRT, 0));

		if (vkBeginCommandBuffer(commandBufferRT, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to begin recording command buffer");

		// Transition the image layout if necessary (depends on your specific case)
		// transitionImageLayouts(commandBuffer);

		// Bind ray tracing pipeline
		vkCmdBindPipeline(commandBufferRT, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rayTracingPipeline);

		// Bind descriptor sets
		vkCmdBindDescriptorSets(commandBufferRT, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayoutRT,
			0, 1, &descriptorSetRT, 0, nullptr);

		// Define the shader binding table regions
		VkStridedDeviceAddressRegionKHR raygenShaderBindingTable{};
		raygenShaderBindingTable.deviceAddress = shaderBindingTables.raygen.stridedDeviceAddressRegion.deviceAddress;
		raygenShaderBindingTable.stride = shaderBindingTables.raygen.stridedDeviceAddressRegion.stride;
		raygenShaderBindingTable.size = shaderBindingTables.raygen.stridedDeviceAddressRegion.size;

		VkStridedDeviceAddressRegionKHR missShaderBindingTable{};
		missShaderBindingTable.deviceAddress = shaderBindingTables.miss.stridedDeviceAddressRegion.deviceAddress;
		missShaderBindingTable.stride = shaderBindingTables.miss.stridedDeviceAddressRegion.stride;
		missShaderBindingTable.size = shaderBindingTables.miss.stridedDeviceAddressRegion.size;

		VkStridedDeviceAddressRegionKHR hitShaderBindingTable{};
		hitShaderBindingTable.deviceAddress = shaderBindingTables.hit.stridedDeviceAddressRegion.deviceAddress;
		hitShaderBindingTable.stride = shaderBindingTables.hit.stridedDeviceAddressRegion.stride;
		hitShaderBindingTable.size = shaderBindingTables.hit.stridedDeviceAddressRegion.size;

		VkStridedDeviceAddressRegionKHR callableShaderBindingTable{};

		vkCmdPushConstants(commandBufferRT, pipelineLayoutRT,
			VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, 0, sizeof(PushConstants), &pushConstants);
		// Issue the ray tracing command
		vkCmdTraceRaysKHR(commandBufferRT, &raygenShaderBindingTable, &missShaderBindingTable,
			&hitShaderBindingTable, &callableShaderBindingTable, swapChainExtent.width, swapChainExtent.height, 1);

		// Transition the image layout if necessary (depends on your specific case)
		// transitionImageLayoutsForSampling(commandBuffer);

		if (vkEndCommandBuffer(commandBufferRT) != VK_SUCCESS)
			throw std::runtime_error("Failed to record command buffer");

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores = {};
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR };
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = &waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBufferRT;

		VkSemaphore signalSemaphores[] = { finishedSemaphoreRT };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		check_vk_result(vkResetFences(device, 1, &renderFenceRT));

		if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, renderFenceRT) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit draw command buffer");
		}
	}

	void MainVulkApplication::recordCommandBuffer() {

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		if (vkBeginCommandBuffer(commandBufferRT, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer");
		}

		// Transition the image layout if necessary (depends on your specific case)
		// transitionImageLayouts(commandBuffer);

		// Bind ray tracing pipeline
		vkCmdBindPipeline(commandBufferRT, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rayTracingPipeline);

		// Bind descriptor sets
		vkCmdBindDescriptorSets(commandBufferRT, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayoutRT,
			0, 1, &descriptorSetRT, 0, nullptr);

		// Define the shader binding table regions
		VkStridedDeviceAddressRegionKHR raygenShaderBindingTable{};
		raygenShaderBindingTable.deviceAddress = shaderBindingTables.raygen.stridedDeviceAddressRegion.deviceAddress;
		raygenShaderBindingTable.stride = shaderBindingTables.raygen.stridedDeviceAddressRegion.stride;
		raygenShaderBindingTable.size = shaderBindingTables.raygen.stridedDeviceAddressRegion.size;

		VkStridedDeviceAddressRegionKHR missShaderBindingTable{};
		missShaderBindingTable.deviceAddress = shaderBindingTables.miss.stridedDeviceAddressRegion.deviceAddress;
		missShaderBindingTable.stride = shaderBindingTables.miss.stridedDeviceAddressRegion.stride;
		missShaderBindingTable.size = shaderBindingTables.miss.stridedDeviceAddressRegion.size;

		VkStridedDeviceAddressRegionKHR hitShaderBindingTable{};
		hitShaderBindingTable.deviceAddress = shaderBindingTables.hit.stridedDeviceAddressRegion.deviceAddress;
		hitShaderBindingTable.stride = shaderBindingTables.hit.stridedDeviceAddressRegion.stride;
		hitShaderBindingTable.size = shaderBindingTables.hit.stridedDeviceAddressRegion.size;

		VkStridedDeviceAddressRegionKHR callableShaderBindingTable{};

		// Issue the ray tracing command
		vkCmdTraceRaysKHR(commandBufferRT, &raygenShaderBindingTable, &missShaderBindingTable,
			&hitShaderBindingTable, &callableShaderBindingTable, swapChainExtent.width, swapChainExtent.height, 1);

		// Transition the image layout if necessary (depends on your specific case)
		// transitionImageLayoutsForSampling(commandBuffer);

		if (vkEndCommandBuffer(commandBufferRT) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer");
		}
	}



	void MainVulkApplication::setupCommandBuffer() {

	}

	void MainVulkApplication::createShaderBindingTable(ShaderBindingTable& shaderBindingTable, uint32_t handleCount) {
		createBuffer(uint64_t(rayTracingPipelineProperties.shaderGroupHandleSize * handleCount),
			VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			shaderBindingTable.buffer, shaderBindingTable.memory, true);
		// Get the strided address to be used when dispatching the rays

		const uint32_t handleSizeAligned = align_up(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);
		VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegionKHR{};

		VkBufferDeviceAddressInfoKHR bufferDeviceAI{};
		bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferDeviceAI.buffer = shaderBindingTable.buffer;
		vkGetBufferDeviceAddressKHR(device, &bufferDeviceAI);

		stridedDeviceAddressRegionKHR.deviceAddress = vkGetBufferDeviceAddressKHR(device, &bufferDeviceAI);
		stridedDeviceAddressRegionKHR.stride = handleSizeAligned;
		stridedDeviceAddressRegionKHR.size = handleCount * handleSizeAligned;

		shaderBindingTable.stridedDeviceAddressRegion = stridedDeviceAddressRegionKHR;
		// Map persistent 
		shaderBindingTable.map();
	}

	/*
		Create the Shader Binding Tables that binds the programs and top-level acceleration structure

		SBT Layout used in this sample:

			/-----------\
			| raygen    |
			|-----------|
			| miss      |
			|-----------|
			| hit + int |
			\-----------/

	*/
	void MainVulkApplication::createSBT() {

		//size (in bytes) of a single shader group handle. This size is provided by the Vulkan ray tracing pipeline properties.
		const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
		//Shader group handles need to be aligned to a certain size, specified by shaderGroupHandleAlignment.
		//This alignment is necessary for efficient memory access and to meet hardware requirements.
		//align_up is a function that aligns shaderGroupHandleSize to the nearest multiple of shaderGroupHandleAlignment.
		const uint32_t handleSizeAligned = align_up(rayTracingPipelineProperties.shaderGroupHandleSize, rayTracingPipelineProperties.shaderGroupHandleAlignment);
		const uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
		//The total size of the Shader Binding Table is calculated 
		//as the number of shader groups(groupCount) multiplied by the aligned handle size(handleSizeAligned).
		const uint32_t sbtSize = groupCount * handleSizeAligned;

		std::vector<uint8_t> shaderHandleStorage(sbtSize);
		check_vk_result(vkGetRayTracingShaderGroupHandlesKHR(device, rayTracingPipeline, 0, groupCount, sbtSize, shaderHandleStorage.data()));

		shaderBindingTables.raygen.device = &device;
		shaderBindingTables.miss.device = &device;
		shaderBindingTables.hit.device = &device;
		shaderBindingTables.callable.device = &device;

		createShaderBindingTable(shaderBindingTables.raygen, 1);
		createShaderBindingTable(shaderBindingTables.miss, 1);
		createShaderBindingTable(shaderBindingTables.hit, 1);

		// Copy handles
		memcpy(shaderBindingTables.raygen.mapped, shaderHandleStorage.data(), handleSize);
		memcpy(shaderBindingTables.miss.mapped, shaderHandleStorage.data() + handleSizeAligned, handleSize);
		memcpy(shaderBindingTables.hit.mapped, shaderHandleStorage.data() + handleSizeAligned * 2, handleSize);
	}

	void MainVulkApplication::createAccelerationStructure(AccelerationStructure& accelerationStructure,
		VkAccelerationStructureTypeKHR type,
		VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo) {

		using std::cout; using std::endl;

		cout << "Acceleration Structure Buffer : " << static_cast<uint64_t>(buildSizeInfo.accelerationStructureSize) << endl;
		createBuffer(buildSizeInfo.accelerationStructureSize,
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
			accelerationStructure.buffer, accelerationStructure.memory, true);
		/*
		// Buffer and memory
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = buildSizeInfo.accelerationStructureSize;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Assuming exclusive sharing mode
		check_vk_result(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &accelerationStructure.buffer));

		VkMemoryRequirements memoryRequirements{};
		vkGetBufferMemoryRequirements(device, accelerationStructure.buffer, &memoryRequirements);

		VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
		memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

		VkMemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		check_vk_result(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &accelerationStructure.memory));
		check_vk_result(vkBindBufferMemory(device, accelerationStructure.buffer, accelerationStructure.memory, 0));
		*/
		// Acceleration structure
		VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
		accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		accelerationStructureCreateInfo.buffer = accelerationStructure.buffer;
		accelerationStructureCreateInfo.size = buildSizeInfo.accelerationStructureSize;
		accelerationStructureCreateInfo.type = type;
		check_vk_result(vkCreateAccelerationStructureKHR(device, &accelerationStructureCreateInfo, nullptr, &accelerationStructure.handle));

		// AS device address
		VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo{};
		accelerationDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
		accelerationDeviceAddressInfo.accelerationStructure = accelerationStructure.handle;
		accelerationStructure.deviceAddress = vkGetAccelerationStructureDeviceAddressKHR(device, &accelerationDeviceAddressInfo);
	}

	void MainVulkApplication::createScratchBuffer(VkDeviceSize size) {

		createBuffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR,
			scratchBuffer.handle, scratchBuffer.memory, true);
		/*
		// Buffer and memory
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = size;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Assuming exclusive sharing mode
		check_vk_result(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &scratchBuffer.handle));

		VkMemoryRequirements memoryRequirements{};
		vkGetBufferMemoryRequirements(device, scratchBuffer.handle, &memoryRequirements);

		VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
		memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;

		VkMemoryAllocateInfo memoryAllocateInfo{};
		memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
		memoryAllocateInfo.allocationSize = memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		check_vk_result(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &scratchBuffer.memory));
		check_vk_result(vkBindBufferMemory(device, scratchBuffer.handle, scratchBuffer.memory, 0));
		*/
		// Buffer device address
		VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
		bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferDeviceAddressInfo.buffer = scratchBuffer.handle;
		scratchBuffer.deviceAddress = vkGetBufferDeviceAddressKHR(device, &bufferDeviceAddressInfo);
	}

}

#endif