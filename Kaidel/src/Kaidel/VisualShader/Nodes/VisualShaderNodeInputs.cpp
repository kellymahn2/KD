#include "KDpch.h"
#include "VisualShaderNodeInputs.h"

#include "Kaidel/Core/BiMap.h"

namespace Kaidel {

	namespace Utils
	{
		BiMap<InputType, std::string> s_InputTypeString =
		{
			{ InputType::FragPos, "FragPos" },
			{ InputType::Tangent, "Tangent" },
			{ InputType::Normal, "Normal" },
			{ InputType::Bitangent, "Bitangent" },
			{ InputType::TexCoords, "TexCoords" },
			{ InputType::CameraPos, "CameraPos" },
			{ InputType::ScreenSize, "ScreenSize" },
			{ InputType::ScreenUV, "ScreenUV" },
			{ InputType::Time, "Time" },
			{ InputType::DeltaTime, "DeltaTime" },
		};
	}


	std::string VisualShaderNodeInput::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		std::string_view type;


		switch (Type)
		{
		case Kaidel::InputType::FragPos:
		case Kaidel::InputType::Tangent:
		case Kaidel::InputType::Normal:
		case Kaidel::InputType::CameraPos:
		case Kaidel::InputType::Bitangent: type = "vec3"; break;
		case Kaidel::InputType::ScreenSize:
		case Kaidel::InputType::ScreenUV:
		case Kaidel::InputType::TexCoords: type = "vec2"; break;
		case Kaidel::InputType::Time:
		case Kaidel::InputType::DeltaTime: type = "float"; break;
		}

		std::string_view inputName;

		switch (Type)
		{
		case Kaidel::InputType::FragPos: inputName = "Input.FragPos"; break;
		case Kaidel::InputType::Tangent: inputName = "Input.T"; break;
		case Kaidel::InputType::Normal: inputName = "Input.N"; break;
		case Kaidel::InputType::Bitangent: inputName = "Input.B"; break;
		case Kaidel::InputType::TexCoords: inputName = "Input.TexCoords"; break;
		case Kaidel::InputType::CameraPos: inputName = "u_SceneData.CameraPos"; break;
		case Kaidel::InputType::ScreenSize: inputName = "u_SceneData.ScreenSize"; break;
		case Kaidel::InputType::ScreenUV: inputName = "gl_FragCoord.xy / vec2(u_SceneData.ScreenSize)"; break;
		case Kaidel::InputType::Time: inputName = "u_SceneData.Time"; break;
		case Kaidel::InputType::DeltaTime: inputName = "u_SceneData.DeltaTime"; break;
		}

		return fmt::format("{} {} = {};\n", type, NODE_PORT_NAME(0), inputName);
	}


	std::string_view VisualShaderNodeInput::GetCaption() const
	{
		return "Shader input";
	}


	uint32_t VisualShaderNodeInput::GetConnectedInputPort(uint32_t port) const
	{
		KD_ASSERT(false, "Constants don't have inputs");
		return -1;
	}


	uint32_t VisualShaderNodeInput::GetInputPortCount() const
	{
		return 0;
	}


	std::string_view VisualShaderNodeInput::GetInputPortName(uint32_t port) const
	{
		KD_ASSERT(false, "Constants don't have inputs");
		return "";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeInput::GetInputPortType(uint32_t port) const
	{
		KD_ASSERT(false, "Constants don't have inputs");
		return VisualShaderNodePortType::Bool;
	}


	uint32_t VisualShaderNodeInput::GetOutputPortCount() const
	{
		return 1;
	}


	std::string_view VisualShaderNodeInput::GetOutputPortName(uint32_t port) const
	{
		static std::string_view Names[] =
		{
			"Position",
			"Tangent",
			"Normal",
			"Bitangent",
			"UV",
			"ViewPos",
			"Resolution",
			"ScreenUV",
			"Time",
			"DeltaTime"
		};

		return Names[(uint32_t)Type];
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeInput::GetOutputPortType(uint32_t port) const
	{
		switch (Type)
		{
		case Kaidel::InputType::FragPos: 
		case Kaidel::InputType::Tangent: 
		case Kaidel::InputType::Normal:
		case Kaidel::InputType::CameraPos:
		case Kaidel::InputType::Bitangent: return VisualShaderNodePortType::Vec3;
		case Kaidel::InputType::ScreenSize:
		case Kaidel::InputType::ScreenUV:
		case Kaidel::InputType::TexCoords: return VisualShaderNodePortType::Vec2;
		case Kaidel::InputType::Time: 
		case Kaidel::InputType::DeltaTime: return VisualShaderNodePortType::Float;
		}
		return VisualShaderNodePortType::Bool;
	}


	bool VisualShaderNodeInput::IsInputPortConnected(uint32_t port) const
	{
		KD_ASSERT(false, "Constants don't have inputs");
		return false;
	}


	void VisualShaderNodeInput::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		KD_ASSERT(false, "Constants don't have inputs");
	}

	void VisualShaderNodeInput::Serialize(YAML::Emitter& out) const
	{
		out << YAML::Key << "IType" << YAML::Value << Utils::s_InputTypeString.At(Type);
 	}

	void VisualShaderNodeInput::Deserialize(YAML::Node& node)
	{
		Type = Utils::s_InputTypeString.At(node["IType"].as<std::string>());
	}

}
