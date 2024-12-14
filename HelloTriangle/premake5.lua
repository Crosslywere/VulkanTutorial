project "HelloTriangle"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetdir "../bin/%{prj.name}/%{cfg.buildcfg}"
	objdir "../obj/%{prj.name}/%{cfg.buildcfg}"
	files {"**.cpp"}
	vpaths {
		["Source"] = "**.cpp"
	}

	filter "system:windows"
		includedirs "$(VULKAN_SDK)/Include"
		libdirs {"$(VULKAN_SDK)/Lib", "$(VULKAN_SDK)/Bin"}
		links {"vulkan-1.lib", "SDL2.lib"}
		defines "SDL_MAIN_HANDLED"

	filter "configurations:Debug"
		defines "DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "NDEBUG"
		optimize "On"
