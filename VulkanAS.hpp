#ifndef __VK_AS_HPP__
#define __VK_AS_HPP__


namespace VkApplication {


	/*
	* For importing assets, Assimp (Open Asset Import Library) is a popular choice for handling 3D model formats like FBX, OBJ, and glTF. For textures, stb_image is a lightweight option. 
	If you need a full scene import with PBR materials, lights, and textures, glTF 2.0 (with libraries like tinygltf) is a great modern choice.

In ray tracing, aligning objects with their textures and materials typically involves binding these assets to descriptors. 
Since there are no draw calls per object (as in rasterization), you usually:

Store all scene materials in a structured buffer (SSBO).
Store all textures in a bindless descriptor array.
Assign each object an index into the material and texture array.
Pass these indices in your acceleration structure.
When a ray hits a triangle, retrieve the material and texture using the stored indices.
Would you like an example setup for handling materials and textures in Vulkan RT?
	
	
	*/

	/*
	BLAS: BLASs ​are responsible for storing the geometric data for individual objects in the scene. 
	Each object can have one or more BLAS associated with it, and each BLAS can contain one or more geometric primitives, such as triangles or instances of other BLASs.
	The BLAS is responsible for determining how rays intersect with the geometry they contains, making it a fundamental part of the ray tracing process.

 
	*/

	void MainVulkApplication::createBLAS() {

		VkAccelerationStructureGeometryKHR geometry = {};
		geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
		geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR; // Skip anyhit shader for performance
		geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
		geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		geometry.geometry.triangles.vertexData.deviceAddress = GetBufferDeviceAddress(vertexBuffer);
		geometry.geometry.triangles.vertexStride = sizeof(Vertex);
		geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
		geometry.geometry.triangles.indexData.deviceAddress = GetBufferDeviceAddress(indexBuffer);

		VkAccelerationStructureBuildGeometryInfoKHR buildInfo = {};
		buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR; // Optimize for ray traversal
		buildInfo.geometryCount = 1;
		buildInfo.pGeometries = &geometry;

		VkAccelerationStructureBuildSizesInfoKHR sizeInfo = {};
		sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &primitiveCount, &sizeInfo);

		VkAccelerationStructureCreateInfoKHR asCreateInfo = {};
		asCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
		asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		asCreateInfo.size = sizeInfo.accelerationStructureSize;
		vkCreateAccelerationStructureKHR(device, &asCreateInfo, nullptr, &blas);

		VkAccelerationStructureBuildRangeInfoKHR rangeInfo = {};
		rangeInfo.primitiveCount = primitiveCount;

		const VkAccelerationStructureBuildRangeInfoKHR* rangeInfos[] = { &rangeInfo };

