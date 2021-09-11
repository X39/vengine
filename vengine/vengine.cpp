//
// Created by marco.silipo on 31.08.2021.
//
#include "vengine.hpp"
#include "log.hpp"
#include "VkBootstrap.h"
#include "vulkan-utils/stringify.hpp"
#include "vulkan-utils/image_builder.hpp"
#include "vulkan-utils/image_view_builder.hpp"
#include "vulkan-utils/render_pass_builder.hpp"
#include "vulkan-utils/descriptor_set_layout_builder.hpp"
#include "vulkan-utils/buffer_builder.hpp"
#include "vulkan-utils/descriptor_pool_builder.hpp"

#include <GLFW/glfw3.h>

#include <sstream>

using namespace vengine::vulkan_utils;

template<typename T>
inline std::string VKB_ERROR(std::string_view message, T error)
{
    std::stringstream sstream;
    sstream << error.error() << ": " << message;
    return sstream.str();
}

template<typename T>
inline std::string VKB_ERROR(std::string_view message, result<T> error)
{
    std::stringstream sstream;
    sstream << stringify::data(error.vk_result()) << " (" << std::uppercase << std::hex << static_cast<uint64_t>(error.vk_result()) << ") - " << error.message() << ": " << message;
    auto ret_val = sstream.str();
    vengine::log::error("VKB_ERROR<VkResult>(std::string_view, VkResult)", ret_val);
    return ret_val;
}
template<>
inline std::string VKB_ERROR<VkResult>(std::string_view message, VkResult error)
{
    std::stringstream sstream;
    sstream << stringify::data(error) << " (" << std::uppercase << std::hex << static_cast<uint64_t>(error) << "): " << message;
    auto ret_val = sstream.str();
    vengine::log::error("VKB_ERROR<VkResult>(std::string_view, VkResult)", ret_val);
    return ret_val;
}

