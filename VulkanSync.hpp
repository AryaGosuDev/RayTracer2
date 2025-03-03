#ifndef __VK_SYNC_HPP__
#define __VK_SYNC_HPP__

namespace VkApplication {

    void MainVulkApplication::createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
            std::string temp1 = "imageAvailableSemaphores_" + i; std::string temp2 = "renderFinishedSemaphores_" + i;
            std::string tempfence = "inFlightFences_" + i;

            SetObjectName(device, reinterpret_cast<uint64_t> (imageAvailableSemaphores[i]), VK_OBJECT_TYPE_SEMAPHORE, temp1);
            SetObjectName(device, reinterpret_cast<uint64_t> (renderFinishedSemaphores[i]), VK_OBJECT_TYPE_SEMAPHORE, temp2);
            SetObjectName(device, reinterpret_cast<uint64_t> (inFlightFences[i]), VK_OBJECT_TYPE_FENCE, tempfence);

        }
    }
}

#endif