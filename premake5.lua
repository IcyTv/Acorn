--Setup vcpkg

local vs2010 = premake.vstudio.vs2010


require('vstudio')

local vs = premake.vstudio.vc2010

local function premakeVersionComment(prj)
    premake.w('<!-- Generated by Premake ' .. _PREMAKE_VERSION .. ' -->')
end

local function vcpkg(prj)
    if prj.name == "Acorn" or prj.name == "Sandbox" or prj.name == "ImGui" or prj.name == "OakTree" then
        premake.w('<VcpkgTriplet>x64-windows-static</VcpkgTriplet>')
        premake.w('<VcpkgEnabled>true</VcpkgEnabled>')
        premake.w('<VcpkgEnableManifest>true</VcpkgEnableManifest>')
    else
        premake.w('<VcpkgEnabled>false</VcpkgEnabled>')
    end
    
end

premake.override(premake.vstudio.vc2010.elements, "project", function(base, prj)
    local calls = base(prj)
    table.insertafter(calls, vs.xmlDeclaration, premakeVersionComment)
    return calls
end)

premake.override(premake.vstudio.vc2010.elements, "globals", function(base, prj)
    local calls = base(prj)
    table.insertafter(calls, vs.globals, vcpkg)
    return calls
end)

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

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["ImGui"] = "Acorn/vendor/ImGui"
IncludeDir["ImNodes"] = "Acorn/vendor/ImNodes"
IncludeDir["ImPlot"] = "Acorn/vendor/ImPlot"

group "Vendors"
    include "Acorn/vendor/ImGui"
    include "Acorn/vendor/ImNodes"
    include "Acorn/vendor/ImPlot"
group ""

project "Acorn"
    location "Acorn"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("build/" .. outputdir .. "/%{prj.name}")

    pchheader "acpch.h"
    pchsource "Acorn/src/acpch.cpp"

    disablewarnings
    {
        "26812"
    }

    files 
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.c",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/res/**/*",
        "%{prj.name}/vendor/stb_image/stb_image.cpp",
        "vcpkg.json",
        "premake5.lua",
        "cpp.hint"
    }

    includedirs
    {
        "%{prj.name}/src",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImNodes}",
		"%{IncludeDir.ImPlot}",
    }

    links
    {
        "glfw3",
        "glad",
        "ImGui",
        "ImNodes",
        "ImPlot",
		"opengl32"
    }

    filter "system:windows"
        systemversion "latest"

        defines
        {
            "AC_PLATFORM_WINDOWS",
            "AC_BUILD_DLL",
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

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("build/" .. outputdir .. "/%{prj.name}")
    
    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.c",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/res/**/*",
    }

    includedirs
    {
        "Acorn/src",
        "Acorn/vendor",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImNodes}",
		"%{IncludeDir.ImPlot}",
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

    filter "configurations:Release"
        defines "AC_DEBUG"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "AC_DIST"
        runtime "Release"
        optimize "Speed"


project "OakTree"
    location "OakTree"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("build/" .. outputdir .. "/%{prj.name}")
    
    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.c",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/res/**/*",
    }

    includedirs
    {
        "Acorn/src",
        "Acorn/vendor",
        "OakTree/src",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImNodes}",
		"%{IncludeDir.ImPlot}",
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

    filter "configurations:Release"
        defines "AC_DEBUG"
        runtime "Release"
        optimize "on"

    filter "configurations:Dist"
        defines "AC_DIST"
        runtime "Release"
        optimize "Speed"