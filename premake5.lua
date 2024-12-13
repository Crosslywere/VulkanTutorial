workspace "Vulkan"
	architecture "x64"
	configurations {"Debug", "Release"}

	include "HelloTriangle"

	-- Specifically configured for windows(I don't know how other os handle vulkan directories)
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
