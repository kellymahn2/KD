project "Kaidel"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "KDpch.h"
	pchsource "src/KDpch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp",
		"vendor/VMA/**.h",
		"vendor/VMA/**.cpp",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"YAML_CPP_STATIC_DEFINE"
	}

	includedirs
	{
		"src",
		"vendor/spdlog/include",
		"vendor/FileWatch",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.DirectX}",
		"%{IncludeDir.Vulkan}",
		"vendor/VMA"
	}

	links
	{
		"Box2D",
		"GLFW",
		"Glad",
		"ImGui",
		"vendor/Assimp/lib/assimp-vc140-mt.lib",
		"yaml-cpp",
		"opengl32.lib",
		"D3D11.lib",
		"%{Library.mono}",
		"%{Library.Vulkan}",

	}

	filter "files:vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

		defines
		{
		}

		links
		{
			"%{Library.WinSock}",
			"%{Library.WinMM}",
			"%{Library.WinVersion}",
			"%{Library.BCrypt}",
		}

	filter "configurations:Debug"
		defines "KD_DEBUG"
		runtime "Debug"
		symbols "on"
		links{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}",
			"%{Library.SPIRV_Reflect_Debug}",
			"%{Library.SPIRV_Cross_Reflect_Debug}",
		}


	filter "configurations:Release"
		defines "KD_RELEASE"
		runtime "Release"
		optimize "on"
		links{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",
			"%{Library.SPIRV_Reflect_Release}",
			"%{Library.SPIRV_Cross_Reflect_Release}",
		}


	filter "configurations:Dist"
		defines "KD_DIST"
		runtime "Release"
		optimize "on"
		links{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",
			"%{Library.SPIRV_Reflect_Release}",
			"%{Library.SPIRV_Cross_Reflect_Release}",
		}