#ifndef __VK_DESCRIPTOR_HPP__
#define __VK_DESCRIPTOR_HPP__

namespace VkApplication {

    void MainVulkApplication::createDescriptorSetLayout() {

        /*
        Separate descriptor sets into different sets so that updates can be less frequent
        for data that doesn't need to be updated constantly.
        */

        /* SET 0 */
        size_t bindingCounter = 0;
        std::vector<VkDescriptorSetLayoutBinding> globalBindings;

        //AS
        VkDescriptorSetLayoutBinding accelLayoutBinding{};
        accelLayoutBinding.binding = bindingCounter++;
        accelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        accelLayoutBinding.descriptorCount = 1;
        accelLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        globalBindings.push_back(accelLayoutBinding);

        //UBO
        VkDescriptorSetLayoutBinding uniformLayoutBinding{};
        uniformLayoutBinding.binding = bindingCounter++;
        uniformLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformLayoutBinding.descriptorCount = 1;
        uniformLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        globalBindings.push_back(uniformLayoutBinding);

        //Light buffer
        VkDescriptorSetLayoutBinding lightBinding{};
        lightBinding.binding = bindingCounter++;
        lightBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        lightBinding.descriptorCount = 1;
        lightBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;;
        globalBindings.push_back(lightBinding);

        VkDescriptorSetLayoutCreateInfo globalLayoutInfo{};
        globalLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        globalLayoutInfo.bindingCount = static_cast<uint32_t>(globalBindings.size());
        globalLayoutInfo.pBindings = globalBindings.data();
        vkCreateDescriptorSetLayout(device, &globalLayoutInfo, nullptr, &globalDescriptorSetLayout);






        /*
        VkDescriptorSetLayoutBinding colorImageLayoutBinding{};
        colorImageLayoutBinding.binding = bindingCounter++;
        colorImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        colorImageLayoutBinding.descriptorCount = 1;
        colorImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        colorImageLayoutBinding.pImmutableSamplers = nullptr; // Not needed for image
        bindings.push_back(colorImageLayoutBinding);
        */

        VkDescriptorSetLayoutBinding depthImageLayoutBinding{};
        depthImageLayoutBinding.binding = bindingCounter++;
        depthImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        depthImageLayoutBinding.descriptorCount = 1;
        depthImageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        depthImageLayoutBinding.pImmutableSamplers = nullptr; // Not needed for image
        bindings.push_back(depthImageLayoutBinding);

        VkDescriptorSetLayoutBinding lightBinding{};
        lightBinding.binding = bindingCounter++;
        lightBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        lightBinding.descriptorCount = 1;
        lightBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        bindings.push_back(lightBinding);

        VkDescriptorSetLayoutBinding materialBinding{};
        materialBinding.binding = bindingCounter++;
        materialBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        materialBinding.descriptorCount = 1;
        materialBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        bindings.push_back(materialBinding);

        // array of textures
        VkDescriptorSetLayoutBinding textureBinding{};
        textureBinding.binding = bindingCounter++;
        textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureBinding.descriptorCount = 1;
        textureBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        bindings.push_back(textureBinding);
 
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void MainVulkApplication::createDescriptorSets() {

        std::vector<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, swapChainImages.size() },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapChainImages.size() },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, objectCount },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, materialCount },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,textureCount },
        };

        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
        descriptorPoolCreateInfo.maxSets = swapChainImages.size() ;
        check_vk_result(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool));

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool; // Assume you have created a descriptor pool
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &descriptorSetLayout;

        std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(swapChainImages.size());
        // Vulkan allocates descriptor sets (not memory for buffers or textures).
        // The descriptor set is simply a handle that references existing GPU resources.
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) 
            throw std::runtime_error("failed to allocate descriptor sets!");
        
        // TODO : make buffers
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            // Updates a descriptor set with a buffer, image, or sampler.
            // Vulkan does not automatically track uniforms/textures like OpenGL.
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }
}

#endif