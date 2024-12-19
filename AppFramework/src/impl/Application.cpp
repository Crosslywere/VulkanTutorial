#include <Application.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <cstdlib>
#include <vector>
#include <cstdint>
#include <map>
#include <optional>

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentationFamily;
	bool IsComplete() {
		return graphicsFamily.has_value() && presentationFamily.has_value();
	}
};

inline std::vector<const char*> g_DeviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

void Application::InitWindow() {
	SDL_Init(SDL_INIT_VIDEO);
	m_Window = SDL_CreateWindow(Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, Width, Height, SDL_WINDOW_VULKAN);
	m_Running = m_Window != nullptr;
	if (!m_Running) {
		SDL_LogError(0, "Failed to create window!");
		exit(EXIT_FAILURE);
	}
}

static std::vector<const char*> GetWindowExtensions(SDL_Window* window) {
	uint32_t extensionCount;
	SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);
	std::vector<const char*> extensions(extensionCount);
	SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions.data());
	return extensions;
}

void Application::InitInstance() {
	auto extensions = GetWindowExtensions(m_Window);
	VkInstanceCreateInfo instanceInfo{};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceInfo.ppEnabledExtensionNames = extensions.data();
	if (vkCreateInstance(&instanceInfo, nullptr, &m_Instance) != VK_SUCCESS) {
		m_Running = false;
		SDL_LogError(0, "Failed to create vulkan instance!");
		exit(EXIT_FAILURE);
	}
}

void Application::InitSurface() {
	if (SDL_Vulkan_CreateSurface(m_Window, m_Instance, &m_Surface) != SDL_TRUE) {
		SDL_LogError(0, "Failed to create window surface!");
		exit(EXIT_FAILURE);
	}
}

static uint32_t ScoreDevice(const VkPhysicalDevice device) {
	uint32_t score = 0;
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);
	if (properties.deviceType >= VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && properties.deviceType <= VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) {
		score += properties.deviceType * 1000U;
	}
	score += properties.limits.maxImageDimension2D;
	SDL_Log("Device '%s' scored %i", properties.deviceName, score);
	return score;
}

static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	uint32_t familyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(familyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, queueFamilies.data());
	QueueFamilyIndices indices;
	uint32_t i = 0;
	for ( const auto & queueFamily : queueFamilies) {
		if (queueFamily.queueFlags bitand VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
			VkBool32 presentSupport;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport == VK_TRUE) {
				indices.presentationFamily = i;
			}
		}
		if (indices.IsComplete()) {
			break;
		}
		i++;
	}
	return indices;
}

void Application::SelectPhysicalDevice() {
	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		SDL_LogError(0, "No vulkan capable devices found!");
		exit(EXIT_FAILURE);
	}
	std::vector<VkPhysicalDevice> deviceCandidates(deviceCount);
	vkEnumeratePhysicalDevices(m_Instance, &deviceCount, deviceCandidates.data());
	std::map<uint32_t, VkPhysicalDevice> candidates;
	for (const auto& candidate : deviceCandidates) {
		candidates.insert(std::make_pair(ScoreDevice(candidate), candidate));
	}
	if (candidates.rbegin()->first > 0) {
		m_PhysicalDevice = candidates.rbegin()->second;
	}
	else {
		SDL_LogError(0, "No physical device was suitable!");
		exit(EXIT_FAILURE);
	}
}

void Application::CreateDevice() {
	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.enabledExtensionCount = static_cast<uint32_t>(g_DeviceExtensions.size());
	deviceInfo.ppEnabledExtensionNames = g_DeviceExtensions.data();

}

void Application::InitVulkan() {
	InitInstance();
	InitSurface();
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
	vkDestroyInstance(m_Instance, nullptr);
	SDL_DestroyWindow(m_Window);
	SDL_Quit();
}