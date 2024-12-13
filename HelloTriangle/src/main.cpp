#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <vector>
#include <string>

#ifdef DEBUG
#define ENABLE_VALIDATION_LAYERS
#endif // DEBUG

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
	VkInstance instance{};
	const std::vector<const char*> validationLayers = {
		"VK_LAYER_KHRONOS_validation",
	};
	VkDebugUtilsMessengerEXT debugMessenger{};
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
	void setupDebugMessenger() {
#ifdef ENABLE_VALIDATION_LAYERS
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populateDebugMessengerCreateInfo(createInfo);
		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to setup debug messenger");
		}
#endif
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