#include "KDpch.h"
#include "VisualShaderNodeTexture.h"
#include "Kaidel/Asset/AssetManager.h"
#include "Kaidel/Renderer/RendererGlobals.h"

#include "Kaidel/Core/BiMap.h"

namespace Kaidel {

	namespace Utils
	{
		static BiMap<SamplerFilter, std::string> s_SamplerFilterString =
		{
			{ SamplerFilter::Nearest, "Nearest" },
			{ SamplerFilter::Linear, "Linear" },
		};

		static BiMap<SamplerMipMapMode, std::string> s_SamplerMipMapModeString =
		{
			{ SamplerMipMapMode::Nearest, "Nearest" },
			{ SamplerMipMapMode::Linear, "Linear" },
		};

		static BiMap<SamplerAddressMode, std::string> s_SamplerAddressModeString =
		{
			{ SamplerAddressMode::Repeat, "Repeat" },
			{ SamplerAddressMode::MirroredRepeat, "MirroredRepeat" },
			{ SamplerAddressMode::ClampToEdge, "ClampToEdge" },
			{ SamplerAddressMode::ClampToBorder, "ClampToBorder" },
			{ SamplerAddressMode::MirrorClampToEdge, "MirrorClampToEdge" },
		};


		using SType = VisualShaderNodeTexture2D::SampleType;

		static BiMap<SType, std::string> s_SampleTypeString =
		{
			{ SType::Data, "Data" },
			{ SType::Color, "Color" },
			{ SType::Normal, "Normal" },
		};
		
	}

	std::string_view VisualShaderNodeSampler::GetCaption() const
	{
		return "Sampler";
	}


	uint32_t VisualShaderNodeSampler::GetConnectedInputPort(uint32_t port) const
	{
		KD_ASSERT(false, "Does not have input ports");
		return -1;
	}


	uint32_t VisualShaderNodeSampler::GetInputPortCount() const
	{
		return 0;
	}


	std::string_view VisualShaderNodeSampler::GetInputPortName(uint32_t port) const
	{
		KD_ASSERT(false, "Does not have input ports");
		return "";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeSampler::GetInputPortType(uint32_t port) const
	{
		KD_ASSERT(false, "Does not have input ports");
		return VisualShaderNodePortType::Bool;
	}


	uint32_t VisualShaderNodeSampler::GetOutputPortCount() const
	{
		return 1;
	}


	std::string_view VisualShaderNodeSampler::GetOutputPortName(uint32_t port) const
	{
		return "Sampler";
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeSampler::GetOutputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::Sampler;
	}


	bool VisualShaderNodeSampler::IsInputPortConnected(uint32_t port) const
	{
		KD_ASSERT(false, "Does not have input ports");
		return false;
	}


	void VisualShaderNodeSampler::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		KD_ASSERT(false, "Does not have input ports");
	}


