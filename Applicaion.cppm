module;

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

export module Core;

namespace glfw {
struct Context {
  Context() {
    auto res = glfwInit();
    if (res != GLFW_TRUE)
      throw std::runtime_error("failed to initialize GLFW");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwSetErrorCallback([](int error, const char *description) {
      std::cerr << "glfw error: " << error << " description: " << description
                << "\n";
    });
  }
  ~Context() { glfwTerminate(); }
};

using Window = std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)>;

} // namespace glfw

// static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
// 	[[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
// 	[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
// 	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
// 	[[maybe_unused]] void* pUserData) {
// 	std::cerr << "validation layer: " << pCallbackData->pMessage <<
// std::endl; 	return VK_FALSE;
// }

namespace vk {

class Instance {
public:
  Instance() {
#ifdef NDEBUG
    enableValidationLayers = false;
#else
    enableValidationLayers = true;
#endif
    assert(enableValidationLayers);
    if (enableValidationLayers && !checkValidationLayerSupport()) {
      throw std::runtime_error(
          "validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    const auto &requiredExtensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = requiredExtensions.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    if (enableValidationLayers) {
      createInfo.enabledLayerCount =
          static_cast<uint32_t>(validationLayers.size());
      // for (const auto& layer : validationLayers) {
      // std::cout << layer << "\n";
      // }
      createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
      createInfo.enabledLayerCount = 0;
    }
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
      throw std::runtime_error("failed to create invulkan instance");
    }
  }
  ~Instance() { vkDestroyInstance(instance, nullptr); }

  [[nodiscard]] VkInstance getVkInstance() const { return instance; }

public:
  static inline bool enableValidationLayers = false;

private:
  VkInstance instance;

  const std::vector<const char *> validationLayers = {
      "VK_LAYER_KHRONOS_validation",
  };
  [[nodiscard]] bool checkValidationLayerSupport() const {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    return std::ranges::any_of(
        validationLayers, [&availableLayers](const auto &validationLayer) {
          return std::ranges::any_of(
              availableLayers.begin(), availableLayers.end(),
              [&validationLayer](const auto &availableLayer) {
                return std::strcmp(validationLayer, availableLayer.layerName) ==
                       0;
              });
        });
  }

  static std::vector<const char *> getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    if (enableValidationLayers) {
      extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
  }
};

class DebugMessenger {
public:
  DebugMessenger(const VkInstance &instance,
                 const VkAllocationCallbacks *pAllocator)
      : instance(instance), pAllocator(pAllocator) {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback =
        []([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT
               messageSeverity,
           [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
           const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
           [[maybe_unused]] void *pUserData) {
          std::cerr << "validation layer: " << pCallbackData->pMessage
                    << std::endl;
          return VK_FALSE;
        };
    createInfo.pUserData = nullptr;
    const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (!func) {
      throw std::runtime_error(
          "failed to create debug messenger VK_ERROR_EXTENSION_NOT_PRESENT");
    }
    if (const auto res =
            func(instance, &createInfo, pAllocator, &debugMessenger);
        res != VK_SUCCESS) {
      throw std::runtime_error("failed to set up debug messenger");
    }
  }
  ~DebugMessenger() {
    if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance,
                                  "vkDestroyDebugUtilsMessengerEXT"))) {
      func(instance, debugMessenger, pAllocator);
    }
  }

private:
  VkDebugUtilsMessengerEXT debugMessenger{};
  const VkInstance &instance;
  const VkAllocationCallbacks *pAllocator;
};
} // namespace vk

export namespace gfx {
class Application {
public:
  Application()
      : window{glfwCreateWindow(800, 600, "vk-tutorial", nullptr, nullptr),
               glfwDestroyWindow},
        instance{}, debugMessenger(instance.getVkInstance(), nullptr) {}
  ~Application() = default;

  void run() {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::cout << extensionCount << " extensions supported\n";

    while (!glfwWindowShouldClose(window.get())) {
      glfwPollEvents();
    }
  }

private:
  glfw::Context context{};
  glfw::Window window;
  vk::Instance instance;
  vk::DebugMessenger debugMessenger;
};
} // namespace gfx
