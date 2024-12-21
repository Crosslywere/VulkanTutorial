#include <Application.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <cstdlib>
#include <vector>
#include <cstdint>
#include <cstring>

std::vector<const char*> g_ValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#pragma region Utilities

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
			if (std::strcmp(layerName, availableLayerName.layerName) == 0) {
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
		SDL_LogWarn(0, "%s", pCallbackData->pMessage);
		return VK_FALSE;
	}
	SDL_LogError(0, "%s", pCallbackData->pMessage);
	return VK_TRUE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAlloc, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto function = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (function != nullptr) {
		return function(instance, pCreateInfo, pAlloc, pDebugMessenger);
	}
	SDL_Log("Failed to get create debug messenger function");
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) {
	auto function = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (function != nullptr) {
		function(instance, messenger, pAllocator);
	}
}

void PopulateDebugUtilsMessengerCreateInfoEXT(VkDebugUtilsMessengerCreateInfoEXT& messengerCreateInfo) {
	messengerCreateInfo = {};
	messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	messengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	messengerCreateInfo.pfnUserCallback = DebugMessengerCallback;
	messengerCreateInfo.pUserData = nullptr;
}

#pragma endregion

void Application::InitWindow() {
	SDL_Init(SDL_INIT_VIDEO);
	m_Window = SDL_CreateWindow(Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, SDL_WINDOW_VULKAN);
	if (m_Window != nullptr) {
		m_Running = true;
	}
	else {
		SDL_LogError(0, "Failed to create window!");
		exit(EXIT_FAILURE);
	}
}

void Application::CreateInstance() {
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	applicationInfo.pApplicationName = Title;
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "VulkanRenderer";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo instanceInfo{};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &applicationInfo;

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
		exit(EXIT_FAILURE);
	}
}

void Application::SelectPhysicalDevice() {
}

void Application::CreateDevice() {
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
#ifdef DEBUG
	DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
#endif
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
	vkDestroyInstance(m_Instance, nullptr);
	SDL_DestroyWindow(m_Window);
	SDL_Quit();
}