		vkCmdBuildAccelerationStructuresKHR(cmdBuffer, 1, &buildInfo, rangeInfos);

	}

	/* 
	* TLAS: The ​TLAS, on the other hand, does not contain geometric data.
	Instead, it contains instances of BLASs. Each instance defines a transformation (such as translation, rotation, or scaling) and a BLAS to apply it to. 
	When ray tracing, the system starts from the TLAS and works its way down to the appropriate BLAS.
	The TLAS essentially acts as a directory that guides the system to the correct BLAS based on the ray’s path.
```*/

	void MainVulkApplication::createTLAS() {

		VkAccelerationStructureInstanceKHR instance = {};
		instance.transform = ConvertToVkTransformMatrix(objectTransform);
		instance.instanceCustomIndex = objectID;
		instance.mask = 0xFF; // Visibility mask
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.accelerationStructureReference = GetASDeviceAddress(blas);

		VkAccelerationStructureBuildGeometryInfoKHR tlasBuildInfo = {};
		tlasBuildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		tlasBuildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		tlasBuildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		tlasBuildInfo.geometryCount = 1;
		tlasBuildInfo.pGeometries = &tlasGeometry;



	}




	void MainVulkApplication::setupAS() {

		// Build
		VkAccelerationStructureGeometryKHR accelerationStructureGeometry{};
		accelerationStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		// Instead of providing actual geometry (e.g. triangles), we only provide the axis aligned bounding boxes (AABBs) of the spheres
		// The data for the actual spheres is passed elsewhere as a shader storage buffer object
		accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
		accelerationStructureGeometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
		accelerationStructureGeometry.geometry.aabbs.data.deviceAddress = bufferDeviceAddressAABB;
		accelerationStructureGeometry.geometry.aabbs.stride = sizeof(VkAabbPositionsKHR);

		// Get size info
		VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{};
		accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationStructureBuildGeometryInfo.geometryCount = 1;
		accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;

		uint32_t aabbCount = 1;
		VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{};
		accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

		vkGetAccelerationStructureBuildSizesKHR(
			device,
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&accelerationStructureBuildGeometryInfo,
			&aabbCount,
			&accelerationStructureBuildSizesInfo);

		createAccelerationStructure(bottomLevelAS, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, accelerationStructureBuildSizesInfo);

		// Create a small scratch buffer used during build of the bottom level acceleration structure
		createScratchBuffer(accelerationStructureBuildSizesInfo.buildScratchSize);

		VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfo{};
		accelerationBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
		accelerationBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		accelerationBuildGeometryInfo.dstAccelerationStructure = bottomLevelAS.handle;
		accelerationBuildGeometryInfo.geometryCount = 1;
		accelerationBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
		accelerationBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;

		VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
		accelerationStructureBuildRangeInfo.primitiveCount = aabbCount;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR*> accelerationBuildStructureRangeInfos = { &accelerationStructureBuildRangeInfo };

		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		// Build the acceleration structure on the device via a one-time command buffer submission
		// Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds
		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer,
			1,
			&accelerationBuildGeometryInfo,
			accelerationBuildStructureRangeInfos.data());

		endSingleTimeCommands(commandBuffer);

		if (scratchBuffer.memory != VK_NULL_HANDLE)
			vkFreeMemory(device, scratchBuffer.memory, nullptr);
		if (scratchBuffer.handle != VK_NULL_HANDLE)
			vkDestroyBuffer(device, scratchBuffer.handle, nullptr);

		VkTransformMatrixKHR transformMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f };

		VkAccelerationStructureInstanceKHR instance{};
		instance.transform = transformMatrix;
		instance.instanceCustomIndex = 0;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.accelerationStructureReference = bottomLevelAS.deviceAddress;

		// Buffer for instance data
		VkBuffer instancesBuffer; VkDeviceMemory instancesBufferMemory;
		createBuffer(sizeof(VkAccelerationStructureInstanceKHR),
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			instancesBuffer, instancesBufferMemory, true);

		void* mappedInstance = nullptr;

		check_vk_result(vkMapMemory(device, instancesBufferMemory, 0,
			sizeof(VkAccelerationStructureInstanceKHR), 0, &mappedInstance));
		memcpy(mappedInstance, &instance, sizeof(VkAccelerationStructureInstanceKHR));
		vkUnmapMemory(device, instancesBufferMemory);
		mappedInstance = NULL;

		VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
		VkBufferDeviceAddressInfoKHR bufferDeviceAInstance{};
		bufferDeviceAInstance.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferDeviceAInstance.buffer = instancesBuffer;
		instanceDataDeviceAddress.deviceAddress = vkGetBufferDeviceAddressKHR(device, &bufferDeviceAInstance);

		VkAccelerationStructureGeometryKHR accelerationStructureGeometryI{};
		accelerationStructureGeometryI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
		accelerationStructureGeometryI.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
		accelerationStructureGeometryI.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
		accelerationStructureGeometryI.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
		accelerationStructureGeometryI.geometry.instances.arrayOfPointers = VK_FALSE;
		accelerationStructureGeometryI.geometry.instances.data = instanceDataDeviceAddress;

		// Get size info
		VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfoI{};
		accelerationStructureBuildGeometryInfoI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationStructureBuildGeometryInfoI.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		accelerationStructureBuildGeometryInfoI.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationStructureBuildGeometryInfoI.geometryCount = 1;
		accelerationStructureBuildGeometryInfoI.pGeometries = &accelerationStructureGeometryI;

		uint32_t primitive_count = 1;

		VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfoI{};
		accelerationStructureBuildSizesInfoI.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
		vkGetAccelerationStructureBuildSizesKHR(
			device,
			VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
			&accelerationStructureBuildGeometryInfoI,
			&primitive_count,
			&accelerationStructureBuildSizesInfoI);

		createAccelerationStructure(topLevelAS, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, accelerationStructureBuildSizesInfoI);

		// Create a small scratch buffer used during build of the top level acceleration structure
		createScratchBuffer(accelerationStructureBuildSizesInfoI.buildScratchSize);

		VkAccelerationStructureBuildGeometryInfoKHR accelerationBuildGeometryInfoTop{};
		accelerationBuildGeometryInfoTop.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
		accelerationBuildGeometryInfoTop.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
		accelerationBuildGeometryInfoTop.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
		accelerationBuildGeometryInfoTop.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
		accelerationBuildGeometryInfoTop.dstAccelerationStructure = topLevelAS.handle;
		accelerationBuildGeometryInfoTop.geometryCount = 1;
		accelerationBuildGeometryInfoTop.pGeometries = &accelerationStructureGeometryI;
		accelerationBuildGeometryInfoTop.scratchData.deviceAddress = scratchBuffer.deviceAddress;

		VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfoTop{};
		accelerationStructureBuildRangeInfoTop.primitiveCount = 1;
		accelerationStructureBuildRangeInfoTop.primitiveOffset = 0;
		accelerationStructureBuildRangeInfoTop.firstVertex = 0;
		accelerationStructureBuildRangeInfoTop.transformOffset = 0;

		accelerationBuildStructureRangeInfos.clear();
		accelerationBuildStructureRangeInfos.push_back(&accelerationStructureBuildRangeInfoTop);

		VkCommandBuffer commandBuffer1 = beginSingleTimeCommands();
		// Build the acceleration structure on the device via a one-time command buffer submission
		// Some implementations may support acceleration structure building on the host (VkPhysicalDeviceAccelerationStructureFeaturesKHR->accelerationStructureHostCommands), but we prefer device builds

		vkCmdBuildAccelerationStructuresKHR(
			commandBuffer1,
			1,
			&accelerationBuildGeometryInfoTop,
			accelerationBuildStructureRangeInfos.data());
		endSingleTimeCommands(commandBuffer1);

		if (scratchBuffer.memory != VK_NULL_HANDLE)
			vkFreeMemory(device, scratchBuffer.memory, nullptr);
		if (scratchBuffer.handle != VK_NULL_HANDLE)
			vkDestroyBuffer(device, scratchBuffer.handle, nullptr);
		if (instancesBuffer)
			vkDestroyBuffer(device, instancesBuffer, nullptr);
		if (instancesBufferMemory)
			vkFreeMemory(device, instancesBufferMemory, nullptr);

	}



}

#endif