vengine::vengine::vengine()
        : m_initialized(false),
        m_glfw_initialized(false),
        m_frame_counter(0),
        m_vulkan_surface(nullptr),
        m_vma_allocator(nullptr),
        m_frame_data_index(0)
{
    glfw_window_init(800, 600, "vengine");
    if (!m_glfw_initialized)
    {
        log::error("vengine::vengine::vengine()", "Failed to initialize glfw.");
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
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create vulkan instance.", instance_result));
        return;
    }
    m_vkb_instance = instance_result.value();

    // Create vulkan surface
    auto glfw_surface_creation_result = glfwCreateWindowSurface(
            m_vkb_instance.instance, static_cast<GLFWwindow *>(m_window_handle), nullptr, &m_vulkan_surface);
    if (glfw_surface_creation_result != VK_SUCCESS)
    {
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create vulkan surface using glfw.", instance_result));
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
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to find suitable vulkan physical device.", instance_result));
        return;
    }
    m_vkb_physical_device = physical_device_result.value();


    // Create logical device
    auto device_result = vkb::DeviceBuilder { m_vkb_physical_device }.build();
    if (!device_result)
    {
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create vulkan device.", instance_result));
        return;
    }
    m_vkb_device = device_result.value();

    // Create descriptor pool
    auto descriptor_pool_result = vulkan_utils::descriptor_pool_builder(m_vkb_device.device, 10 * frame_data_structures_count)
            .add_layout_binding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10)
            .build();
    if (!descriptor_pool_result)
    {
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create descriptor pool.", descriptor_pool_result));
        return;
    }
    m_descriptor_pool = descriptor_pool_result.value();

    // Create descriptor set layout
    auto descriptor_set_layout_result = vulkan_utils::descriptor_set_layout_builder(m_vkb_device.device)
            .add_layout_binding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT)
            .build();
    if (!descriptor_set_layout_result)
    {
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create descriptor set layout.", descriptor_pool_result));
        return;
    }
    m_descriptor_set_layout = descriptor_set_layout_result.value();

    // Create swap chain
    auto
            swap_chain_result
            = vkb::SwapchainBuilder { m_vkb_device }.set_desired_format({ .format = VK_FORMAT_B8G8R8A8_SRGB, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
                                                    .build();
    if (!swap_chain_result)
    {
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create vulkan swap chain.", instance_result));
        return;
    }
    m_vkb_swap_chain = swap_chain_result.value();

    // Create allocator
    {
        VmaAllocatorCreateInfo allocator_create_info = {};
        allocator_create_info.physicalDevice = m_vkb_physical_device.physical_device;
        allocator_create_info.device = m_vkb_device.device;
        allocator_create_info.instance = m_vkb_instance.instance;
        auto allocator_create_result = vmaCreateAllocator(&allocator_create_info, &m_vma_allocator);
        if (allocator_create_result != VK_SUCCESS)
        {
            log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create vulkan allocator.", allocator_create_result));
            return;
        }
    }


    // Create Depths image
    m_depths_format = VK_FORMAT_D32_SFLOAT;
    auto depths_image_result = vulkan_utils::image_builder(m_vma_allocator, {m_vkb_swap_chain.extent.width, m_vkb_swap_chain.extent.height, 1})
            .set_memory_usage(VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY)
            .set_image_usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            .set_memory_property_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            .set_format(m_depths_format)
            .build();
    if (!depths_image_result)
    {
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create depths image.", depths_image_result));
        return;
    }
    m_depth_image = depths_image_result.value();

    // Create Depths image view
    auto depths_image_view_result = vulkan_utils::image_view_builder(m_vkb_device.device, m_depth_image.image)
            .set_memory_usage(VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY)
            .set_image_usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
            .set_memory_property_flags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            .set_format(m_depths_format)
            .set_image_aspect(VK_IMAGE_ASPECT_DEPTH_BIT)
            .build();
    if (!depths_image_view_result)
    {
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create depths image view.", depths_image_view_result));
        return;
    }
    m_depths_image_view = depths_image_view_result.value();

    // Get images
    auto swap_chain_images_result = m_vkb_swap_chain.get_images();
    if (!swap_chain_images_result)
    {
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to receive images from swap chain.", instance_result));
        return;
    }
    m_swap_chain_images = swap_chain_images_result.value();

    // Get image views
    auto swap_chain_image_views_result = m_vkb_swap_chain.get_image_views();
    if (!swap_chain_image_views_result)
    {
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to receive image views from swap chain.", instance_result));
        return;
    }
    m_swap_chain_image_views = swap_chain_image_views_result.value();


    // Get Graphics Queue
    auto graphics_queue_result = m_vkb_device.get_queue(vkb::QueueType::graphics);
    if (!graphics_queue_result)
    {
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to receive graphics queue.", instance_result));
        return;
    }
    m_vkb_graphics_queue = graphics_queue_result.value();

    // Get Graphics Queue Index
    auto graphics_queue_index_result = m_vkb_device.get_queue_index(vkb::QueueType::graphics);
    if (!graphics_queue_index_result)
    {
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to receive graphics queue index.", instance_result));
        return;
    }
    m_vkb_graphics_queue_index = graphics_queue_index_result.value();

    // Create render pass
    {

        VkAttachmentReference color_attachment_ref = { };
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attachment_ref = {};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


        VkSubpassDescription sub_pass_description = { };
        sub_pass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub_pass_description.colorAttachmentCount = 1;
        sub_pass_description.pColorAttachments = &color_attachment_ref;
        sub_pass_description.pDepthStencilAttachment = &depth_attachment_ref;

        auto render_pass_create_result = vulkan_utils::render_pass_builder(m_vkb_device.device)
                .add_attachment_description(
                        0,
                        m_vkb_swap_chain.image_format,
                        VK_SAMPLE_COUNT_1_BIT,
                        VK_ATTACHMENT_LOAD_OP_CLEAR,
                        VK_ATTACHMENT_STORE_OP_STORE,
                        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
                .add_attachment_description(
                        0,
                        m_depths_format,
                        VK_SAMPLE_COUNT_1_BIT,
                        VK_ATTACHMENT_LOAD_OP_CLEAR,
                        VK_ATTACHMENT_STORE_OP_STORE,
                        VK_ATTACHMENT_LOAD_OP_CLEAR,
                        VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
                .add_sub_pass_description(sub_pass_description)
                .build();
        if (!render_pass_create_result)
        {
            log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create render pass.", render_pass_create_result));
            return;
        }
        m_vulkan_render_pass = render_pass_create_result.value();
    }

    // Create frame buffers
    {
        VkFramebufferCreateInfo framebuffer_create_info = { };
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.pNext = nullptr;

        framebuffer_create_info.renderPass = m_vulkan_render_pass;
        framebuffer_create_info.width = m_vkb_swap_chain.extent.width;
        framebuffer_create_info.height = m_vkb_swap_chain.extent.height;
        framebuffer_create_info.layers = 1;

        m_frame_buffers = std::vector<VkFramebuffer>(m_swap_chain_image_views.size(), nullptr);
        for (size_t i = 0; i < m_swap_chain_image_views.size(); i++)
        {
            auto attachments = std::array{ m_swap_chain_image_views[i], m_depths_image_view };
            framebuffer_create_info.attachmentCount = (uint32_t)attachments.size();
            framebuffer_create_info.pAttachments = attachments.data();
            auto
                    create_frame_buffer_result = vkCreateFramebuffer(
                    m_vkb_device.device,
                    &framebuffer_create_info,
                    nullptr,
                    &m_frame_buffers[i]);
            if (create_frame_buffer_result != VK_SUCCESS)
            {
                log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create frame buffer.", create_frame_buffer_result));
                return;
            }
        }
    }

    for (size_t i = 0; i < frame_data_structures_count; i++)
    {
        auto& data = m_frame_data_structures.emplace_back();
        // Create command pools
        {
            VkCommandPoolCreateInfo command_pool_create_info = { };
            command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            command_pool_create_info.pNext = nullptr;
            command_pool_create_info.queueFamilyIndex = m_vkb_graphics_queue_index;
            command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

            auto command_pool_result = vkCreateCommandPool(
                    m_vkb_device.device, &command_pool_create_info, nullptr, &data.command_pool);
            if (command_pool_result != VK_SUCCESS)
            {
                log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create command pool.", command_pool_result));
                return;
            }
        }

        // Create render fence
        {
            VkFenceCreateInfo fence_create_info = { };
            fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_create_info.pNext = nullptr;

            //we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            auto fence_create_result = vkCreateFence(
                    m_vkb_device.device, &fence_create_info, nullptr, &data.render_fence);
            if (fence_create_result != VK_SUCCESS)
            {
                log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create render fence.", fence_create_result));
                return;
            }
        }

        // Create present and render semaphore
        {
            VkSemaphoreCreateInfo semaphoreCreateInfo = { };
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semaphoreCreateInfo.pNext = nullptr;
            semaphoreCreateInfo.flags = 0;

            auto semaphore_create_result = vkCreateSemaphore(
                    m_vkb_device.device, &semaphoreCreateInfo, nullptr, &data.present_semaphore);
            if (semaphore_create_result != VK_SUCCESS)
            {
                log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create present semaphore.", semaphore_create_result));
                return;
            }
            semaphore_create_result = vkCreateSemaphore(
                    m_vkb_device.device, &semaphoreCreateInfo, nullptr, &data.render_semaphore);
            if (semaphore_create_result != VK_SUCCESS)
            {
                log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create render semaphore.", semaphore_create_result));
                return;
            }
        }

        auto buffer = create_command_buffer(data);

        auto camera_buffer_result = vulkan_utils::buffer_builder(m_vma_allocator, sizeof(camera_data))
                .set_buffer_usage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
                .set_memory_usage(VMA_MEMORY_USAGE_CPU_TO_GPU)
                .build();
        if (!camera_buffer_result)
        {
            log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create camera buffer.", camera_buffer_result));
            return;
        }
        data.camera_buffer = camera_buffer_result.value();


        // Create descriptor set
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.pNext = nullptr;
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptor_pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_descriptor_set_layout;
        auto descriptor_sets_result = vkAllocateDescriptorSets(m_vkb_device.device, &allocInfo, &data.descriptor_set);
        if (descriptor_sets_result != VK_SUCCESS)
        {
            log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create descriptor set.", descriptor_sets_result));
            return;
        }

        // Bind camera buffer to descriptor set
        VkDescriptorBufferInfo descriptor_buffer_info;
        descriptor_buffer_info.buffer = data.camera_buffer.buffer;
        descriptor_buffer_info.offset = 0;
        descriptor_buffer_info.range = sizeof(camera_data);

        VkWriteDescriptorSet write_descriptor_set = {};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.pNext = nullptr;
        write_descriptor_set.dstBinding = 0;
        write_descriptor_set.dstSet = data.descriptor_set;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_descriptor_set.pBufferInfo = &descriptor_buffer_info;

        vkUpdateDescriptorSets(m_vkb_device.device, 1, &write_descriptor_set, 0, nullptr);

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
    for (auto& data : m_frame_data_structures)
    {
        if (data.camera_buffer.uploaded())
        {
            data.camera_buffer.destroy();
        }
        if (data.render_fence)
        {
            vkDestroyFence(m_vkb_device.device, data.render_fence, nullptr);
        }
        if (data.present_semaphore)
        {
            vkDestroySemaphore(m_vkb_device.device, data.present_semaphore, nullptr);
        }
        if (data.render_semaphore)
        {
            vkDestroySemaphore(m_vkb_device.device, data.render_semaphore, nullptr);
        }
        if (!data.command_buffers.empty())
        {
            if (data.command_buffers.size() > UINT32_MAX)
            {
                log::warning("vengine::vengine::~vengine()", "More command buffers have been created then the vulkan api supports. Cannot destroy all.");
            }
            vkFreeCommandBuffers(
                    m_vkb_device.device,
                    data.command_pool,
                    (uint32_t)data.command_buffers.size(),
                    data.command_buffers.data());
        }
        if (data.command_pool)
        {
            vkDestroyCommandPool(m_vkb_device.device, data.command_pool, nullptr);
        }
    }
    m_frame_data_structures.clear();
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


    if (m_depths_image_view)
    {
        vkDestroyImageView(m_vkb_device.device, m_depths_image_view, nullptr);
        m_depths_image_view = nullptr;
    }
    if (m_depth_image.uploaded())
    {
        m_depth_image.destroy();
    }
    if (m_vkb_swap_chain.swapchain)
    {
        vkb::destroy_swapchain(m_vkb_swap_chain);
    }
    if (m_vma_allocator)
    {
        vmaDestroyAllocator(m_vma_allocator);
        m_vma_allocator = nullptr;
    }
    if (m_descriptor_set_layout)
    {
        vkDestroyDescriptorSetLayout(m_vkb_device.device, m_descriptor_set_layout, nullptr);
        m_descriptor_set_layout = nullptr;
    }
    if (m_descriptor_pool)
    {
        vkDestroyDescriptorPool(m_vkb_device.device, m_descriptor_pool, nullptr);
        m_descriptor_pool = nullptr;
    }
    if (m_vkb_device.device)
    {
        vkb::destroy_device(m_vkb_device);
    }
    if (m_vulkan_surface)
    {
        vkb::destroy_surface(m_vkb_instance, m_vulkan_surface);
        m_vulkan_surface = nullptr;
    }
    if (m_vkb_instance.instance)
    {
        vkb::destroy_instance(m_vkb_instance);
    }

    glfw_window_destroy();
}

std::optional<VkCommandBuffer> vengine::vengine::create_command_buffer(frame_data& frame)
{
    VkCommandBufferAllocateInfo cmdAllocInfo = { };
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext = nullptr;

    cmdAllocInfo.commandPool = frame.command_pool;
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer vk_command_buffer;
    auto command_buffer_result = vkAllocateCommandBuffers(m_vkb_device.device, &cmdAllocInfo, &vk_command_buffer);
    if (command_buffer_result != VK_SUCCESS)
    {
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create command buffer.", command_buffer_result));
        return { };
    }
    frame.command_buffers.push_back(vk_command_buffer);
    return vk_command_buffer;
}

void vengine::vengine::destroy_command_buffer(frame_data& data, VkCommandBuffer buffer)
{
    auto it = std::find(data.command_buffers.begin(), data.command_buffers.end(), buffer);
    data.command_buffers.erase(it);
    vkFreeCommandBuffers(m_vkb_device.device, data.command_pool, 1, &buffer);
}


void vengine::vengine::render()
{
    const size_t one_second_in_nano_seconds = 1'000'0000'000;
    // Wait for render fence

    auto& data = current_frame_data();
    {
        VkResult wait_for_fence_result;
        do
        {
            wait_for_fence_result = vkWaitForFences(
                    m_vkb_device.device, 1, &data.render_fence, true, one_second_in_nano_seconds);
            if (wait_for_fence_result != VK_SUCCESS && wait_for_fence_result != VK_TIMEOUT)
            {
                log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to wait for render fence.", wait_for_fence_result));
                return;
            }
        }
        while (wait_for_fence_result == VK_TIMEOUT);
        auto reset_fences_result = vkResetFences(m_vkb_device.device, 1, &data.render_fence);
        if (reset_fences_result != VK_SUCCESS)
        {
            log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to reset render fence.", reset_fences_result));
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
                data.present_semaphore,
                nullptr,
                &swap_chain_image_index);
        if (acquire_next_image_result != VK_SUCCESS)
        {
            log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to receive next swap chain image.", acquire_next_image_result));
            return;
        }
    }


    // Reset command buffers
    for (auto command_buffer: data.command_buffers)
    {
        auto reset_command_buffer_result = vkResetCommandBuffer(command_buffer, 0);
        if (reset_command_buffer_result != VK_SUCCESS)
        {
            log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to reset command buffer.", reset_command_buffer_result));
            return;
        }
    }

    for (auto command_buffer: data.command_buffers)
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
                log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to begin command buffer.", command_buffer_begin_result));
                return;
            }
        }

        // Begin render pass
        {
            VkClearValue color_clear_value = {};
            color_clear_value.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

            VkClearValue depth_clear_value = {};
            depth_clear_value.depthStencil.depth = 1.0f;

            auto clear_values = std::array{color_clear_value, depth_clear_value};


            VkRenderPassBeginInfo render_pass_begin_info = { };
            render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_begin_info.pNext = nullptr;

            render_pass_begin_info.renderPass = m_vulkan_render_pass;
            render_pass_begin_info.renderArea.offset.x = 0;
            render_pass_begin_info.renderArea.offset.y = 0;
            render_pass_begin_info.renderArea.extent = m_vkb_swap_chain.extent;
            render_pass_begin_info.framebuffer = m_frame_buffers[swap_chain_image_index];
            render_pass_begin_info.clearValueCount = (uint32_t)clear_values.size();
            render_pass_begin_info.pClearValues = clear_values.data();

            vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
        }
    }

    // Raise render event
    on_render_pass.raise(this, { data, data.command_buffers.front() });

    for (auto command_buffer: data.command_buffers)
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
                log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to end command buffer.", command_buffer_end_result));
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
        submit.pWaitSemaphores = &data.present_semaphore;

        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &data.render_semaphore;

        if (data.command_buffers.size() > UINT32_MAX)
        {
            log::warning("vengine::vengine::~vengine()", "More command buffers have been created then the vulkan api supports. Cannot submit all.");
        }
        submit.commandBufferCount = (uint32_t)data.command_buffers.size();
        submit.pCommandBuffers = data.command_buffers.data();

        auto queue_submit_result = vkQueueSubmit(m_vkb_graphics_queue, 1, &submit, data.render_fence);
        if (queue_submit_result != VK_SUCCESS)
        {
            log::error("vengine::vengine::vengine()", VKB_ERROR("Failed submit render queue.", queue_submit_result));
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

        presentInfo.pWaitSemaphores = &data.render_semaphore;
        presentInfo.waitSemaphoreCount = 1;

        presentInfo.pImageIndices = &swap_chain_image_index;

        auto queue_present_result = vkQueuePresentKHR(m_vkb_graphics_queue, &presentInfo);
        if (queue_present_result != VK_SUCCESS)
        {
            log::error("vengine::vengine::vengine()", VKB_ERROR("Failed present render queue.", queue_present_result));
            return;
        }

    }
    // Increase frame counter
    m_frame_counter++;
    m_frame_data_index = m_frame_data_index + 1 >= frame_data_structures_count ? 0 : m_frame_data_index + 1;
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
        log::error("vengine::vengine::vengine()", VKB_ERROR("Failed to create shader module.", create_shader_module_result));
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
