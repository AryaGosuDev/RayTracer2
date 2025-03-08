#ifndef __VK_RENDER_SETTINGS_HPP__
#define __VK_RENDER_SETTINGS_HPP__


namespace VkApplication {

    auto createShaderStage(VkShaderModule & module, VkShaderStageFlagBits stage) {
        VkPipelineShaderStageCreateInfo stageInfo{};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage = stage;
        stageInfo.module = module;
        stageInfo.pName = "main";
        return stageInfo;
    }

    VkRayTracingShaderGroupCreateInfoKHR makeGeneralGroup(uint32_t shaderIndex) {
        VkRayTracingShaderGroupCreateInfoKHR group{};
        group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        group.generalShader = shaderIndex;
        group.closestHitShader = VK_SHADER_UNUSED_KHR;
        group.anyHitShader = VK_SHADER_UNUSED_KHR;
        group.intersectionShader = VK_SHADER_UNUSED_KHR;
        return group;
    }

    VkRayTracingShaderGroupCreateInfoKHR makeHitGroup(uint32_t closestHit, uint32_t anyHit, uint32_t intersection) {
        VkRayTracingShaderGroupCreateInfoKHR group{};
        group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        group.generalShader = VK_SHADER_UNUSED_KHR;
        group.closestHitShader = closestHit;
        group.anyHitShader = anyHit;
        group.intersectionShader = intersection;
        return group;
    }

    void MainVulkApplication::createGraphicsPipeline() {

        // Get ray tracing pipeline properties

        rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
        VkPhysicalDeviceProperties2 deviceProperties2{};
        deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProperties2.pNext = &rayTracingPipelineProperties;
        vkGetPhysicalDeviceProperties2(physicalDevice, &deviceProperties2);

        // Get acceleration structure properties
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

        std::vector<std::string> shaderPaths = {
            "shaders/RT_raygen.spv",
            "shaders/RT_miss.spv",
            "shaders/RT_closesthit.spv",
            "shaders/RT_anyhit.spv",
            "shaders/RT_intersection.spv",
            "shaders/RT_missShadow.spv",
            "shaders/RT_anyhit_shadow.spv"
        };

        std::vector<VkShaderModule> shaderModules;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

        for (const auto& path : shaderPaths) {
            auto shaderCode = readFile(path);
            VkShaderModule module = createShaderModule(shaderCode);
            shaderModules.push_back(module);
        }

        std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

        shaderStages.push_back(createShaderStage(shaderModules[0], VK_SHADER_STAGE_RAYGEN_BIT_KHR));
        shaderStages.push_back(createShaderStage(shaderModules[1], VK_SHADER_STAGE_MISS_BIT_KHR));
        shaderStages.push_back(createShaderStage(shaderModules[2], VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));
        shaderStages.push_back(createShaderStage(shaderModules[3], VK_SHADER_STAGE_ANY_HIT_BIT_KHR));
        shaderStages.push_back(createShaderStage(shaderModules[4], VK_SHADER_STAGE_INTERSECTION_BIT_KHR));
        shaderStages.push_back(createShaderStage(shaderModules[5], VK_SHADER_STAGE_MISS_BIT_KHR));
        shaderStages.push_back(createShaderStage(shaderModules[6], VK_SHADER_STAGE_ANY_HIT_BIT_KHR));

        VkRayTracingShaderGroupCreateInfoKHR NormalRaygenGroup{};
        NormalRaygenGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        NormalRaygenGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        NormalRaygenGroup.generalShader = 0;  // Index of raygen shader in pStages
        NormalRaygenGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        NormalRaygenGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        NormalRaygenGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(NormalRaygenGroup);

        VkRayTracingShaderGroupCreateInfoKHR normalMissGroup{};
        normalMissGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        normalMissGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        normalMissGroup.generalShader = 1;  // Index in pStages array
        normalMissGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        normalMissGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        normalMissGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(normalMissGroup);

        VkRayTracingShaderGroupCreateInfoKHR shadowMissGroup{};
        shadowMissGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shadowMissGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shadowMissGroup.generalShader = 5;  // Index in pStages array
        shadowMissGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        shadowMissGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shadowMissGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(shadowMissGroup);

        VkRayTracingShaderGroupCreateInfoKHR normalHitGroup{};
        normalHitGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        normalHitGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;  // For procedural
        normalHitGroup.generalShader = VK_SHADER_UNUSED_KHR;  // Not used in hit groups
        normalHitGroup.closestHitShader = 2;  // Index in pStages array
        normalHitGroup.anyHitShader = VK_SHADER_UNUSED_KHR;  // Optional: VK_SHADER_UNUSED_KHR if not used
        normalHitGroup.intersectionShader = VK_SHADER_UNUSED_KHR;  // Not used for triangles
        shaderGroups.push_back(normalHitGroup);

        VkRayTracingShaderGroupCreateInfoKHR shadowHitGroup{};
        shadowHitGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shadowHitGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;  // For procedural
        shadowHitGroup.generalShader = VK_SHADER_UNUSED_KHR;  // Not used in hit groups
        shadowHitGroup.closestHitShader = VK_SHADER_UNUSED_KHR;  // Index in pStages array
        shadowHitGroup.anyHitShader = 6;  // Optional: VK_SHADER_UNUSED_KHR if not used
        shadowHitGroup.intersectionShader = VK_SHADER_UNUSED_KHR;  // Not used for triangles
        shaderGroups.push_back(shadowHitGroup);

        VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCI{};
        rayTracingPipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        rayTracingPipelineCI.pNext = nullptr;
        rayTracingPipelineCI.flags = 0;
        rayTracingPipelineCI.stageCount = sizeof(shaderStages) / sizeof(shaderStages[0]); // Number of shader stages
        rayTracingPipelineCI.pStages = shaderStages.data();  // Pointer to shader stages array
        rayTracingPipelineCI.groupCount = shaderGroups.size();  // Number of shader groups
        rayTracingPipelineCI.pGroups = shaderGroups.data();  // Pointer to shader groups array
        rayTracingPipelineCI.maxPipelineRayRecursionDepth = 1;  // Adjust based on your ray tracing depth requirements
        rayTracingPipelineCI.layout = pipelineLayout;  // The pipeline layout that matches the descriptors used by the shaders

        check_vk_result(vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCI, nullptr, &graphicsPipeline));
        SetObjectName(device, reinterpret_cast<uint64_t> (graphicsPipeline), VK_OBJECT_TYPE_PIPELINE, "graphicsPipeline");
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