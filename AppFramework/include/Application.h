#pragma once

#include <vulkan/vulkan.hpp>

// Forward declearations
struct SDL_Window;

class Application {
public:
	virtual void OnCreate() = 0;
	virtual void OnUpdate(float dt) = 0;
	virtual void OnRender() = 0;
	void Run();
protected:
	uint32_t Width = 800;
	uint32_t Height = 600;
	const char* Title = "Application";
private:
	bool m_Running = false;
	SDL_Window* m_Window = nullptr;
	VkInstance m_Instance = nullptr;
	VkSurfaceKHR m_Surface = nullptr;
	VkPhysicalDevice m_PhysicalDevice = nullptr;
	VkDevice m_Device = nullptr;
	void InitWindow();
	void InitVulkan();
	void InitInstance();
	void InitSurface();
	void SelectPhysicalDevice();
	void CreateDevice();
	void CleanUp();
};