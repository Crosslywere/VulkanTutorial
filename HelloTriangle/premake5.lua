project "HelloTriangle"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetdir "../bin/%{prj.name}/%{cfg.buildcfg}"
	objdir "../obj/%{prj.name}/%{cfg.buildcfg}"
	files {"**.cpp", "**.vert", "**.frag"}
	vpaths {
		["Source"] = "**.cpp",
		["Resource"] = {"**.vert", "**.frag"}
	}

	-- Prebuild commands to compile shaders and move them into the correct directory
	prebuildcommands {
		"{MKDIR} shaders",
		"glslc res/shader.vert -o vert.spv",
		"{MOVE} vert.spv shaders/vert.spv",
		"glslc res/shader.frag -o frag.spv",
		"{MOVE} frag.spv shaders/frag.spv",
		"{COPYFILE} shaders ../bin/%{prj.name}/%{cfg.buildcfg}/shaders"
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
