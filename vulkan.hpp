#pragma once

#include "version.hpp"
#include "severity.hpp"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <optional>
#include <map>
#include <set>

namespace x39
{

    struct queue_family
    {

        struct swap_chain_support_details
        {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> present_modes;
        };
        // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkQueueFlagBits.html
        enum queue_family_flags
        {
            // No flags set.
            empty [[maybe_unused]] = 0,
            // Specifies that queues in this queue family support graphics operations
            queue_graphics = 0x00000001,
            // Specifies that queues in this queue family support compute operations.
            queue_compute = 0x00000002,
            // Specifies that queues in this queue family support transfer operations.
            queue_transfer = 0x00000004,
            // Specifies that queues in this queue family support sparse
            // memory management operations (see Sparse Resources).
            // If queue_all of the sparse resource features are enabled,
            // then at least one queue family must support this bit.
            queue_sparse_binding = 0x00000008,
            // Specifies that queues in this queue family support the
            // VK_DEVICE_QUEUE_CREATE_PROTECTED_BIT bit. (see Protected Memory).
            // If the physical device supports the protectedMemory feature,
            // at least one of its queue families must support this bit.
            queue_protected = 0x00000010,
            queue_all_vulkan = queue_graphics | queue_compute | queue_transfer | queue_sparse_binding | queue_protected,
            // Specifies that queues in this queue family support the
            // vkGetPhysicalDeviceSurfaceSupportKHR for the surface field present inside of this.
            queue_present = 0x10000000,

            queue_swap_chain_has_formats = 0x20000000,

            queue_swap_chain_has_present_modes = 0x40000000,


            queue_all_surface = queue_present | queue_swap_chain_has_formats | queue_swap_chain_has_present_modes,
            queue_all = queue_all_vulkan | queue_all_surface,


        };
        queue_family_flags flags;
        uint32_t index;
        [[maybe_unused]] VkSurfaceKHR surface;
        [[maybe_unused]] swap_chain_support_details swap_chain_support;

        [[maybe_unused]] [[nodiscard]] bool supports_graphics() const
        {
            return (flags & queue_graphics) == queue_graphics;
        }

        [[maybe_unused]] [[nodiscard]] bool supports_compute() const
        {
            return (flags & queue_compute) == queue_compute;
        }

        [[maybe_unused]] [[nodiscard]] bool supports_transfer() const
        {
            return (flags & queue_transfer) == queue_transfer;
        }

        [[maybe_unused]] [[nodiscard]] bool supports_sparse_binding() const
        {
            return (flags & queue_sparse_binding) == queue_sparse_binding;
        }

        [[maybe_unused]] [[nodiscard]] bool supports_protected() const
        {
            return (flags & queue_protected) == queue_protected;
        }

        [[maybe_unused]] [[nodiscard]] bool supports_present() const
        {
            return (flags & queue_present) == queue_present;
        }
    };

    inline void operator|=(queue_family::queue_family_flags &l, queue_family::queue_family_flags r)
    {
        l = static_cast<queue_family::queue_family_flags>(static_cast<int>(l) | static_cast<int>(r));
    }

    inline void operator&=(queue_family::queue_family_flags &l, queue_family::queue_family_flags r)
    {
        l = static_cast<queue_family::queue_family_flags>(static_cast<int>(l) & static_cast<int>(r));
    }

    inline queue_family::queue_family_flags
    operator|(queue_family::queue_family_flags l, queue_family::queue_family_flags r)
    {
        return static_cast<queue_family::queue_family_flags>(static_cast<int>(l) | static_cast<int>(r));
    }

    inline queue_family::queue_family_flags
    operator&(queue_family::queue_family_flags l, queue_family::queue_family_flags r)
    {
        return static_cast<queue_family::queue_family_flags>(static_cast<int>(l) & static_cast<int>(r));
    }

    class vulkan
    {
    public:
        struct stage_error
        {
            VkResult result;
            std::string info;
        };

        enum class debug_message_type
        {
            general [[maybe_unused]] = 0b00000001,
            validation [[maybe_unused]] = 0b00000010,
            performance [[maybe_unused]] = 0b00000100
        };
#pragma region Vulkan EXT Loaders

