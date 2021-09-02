//
// Created by marco.silipo on 31.08.2021.
//
#include "vengine.h"
#include "VkBootstrap.h"

#include <GLFW/glfw3.h>

#include <sstream>


template<typename T>
inline std::string VKB_ERROR(std::string_view message, T error)
{
    std::stringstream sstream;
    sstream << error.error() << ": " << message;
    return sstream.str();
}

template<>
inline std::string VKB_ERROR<VkResult>(std::string_view message, VkResult error)
{
    std::stringstream sstream;
    switch (error)
    {
        default: sstream << "NOT AVAILABLE";
            break;
        case VK_SUCCESS: sstream << "VK_SUCCESS";
            break;
        case VK_NOT_READY: sstream << "VK_NOT_READY";
            break;
        case VK_TIMEOUT: sstream << "VK_TIMEOUT";
            break;
        case VK_EVENT_SET: sstream << "VK_EVENT_SET";
            break;
        case VK_EVENT_RESET: sstream << "VK_EVENT_RESET";
            break;
        case VK_INCOMPLETE: sstream << "VK_INCOMPLETE";
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY: sstream << "VK_ERROR_OUT_OF_HOST_MEMORY";
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: sstream << "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            break;
        case VK_ERROR_INITIALIZATION_FAILED: sstream << "VK_ERROR_INITIALIZATION_FAILED";
            break;
        case VK_ERROR_DEVICE_LOST: sstream << "VK_ERROR_DEVICE_LOST";
            break;
        case VK_ERROR_MEMORY_MAP_FAILED: sstream << "VK_ERROR_MEMORY_MAP_FAILED";
            break;
        case VK_ERROR_LAYER_NOT_PRESENT: sstream << "VK_ERROR_LAYER_NOT_PRESENT";
            break;
        case VK_ERROR_EXTENSION_NOT_PRESENT: sstream << "VK_ERROR_EXTENSION_NOT_PRESENT";
            break;
        case VK_ERROR_FEATURE_NOT_PRESENT: sstream << "VK_ERROR_FEATURE_NOT_PRESENT";
            break;
        case VK_ERROR_INCOMPATIBLE_DRIVER: sstream << "VK_ERROR_INCOMPATIBLE_DRIVER";
            break;
        case VK_ERROR_TOO_MANY_OBJECTS: sstream << "VK_ERROR_TOO_MANY_OBJECTS";
            break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED: sstream << "VK_ERROR_FORMAT_NOT_SUPPORTED";
            break;
        case VK_ERROR_FRAGMENTED_POOL: sstream << "VK_ERROR_FRAGMENTED_POOL";
            break;
        case VK_ERROR_UNKNOWN: sstream << "VK_ERROR_UNKNOWN";
            break;
        case VK_ERROR_OUT_OF_POOL_MEMORY: sstream << "VK_ERROR_OUT_OF_POOL_MEMORY";
            break;
        case VK_ERROR_INVALID_EXTERNAL_HANDLE: sstream << "VK_ERROR_INVALID_EXTERNAL_HANDLE";
            break;
        case VK_ERROR_FRAGMENTATION: sstream << "VK_ERROR_FRAGMENTATION";
            break;
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: sstream << "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
            break;
        case VK_ERROR_SURFACE_LOST_KHR: sstream << "VK_ERROR_SURFACE_LOST_KHR";
            break;
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: sstream << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
            break;
        case VK_SUBOPTIMAL_KHR: sstream << "VK_SUBOPTIMAL_KHR";
            break;
        case VK_ERROR_OUT_OF_DATE_KHR: sstream << "VK_ERROR_OUT_OF_DATE_KHR";
            break;
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: sstream << "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
            break;
        case VK_ERROR_VALIDATION_FAILED_EXT: sstream << "VK_ERROR_VALIDATION_FAILED_EXT";
            break;
        case VK_ERROR_INVALID_SHADER_NV: sstream << "VK_ERROR_INVALID_SHADER_NV";
            break;
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
            sstream
                    << "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
            break;
        case VK_ERROR_NOT_PERMITTED_EXT: sstream << "VK_ERROR_NOT_PERMITTED_EXT";
            break;
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: sstream << "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
            break;
        case VK_THREAD_IDLE_KHR: sstream << "VK_THREAD_IDLE_KHR";
            break;
        case VK_THREAD_DONE_KHR: sstream << "VK_THREAD_DONE_KHR";
            break;
        case VK_OPERATION_DEFERRED_KHR: sstream << "VK_OPERATION_DEFERRED_KHR";
            break;
        case VK_OPERATION_NOT_DEFERRED_KHR: sstream << "VK_OPERATION_NOT_DEFERRED_KHR";
            break;
        case VK_PIPELINE_COMPILE_REQUIRED_EXT: sstream << "VK_PIPELINE_COMPILE_REQUIRED_EXT";
            break;
        case VK_RESULT_MAX_ENUM: sstream << "VK_RESULT_MAX_ENUM";
            break;
    }
    sstream << " (" << std::uppercase << std::hex << static_cast<uint64_t>(error) << "): " << message;
    return sstream.str();
}

