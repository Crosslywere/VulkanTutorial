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
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
private: // Member variables
	const int WIDTH{ 800 };
	const int HEIGHT{ 600 };
	SDL_Window* window = nullptr;
	bool running{};
private: // Internal functions
	void initWindow() {
		SDL_Init(SDL_INIT_VIDEO);
		window = SDL_CreateWindow("Hello Triangle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			WIDTH, HEIGHT, SDL_WINDOW_VULKAN);
		if (window == nullptr) {
			throw std::runtime_error("failed to create SDL window");
		}
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