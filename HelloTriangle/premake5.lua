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
