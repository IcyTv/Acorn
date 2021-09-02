include "./vendor/premake/customization/solution_items.lua"
include "./vendor/premake/customization/vcpkg.lua"
require "./vendor/premake/export-compile-commands/export-compile-commands"

VULKAN_SDK = os.getenv("VULKAN_SDK")

if VULKAN_SDK == '' or VULKAN_SDK == nil then
    premake.error("Vulkan SDK is required")
end

workspace "Acorn"
    architecture "x64"
    startproject "OakTree"

    filter "system:windows"
        linkoptions { "-IGNORE:4099" }

    configurations 
    {
        "Debug",
        "Release",
        "Dist"
    }

    libdirs
    {
        "vcpkg_installed/x64-windows/lib"
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
IncludeDir["vcpkg"] = "%{wks.location}/vcpkg_installed/x64-windows/include"
IncludeDir["ImGui"] = "%{wks.location}/Acorn/vendor/ImGui"
IncludeDir["ImNodes"] = "%{wks.location}/Acorn/vendor/ImNodes"
IncludeDir["ImPlot"] = "%{wks.location}/Acorn/vendor/ImPlot"
IncludeDir["yaml_cpp"] = "%{wks.location}/Acorn/vendor/yaml-cpp/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/Acorn/vendor/ImGuizmo"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_DebugDLL"] = "%{VULKAN_SDK}/Bin"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

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