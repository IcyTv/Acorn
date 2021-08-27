project "ImNodes"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "ImNodes.cpp",
        "ImNodes.h",
        "ImNodesEz.cpp",
        "ImNodesEz.h"
    }

    includedirs 
	{
		".",
		"../ImGui"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"