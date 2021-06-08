#if TZ_VULKAN
#include "gl/vk/tz_vulkan.hpp"
#include "gl/vk/setup/vulkan_instance.hpp"
#include "gl/vk/validation/debug_messenger.hpp"
#include "core/tz.hpp"
#include "core/assert.hpp"
namespace tz::gl::vk
{
    VulkanApplicationInfo::VulkanApplicationInfo(tz::GameInfo game_info):
    engine_name(game_info.engine.to_string()),
    app_info()
    {
        std::string engine_name = game_info.engine.to_string();
        auto ToVkVersion = [](tz::Version ver)->std::uint32_t
        {
            return VK_MAKE_VERSION(ver.major, ver.minor, ver.patch);
        };

        this->app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        this->app_info.pApplicationName = game_info.name;
        this->app_info.applicationVersion = ToVkVersion(game_info.version);
        
        this->app_info.pEngineName = engine_name.c_str();
        this->app_info.engineVersion = ToVkVersion(game_info.engine.version);
        this->app_info.apiVersion = ToVkVersion(vk::get_vulkan_version());
    }

    VkApplicationInfo VulkanApplicationInfo::get_info() const
    {
        return this->app_info;
    }

    VulkanInstance::VulkanInstance(VulkanApplicationInfo vk_app_info, ExtensionList extensions, ValidationLayerList validation_layers):
    instance()
    {
        VkInstanceCreateInfo create{};
        VkApplicationInfo app_info = vk_app_info.get_info();
        create.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create.pApplicationInfo = &app_info;

        tz_assert(tz::is_initialised(), "Cannot create VulkanInstanced when tz::initialise() hasn't been invoked");
        ExtensionList default_extensions = tz::gl::vk::get_default_required_extensions();
        // Add GLFW required extensions too.
        extensions.append(default_extensions);
        create.enabledExtensionCount = extensions.length();
        create.ppEnabledExtensionNames = extensions.data();

        // Validation Layers
        validation_layers.append(tz::gl::vk::get_default_validation_layers());
        std::vector<const char*> c_strings;
        create.enabledLayerCount = validation_layers.length();
        for(const VulkanValidationLayer& layer : validation_layers)
        {
            c_strings.push_back(layer.c_str());
        }
        create.ppEnabledLayerNames = c_strings.data();

        if(vkCreateInstance(&create, nullptr, &this->instance) != VK_SUCCESS)
        {
            tz_error("VulkanInstance::VulkanInstance(...): Failed to create instance.");
        }

        tz::gl::vk::validation::initialise_default_messenger(this->instance);
    }

    VulkanInstance::~VulkanInstance()
    {
        tz::gl::vk::validation::destroy_default_messenger();
        vkDestroyInstance(this->instance, nullptr);
    }

    VkInstance VulkanInstance::operator()() const
    {
        return this->native();
    }

    VkInstance VulkanInstance::native() const
    {
        return this->instance;
    }
}

#endif