vengine::vengine::vengine()
        : m_initialized(false), m_glfw_initialized(false), m_frame_counter(0), m_vulkan_surface(nullptr),
        m_vulkan_render_fence(nullptr), m_vulkan_command_pool(nullptr), m_vulkan_render_semaphore(nullptr),
        m_vulkan_present_semaphore(nullptr), m_vulkan_render_pass(nullptr)
{
    glfw_window_init(800, 600, "vengine");
    if (!m_glfw_initialized)
    {
        m_errors.emplace_back("Failed to initialize glfw.");
        return;
    }
    // Create vulkan instance
    auto
            instance_result = vkb::InstanceBuilder { }.set_app_name("vengine")
                                                      .request_validation_layers()
                                                      .use_default_debug_messenger()
                                                      .build();
    if (!instance_result)
    {
        m_errors.push_back(VKB_ERROR("Failed to create vulkan instance.", instance_result));
        return;
    }
    m_vkb_instance = instance_result.value();

    // Create vulkan surface
    auto glfw_surface_creation_result = glfwCreateWindowSurface(
            m_vkb_instance.instance, static_cast<GLFWwindow *>(m_window_handle), nullptr, &m_vulkan_surface);
    if (glfw_surface_creation_result != VK_SUCCESS)
    {
        m_errors.push_back(VKB_ERROR("Failed to create vulkan surface using glfw.", instance_result));
        return;
    }


    // Pick vulkan physical device
    auto
            physical_device_result = vkb::PhysicalDeviceSelector { m_vkb_instance }.set_surface(m_vulkan_surface)
                                                                                   .set_minimum_version(1, 1)
                                                                                   .require_dedicated_transfer_queue()
                                                                                   .require_present()
                                                                                   .select();
    if (!physical_device_result)
    {
        m_errors.push_back(VKB_ERROR("Failed to find suitable vulkan physical device.", instance_result));
        return;
    }
    m_vkb_physical_device = physical_device_result.value();


    // Create logical device
    auto device_result = vkb::DeviceBuilder { m_vkb_physical_device }.build();
    if (!device_result)
    {
        m_errors.push_back(VKB_ERROR("Failed to create vulkan device.", instance_result));
        return;
    }
    m_vkb_device = device_result.value();

    // Create logical device
    auto
            swap_chain_result
            = vkb::SwapchainBuilder { m_vkb_device }.set_desired_format({ .format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
                                                    .build();
    if (!swap_chain_result)
    {
        m_errors.push_back(VKB_ERROR("Failed to create vulkan swap chain.", instance_result));
        return;
    }
    m_vkb_swap_chain = swap_chain_result.value();

    // Get images
    auto swap_chain_images_result = m_vkb_swap_chain.get_images();
    if (!swap_chain_images_result)
    {
        m_errors.push_back(VKB_ERROR("Failed to receive images from swap chain.", instance_result));
        return;
    }
    m_swap_chain_images = swap_chain_images_result.value();

    // Get image views
    auto swap_chain_image_views_result = m_vkb_swap_chain.get_image_views();
    if (!swap_chain_image_views_result)
    {
        m_errors.push_back(VKB_ERROR("Failed to receive image views from swap chain.", instance_result));
        return;
    }
    m_swap_chain_image_views = swap_chain_image_views_result.value();


    // Get Graphics Queue
    auto graphics_queue_result = m_vkb_device.get_queue(vkb::QueueType::graphics);
    if (!graphics_queue_result)
    {
        m_errors.push_back(VKB_ERROR("Failed to receive graphics queue.", instance_result));
        return;
    }
    m_vkb_graphics_queue = graphics_queue_result.value();

    // Get Graphics Queue Index
    auto graphics_queue_index_result = m_vkb_device.get_queue_index(vkb::QueueType::graphics);
    if (!graphics_queue_index_result)
    {
        m_errors.push_back(VKB_ERROR("Failed to receive graphics queue index.", instance_result));
        return;
    }
    m_vkb_graphics_queue_index = graphics_queue_index_result.value();

    // Create command pool
    {
        VkCommandPoolCreateInfo command_pool_create_info = { };
        command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_create_info.pNext = nullptr;
        command_pool_create_info.queueFamilyIndex = m_vkb_graphics_queue_index;
        command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        auto
                command_pool_result = vkCreateCommandPool(
                m_vkb_device.device,
                &command_pool_create_info,
                nullptr,
                &m_vulkan_command_pool);
        if (command_pool_result != VK_SUCCESS)
        {
            m_errors.push_back(VKB_ERROR("Failed to create command pool.", command_pool_result));
            return;
        }
    }

    // Create render pass
    {
        VkAttachmentDescription color_attachment = { };
        color_attachment.format = m_vkb_swap_chain.image_format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_ref = { };
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription sub_pass_description = { };
        sub_pass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub_pass_description.colorAttachmentCount = 1;
        sub_pass_description.pColorAttachments = &color_attachment_ref;

        VkRenderPassCreateInfo render_pass_info = { };
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &color_attachment;

        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &sub_pass_description;


        auto
                render_pass_result = vkCreateRenderPass(
                m_vkb_device.device,
                &render_pass_info,
                nullptr,
                &m_vulkan_render_pass);
        if (render_pass_result != VK_SUCCESS)
        {
            m_errors.push_back(VKB_ERROR("Failed to create render pass.", render_pass_result));
            return;
        }
    }

    // Create frame buffers
    {
        VkFramebufferCreateInfo framebuffer_create_info = { };
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.pNext = nullptr;

        framebuffer_create_info.renderPass = m_vulkan_render_pass;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.width = m_vkb_swap_chain.extent.width;
        framebuffer_create_info.height = m_vkb_swap_chain.extent.height;
        framebuffer_create_info.layers = 1;

        m_frame_buffers = std::vector<VkFramebuffer>(m_swap_chain_image_views.size(), nullptr);
        for (size_t i = 0; i < m_swap_chain_image_views.size(); i++)
        {
            framebuffer_create_info.pAttachments = &m_swap_chain_image_views[i];
            auto
                    create_frame_buffer_result = vkCreateFramebuffer(
                    m_vkb_device.device,
                    &framebuffer_create_info,
                    nullptr,
                    &m_frame_buffers[i]);
            if (create_frame_buffer_result != VK_SUCCESS)
            {
                m_errors.push_back(VKB_ERROR("Failed to create frame buffer.", create_frame_buffer_result));
                return;
            }
        }
    }

    // Create render fence
    {
        VkFenceCreateInfo fence_create_info = { };
        fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create_info.pNext = nullptr;

        //we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        auto
                fence_create_result = vkCreateFence(
                m_vkb_device.device,
                &fence_create_info,
                nullptr,
                &m_vulkan_render_fence);
        if (fence_create_result != VK_SUCCESS)
        {
            m_errors.push_back(VKB_ERROR("Failed to create render fence.", fence_create_result));
            return;
        }
    }

    // Create present and render semaphore
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = { };
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;
        semaphoreCreateInfo.flags = 0;

        auto
                semaphore_create_result = vkCreateSemaphore(
                m_vkb_device.device,
                &semaphoreCreateInfo,
                nullptr,
                &m_vulkan_present_semaphore);
        if (semaphore_create_result != VK_SUCCESS)
        {
            m_errors.push_back(VKB_ERROR("Failed to create present semaphore.", semaphore_create_result));
            return;
        }
        semaphore_create_result = vkCreateSemaphore(
                m_vkb_device.device,
                &semaphoreCreateInfo,
                nullptr,
                &m_vulkan_render_semaphore);
        if (semaphore_create_result != VK_SUCCESS)
        {
            m_errors.push_back(VKB_ERROR("Failed to create render semaphore.", semaphore_create_result));
            return;
        }
    }

    // Set initialized to true
    m_initialized = true;
}

vengine::vengine::~vengine()
{
    if (!m_shader_modules.empty())
    {
        for (auto it: m_shader_modules)
        {
            if (!it)
            {
                continue;
            }
            vkDestroyShaderModule(m_vkb_device.device, it, nullptr);
        }
        m_frame_buffers.clear();
    }
    if (m_vulkan_render_fence)
    {
        vkDestroyFence(m_vkb_device.device, m_vulkan_render_fence, nullptr);
        m_vulkan_render_fence = nullptr;
    }
    if (m_vulkan_present_semaphore)
    {
        vkDestroySemaphore(m_vkb_device.device, m_vulkan_present_semaphore, nullptr);
        m_vulkan_present_semaphore = nullptr;
    }
    if (m_vulkan_render_semaphore)
    {
        vkDestroySemaphore(m_vkb_device.device, m_vulkan_render_semaphore, nullptr);
        m_vulkan_render_semaphore = nullptr;
    }
    if (!m_frame_buffers.empty())
    {
        for (auto it: m_frame_buffers)
        {
            if (!it)
            {
                continue;
            }
            vkDestroyFramebuffer(m_vkb_device.device, it, nullptr);
        }
        m_frame_buffers.clear();
    }
    if (m_vulkan_render_pass)
    {
        vkDestroyRenderPass(m_vkb_device.device, m_vulkan_render_pass, nullptr);
        m_vulkan_render_pass = nullptr;
    }
    if (!m_command_buffers.empty())
    {
        vkFreeCommandBuffers(
                m_vkb_device.device,
                m_vulkan_command_pool,
                m_command_buffers.size(),
                m_command_buffers.data());
        m_command_buffers.clear();
    }
    if (m_vulkan_command_pool)
    {
        vkDestroyCommandPool(m_vkb_device.device, m_vulkan_command_pool, nullptr);
        m_vulkan_command_pool = nullptr;
    }
    for (auto it: m_swap_chain_image_views)
    {
        vkDestroyImageView(m_vkb_device.device, it, nullptr);
    }
    m_swap_chain_image_views.clear();
    // for (auto it : m_swap_chain_images)
    // {
    //     vkDestroyImage(m_vkb_device.device, it, nullptr);
    // }
    m_swap_chain_images.clear();
    if (m_vkb_swap_chain.swapchain)
    {
        vkb::destroy_swapchain(m_vkb_swap_chain);
    }
    if (m_vkb_device.device)
    {
        vkb::destroy_device(m_vkb_device);
    }
    if (m_vulkan_surface)
    {
        vkb::destroy_surface(m_vkb_instance, m_vulkan_surface);
    }
    if (m_vkb_instance.instance)
    {
        vkb::destroy_instance(m_vkb_instance);
    }

    glfw_window_destroy();
}

std::optional<VkCommandBuffer> vengine::vengine::create_command_buffer()
{
    if (!good())
    {
        return { };
    }

    VkCommandBufferAllocateInfo cmdAllocInfo = { };
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext = nullptr;

    cmdAllocInfo.commandPool = m_vulkan_command_pool;
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer vk_command_buffer;
    auto command_buffer_result = vkAllocateCommandBuffers(m_vkb_device.device, &cmdAllocInfo, &vk_command_buffer);
    if (command_buffer_result != VK_SUCCESS)
    {
        m_errors.push_back(VKB_ERROR("Failed to create command buffer.", command_buffer_result));
        return { };
    }
    m_command_buffers.push_back(vk_command_buffer);
    return vk_command_buffer;
}

void vengine::vengine::destroy_command_buffer(VkCommandBuffer buffer)
{
    auto it = std::find(m_command_buffers.begin(), m_command_buffers.end(), buffer);
    m_command_buffers.erase(it);
    vkFreeCommandBuffers(m_vkb_device.device, m_vulkan_command_pool, 1, &buffer);
}


void vengine::vengine::render()
{
    const size_t one_second_in_nano_seconds = 1'000'0000'000;
    // Wait for render fence
    {
        VkResult wait_for_fence_result;
        do
        {
            wait_for_fence_result = vkWaitForFences(
                    m_vkb_device.device, 1, &m_vulkan_render_fence, true, one_second_in_nano_seconds);
            if (wait_for_fence_result != VK_SUCCESS && wait_for_fence_result != VK_TIMEOUT)
            {
                m_errors.push_back(VKB_ERROR("Failed to wait for render fence.", wait_for_fence_result));
                return;
            }
        }
        while (wait_for_fence_result == VK_TIMEOUT);
        auto reset_fences_result = vkResetFences(m_vkb_device.device, 1, &m_vulkan_render_fence);
        if (reset_fences_result != VK_SUCCESS)
        {
            m_errors.push_back(VKB_ERROR("Failed to reset render fence.", reset_fences_result));
            return;
        }
    }

    // Acquire next swap chain image index
    uint32_t swap_chain_image_index;
    {
        auto acquire_next_image_result = vkAcquireNextImageKHR(
                m_vkb_device.device,
                m_vkb_swap_chain.swapchain,
                one_second_in_nano_seconds,
                m_vulkan_present_semaphore,
                nullptr,
                &swap_chain_image_index);
        if (acquire_next_image_result != VK_SUCCESS)
        {
            m_errors.push_back(VKB_ERROR("Failed to receive next swap chain image.", acquire_next_image_result));
            return;
        }
    }

    // Reset command buffers
    for (auto command_buffer: m_command_buffers)
    {
        auto reset_command_buffer_result = vkResetCommandBuffer(command_buffer, 0);
        if (reset_command_buffer_result != VK_SUCCESS)
        {
            m_errors.push_back(VKB_ERROR("Failed to reset command buffer.", reset_command_buffer_result));
            return;
        }
    }

    for (auto command_buffer: m_command_buffers)
    {
        // Begin command buffers
        {
            VkCommandBufferBeginInfo command_buffer_begin_info = { };
            command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            command_buffer_begin_info.pNext = nullptr;

            command_buffer_begin_info.pInheritanceInfo = nullptr;
            command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;


            auto command_buffer_begin_result = vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);
            if (command_buffer_begin_result != VK_SUCCESS)
            {
                m_errors.push_back(VKB_ERROR("Failed to begin command buffer.", command_buffer_begin_result));
                return;
            }
        }

        // Begin render pass
        {
            VkClearValue clearValue;
            clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

            VkRenderPassBeginInfo render_pass_begin_info = { };
            render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_begin_info.pNext = nullptr;

            render_pass_begin_info.renderPass = m_vulkan_render_pass;
            render_pass_begin_info.renderArea.offset.x = 0;
            render_pass_begin_info.renderArea.offset.y = 0;
            render_pass_begin_info.renderArea.extent = m_vkb_swap_chain.extent;
            render_pass_begin_info.framebuffer = m_frame_buffers[swap_chain_image_index];

            render_pass_begin_info.clearValueCount = 1;
            render_pass_begin_info.pClearValues = &clearValue;

            vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        }
    }

    // Raise render event
    on_render_pass.raise(this, { });

    for (auto command_buffer: m_command_buffers)
    {
        // End render pass
        {
            vkCmdEndRenderPass(command_buffer);
        }

        // End command buffer
        {
            auto command_buffer_end_result = vkEndCommandBuffer(command_buffer);
            if (command_buffer_end_result != VK_SUCCESS)
            {
                m_errors.push_back(VKB_ERROR("Failed to end command buffer.", command_buffer_end_result));
                return;
            }
        }
    }


    // Submit queue
    {
        VkSubmitInfo submit = { };
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.pNext = nullptr;

        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        submit.pWaitDstStageMask = &waitStage;

        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &m_vulkan_present_semaphore;

        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &m_vulkan_render_semaphore;

        submit.commandBufferCount = m_command_buffers.size();
        submit.pCommandBuffers = m_command_buffers.data();

        auto queue_submit_result = vkQueueSubmit(m_vkb_graphics_queue, 1, &submit, m_vulkan_render_fence);
        if (queue_submit_result != VK_SUCCESS)
        {
            m_errors.push_back(VKB_ERROR("Failed submit render queue.", queue_submit_result));
            return;
        }
    }

    // Present image to screen
    {
        VkPresentInfoKHR presentInfo = { };
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pNext = nullptr;

        presentInfo.pSwapchains = &m_vkb_swap_chain.swapchain;
        presentInfo.swapchainCount = 1;

        presentInfo.pWaitSemaphores = &m_vulkan_render_semaphore;
        presentInfo.waitSemaphoreCount = 1;

        presentInfo.pImageIndices = &swap_chain_image_index;

        auto queue_present_result = vkQueuePresentKHR(m_vkb_graphics_queue, &presentInfo);
        if (queue_present_result != VK_SUCCESS)
        {
            m_errors.push_back(VKB_ERROR("Failed present render queue.", queue_present_result));
            return;
        }

    }
    // Increase frame counter
    m_frame_counter++;
}

