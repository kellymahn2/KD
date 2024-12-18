project "KaidelEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/Kaidel/vendor/spdlog/include",
		"%{wks.location}/Kaidel/src",
		"%{wks.location}/KaidelEditor/src",
		"%{wks.location}/Kaidel/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.Assimp}"
	}

	links
	{
		"Kaidel"
	}
	defines{
		"YAML_CPP_STATIC_DEFINE",
	}
	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "KD_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "KD_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "KD_DIST"
		runtime "Release"
		optimize "on"