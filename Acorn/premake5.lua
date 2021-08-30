project "Acorn"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    vcpkg "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")

    pchheader "acpch.h"
    pchsource "src/acpch.cpp"

    disablewarnings
    {
        "26812"
    }

    files 
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.c",
        "src/**.cpp",
        "res/**/*",

        "vendor/stb_image/stb_image.cpp",
		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp"
    }

    includedirs
    {
        "src",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImNodes}",
		"%{IncludeDir.ImPlot}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGuizmo}",
    }

    links
    {
        "glfw3",
        "glad",
        "ImGui",
        "ImNodes",
        "ImPlot",
		"opengl32",
        "yaml-cpp"
    }
    

    filter "files:vendor/ImGuizmo/**.cpp"
        flags {"NoPCH"}
    
    filter "system:windows"
        systemversion "latest"

        defines
        {
            "AC_PLATFORM_WINDOWS",
            "GLFW_INCLUDE_NONE"
        }

    filter "configurations:Debug"
        defines {"AC_DEBUG", "PSNIP_DEBUG", "AC_ENABLE_ASSERTS"}
        runtime "Debug"
        symbols "on"
        
    filter "configurations:Release"
        defines {"AC_DEBUG", "NDEBUG", "AC_ENABLE_ASSERTS"}
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "AC_DIST"
        runtime "Release"
        optimize "Speed"