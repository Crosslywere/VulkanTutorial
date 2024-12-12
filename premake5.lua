workspace "Vulkan"
	architecture "x64"
	configurations {"Debug", "Release"}

project "HelloTriangle"
	location "HelloTriangle"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetdir "bin/%{prj.name}"
	objdir "obj/%{prj.name}"
	files {"%{prj.name}/**.cpp", "%{prj.name}/**.h"}
	defines {"SDL_MAIN_HANDLED"} -- For SDL2 to not do anything to main()

	-- Specifically configured for windows(I don't know how other os handle vulkan directories)
	filter "system:windows"
		includedirs {"$(VULKAN_SDK)/Include"}
		libdirs {"$(VULKAN_SDK)/Lib", "$(VULKAN_SDK)/Bin"}
		links {"vulkan-1.lib", "SDL2.lib"}

	filter "configurations:Debug"
		defines {"DEBUG"}
		symbols "On"

	filter "configurations:Release"
		defines {"NDEBUG"}
		optimize "On"
