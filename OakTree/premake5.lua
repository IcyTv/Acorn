project "OakTree"
    kind "ConsoleApp"
    language "C++"
    filter "action:vs*"
        cppdialect "C++latest"
    filter {}
    staticruntime "off"

    vcpkg "on"
    
    dependson { "vendor/premake" }

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")
    
    files
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.c",
        "src/**.cpp",
        "res/**/*",
    }

    includedirs
    {
        "%{wks.location}/Acorn/src",
        "%{wks.location}/Acorn/vendor",
        "src",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImNodes}",
		"%{IncludeDir.ImPlot}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.VulkanSDK}",
    }

    links
    {
        "Acorn"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "AC_PLATFORM_WINDOWS"
        }

    filter "configurations:Debug"
        defines "AC_DEBUG"
        runtime "Debug"
        symbols "on"
        ignoredefaultlibraries { "libcmt", "MSVCRT" }

    filter "configurations:Release"
        defines "AC_DEBUG"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "AC_DIST"
        runtime "Release"
        optimize "Speed"