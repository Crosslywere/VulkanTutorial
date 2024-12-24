#include <Application.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <cstdlib>
#include <vector>
#include <cstdint>
#include <cstring>
#include <set>
#include <string>
#include <map>
#include <optional>

std::vector<const char*> g_ValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

std::vector<const char*> g_ExtensionNames = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#pragma region Utilities

struct QueueFamilies {
	std::optional<uint32_t> graphics;
	std::optional<uint32_t> present;
	bool IsComplete() const {
		return graphics.has_value() && present.has_value();
	}
};

static std::vector<const char*> GetGlobalExtensions(SDL_Window* window) {
	uint32_t extensionCount;
	SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
	std::vector<const char*> extensions(extensionCount);
	SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data());
#ifdef DEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
	return extensions;
}

static bool ValidationLayersSupported() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	if (layerCount == 0 && g_ValidationLayers.size() > 0) {
		SDL_Log("Failed to get global layer properties!");
		return false;
	}
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	for (const auto& layerName : g_ValidationLayers) {
		bool supported = false;
		for (const auto& availableLayerName : availableLayers) {
			if (std::strncmp(layerName, availableLayerName.layerName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE) == 0) {
				supported = true;
				break;
			}
		}
		if (!supported) {
			return false;
		}
	}
	return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		SDL_LogWarn(0, "\x1b[33m%s\x1b[0m", pCallbackData->pMessage);
		return VK_FALSE;
	}
	SDL_LogError(0, "\x1b[31m%s\x1b[0m", pCallbackData->pMessage);
	return VK_TRUE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAlloc, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (function != nullptr) {
		SDL_Log("\x1b[32mCreating debug messenger...\x1b[0m");
		return function(instance, pCreateInfo, pAlloc, pDebugMessenger);
	}
	SDL_LogWarn(0, "Failed to get create debug messenger function");
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) {
	auto function = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (function != nullptr) {
		function(instance, messenger, pAllocator);
	}
}

static void PopulateDebugUtilsMessengerCreateInfoEXT(VkDebugUtilsMessengerCreateInfoEXT& messengerCreateInfo) {
	messengerCreateInfo = {};
	messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	messengerCreateInfo.pfnUserCallback = DebugMessengerCallback;
	messengerCreateInfo.pUserData = nullptr;
}

static bool CheckDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t propertyCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &propertyCount, nullptr);
	std::vector<VkExtensionProperties> properties(propertyCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &propertyCount, properties.data());
	std::set<std::string> extSet(g_ExtensionNames.begin(), g_ExtensionNames.end());
	for (const auto& property : properties) {
		extSet.erase(property.extensionName);
	}
	return extSet.empty();
}

static QueueFamilies FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	uint32_t familyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
	std::vector<VkQueueFamilyProperties> familyProps(familyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, familyProps.data());
	QueueFamilies families;
	uint32_t i = 0;
	for (const auto& prop : familyProps) {
		if (prop.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			families.graphics = i;
			VkBool32 present;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present);
			if (present == VK_TRUE) {
				families.present = i;
			}
		}
		if (families.IsComplete()) {
			break;
		}
		i++;
	}
	return families;
}

static bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
	return CheckDeviceExtensionSupport(device) && FindQueueFamilies(device, surface).IsComplete();
}

static uint32_t RateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface) {
	if (!IsDeviceSuitable(device, surface)) {
		return 0;
	}
	uint32_t score = 0;

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);

	if (properties.deviceType <= VK_PHYSICAL_DEVICE_TYPE_OTHER) {
		return 0;
	}
	score += 500 * properties.deviceType;
	score += properties.limits.maxImageDimension2D;
	SDL_LogInfo(0, "Device %s scored %i", properties.deviceName, score);
	return score;
}

#pragma endregion

void Application::InitWindow() {
	SDL_Init(SDL_INIT_VIDEO);
	m_Window = SDL_CreateWindow(Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, SDL_WINDOW_VULKAN);
	if (m_Window != nullptr) {
		m_Running = true;
		return;
	}
	SDL_LogError(0, "Failed to create window!");
	exit(EXIT_FAILURE);
}