        static VkResult
        CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                     const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
        {
            static auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
                    instance, "vkCreateDebugUtilsMessengerEXT");
            if (func != nullptr)
            {
                return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
            }
            else
            {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        }

        static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                  const VkAllocationCallbacks *pAllocator)
        {
            static auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
                    instance, "vkDestroyDebugUtilsMessengerEXT");
            if (func != nullptr)
            {
                func(instance, debugMessenger, pAllocator);
            }
        }

#pragma endregion
    private:
        std::vector<stage_error> m_errors;
        GLFWwindow *m_window_handle;
        VkInstance m_instance;
        VkDebugUtilsMessengerEXT m_debug_messenger;
        VkPhysicalDevice m_physical_device;
        VkDevice m_logical_device;
        VkQueue m_graphics_queue;
        VkQueue m_present_queue;
        VkSurfaceKHR m_surface;
        bool m_initialized;
        VkSwapchainKHR m_swap_chain;
        std::vector<VkImage> m_swap_chain_images;
        VkFormat m_swap_chain_format;
        VkExtent2D m_swap_chain_extent;
    protected:
        [[nodiscard]] virtual std::vector<std::string> get_expected_instance_extensions() const
        {
            std::vector<std::string> output;
            uint32_t glfwExtensionCount = 0;
            const char **glfwExtensions;

            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            for (uint32_t i = 0; i < glfwExtensionCount; i++)
            {
                output.emplace_back(glfwExtensions[i]);
            }

            return output;
        }

        [[nodiscard]] virtual std::vector<std::string> get_expected_device_extensions() const
        {
            return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        }

        [[nodiscard]] virtual std::vector<std::string> get_expected_validation_layers() const
        {
            return { };
        }

        using debug_callback_data = const VkDebugUtilsMessengerCallbackDataEXT;

        virtual void on_debug_message(severity severity, debug_message_type type, debug_callback_data *data) = 0;

    private:

#pragma region Instance

        void create_instance(const std::string &appName, const version &appVersion, const std::string &engName,
                             const version &engVersion)
        {
            // Create version info
            VkApplicationInfo appInfo { };
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = appName.c_str();
            appInfo.applicationVersion = VK_MAKE_VERSION(appVersion.major, appVersion.minor, appVersion.patch);
            appInfo.pEngineName = engName.c_str();
            appInfo.engineVersion = VK_MAKE_VERSION(engVersion.major, engVersion.minor, engVersion.patch);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            // Create create info
            VkInstanceCreateInfo instance_create_info { };
            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo { };


#pragma  region Receive and set validation layers
            auto expected_validation_layers = get_expected_validation_layers();
            if (expected_validation_layers.size() > UINT32_MAX)
            {
                m_errors.push_back(
                        {
                                VK_ERROR_UNKNOWN, "Number of expected validation layers exceeds UINT32_MAX." });
                return;
            }
            // Ensure Expected validation layer support
            if (!expected_validation_layers.empty())
            {
                populate(debugCreateInfo);
                instance_create_info.pNext = &debugCreateInfo;
            }

            std::vector<const char *> expected_validation_layers_transformed;
            expected_validation_layers_transformed.reserve((uint32_t) expected_validation_layers.size());
            for (auto &it : expected_validation_layers)
            {
                expected_validation_layers_transformed.push_back(it.c_str());
            }

            instance_create_info.enabledLayerCount = (uint32_t) expected_validation_layers_transformed.size();
            instance_create_info.ppEnabledLayerNames = expected_validation_layers_transformed.data();
#pragma  endregion
#pragma  region Receive and set extensions
            auto expected_instance_extensions = get_expected_instance_extensions();
            if (!expected_validation_layers.empty())
            { // Add Message Callback Extension if queue_all validation layer is active
                expected_instance_extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            }
            if (expected_instance_extensions.size() > UINT32_MAX)
            {
                m_errors.push_back({ VK_ERROR_UNKNOWN, "Number of expected extensions exceeds UINT32_MAX." });
                return;
            }
            std::vector<const char *> expected_instance_extensions_transformed;
            expected_instance_extensions_transformed.reserve((uint32_t) expected_instance_extensions.size());
            for (auto &it : expected_instance_extensions)
            {
                expected_instance_extensions_transformed.push_back(it.c_str());
            }

            instance_create_info.enabledExtensionCount = (uint32_t) expected_instance_extensions_transformed.size();
            instance_create_info.ppEnabledExtensionNames = expected_instance_extensions_transformed.data();
#pragma  endregion
            instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            instance_create_info.pApplicationInfo = &appInfo;

            // Create the actual vulkan instance
            auto result = vkCreateInstance(&instance_create_info, nullptr, &m_instance);
            if (result != VkResult::VK_SUCCESS)
            {
                m_errors.push_back({ result, "Failed to create instance" });
                return;
            }
        }

        void destroy_instance()
        {
            if (m_instance)
            {
                vkDestroyInstance(m_instance, nullptr);
                m_instance = nullptr;
            }
        }

