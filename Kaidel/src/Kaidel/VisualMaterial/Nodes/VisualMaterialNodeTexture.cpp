#include "KDpch.h"
#include "VisualMaterialNodeTexture.h"


namespace Kaidel {


	VisualMaterialNodeTexture2D::VisualMaterialNodeTexture2D()
	{
		Parameter.Type = VisualMaterialParameterType::Texture;
	}

	std::string_view VisualMaterialNodeTexture2D::GetCaption() const
	{
		return "Texture2D";
	}


	uint32_t VisualMaterialNodeTexture2D::GetConnectedInputPort(uint32_t port) const
	{
		return InputConnections[port];
	}


	uint32_t VisualMaterialNodeTexture2D::GetInputPortCount() const
	{
		return PORT_MAX;
	}


	std::string_view VisualMaterialNodeTexture2D::GetInputPortName(uint32_t port) const
	{
		static std::string_view Names[] = {"UV", "Sampler"};

		return Names[port];
	}

	Kaidel::VisualMaterialNodePortType VisualMaterialNodeTexture2D::GetInputPortType(uint32_t port) const
	{
		switch (port)
		{
		case PORT_UV: return VisualMaterialNodePortType::Vec2;
		case PORT_SAMPLER: return VisualMaterialNodePortType::Sampler;
		}
	}

	uint32_t VisualMaterialNodeTexture2D::GetOutputPortCount() const
	{
		return 5;
	}


	std::string_view VisualMaterialNodeTexture2D::GetOutputPortName(uint32_t port) const
	{
		static std::string_view Names[] = { "Color", "R", "G", "B", "A" };
		
		return Names[port];
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeTexture2D::GetOutputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::Vec4;
	}


	bool VisualMaterialNodeTexture2D::IsInputPortConnected(uint32_t port) const
	{
		return InputConnections[port] != -1;
	}


	void VisualMaterialNodeTexture2D::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		InputConnections[port] = connection;
	}


	std::string VisualMaterialNodeTexture2D::GenerateCode(const std::vector<std::string>& inputVars) const
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
		case Kaidel::VisualMaterialNodeTexture2D::SampleType::Data: sample = textureSample; break;
		case Kaidel::VisualMaterialNodeTexture2D::SampleType::Color: sample = fmt::format("pow({}, vec4({:.3f}))", textureSample, 2.2f); break;
		case Kaidel::VisualMaterialNodeTexture2D::SampleType::Normal: sample = fmt::format("vec4(normalize(2.0 * {}.rgb - 1.0), 1.0)", textureSample); break;
		}

		return fmt::format("vec4 {} = {};\n", NODE_PORT_NAME(0), sample) + EXPANDED_PORTS_DEFINE_VEC4(0);
	}


	Kaidel::Ref<Kaidel::Texture> VisualMaterialNodeTexture2D::GetDescriptorTexture(uint32_t index) const
	{
		return Image;
	}

	std::string_view VisualMaterialNodeSampler::GetCaption() const
	{
		return "Sampler";
	}


	uint32_t VisualMaterialNodeSampler::GetConnectedInputPort(uint32_t port) const
	{
		KD_ASSERT(false, "Does not have input ports");
		return -1;
	}


	uint32_t VisualMaterialNodeSampler::GetInputPortCount() const
	{
		return 0;
	}


	std::string_view VisualMaterialNodeSampler::GetInputPortName(uint32_t port) const
	{
		KD_ASSERT(false, "Does not have input ports");
		return "";
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeSampler::GetInputPortType(uint32_t port) const
	{
		KD_ASSERT(false, "Does not have input ports");
		return VisualMaterialNodePortType::Bool;
	}


	uint32_t VisualMaterialNodeSampler::GetOutputPortCount() const
	{
		return 1;
	}


	std::string_view VisualMaterialNodeSampler::GetOutputPortName(uint32_t port) const
	{
		return "Sampler";
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeSampler::GetOutputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::Sampler;
	}


	bool VisualMaterialNodeSampler::IsInputPortConnected(uint32_t port) const
	{
		KD_ASSERT(false, "Does not have input ports");
		return false;
	}


	void VisualMaterialNodeSampler::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		KD_ASSERT(false, "Does not have input ports");
	}


	std::string VisualMaterialNodeSampler::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		return "";
	}

}
