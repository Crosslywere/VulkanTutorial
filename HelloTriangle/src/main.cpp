#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <optional>

#ifdef DEBUG
#define ENABLE_VALIDATION_LAYERS
#endif

VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto fn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (fn != nullptr) {
		return fn(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator) {
	auto fn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (fn != nullptr) {
		return fn(instance, debugMessenger, pAllocator);
	}
}


struct QueueFamilyIndices {
	std::optional<unsigned> graphicsFamily;
	bool isComplete() {
		return graphicsFamily.has_value();
	}
};

class HelloTriangle {
public:
	void run() {
		init();
		mainLoop();
		cleanUp();
	}
private: // Private functions
	void init() {
		initWindow();
		initVulkan();
	}
	void mainLoop() {
		running = true;
		while (running) {
			// Polling events
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_QUIT:
					running = false;
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) {
						running = false;
					}
					break;
				}
			}
			// Render Code Here
		}
	}
	void cleanUp() {
#ifdef ENABLE_VALIDATION_LAYERS
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif
		vkDestroyInstance(instance, nullptr);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
#pragma region Member_Variables
	const int WIDTH{ 800 };
	const int HEIGHT{ 600 };
	SDL_Window* window = nullptr;
	bool running{};
	// Vulkan member variables
	VkInstance instance = VK_NULL_HANDLE;
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation",
	};
	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // No need to destroy(implicitly destroyed with instance)
#pragma endregion
#pragma region Internal_Functions
private: // Internal functions
	void initWindow() {
		SDL_Init(SDL_INIT_VIDEO);
		window = SDL_CreateWindow("Hello Triangle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			WIDTH, HEIGHT, SDL_WINDOW_VULKAN);
		if (window == nullptr) {
			throw std::runtime_error("failed to create SDL window");
		}
	}
	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		pickPhysicalDevice();
	}
#pragma endregion
	// Initialize an instance of Vulkan for the application
	void createInstance() {
#ifdef ENABLE_VALIDATION_LAYERS
		// Checking that validation layers exist before creating an instance that requires the layers
		if (!checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}
#endif // ENABLE_VALIDATION_LAYERS

		// The application info needed for the instance creation info
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Querying for general device capabilities via extensions
		auto extensions = getRequiredExtensions();
		// Checking if all the extension specified by SDL are available
		if (!checkExtensionsPresentInLayer(nullptr, extensions)) {
			throw std::runtime_error("not all required extensions are available!");
		}

		// Specification for creating the vulkan instance
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<unsigned>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
#ifdef ENABLE_VALIDATION_LAYERS
		createInfo.enabledLayerCount = static_cast<unsigned>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
#else
		createInfo.enabledLayerCount = 0;
#endif // ENABLE_VALIDATION_LAYERS
		// Creating the vulkan instance. See https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateInstance.html
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vulkan instance");
		}
	}
	// Setting up the debug messenger
	void setupDebugMessenger() {
#ifdef ENABLE_VALIDATION_LAYERS
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populateDebugMessengerCreateInfo(createInfo);
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to setup debug messenger");
		}
#endif
	}
	// Picking the Vulkan device that best matches requirements
	void pickPhysicalDevice() {
		unsigned deviceCount;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		// Deciding on the device that is most suitable
		std::map<int, VkPhysicalDevice> candidates;
		for (const auto& device : devices) {
			int score = rateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}
		if (candidates.rbegin()->first > 0 and isDeviceSuitable(candidates.rbegin()->second)) {
			physicalDevice = candidates.rbegin()->second;
		}
		else {
			throw std::runtime_error("failed to ind a suitable GPU");
		}
	}
#pragma region Utility_Functions
	bool checkExtensionsPresentInLayer(const char* const layerName, const std::vector<const char*>& extensions) {
		// Getting all the supported extensions
		unsigned supportedExtensionCount{};
		vkEnumerateInstanceExtensionProperties(layerName, &supportedExtensionCount, nullptr);
		std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
		vkEnumerateInstanceExtensionProperties(layerName, &supportedExtensionCount, supportedExtensions.data());
		bool available = true;
		for (const auto& extension : extensions) {
			bool extensionAvailable = false;
			for (const auto& supportedExtension : supportedExtensions) {
				if (strcmp(extension, supportedExtension.extensionName) == 0) {
					extensionAvailable = true;
					break;
				}
			}
			if (!extensionAvailable) {
				available = false;
			}
		}
		return available;
	}
	bool checkValidationLayerSupport() {
		// Getting all the available layers
		unsigned layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
		// Checking if layers are present
		for (const auto& layerName : validationLayers) {
			bool layerFound = false;
			for (const auto& layerProperty : availableLayers) {
				if (strcmp(layerName, layerProperty.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			if (!layerFound) {
				return false;
			}
		}
		return true;
	}
	std::vector<const char*> getRequiredExtensions() {
		unsigned sdlExtensionCount;
		SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, nullptr);
		std::vector<const char*> extensions(sdlExtensionCount);
		SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, extensions.data());
#ifdef ENABLE_VALIDATION_LAYERS
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		return extensions;
	}
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		std::cerr << pCallbackData->pMessage << std::endl;
		return VK_FALSE;// Return VK_TRUE if call to this should abort
	}
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}
	bool isDeviceSuitable(VkPhysicalDevice device) {
		QueueFamilyIndices indices = findQueueFamilies(device);

		return indices.isComplete();
	}
	int rateDeviceSuitability(VkPhysicalDevice device) {
		int score = 0;

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		score += deviceProperties.deviceType * 500;
		score += deviceProperties.limits.maxImageDimension2D;

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		if (!deviceFeatures.geometryShader) {
			return 0;
		}
		return score;
	}
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;
		unsigned queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}
			if (indices.isComplete()) {
				break;
			}
			i++;
		}
		return indices;
	}
#pragma endregion
};

int main() {
	HelloTriangle app;
	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
