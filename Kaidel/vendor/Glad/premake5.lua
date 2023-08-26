project "GLAD"
	kind "StaticLib"
	language "C"
	targetdir ("bin/"..outputdir.."/%{prj.name}")
	targetdir ("bin-int/"..outputdir.."/%{prj.name}")
	staticruntime "on"
	files{
		"include/Glad/glad.h",
		"include/KHR/khrplatform.h",
		"src/glad.c"

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