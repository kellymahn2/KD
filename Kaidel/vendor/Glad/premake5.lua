project "GLAD"
	kind "StaticLib"
	language "C"
	targetdir ("bin/"..outputdir.."/%{prj.name}")
	targetdir ("bin-int/"..outputdir.."/%{prj.name}")
	staticruntime "on"
	files{
		"include/glad/glad.h",
		"include/glad/vulkan.h",
		"include/KHR/khrplatform.h",
		"src/glad.c",
		"src/vulkan.c"

	}
	includedirs{
		"include"
	}
	filter "system:windows"
		systemversion "latest"
	filter "configurations:Debug"
		runtime "Debug"
		buildoptions "/MDd"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		buildoptions "/MD"
		optimize "on"