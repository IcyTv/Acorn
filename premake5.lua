include "./vendor/premake/customization/solution_items.lua"
include "./vendor/premake/customization/vcpkg.lua"


workspace "Acorn"
    architecture "x64"
    startproject "OakTree"

    configurations 
    {
        "Debug",
        "Release",
        "Dist"
    }

    libdirs
    {
        "vcpkg_installed/x64-windows-static/lib"
    }

    solution_items
    {
        "premake.lua",
        "vcpkg.json"
    }

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["ImGui"] = "%{wks.location}/Acorn/vendor/ImGui"
IncludeDir["ImNodes"] = "%{wks.location}/Acorn/vendor/ImNodes"
IncludeDir["ImPlot"] = "%{wks.location}/Acorn/vendor/ImPlot"
IncludeDir["yaml_cpp"] = "%{wks.location}/Acorn/vendor/yaml-cpp/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/Acorn/vendor/ImGuizmo"

group "Dependencies"
    include "vendor/premake"
    include "Acorn/vendor/ImGui"
    include "Acorn/vendor/ImNodes"
    include "Acorn/vendor/ImPlot"
    include "Acorn/vendor/yaml-cpp"
group ""

include "Acorn"
include "OakTree"
include "Sandbox"