	std::string VisualShaderNodeSampler::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		return "";
	}

	void VisualShaderNodeSampler::Serialize(YAML::Emitter& out) const
	{
		out << YAML::Key << "Filter" << YAML::Value << Utils::s_SamplerFilterString.At(Filter);

		out << YAML::Key << "MipFilter" << YAML::Value << Utils::s_SamplerMipMapModeString.At(MipFilter);

		out << YAML::Key << "AddressMode" << YAML::Value << Utils::s_SamplerAddressModeString.At(AddressMode);
	}

	void VisualShaderNodeSampler::Deserialize(YAML::Node& node)
	{
		Filter = Utils::s_SamplerFilterString.At(node["Filter"].as<std::string>());

		MipFilter = Utils::s_SamplerMipMapModeString.At(node["MipFilter"].as<std::string>());

		AddressMode = Utils::s_SamplerAddressModeString.At(node["AddressMode"].as<std::string>());
	}

	VisualShaderNodeTexture2D::VisualShaderNodeTexture2D()
	{
		Parameter.Type = VisualShaderParameterType::Texture;
		Image = RendererGlobals::GetSingleColorTexture(glm::vec4(0.0f));
	}

	std::string_view VisualShaderNodeTexture2D::GetCaption() const
	{
		return "Texture2D";
	}


	uint32_t VisualShaderNodeTexture2D::GetConnectedInputPort(uint32_t port) const
	{
		return InputConnections[port];
	}


	uint32_t VisualShaderNodeTexture2D::GetInputPortCount() const
	{
		return PORT_MAX;
	}


	std::string_view VisualShaderNodeTexture2D::GetInputPortName(uint32_t port) const
	{
		static std::string_view Names[] = {"UV", "Sampler"};

		return Names[port];
	}

	Kaidel::VisualShaderNodePortType VisualShaderNodeTexture2D::GetInputPortType(uint32_t port) const
	{
		switch (port)
		{
		case PORT_UV: return VisualShaderNodePortType::Vec2;
		case PORT_SAMPLER: return VisualShaderNodePortType::Sampler;
		}

		KD_CORE_ASSERT(false);
		return VisualShaderNodePortType::Bool;
	}

	uint32_t VisualShaderNodeTexture2D::GetOutputPortCount() const
	{
		return 5;
	}


	std::string_view VisualShaderNodeTexture2D::GetOutputPortName(uint32_t port) const
	{
		static std::string_view Names[] = { "Color", "R", "G", "B", "A" };
		
		return Names[port];
	}


	Kaidel::VisualShaderNodePortType VisualShaderNodeTexture2D::GetOutputPortType(uint32_t port) const
	{
		return VisualShaderNodePortType::Vec4;
	}


	bool VisualShaderNodeTexture2D::IsInputPortConnected(uint32_t port) const
	{
		return InputConnections[port] != -1;
	}


	void VisualShaderNodeTexture2D::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		InputConnections[port] = connection;
	}


	std::string VisualShaderNodeTexture2D::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		std::string samplerName = "SAMPLER_LINEAR_MIPPED_REPEAT";
		
		//Sampler is connected
		if (IsInputPortConnected(1))
		{
			samplerName = inputVars[1];
		}

		std::string uv;

		if (IsInputPortConnected(0))
		{
			uv = inputVars[0];
		}
		else
		{
			uv = "Input.TexCoords";
		}

		std::string descName;

		if (Parameter.Active)
			descName = Parameter.Name;
		else
			descName = NODE_DESCRIPTOR_NAME(NodeID, 0);

		std::string textureSample = fmt::format("texture(sampler2D({}, {}), {})", descName, samplerName, uv);

		std::string sample;

		switch (SamplingType)
		{
		case Kaidel::VisualShaderNodeTexture2D::SampleType::Data: sample = textureSample; break;
		case Kaidel::VisualShaderNodeTexture2D::SampleType::Color: sample = fmt::format("pow({}, vec4({:.3f}))", textureSample, 2.2f); break;
		case Kaidel::VisualShaderNodeTexture2D::SampleType::Normal: sample = fmt::format("vec4(normalize(2.0 * {}.rgb - 1.0), 1.0)", textureSample); break;
		}

		return fmt::format("vec4 {} = {};\n", NODE_PORT_NAME(0), sample) + EXPANDED_PORTS_DEFINE_VEC4(0);
	}


	Kaidel::Ref<Kaidel::SpriteSheet> VisualShaderNodeTexture2D::GetDescriptorTexture(uint32_t index) const
	{
		return Image;
	}

	void VisualShaderNodeTexture2D::Serialize(YAML::Emitter& out) const
	{
		out << YAML::Key << "SType" << YAML::Value << Utils::s_SampleTypeString.At(SamplingType);

		if (Image)
		{
			AssetManager::Wait();
			//out << YAML::Key << "Image" << YAML::Value << (uint64_t)Image->GetHandle();
		}
	}

	void VisualShaderNodeTexture2D::Deserialize(YAML::Node& node)
	{
		SamplingType = Utils::s_SampleTypeString.At(node["SType"].as<std::string>());
		
		if (auto image = node["Image"]; image)
		{
			AssetHandle handle = image.as<uint64_t>();

			if (AssetManager::IsAssetHandleValid(handle))
			{
				Image = AssetManager::GetAsset(handle).As<SpriteSheet>();
			}
		}
	}

	

}