void Application::CreateInstance() {
	VkInstanceCreateInfo instanceInfo{};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	
	auto globalExtensions = GetGlobalExtensions(m_Window);

	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(globalExtensions.size());
	instanceInfo.ppEnabledExtensionNames = globalExtensions.data();

#ifdef DEBUG
	if (!ValidationLayersSupported()) {
		SDL_LogError(0, "Validation layers are not supported!");
		exit(EXIT_FAILURE);
	}
	instanceInfo.enabledLayerCount = static_cast<uint32_t>(g_ValidationLayers.size());
	instanceInfo.ppEnabledLayerNames = g_ValidationLayers.data();

	VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
	PopulateDebugUtilsMessengerCreateInfoEXT(debugMessengerInfo);

	instanceInfo.pNext = &debugMessengerInfo;
#endif

	if (vkCreateInstance(&instanceInfo, nullptr, &m_Instance) != VK_SUCCESS) {
		SDL_LogError(0, "Failed to create vulkan instance!");
		SDL_DestroyWindow(m_Window);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
}

void Application::SetupDebugMessenger() {
#ifdef DEBUG
	VkDebugUtilsMessengerCreateInfoEXT debugMessengerInfo{};
	PopulateDebugUtilsMessengerCreateInfoEXT(debugMessengerInfo);
	if (CreateDebugUtilsMessengerEXT(m_Instance, &debugMessengerInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS) {
		SDL_LogError(0, "Failed to setup debug messenger!");
	}
#endif
}

void Application::CreateSurface() {
	if (SDL_Vulkan_CreateSurface(m_Window, m_Instance, &m_Surface) != SDL_TRUE) {
		SDL_LogError(0, "Failed to create window surface!");
#ifdef DEBUG
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
#endif
		vkDestroyInstance(m_Instance, nullptr);
		SDL_DestroyWindow(m_Window);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
}

void Application::SelectPhysicalDevice() {
	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		SDL_LogError(0, "Failed to find any available physical device!");
		exit(EXIT_FAILURE);
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());
	std::map<uint32_t, VkPhysicalDevice> deviceCandidates;
	for (const auto& device : devices) {
		deviceCandidates.insert(std::make_pair(RateDeviceSuitability(device, m_Surface), device));
	}
	if (deviceCandidates.rbegin()->first == 0) {
		SDL_LogError(0, "Failed to find any suitable physical device!");
#ifdef DEBUG
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
#endif
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
		SDL_DestroyWindow(m_Window);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	else {
		m_PhysicalDevice = deviceCandidates.rbegin()->second;
	}
}

void Application::CreateDevice() {
	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	deviceInfo.enabledExtensionCount = static_cast<uint32_t>(g_ExtensionNames.size());
	deviceInfo.ppEnabledExtensionNames = g_ExtensionNames.data();

#ifdef DEBUG
	deviceInfo.enabledLayerCount = static_cast<uint32_t>(g_ValidationLayers.size());
	deviceInfo.ppEnabledLayerNames = g_ValidationLayers.data();
#else
	deviceInfo.enabledLayerCount = 0;
#endif

	QueueFamilies families = FindQueueFamilies(m_PhysicalDevice, m_Surface);
	std::set<uint32_t> queues = { families.graphics.value(), families.present.value() };
	std::vector<VkDeviceQueueCreateInfo> queueInfos;
	float priority = 1.0;
	for (const auto& queue : queues) {
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = queue;
		queueInfo.pQueuePriorities = &priority;
		queueInfo.queueCount = 1U;
		queueInfos.push_back(queueInfo);
	}

	deviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
	deviceInfo.pQueueCreateInfos = queueInfos.data();

	VkPhysicalDeviceFeatures deviceFeatures{};

	deviceInfo.pEnabledFeatures = &deviceFeatures;

	if (vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device) != VK_SUCCESS) {
		SDL_LogError(0, "Failed to create logical device!");
#ifdef DEBUG
		DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
#endif
		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
		SDL_DestroyWindow(m_Window);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
}

void Application::InitVulkan() {
	CreateInstance();
	SetupDebugMessenger();
	CreateSurface();
	SelectPhysicalDevice();
	CreateDevice();
}

void Application::Run() {
	InitWindow();
	InitVulkan();
	OnCreate();
	float past = SDL_GetTicks() / 1000.0f;
	while (m_Running) {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_QUIT:
				m_Running = false;
			}
		}
		float now = SDL_GetTicks() / 1000.0f;
		float deltaTime = now - past;
		past = now;
		OnUpdate(deltaTime);
		OnRender();
	}
	CleanUp();
}

void Application::CleanUp() {
	vkDestroyDevice(m_Device, nullptr);
#ifdef DEBUG
	DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
#endif
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	vkDestroyInstance(m_Instance, nullptr);
	SDL_DestroyWindow(m_Window);
	SDL_Quit();
}