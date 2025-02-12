include "Dependencies.lua"

workspace "KD"
	architecture "x86_64"
	startproject "KaidelEditor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	flags
	{
		"MultiProcessorCompile"
	}
	buildoptions{
		"/wd4005"
	}
	linkoptions{
		"/IGNORE:4006",
		"/IGNORE:4099",
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "Kaidel/vendor/Box2D"
	include "Kaidel/vendor/GLFW"
	include "Kaidel/vendor/Glad"
	include "Kaidel/vendor/imgui"
	include "Kaidel/vendor/msdf-atlas-gen"
	include "Kaidel/vendor/yaml-cpp"
group ""

group "Core"
	include "Kaidel"
	include "KaidelCore"
group ""

group "Tools"
	include "KaidelEditor"
group ""
