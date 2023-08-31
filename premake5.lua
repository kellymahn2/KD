include "Dependencies.lua"

workspace "Kaidel"
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

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "vendor/premake"
	include "Kaidel/vendor/Box2D"
	include "Kaidel/vendor/GLFW"
	include "Kaidel/vendor/Glad"
	include "Kaidel/vendor/imgui"
	include "Kaidel/vendor/yaml-cpp"
group ""

include "Kaidel"
include "KaidelEditor"