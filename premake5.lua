workspace "Kaidel"
	
	startproject "KaidelEditor"
	architecture "x64"
	configurations{
		"Debug",
		"Release",
		"Dist"
	}
	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	IncludeDir ={}
	IncludeDir["GLFW"]= "Kaidel/vendor/GLFW/include/"
	IncludeDir["GLAD"] = "Kaidel/vendor/Glad/include/"
	IncludeDir["ImGui"] = "Kaidel/vendor/imgui/"
	IncludeDir["EnTT"] = "Kaidel/vendor/EnTT/"
	IncludeDir["ImGuizmo"] = "Kaidel/vendor/ImGuizmo/"
	IncludeDir["Yaml_cpp"] = "Kaidel/vendor/yaml-cpp/include/"
	IncludeDir["KaidelUI"] = "KaidelUI/src/"
	IncludeDir["Box2D"] = "Kaidel/vendor/Box2D/include/"
	IncludeDir["mono"] = "Kaidel/vendor/mono/include"
	LibraryDir = {}
	LibraryDir["mono"] = "vendor/mono/Lib/%{cfg.buildcfg}"
	Library = {}
	Library["mono"] = "%{LibraryDir.mono}/libmono-static-sgen.lib"
	Library["WinSock"] = "Ws2_32.lib"
	Library["Version"] = "Version.lib"
	Library["Bcrypt"] = "Bcrypt.lib"
	Library["Winmm"] = "Winmm.lib"

	group "Dependencies"
	include "Kaidel/vendor/GLFW"
	include "Kaidel/vendor/Glad"
	include "Kaidel/vendor/imgui"
	include "Kaidel/vendor/yaml-cpp"
	include "Kaidel/vendor/Box2D"
	group ""
	include "KaidelCore"
	project "Kaidel"
		location "Kaidel"
		kind "StaticLib"
		staticruntime "on"
		language "C++"
		targetdir("bin/"..outputdir.."/%{prj.name}")
		objdir("bin-int/"..outputdir.."/%{prj.name}")
		pchheader "KDpch.h"
		pchsource "Kaidel/src/KDpch.cpp"
		files{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp",
			"%{prj.name}/vendor/glm/**.hpp",
			"%{prj.name}/vendor/stb_image/stb_image.h",
			"%{prj.name}/vendor/stb_image/stb_image.cpp",
			"%{prj.name}/vendor/glm/**.inl",
			"%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
			"%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp"
		}
		includedirs {
			"%{prj.name}/src",
			"%{prj.name}/vendor",
			"%{prj.name}/vendor/spdlog/include",
			"%{IncludeDir.ImGui}",
			"%{IncludeDir.GLAD}",
			"Kaidel/vendor/glm/",
			"Kaidel/vendor/glm",
			"%{IncludeDir.EnTT}",
			"Kaidel/vendor/stb_image",
			"%{IncludeDir.GLFW}",
			"%{IncludeDir.ImGuizmo}",
			"%{IncludeDir.Yaml_cpp}",
			"%{IncludeDir.Box2D}",	
			"%{IncludeDir.mono}"
		}
		links{
			"GLFW",
			"GLAD",
			"ImGui",
			"Box2D",
			"%{Library.mono}",
			"yaml-cpp",
			
			"opengl32.lib"
		}

		cppdialect "C++17"
		systemversion "latest"
		defines{
			"DLLEXPORT",
			"GLFW_INCLUDE_NONE",
			"_CRT_SECURE_NO_WARNINGS"
		}
		filter "system:windows"
			links{
				"%{Library.WinSock}",
				"%{Library.Version}",
				"%{Library.Bcrypt}",
				"%{Library.Winmm}"
			}
		filter "files:%{prj.name}/vendor/imguizmo/ImGuizmo.cpp"
		flags "NoPCH"
	filter "configurations:Debug"
		defines {"KAIDEL_DEBUG","KAIDEL_PROFILE","KD_ENABLE_ASSERTS"}
		runtime "Debug"
		buildoptions "/MDd"
		symbols "on"

	filter "configurations:Release"
		defines "KAIDEL_RELEASE"
		runtime "Release"
		buildoptions "/MD"
		optimize "on"

	filter "configurations:Dist"
		defines "KAIDEL_DIST"
		runtime "Release"
		buildoptions "/MD"
		optimize "on"
project "KaidelEditor"
	location "KaidelEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	targetdir("bin/"..outputdir.."/%{prj.name}")
		objdir("bin-int/"..outputdir.."/%{prj.name}")
		files{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp"
		}
		includedirs {
			"Kaidel/src",
			"Kaidel/vendor",
			"Kaidel/vendor/spdlog/include",
			"%{IncludeDir.ImGui}",
			"Kaidel/vendor/glm",
			"%{IncludeDir.EnTT}",

		}
		systemversion "latest"
		defines{
			"DLLIMPORT"
		}
		links {
			"Kaidel"
		}
	filter "configurations:Debug"
		defines {"KAIDEL_DEBUG","KAIDEL_PROFILE","KD_ENABLE_ASSERTS"}
		runtime "Debug"
		buildoptions "/MDd"
		symbols "on"

	filter "configurations:Release"
		defines "KAIDEL_RELEASE"
		runtime "Release"
		buildoptions "/MD"
		optimize "on"

	filter "configurations:Dist"
		defines "KAIDEL_DIST"
		runtime "Release"
		buildoptions "/MD"
		optimize "on"
project "SandBox"
	location "SandBox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
		targetdir("bin/"..outputdir.."/%{prj.name}")
		objdir("bin-int/"..outputdir.."/%{prj.name}")
		files{
			"%{prj.name}/src/**.h",
			"%{prj.name}/src/**.cpp"
		}
		includedirs {
			"Kaidel/src",
			"Kaidel/vendor",
			"%{IncludeDir.ImGui}",
			"Kaidel/vendor/glm"

		}
		systemversion "latest"
		defines{
			"DLLIMPORT"
		}
		links {
			"Kaidel"
		}
	filter "configurations:Debug"
		defines {"KAIDEL_DEBUG","KAIDEL_PROFILE"}
		runtime "Debug"
		buildoptions "/MDd"
		symbols "on"

	filter "configurations:Release"
		defines "KAIDEL_RELEASE"
		runtime "Release"
		buildoptions "/MD"
		optimize "on"

	filter "configurations:Dist"
		defines "KAIDEL_DIST"
		runtime "Release"
		buildoptions "/MD"
		optimize "on"






	