#pragma endregion
#pragma region Debug Callback

        void setup_debug_callback()
        {
            auto expected_validation_layers = get_expected_validation_layers();
            if (expected_validation_layers.empty())
            {
                return;
            }

            VkDebugUtilsMessengerCreateInfoEXT createInfo { };
            populate(createInfo);

            auto result = CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debug_messenger);
            if (result != VK_SUCCESS)
            {
                m_errors.push_back({ result, "Failed to setup debug callback" });
            }
        }

        void populate(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
        {
            createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT;
            createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT;
            createInfo.pfnUserCallback = debug_callback;
            createInfo.pUserData = this; // Optional
        }

        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                             VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                             const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                             void *pUserData)
        {

            // Get "this pointer"
            auto vul = static_cast<vulkan *>(pUserData);

            // Transform severity into "known enum"
            severity sev;
            switch (messageSeverity)
            {
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:sev = severity::verbose;
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:sev = severity::info;
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:sev = severity::warning;
                    break;
                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: sev = severity::error;
                    break;

                case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT:
                default:sev = severity::na;
                    break;
            }

            // Transform the type info from the message
            auto type = static_cast<debug_message_type>(messageType);

            vul->on_debug_message(sev, type, pCallbackData);
            return VK_FALSE;
        }

        void destroy_debug_callback()
        {
            auto expected_validation_layers = get_expected_validation_layers();
            if (expected_validation_layers.empty())
            {
                return;
            }

            DestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);
        }

