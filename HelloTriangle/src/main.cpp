#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif // _WIN32
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>

#include <iostream>

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
		vkDestroyInstance(instance, nullptr);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
private: // Member variables
	const int WIDTH{ 800 };
	const int HEIGHT{ 600 };
	SDL_Window* window = nullptr;
	bool running{};
	// Vulkan member variables
	VkInstance instance{};
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
	}
	// Initialize an instance of Vulkan for the application
	void createInstance() {
		// The application info needed for the instance creation info
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		// Querying for device capabilitys via extensions
		unsigned sdlExtensionCount;
		SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, nullptr);
		std::vector<const char*> sdlExtensions(sdlExtensionCount);
		SDL_Vulkan_GetInstanceExtensions(window, &sdlExtensionCount, sdlExtensions.data());
		// Checking if all the extension specified by SDL are available
		if (!checkExtensionsPresentInLayer(nullptr, sdlExtensions)) {
			throw std::runtime_error("not all required extensions are available!");
		}

		// Specification for creating the vulkan instance
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = sdlExtensionCount;
		createInfo.ppEnabledExtensionNames = sdlExtensions.data();
		createInfo.enabledLayerCount = 0;

		// Creating the vulkan instance. See https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateInstance.html
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vulkan instance");
		}
	}
	bool checkExtensionsPresentInLayer(const char* const layerName, const std::vector<const char*> extensions) {
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
			if (extensionAvailable) {
				std::cout << extension << " available\n";
			}
			else {
				std::cerr << extension << " not available\n";
				available = false;
			}
		}
		return available;
	}
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