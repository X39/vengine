//
// Created by marco.silipo on 12.09.2021.
//

#ifndef GAME_PROJ_write_descriptor_set_updater_HPP
#define GAME_PROJ_write_descriptor_set_updater_HPP

#include "result.hpp"
#include "../log.hpp"
#include "stringify.hpp"

#include <vulkan/vulkan.h>
#include <vector>

namespace vengine::vulkan_utils
{
    class descriptor_set_updater
    {
    public:
        class write_descriptor_set_builder
        {
            VkDescriptorSet m_descriptor_set;
            std::vector<VkDescriptorBufferInfo> m_descriptor_buffer_infos;
            VkDescriptorType m_descriptor_type;
            descriptor_set_updater& m_ref;
            friend class descriptor_set_updater;
            explicit write_descriptor_set_builder(descriptor_set_updater& ref, VkDescriptorSet descriptor_set)
                    : m_ref(ref), m_descriptor_set(descriptor_set), m_descriptor_type(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            {

            }

            result<VkWriteDescriptorSet> get() // NOLINT(readability-convert-member-functions-to-static)
            {
                if (m_descriptor_buffer_infos.size() > UINT32_MAX)
                {
                    auto message = "More descriptor buffer infos have been pushed then vulkan can handle.";
                    log::error("vengine::vulkan_utils::descriptor_set_updater::write_descriptor_set_builder::build()", message);
                    return { message };
                }

                VkWriteDescriptorSet write_descriptor_set = {};
                write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                write_descriptor_set.pNext = nullptr;
                write_descriptor_set.dstBinding = 0;
                write_descriptor_set.dstSet = m_descriptor_set;
                write_descriptor_set.descriptorCount = (uint32_t)m_descriptor_buffer_infos.size();
                write_descriptor_set.descriptorType = m_descriptor_type;
                write_descriptor_set.pBufferInfo = m_descriptor_buffer_infos.data();
                return { write_descriptor_set };
            }
        public:

            write_descriptor_set_builder& descriptor_set_add_descriptor_buffer_info(VkBuffer buffer, size_t offset, size_t size)
            {
                if (offset > UINT32_MAX)
                {
                    auto message = "Offset is outside of supported range for vulkan.";
                    log::warning("vengine::vulkan_utils::descriptor_set_updater::write_descriptor_set_builder::add_descriptor_buffer_info(VkBuffer, size_t, size_t)", message);
                    offset = UINT32_MAX;
                }
                if (size > UINT32_MAX)
                {
                    auto message = "Size is outside of supported range for vulkan.";
                    log::warning("vengine::vulkan_utils::descriptor_set_updater::write_descriptor_set_builder::add_descriptor_buffer_info(VkBuffer, size_t, size_t)", message);
                    size = UINT32_MAX;
                }
                VkDescriptorBufferInfo descriptor_buffer_info = {};
                descriptor_buffer_info.buffer = buffer;
                descriptor_buffer_info.offset = offset;
                descriptor_buffer_info.range = size;

                m_descriptor_buffer_infos.push_back(descriptor_buffer_info);
                return *this;
            }

            write_descriptor_set_builder& descriptor_set_set_descriptor_type(VkDescriptorType descriptor_type)
            {
                m_descriptor_type = descriptor_type;
                return *this;
            }

            descriptor_set_updater& descriptor_set_build() // NOLINT(readability-convert-member-functions-to-static)
            {
                return m_ref;
            }
        };
    private:
        VkDevice m_device;
        std::vector<write_descriptor_set_builder> m_write_descriptor_set_builders;
    public:
        explicit descriptor_set_updater(VkDevice device)
                : m_device(device)
        {

        }

        write_descriptor_set_builder& add_descriptor_set(VkDescriptorSet descriptor_set)
        {
            m_write_descriptor_set_builders.push_back(write_descriptor_set_builder{ *this, descriptor_set });
            return m_write_descriptor_set_builders.back();
        }


        result<void> update() // NOLINT(readability-convert-member-functions-to-static)
        {
            if (m_write_descriptor_set_builders.size() > UINT32_MAX)
            {
                auto message = "More descriptor sets have been pushed then vulkan can handle.";
                log::error("vengine::vulkan_utils::descriptor_set_updater::build()", message);
                return message;
            }
            std::vector<VkWriteDescriptorSet> write_descriptor_sets;
            for (auto& it : m_write_descriptor_set_builders)
            {
                auto res = it.get();
                if (!res)
                {
                    return { res.vk_result(), res.message() };
                }
                write_descriptor_sets.push_back(res.value());
            }
            vkUpdateDescriptorSets(m_device, (uint32_t)write_descriptor_sets.size(), write_descriptor_sets.data(), 0, nullptr);
            return {};
        }
    };
}

#endif //GAME_PROJ_write_descriptor_set_updater_HPP