#pragma endregion
#pragma region Pick Physical Device


        virtual size_t score_gpu(VkPhysicalDevice device) const
        {
            size_t score = 0;
            VkPhysicalDeviceProperties device_properties;
            VkPhysicalDeviceFeatures device_features;
            vkGetPhysicalDeviceProperties(device, &device_properties);
            vkGetPhysicalDeviceFeatures(device, &device_features);

            // We only rate GPU here
            if (device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
                && device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
                && device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
            {
                return 0;
            }

            if (!check_physical_device_supports_required_extensions(device))
            {
                return 0;
            }

            // We require Geometry Shaders
            if (!device_features.geometryShader)
            {
                return 0;
            }

            if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            { // ALWAYS favor discrete GPUs over anything else
                score += 1000000000;
            }

            // Maximum possible size of textures affects graphics quality
            score += device_properties.limits.maxImageDimension2D;


            return score;
        }

        bool check_physical_device_supports_required_extensions(VkPhysicalDevice device) const
        {
            uint32_t extension_count;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

            std::vector<VkExtensionProperties> available_extensions(extension_count);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

            auto device_extensions = get_expected_device_extensions();
            std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

            for (const auto &extension : available_extensions)
            {
                required_extensions.erase(extension.extensionName);
            }

            return required_extensions.empty();
        }

        void pick_physical_device()
        {
            auto devices = get_physical_devices();

            // Pick GPU
            {
                // Create sorted map with score
                std::multimap<size_t, VkPhysicalDevice> candidates;
                for (const auto &device : devices)
                {
                    auto score = score_gpu(device);
                    candidates.insert(std::make_pair(score, device));
                }

                // Check if the best candidate is suitable at all
                if (candidates.rbegin()->first > 0)
                {
                    m_physical_device = candidates.rbegin()->second;
                }
                else
                {
                    m_errors.push_back(
                            {
                                    VkResult::VK_ERROR_FEATURE_NOT_PRESENT, "Failed to find device that is usable" });
                    return;
                }
            }
        }

#pragma endregion
#pragma region Surface

        void create_surface()
        {
            auto result = glfwCreateWindowSurface(m_instance, m_window_handle, nullptr, &m_surface);
            if (result != VK_SUCCESS)
            {
                m_errors.push_back({ result, "Failed to create surface device" });
            }
        }

        void destroy_surface()
        {
            if (m_surface)
            {
                vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
                m_surface = nullptr;
            }
        }

#pragma endregion
#pragma region Create Logical Device

        static std::vector<queue_family>
        find_queue_families(VkPhysicalDevice device, queue_family::queue_family_flags family = queue_family::queue_all,
                            VkSurfaceKHR surface = nullptr)
        {
            uint32_t queue_family_count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

            std::vector<VkQueueFamilyProperties> queried_queue_families(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queried_queue_families.data());

            std::vector<queue_family> queue_families_result(queue_family_count);
            uint32_t i = 0;
            for (const auto &queueFamily : queried_queue_families)
            {
                if ((queueFamily.queueFlags & family) == family)
                {
                    queue_family qf { static_cast<queue_family::queue_family_flags>(queueFamily.queueFlags), i++ };
                    if (surface)
                    {
                        VkBool32 present_support = false;
                        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
                        if (present_support)
                        {
                            qf.flags |= queue_family::queue_present;
                        }

                        qf.swap_chain_support = get_swap_chain_support(device, surface);
                        if (!qf.swap_chain_support.formats.empty())
                        {
                            qf.flags |= queue_family::queue_swap_chain_has_formats;
                        }
                        if (!qf.swap_chain_support.present_modes.empty())
                        {
                            qf.flags |= queue_family::queue_swap_chain_has_present_modes;
                        }
                    }
                    else if ((family & queue_family::queue_present) == queue_family::queue_present)
                    {
                        /*
                         * If no surface is present and queue_present is requested, we cannot check.
                         * Hence no support.
                         */
                        continue;
                    }
                    if ((qf.flags & family) == family)
                    {
                        queue_families_result.push_back(qf);
                    }
                }
            }

            return queue_families_result;
        }

        void create_logical_device()
        {
            // Create queue create infos
            std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
            auto graphics_queue = find_queue_families(m_physical_device, queue_family::queue_graphics).front();
            auto present_queue = find_queue_families(m_physical_device, queue_family::queue_all_surface, m_surface).front();
            std::vector<queue_family> families = { graphics_queue, present_queue };
            std::map<uint32_t, uint32_t> queue_index_to_device_queue_map;
            VkPhysicalDeviceFeatures deviceFeatures { };
            VkDeviceCreateInfo device_create_info { };

            // Set queue create infos
            for (auto &family : families)
            {
                if (queue_index_to_device_queue_map.find(family.index) == queue_index_to_device_queue_map.end())
                {
                    VkDeviceQueueCreateInfo queue_create_info { };
                    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queue_create_info.queueFamilyIndex = family.index;
                    queue_create_info.queueCount = 1;

                    float queuePriority = 1.0f;
                    queue_create_info.pQueuePriorities = &queuePriority;

                    queue_index_to_device_queue_map[family.index]
                            = (uint32_t) queue_create_infos.size(); // Safe cast due to check later
                    queue_create_infos.push_back(queue_create_info);
                }
            }
            if (queue_create_infos.size() > UINT32_MAX)
            {
                m_errors.push_back({ VK_ERROR_UNKNOWN, "Number of VkDeviceQueueCreateInfos exceeds UINT32_MAX." });
                return;
            }
            device_create_info.queueCreateInfoCount = (uint32_t) queue_create_infos.size();
            device_create_info.pQueueCreateInfos = queue_create_infos.data();


            // Set validation layers
            auto validation_layers = get_expected_validation_layers();
            if (validation_layers.size() > UINT32_MAX)
            {
                m_errors.push_back({ VK_ERROR_UNKNOWN, "Number of validation layers exceeds UINT32_MAX." });
            }
            std::vector<const char *> validation_layers_transformed;
            validation_layers_transformed.reserve(validation_layers.size());
            for (const auto &it : validation_layers)
            {
                validation_layers_transformed.push_back(it.c_str());
            }
            device_create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            device_create_info.ppEnabledLayerNames = validation_layers_transformed.data();



            // Set device extensions
            auto device_extensions = get_expected_device_extensions();
            if (device_extensions.size() > UINT32_MAX)
            {
                m_errors.push_back({ VK_ERROR_UNKNOWN, "Number of device extensions exceeds UINT32_MAX." });
            }
            std::vector<const char *> device_extensions_transformed;
            device_extensions_transformed.reserve((uint32_t) device_extensions.size());
            for (auto &it : device_extensions)
            {
                device_extensions_transformed.push_back(it.c_str());
            }
            device_create_info.enabledExtensionCount = (uint32_t) device_extensions_transformed.size();
            device_create_info.ppEnabledExtensionNames = device_extensions_transformed.data();



            device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            device_create_info.pEnabledFeatures = &deviceFeatures;

            auto result = vkCreateDevice(m_physical_device, &device_create_info, nullptr, &m_logical_device);
            if (result != VK_SUCCESS)
            {
                m_errors.push_back({ result, "Failed to create logical device" });
                return;
            }

            vkGetDeviceQueue(
                    m_logical_device,
                    graphics_queue.index,
                    queue_index_to_device_queue_map[graphics_queue.index],
                    &m_graphics_queue);
            vkGetDeviceQueue(
                    m_logical_device,
                    present_queue.index,
                    queue_index_to_device_queue_map[present_queue.index],
                    &m_present_queue);
        }

        void destroy_logical_device()
        {
            if (m_logical_device)
            {
                vkDestroyDevice(m_logical_device, nullptr);
                m_logical_device = nullptr;
            }
        }

#pragma endregion
#pragma region swap-chain
        static VkSurfaceFormatKHR pick_swap_chain_surface_format(const std::vector<VkSurfaceFormatKHR>& formats)
        {
            for (auto& format : formats)
            {
                if (format.format == VK_FORMAT_B8G8R8A8_SRGB
                    && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    return format;
                }
            }
            return formats.front();
        }
        static VkPresentModeKHR pick_swap_chain_present_mode(const std::vector<VkPresentModeKHR>& present_modes)
        {
            for (auto& present_mode : present_modes)
            {
                if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
                {
                    return present_mode;
                }
            }

            return present_modes.front();
        }

        static VkExtent2D get_swap_chain_extend(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities)
        {
            if (capabilities.currentExtent.width != UINT32_MAX)
            {
                return capabilities.currentExtent;
            }
            else
            {
                int width;
                int height;
                glfwGetFramebufferSize(window, &width, &height);

                VkExtent2D actual_extend = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };

                actual_extend.width = std::clamp(actual_extend.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                actual_extend.height = std::clamp(actual_extend.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

                return actual_extend;
            }
        }
        void create_swap_chain()
        {
            auto swap_chain_support = get_swap_chain_support(m_physical_device, m_surface);

            VkSurfaceFormatKHR surface_format = pick_swap_chain_surface_format(swap_chain_support.formats);
            VkPresentModeKHR present_mode = pick_swap_chain_present_mode(swap_chain_support.present_modes);
            VkExtent2D extent = get_swap_chain_extend(m_window_handle, swap_chain_support.capabilities);

            m_swap_chain_format = surface_format.format;
            m_swap_chain_extent = extent;


            uint32_t swap_chain_images = swap_chain_support.capabilities.minImageCount + 1;
            if (swap_chain_support.capabilities.maxImageCount > 0
            && swap_chain_images > swap_chain_support.capabilities.maxImageCount)
            {
                swap_chain_images = swap_chain_support.capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = m_surface;

            createInfo.minImageCount = swap_chain_images;
            createInfo.imageFormat = surface_format.format;
            createInfo.imageColorSpace = surface_format.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            auto graphics_queue = find_queue_families(m_physical_device, queue_family::queue_graphics).front();
            auto present_queue = find_queue_families(m_physical_device, queue_family::queue_present).front();
            uint32_t queue_family_indices[] = { graphics_queue.index, present_queue.index};

            if (graphics_queue.index != present_queue.index)
            {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queue_family_indices;
            }
            else
            {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0; // Optional
                createInfo.pQueueFamilyIndices = nullptr; // Optional
            }

            createInfo.preTransform = swap_chain_support.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = present_mode;
            createInfo.clipped = VK_TRUE;
            createInfo.oldSwapchain = VK_NULL_HANDLE;




            auto result = vkCreateSwapchainKHR(m_logical_device, &createInfo, nullptr, &m_swap_chain);
            if (result != VK_SUCCESS)
            {
                m_errors.push_back({ result, "Failed to create swap chain." });
                return;
            }

            uint32_t image_count;
            vkGetSwapchainImagesKHR(m_logical_device, m_swap_chain, &image_count, nullptr);
            m_swap_chain_images.resize(image_count);
            if (image_count > 0)
            {
                vkGetSwapchainImagesKHR(m_logical_device, m_swap_chain, &image_count, m_swap_chain_images.data());
            }
        }
        void destroy_swap_chain()
        {
            if (m_swap_chain)
            {
                vkDestroySwapchainKHR(m_logical_device, m_swap_chain, nullptr);
                m_swap_chain = nullptr;
            }
        }
#pragma endregion
    public:
        // A call to vulkan_initialize is required prior to using anything!
        explicit vulkan(GLFWwindow *window_handle)
                : m_window_handle(window_handle), m_instance(nullptr), m_debug_messenger(nullptr),
                  m_physical_device(nullptr), m_logical_device(nullptr), m_graphics_queue(nullptr), m_surface(nullptr),
                  m_present_queue(nullptr), m_initialized(false), m_swap_chain(nullptr), m_swap_chain_format(VkFormat{}),
                  m_swap_chain_extent({})
        {
        }

        void vulkan_initialize(const std::string &appName, const version &appVersion, const std::string &engName,
                               const version &engVersion)
        {
            // ! READ CAREFUL !
            // The scheme used by this class is required due to virtual methods not being safe to call during
            // initialization.
            // Hence: IF you ever come up with the idea to merge this method with the constructor, drop it now.
            // It is, simply said, not possible.
            // See https://isocpp.org/wiki/faq/strange-inheritance#calling-virtuals-from-ctors for more info


            m_initialized = true;
            // 1.
            create_instance(appName, appVersion, engName, engVersion);
            if (!good())
            {
                return;
            }

            // 2.
            setup_debug_callback();
            if (!good())
            {
                return;
            }

            // 3. Pick physical (GPU) device
            pick_physical_device();
            if (!good())
            {
                return;
            }

            // 4.
            create_surface();
            if (!good())
            {
                return;
            }

            // 5.
            create_logical_device();
            if (!good())
            {
                return;
            }

            // 6.
            create_swap_chain();
            if (!good())
            {
                return;
            }
        }

        virtual ~vulkan()
        {
            vulkan_destroy();
        }
        void vulkan_destroy()
        {
            destroy_swap_chain();
            destroy_logical_device();
            destroy_surface();
            destroy_debug_callback();
            destroy_instance();
        }

        [[nodiscard]] bool good() const
        {
            return m_initialized && m_errors.empty();
        }

        [[nodiscard]] std::vector<stage_error> errors() const
        {
            if (!m_initialized)
            {
                return { { VK_ERROR_UNKNOWN, "vulkan_initialize not called." } };
            }
            return m_errors;
        }

        static queue_family::swap_chain_support_details get_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            queue_family::swap_chain_support_details details;

            // Get Capabilities
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);


            // Get SurfaceFormats
            uint32_t format_count;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

            details.formats.resize(format_count);
            if (format_count != 0)
            {
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
            }

            // Get PresentModes
            uint32_t present_mode_count;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

            details.present_modes.resize(present_mode_count);
            if (present_mode_count != 0)
            {
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.data());
            }

            return details;
        }

        static std::vector<VkExtensionProperties> get_device_extension_properties(VkPhysicalDevice physical_device)
        {
            uint32_t extension_count;
            vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);

            std::vector<VkExtensionProperties> extensions(extension_count);
            if (extension_count != 0)
            {
                vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extensions.data());
            }

            return extensions;
        }

        static std::vector<VkLayerProperties> get_device_layer_properties(VkPhysicalDevice physical_device)
        {
            uint32_t layer_count;
            vkEnumerateDeviceLayerProperties(physical_device, &layer_count, nullptr);

            std::vector<VkLayerProperties> layers(layer_count);
            if (layer_count != 0)
            {
                vkEnumerateDeviceLayerProperties(physical_device, &layer_count, layers.data());
            }

            return layers;
        }

        static std::vector<VkExtensionProperties> get_instance_extension_properties()
        {
            uint32_t extension_count;
            vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

            std::vector<VkExtensionProperties> extensions(extension_count);
            if (extension_count != 0)
            {
                vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());
            }

            return extensions;
        }

        static std::vector<VkLayerProperties> get_instance_layer_properties()
        {
            uint32_t layer_count;
            vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

            std::vector<VkLayerProperties> layers(layer_count);
            if (layer_count != 0)
            {
                vkEnumerateInstanceLayerProperties(&layer_count, layers.data());
            }

            return layers;
        }

        // Returns the available Vulkan devices
        [[nodiscard]] std::vector<VkPhysicalDevice> get_physical_devices() const
        {
            if (!m_instance)
            {
                return { };
            }
            // Get the count of physical_devices
            uint32_t physical_device_count;
            vkEnumeratePhysicalDevices(m_instance, &physical_device_count, nullptr);

            // Get the actual physical_devices
            std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
            vkEnumeratePhysicalDevices(m_instance, &physical_device_count, physical_devices.data());

            return physical_devices;
        }

    };

    inline static std::string_view to_string(VkResult res)
    {
        using namespace std::string_view_literals;
        switch (res)
        {
            case VK_SUCCESS: return "VK_SUCCESS"sv;
            case VK_NOT_READY: return "VK_NOT_READY"sv;
            case VK_TIMEOUT: return "VK_TIMEOUT"sv;
            case VK_EVENT_SET: return "VK_EVENT_SET"sv;
            case VK_EVENT_RESET: return "VK_EVENT_RESET"sv;
            case VK_INCOMPLETE: return "VK_INCOMPLETE"sv;
            case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY"sv;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY"sv;
            case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED"sv;
            case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST"sv;
            case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED"sv;
            case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT"sv;
            case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT"sv;
            case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT"sv;
            case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER"sv;
            case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS"sv;
            case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED"sv;
            case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL"sv;
            case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN"sv;
            case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY"sv;
            case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE"sv;
            case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION"sv;
            case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS"sv;
            case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR"sv;
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR"sv;
            case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR"sv;
            case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR"sv;
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR"sv;
            case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT"sv;
            case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV"sv;
            case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT: return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT"sv;
            case VK_ERROR_NOT_PERMITTED_EXT: return "VK_ERROR_NOT_PERMITTED_EXT"sv;
            case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT"sv;
            case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR"sv;
            case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR"sv;
            case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR"sv;
            case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR"sv;
            case VK_PIPELINE_COMPILE_REQUIRED_EXT: return "VK_PIPELINE_COMPILE_REQUIRED_EXT"sv;
                // case VK_ERROR_OUT_OF_POOL_MEMORY_KHR: return "VK_ERROR_OUT_OF_POOL_MEMORY_KHR"sv;
                // case VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR: return "VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR"sv;
                // case VK_ERROR_FRAGMENTATION_EXT: return "VK_ERROR_FRAGMENTATION_EXT"sv;
                // case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT: return "VK_ERROR_INVALID_DEVICE_ADDRESS_EXT"sv;
                // case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS_KHR"sv;
                // case VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT: return "VK_ERROR_PIPELINE_COMPILE_REQUIRED_EXT"sv;
            case VK_RESULT_MAX_ENUM: return "VK_RESULT_MAX_ENUM"sv;
            default: return "UNKNOWN"sv;
        }
    }
}