project "AppFramework"
	kind "ConsoleApp" -- For testing
	-- kind "StaticLib" -- 
	language "C++"
	cppdialect "C++17"
	targetdir "../bin/%{prj.name}/%{cfg.buildcfg}"
	objdir "../obj/%{prj.name}/%{cfg.buildcfg}"
	files { "**.cpp", "**.h" }
	vpaths {
		["Header"] = "**.h",
		["Source"] = "**.cpp"
	}
	filter "system:windows"
		includedirs { "$(VULKAN_SDK)/Include", "include" }
		libdirs { "$(VULKAN_SDK)/Lib", "$(VULKAN_SDK)/Bin" }
		links { "vulkan-1.lib", "SDL2.lib" }
	filter "configurations:Debug"
		defines "DEBUG"
		symbols "On"
		-- targetname "appframework-d"
	filter "configurations:Release"
		defines "NDEBUG"
		optimize "On"
		-- targetname "appframework-r"
