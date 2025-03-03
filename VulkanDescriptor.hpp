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
        check_vk_result ( vkCreateDescriptorSetLayout(device, &globalLayoutInfo, nullptr, &globalDescriptorSetLayout));
        SetObjectName(device, reinterpret_cast<uint64_t> (globalDescriptorSetLayout), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "globalDescriptorSetLayout");

        /* material set 1 */
        std::vector<VkDescriptorSetLayoutBinding> materialBindings;
        bindingCounter = 0;

        // Material Storage Buffer
        VkDescriptorSetLayoutBinding materialBinding{};
        materialBinding.binding = bindingCounter++;
        materialBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        materialBinding.descriptorCount = 1;
        materialBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        materialBindings.push_back(materialBinding);

        // Texture Array (Bindless)
        VkDescriptorSetLayoutBinding textureBinding{};
        textureBinding.binding = bindingCounter++;
        textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        textureBinding.descriptorCount = 1;
        textureBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        materialBindings.push_back(textureBinding);

        VkDescriptorSetLayoutCreateInfo materialLayoutInfo{};
        materialLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        materialLayoutInfo.bindingCount = static_cast<uint32_t>(materialBindings.size());
        materialLayoutInfo.pBindings = materialBindings.data();
        check_vk_result(vkCreateDescriptorSetLayout(device, &materialLayoutInfo, nullptr, &materialDescriptorSetLayout));
        SetObjectName(device, reinterpret_cast<uint64_t> (materialDescriptorSetLayout), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "materialDescriptorSetLayout");

        /* image set data set 2 */
        std::vector<VkDescriptorSetLayoutBinding> frameBindings;
        bindingCounter = 0;

        // Color Storage Image
        VkDescriptorSetLayoutBinding colorImageBinding{};
        colorImageBinding.binding = bindingCounter++;
        colorImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        colorImageBinding.descriptorCount = 1;
        colorImageBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        frameBindings.push_back(colorImageBinding);

        // Depth Storage Image
        VkDescriptorSetLayoutBinding depthImageBinding{};
        depthImageBinding.binding = bindingCounter++;
        depthImageBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        depthImageBinding.descriptorCount = 1;
        depthImageBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        frameBindings.push_back(depthImageBinding);

        VkDescriptorSetLayoutCreateInfo frameLayoutInfo{};
        frameLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        frameLayoutInfo.bindingCount = static_cast<uint32_t>(frameBindings.size());
        frameLayoutInfo.pBindings = frameBindings.data();
        check_vk_result(vkCreateDescriptorSetLayout(device, &frameLayoutInfo, nullptr, &frameDescriptorSetLayout));
        SetObjectName(device, reinterpret_cast<uint64_t> (frameDescriptorSetLayout), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "frameDescriptorSetLayout");
    }

    void MainVulkApplication::createDescriptorSets() {

        std::vector<VkDescriptorPoolSize> poolSizes = {
            { VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, swapChainImages.size() * 2 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapChainImages.size() },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, objectCount }, // transformations
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, materialCount },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, textureCount },
        };

        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
        descriptorPoolCreateInfo.maxSets = swapChainImages.size() * 3 ; // 3 descriptor sets per frame
        check_vk_result(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, nullptr, &descriptorPool));
        SetObjectName(device, reinterpret_cast<uint64_t> (descriptorPool), VK_OBJECT_TYPE_DESCRIPTOR_POOL, "descriptorPool");

        std::vector<VkDescriptorSetLayout> layouts = { globalDescriptorSetLayout, materialDescriptorSetLayout, frameDescriptorSetLayout };

        std::vector<VkDescriptorSetAllocateInfo> allocInfos(layouts.size());
        for (int i = 0; i < layouts.size(); i++) {
            allocInfos[i].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfos[i].descriptorPool = descriptorPool;
            allocInfos[i].descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
            allocInfos[i].pSetLayouts = &layouts[i];
        }
        globalDescriptorSet.resize(swapChainImages.size());
        materialDescriptorSet.resize(swapChainImages.size());
        frameDescriptorSet.resize(swapChainImages.size());

        check_vk_result(vkAllocateDescriptorSets(device, &allocInfos[0], globalDescriptorSet.data()));
        check_vk_result(vkAllocateDescriptorSets(device, &allocInfos[1], materialDescriptorSet.data()));
        check_vk_result(vkAllocateDescriptorSets(device, &allocInfos[2], frameDescriptorSet.data()));
        SetObjectName(device, reinterpret_cast<uint64_t> (globalDescriptorSet[0]), VK_OBJECT_TYPE_DESCRIPTOR_SET, "globalDescriptorSet");
        SetObjectName(device, reinterpret_cast<uint64_t> (materialDescriptorSet[0]), VK_OBJECT_TYPE_DESCRIPTOR_SET, "materialDescriptorSet");
        SetObjectName(device, reinterpret_cast<uint64_t> (frameDescriptorSet[0]), VK_OBJECT_TYPE_DESCRIPTOR_SET, "frameDescriptorSet");

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