void vengine::vengine::destroy_shader_module(VkShaderModule buffer)
{
    auto it = std::find(m_shader_modules.begin(), m_shader_modules.end(), buffer);
    m_shader_modules.erase(it);
    vkDestroyShaderModule(m_vkb_device.device, buffer, nullptr);
}

std::optional<VkShaderModule> vengine::vengine::create_shader_module(const ram_file& file)
{
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    createInfo.codeSize = file.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(file.data());

    //check that the creation goes well.
    VkShaderModule shaderModule;

    auto create_shader_module_result = vkCreateShaderModule(m_vkb_device.device, &createInfo, nullptr, &shaderModule);
    if (create_shader_module_result != VK_SUCCESS)
    {
        m_errors.push_back(VKB_ERROR("Failed to create shader module.", create_shader_module_result));
        return { };
    }
    m_shader_modules.push_back(shaderModule);
    return shaderModule;
}

#pragma region glfw
#define glfw_wnd (static_cast<GLFWwindow*>(m_window_handle))

void vengine::vengine::set_error_callback(void (*error_handle)(int, const char *))
{
    glfwSetErrorCallback(error_handle);
}

void vengine::vengine::glfw_window_destroy()
{
    glfw_unset_window_callbacks();
    glfwDestroyWindow(glfw_wnd);
    m_window_handle = nullptr;
    if (m_glfw_initialized)
    {
        glfwTerminate();
    }
}

