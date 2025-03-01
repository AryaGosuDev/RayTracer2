#ifndef __VK_RENDER_SETTINGS_HPP__
#define __VK_RENDER_SETTINGS_HPP__


namespace VkApplication {

    void MainVulkApplication::createGraphicsPipeline() {

        // Get ray tracing pipeline properties

        rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
        VkPhysicalDeviceProperties2 deviceProperties2{};
        deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties2.pNext = &rayTracingPipelineProperties;
        vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);

        // Get acceleration structure properties, which will be used later on in the sample
        accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
        VkPhysicalDeviceFeatures2 deviceFeatures2{};
        deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        deviceFeatures2.pNext = &accelerationStructureFeatures;
        vkGetPhysicalDeviceFeatures2(physicalDevice, &deviceFeatures2);

        // Get the function pointers required for ray tracing
        /*
        Ray Tracing is an Extension(VK_KHR_ray_tracing_pipeline and VK_KHR_acceleration_structure)

            These features are not part of core Vulkan and must be loaded dynamically.
            Vulkan Uses a Function Pointer Mechanism

            Unlike OpenGL, Vulkan does not expose all functions by default.
            Instead, you must load function pointers at runtime using vkGetDeviceProcAddr().
            This keeps Vulkan lightweight and modular.
            Different GPUs and Drivers May or May Not Support Ray Tracing

            Even if Vulkan is available, not all GPUs support ray tracing extensions.
            By checking and loading functions dynamically, your code can gracefully fall back when ray tracing isn’t available.
        */
        vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR"));
        vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR"));
        vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device, "vkBuildAccelerationStructuresKHR"));
        vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR"));
        vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureKHR"));
        vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR"));
        vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureDeviceAddressKHR"));
        vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysKHR"));
        vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesKHR"));
        vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesKHR"));

        //create output buffer images
        createImage(swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            rtImageViews.RTColorImage, rtImageViews.RTColorImageMemory);
        transitionImageLayout(rtImageViews.RTColorImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        rtImageViews.RTColorImageView = createImageView(rtImageViews.RTColorImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

        createImage(swapChainExtent.width, swapChainExtent.height, VK_FORMAT_R32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            rtImageViews.RTDepthImage, rtImageViews.RTDepthImageMemory);
        transitionImageLayout(rtImageViews.RTDepthImage, VK_FORMAT_R32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
        rtImageViews.RTDepthImageView = createImageView(rtImageViews.RTDepthImage, VK_FORMAT_R32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);

        auto rayGShaderCode = readFile("shaders/RT_raygen.spv");
        auto missRTShaderCode = readFile("shaders/RT_miss.spv");
        auto closeHitRTShaderCode = readFile("shaders/RT_closesthit.spv");
        auto intersectRTShaderCode = readFile("shaders/RT_intersection.spv");

        VkShaderModule rayGShaderModule = createShaderModule(rayGShaderCode);
        VkShaderModule missRTShaderModule = createShaderModule(missRTShaderCode);
        VkShaderModule closeHitRTShaderModule = createShaderModule(closeHitRTShaderCode);
        VkShaderModule intersectRTShaderModule = createShaderModule(intersectRTShaderCode);

        VkPipelineShaderStageCreateInfo raygenShaderStageInfo{};
        raygenShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        raygenShaderStageInfo.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        raygenShaderStageInfo.module = rayGShaderModule;
        raygenShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo missRTShaderStageInfo{};
        missRTShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        missRTShaderStageInfo.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
        missRTShaderStageInfo.module = missRTShaderModule;
        missRTShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo closeHitRTShaderStageInfo{};
        closeHitRTShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        closeHitRTShaderStageInfo.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        closeHitRTShaderStageInfo.module = closeHitRTShaderModule;
        closeHitRTShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo intersectRTShaderStageInfo{};
        intersectRTShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        intersectRTShaderStageInfo.stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        intersectRTShaderStageInfo.module = intersectRTShaderModule;
        intersectRTShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { raygenShaderStageInfo, missRTShaderStageInfo, closeHitRTShaderStageInfo,intersectRTShaderStageInfo };

        VkRayTracingShaderGroupCreateInfoKHR raygenGroup{};
        raygenGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        raygenGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        raygenGroup.generalShader = 0;  // Index of raygen shader in pStages
        raygenGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        raygenGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        raygenGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(raygenGroup);

        VkRayTracingShaderGroupCreateInfoKHR missGroup{};
        missGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        missGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR; 
        missGroup.generalShader = 1;  // Index in pStages array
        missGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        missGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        missGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(missGroup);

        VkRayTracingShaderGroupCreateInfoKHR hitGroup{};
        hitGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        hitGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;  // For procedural
        hitGroup.generalShader = VK_SHADER_UNUSED_KHR;  // Not used in hit groups
        hitGroup.closestHitShader = 2;  // Index in pStages array
        hitGroup.anyHitShader = VK_SHADER_UNUSED_KHR;  // Optional: VK_SHADER_UNUSED_KHR if not used
        hitGroup.intersectionShader = 3;  // Not used for triangles
        shaderGroups.push_back(hitGroup);

        VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
        rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        rayTracingPipelineCI.pNext = nullptr;
        rayTracingPipelineCI.flags = 0;
        rayTracingPipelineCI.stageCount = sizeof(shaderStages) / sizeof(shaderStages[0]); // Number of shader stages
        rayTracingPipelineCI.pStages = shaderStages;  // Pointer to shader stages array
        rayTracingPipelineCI.groupCount = shaderGroups.size();  // Number of shader groups
        VkRayTracingShaderGroupCreateInfoKHR shaderGroups[] = { raygenGroup, missGroup, hitGroup };
        rayTracingPipelineCI.pGroups = shaderGroups;  // Pointer to shader groups array
        rayTracingPipelineCI.maxPipelineRayRecursionDepth = 1;  // Adjust based on your ray tracing depth requirements
        rayTracingPipelineCI.layout = pipelineLayout;  // The pipeline layout that matches the descriptors used by the shaders
    }

    VkShaderModule MainVulkApplication::createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }
}

#endif