void vengine::vengine::glfw_window_init(int width, int height, const std::string &title)
{
    auto glfw_init_result = glfwInit();
    if (!glfw_init_result)
    {
        m_window_handle = nullptr;
        m_glfw_initialized = false;
        return;
    }
    m_glfw_initialized = true;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, false);
    m_window_handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfw_set_window_callbacks();
}

void vengine::vengine::glfw_unset_window_callbacks()
{
    if (!m_window_handle)
    {
        return;
    }
    glfwSetWindowCloseCallback(glfw_wnd, nullptr);
    glfwSetWindowFocusCallback(glfw_wnd, nullptr);
    glfwSetWindowIconifyCallback(glfw_wnd, nullptr);
    glfwSetWindowMaximizeCallback(glfw_wnd, nullptr);
    glfwSetWindowPosCallback(glfw_wnd, nullptr);
    glfwSetWindowRefreshCallback(glfw_wnd, nullptr);
    glfwSetWindowSizeCallback(glfw_wnd, nullptr);
    glfwSetWindowContentScaleCallback(glfw_wnd, nullptr);
    glfwSetWindowUserPointer(glfw_wnd, nullptr);
}

void vengine::vengine::glfw_set_window_callbacks()
{
    if (!m_window_handle)
    {
        return;
    }
    glfwSetWindowCloseCallback(
            glfw_wnd, [](GLFWwindow *window)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<vengine *>(user_pointer);
                    instance->on_window_close.raise(*instance, { });
                }
            });
    glfwSetWindowFocusCallback(
            glfw_wnd, [](GLFWwindow *window, int focused)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<vengine *>(user_pointer);
                    instance->on_window_focus.raise(*instance, { static_cast<bool>(focused) });
                }
            });
    glfwSetWindowIconifyCallback(
            glfw_wnd, [](GLFWwindow *window, int iconified)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<vengine *>(user_pointer);
                    instance->on_window_iconified.raise(*instance, { static_cast<bool>(iconified) });
                }
            });
    glfwSetWindowMaximizeCallback(
            glfw_wnd, [](GLFWwindow *window, int maximized)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<vengine *>(user_pointer);
                    instance->on_window_maximize.raise(*instance, { static_cast<bool>(maximized) });
                }
            });
    glfwSetWindowPosCallback(
            glfw_wnd, [](GLFWwindow *window, int x_pos, int y_pos)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<vengine *>(user_pointer);
                    instance->on_window_pos.raise(*instance, { x_pos, y_pos });
                }
            });
    glfwSetWindowRefreshCallback(
            glfw_wnd, [](GLFWwindow *window)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<vengine *>(user_pointer);
                    instance->on_window_refresh.raise(*instance, { });
                }
            });
    glfwSetWindowSizeCallback(
            glfw_wnd, [](GLFWwindow *window, int width, int height)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<vengine *>(user_pointer);
                    instance->on_window_size.raise(*instance, { width, height });
                }
            });
    glfwSetWindowContentScaleCallback(
            glfw_wnd, [](GLFWwindow *window, float x_scale, float y_scale)
            {
                auto user_pointer = glfwGetWindowUserPointer(window);
                if (user_pointer)
                {
                    auto instance = reinterpret_cast<vengine *>(user_pointer);
                    instance->on_window_content_scale.raise(*instance, { x_scale, y_scale });
                }
            });
    glfwSetWindowUserPointer(glfw_wnd, this);
}

vengine::vengine::size vengine::vengine::window_size() const
{
    vengine::vengine::size size { 0 };
    glfwGetWindowSize(glfw_wnd, &size.width, &size.height);
    return size;
}

void vengine::vengine::window_size(const vengine::vengine::size &size)
{
    glfwSetWindowSize(glfw_wnd, size.width, size.height);
}

void vengine::vengine::window_title(const std::string &title)
{
    glfwSetWindowTitle(glfw_wnd, title.c_str());
}

void vengine::vengine::handle_pending_events()
{
    glfwPollEvents();
}

void vengine::vengine::swap_buffers()
{
    glfwSwapBuffers(glfw_wnd);
}

#pragma endregion
