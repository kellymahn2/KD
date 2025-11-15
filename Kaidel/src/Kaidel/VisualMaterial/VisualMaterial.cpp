#include "KDpch.h"

#include "VisualMaterial.h"

#include "Kaidel/Renderer/Renderer3D.h"

#include <unordered_set>

namespace std {
	template<>
	struct hash<Kaidel::PortCacheID>
	{
		size_t operator()(const Kaidel::PortCacheID& _Keyval) const { return std::hash<uint32_t>()(_Keyval.Node); }
	};
}


namespace Kaidel
{
	namespace Utils {
		static uint32_t GetBaseAlignmentForParameterType(VisualMaterialParameterType type)
		{
			switch (type)
			{
			case Kaidel::VisualMaterialParameterType::Bool: return 1;
			case Kaidel::VisualMaterialParameterType::Int: return 4;
			case Kaidel::VisualMaterialParameterType::UInt: return 4;
			case Kaidel::VisualMaterialParameterType::Float: return 4;
			case Kaidel::VisualMaterialParameterType::Vec2: return 8;
			case Kaidel::VisualMaterialParameterType::Vec3: return 16;
			case Kaidel::VisualMaterialParameterType::Vec4: return 16;
			}
		}

		static uint32_t GetSizeForParameterType(VisualMaterialParameterType type)
		{
			switch (type)
			{
			case Kaidel::VisualMaterialParameterType::Bool: return 1;
			case Kaidel::VisualMaterialParameterType::Int: return 4;
			case Kaidel::VisualMaterialParameterType::UInt: return 4;
			case Kaidel::VisualMaterialParameterType::Float: return 4;
			case Kaidel::VisualMaterialParameterType::Vec2: return 8;
			case Kaidel::VisualMaterialParameterType::Vec3: return 12;
			case Kaidel::VisualMaterialParameterType::Vec4: return 16;
			}
		}
	}
	
	using VariableNameMap = std::unordered_set<PortCacheID>;

	static void GenerateCodeForNode(VisualMaterial& material, uint32_t nodeID, VisualMaterial::CodeSegment& code, VariableNameMap& nameMap);
	static void GenerateGlobalCodeForNode(VisualMaterial& material, uint32_t nodeID, VisualMaterial::CodeSegment& code, VariableNameMap& nameMap, uint32_t& currentSet, uint32_t& currentBinding);

	const char* VisualMaterialNodeTypeToName(VisualMaterialNodeType type)
	{
		switch (type)
		{
		case VisualMaterialNodeType::ConstantBool: return "Boolean Constant";
		case VisualMaterialNodeType::ConstantInt: return "Int Constant";
		case VisualMaterialNodeType::ConstantUInt: return "UInt Constant";
		case VisualMaterialNodeType::ConstantFloat: return "Float Constant";
		case VisualMaterialNodeType::ConstantVec2: return "Vec2 Constant";
		case VisualMaterialNodeType::ConstantVec3: return "Vec3 Constant";
		case VisualMaterialNodeType::ConstantVec4: return "Vec4 Constant";
		case VisualMaterialNodeType::IntOp: return "Int Op";
		case VisualMaterialNodeType::UIntOp: return "UInt Op";
		case VisualMaterialNodeType::FloatOp: return "Float Op";
		case VisualMaterialNodeType::VectorOp: return "Vector Op";
		case VisualMaterialNodeType::IntFunc: return "Int Func";
		case VisualMaterialNodeType::UIntFunc: return "UInt Func";
		case VisualMaterialNodeType::FloatFunc: return "Float Func";
		case VisualMaterialNodeType::VectorFunc: return "Vector Func";
		case VisualMaterialNodeType::FragmentOutput: return "Output";
		default:
			break;
		}

		return "";
	}


	std::string_view VisualMaterialNodeFragmentOutput::GetCaption() const
	{
		return "Fragment";
	}


	uint32_t VisualMaterialNodeFragmentOutput::GetConnectedInputPort(uint32_t port) const
	{
		return InputConnections[port];
	}


	uint32_t VisualMaterialNodeFragmentOutput::GetInputPortCount() const
	{
		return ARRAYSIZE(InputConnections);
	}


	std::string_view VisualMaterialNodeFragmentOutput::GetInputPortName(uint32_t port) const
	{
		static std::string_view Names[] =
		{
			"Albedo",
			"Metallic",
			"Roughness",
			"Emissive",
			"Normal"
		};

		return Names[port];
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeFragmentOutput::GetInputPortType(uint32_t port) const
	{
		static VisualMaterialNodePortType Types[] =
		{
			VisualMaterialNodePortType::Vec3,
			VisualMaterialNodePortType::Float,
			VisualMaterialNodePortType::Float,
			VisualMaterialNodePortType::Vec3,
			VisualMaterialNodePortType::Vec3,
		};

		return Types[port];
	}


	uint32_t VisualMaterialNodeFragmentOutput::GetOutputPortCount() const
	{
		return 0;
	}


	std::string_view VisualMaterialNodeFragmentOutput::GetOutputPortName(uint32_t port) const
	{
		return {};
	}


	Kaidel::VisualMaterialNodePortType VisualMaterialNodeFragmentOutput::GetOutputPortType(uint32_t port) const
	{
		return VisualMaterialNodePortType::Bool;
	}


	bool VisualMaterialNodeFragmentOutput::IsInputPortConnected(uint32_t port) const
	{
		return InputConnections[port] != -1;
	}


	void VisualMaterialNodeFragmentOutput::SetConnectedInputPort(uint32_t port, uint32_t connection)
	{
		InputConnections[port] = connection;
	}



	std::string VisualMaterialNodeFragmentOutput::GenerateCode(const std::vector<std::string>& inputVars) const
	{
		std::string_view Names[] = { "Albedo", "Metallic", "Roughness", "Emissive", "Normal" };

		std::string code;

		for (uint32_t i = 0; i < inputVars.size(); ++i)
		{
			if(inputVars[i].empty())
				continue;
			code += fmt::format("{} = {};\n", Names[i], inputVars[i]);
		}
		return code;
	}

	//technically could be an expression
	static void GetCodeForNodeInput(VisualMaterial& material, uint32_t nodeID, uint32_t inputPort, VisualMaterial::CodeSegment& code, VariableNameMap& nameMap)
	{
		VisualMaterialNode& node = *material.Nodes[nodeID];
	
		uint32_t connectionID = node.GetConnectedInputPort(inputPort);
		
		VisualMaterialNodeConnection& connection = material.Connections[connectionID];
	
		PortCacheID portCacheID(connection.FromNode, 0);
	
		if (auto it = nameMap.find(portCacheID); it != nameMap.end())
			return;
	
		GenerateCodeForNode(material, connection.FromNode, code, nameMap);
	}

	static void GetGlobalCodeForNodeInput(VisualMaterial& material, uint32_t nodeID, uint32_t inputPort, VisualMaterial::CodeSegment& code, VariableNameMap& nameMap, uint32_t& currentSet, uint32_t& currentBinding)
	{
		VisualMaterialNode& node = *material.Nodes[nodeID];

		uint32_t connectionID = node.GetConnectedInputPort(inputPort);

		VisualMaterialNodeConnection& connection = material.Connections[connectionID];

		PortCacheID portCacheID(connection.FromNode, 0);

		if (auto it = nameMap.find(portCacheID); it != nameMap.end())
			return;

		GenerateGlobalCodeForNode(material, connection.FromNode, code, nameMap, currentSet, currentBinding);
	}

	static std::string CastFromPort(VisualMaterialNodePortType input, VisualMaterialNodePortType output, const std::string& srcVar)
	{
		if (input == output)
			return srcVar;

		switch (input) {
		case VisualMaterialNodePortType::Float: {
			switch (output) {
			case VisualMaterialNodePortType::Int: {
				return "float(" + srcVar + ")";
			} break;
			case VisualMaterialNodePortType::UInt: {
				return "float(" + srcVar + ")";
			} break;
			case VisualMaterialNodePortType::Bool: {
				return "(" + srcVar + " ? 1.0 : 0.0)";
			} break;
			case VisualMaterialNodePortType::Vec2: {
				return srcVar + ".x";
			} break;
			case VisualMaterialNodePortType::Vec3: {
				return srcVar + ".x";
			} break;
			case VisualMaterialNodePortType::Vec4: {
				return srcVar + ".x";
			} break;
			}
		} break;
		case VisualMaterialNodePortType::Int: {
			switch (output) {
			case VisualMaterialNodePortType::Float: {
				return "int(" + srcVar + ")";
			} break;
			case VisualMaterialNodePortType::UInt: {
				return "int(" + srcVar + ")";
			} break;
			case VisualMaterialNodePortType::Bool: {
				return "(" + srcVar + " ? 1 : 0)";
			} break;
			case VisualMaterialNodePortType::Vec2: {
				return "int(" + srcVar + ".x)";
			} break;
			case VisualMaterialNodePortType::Vec3: {
				return "int(" + srcVar + ".x)";
			} break;
			case VisualMaterialNodePortType::Vec4: {
				return "int(" + srcVar + ".x)";
			} break;
			}
		} break;
		case VisualMaterialNodePortType::UInt: {
			switch (output) {
			case VisualMaterialNodePortType::Float: {
				return "uint(" + srcVar + ")";
			} break;
			case VisualMaterialNodePortType::Int: {
				return "uint(" + srcVar + ")";
			} break;
			case VisualMaterialNodePortType::Bool: {
				return "(" + srcVar + " ? 1u : 0u)";
			} break;
			case VisualMaterialNodePortType::Vec2: {
				return "uint(" + srcVar + ".x)";
			} break;
			case VisualMaterialNodePortType::Vec3: {
				return "uint(" + srcVar + ".x)";
			} break;
			case VisualMaterialNodePortType::Vec4: {
				return "uint(" + srcVar + ".x)";
			} break;
			}
		} break;
		case VisualMaterialNodePortType::Bool: {
			switch (output) {
			case VisualMaterialNodePortType::Float: {
				return srcVar + " == 0.0 ? true : false";
			} break;
			case VisualMaterialNodePortType::Int: {
				return srcVar + " == 0 ? true : false";
			} break;
			case VisualMaterialNodePortType::UInt: {
				return srcVar + " == 0u ? true : false";
			} break;
			case VisualMaterialNodePortType::Vec2: {
				return "all(bvec2(" + srcVar + "))";
			} break;
			case VisualMaterialNodePortType::Vec3: {
				return "all(bvec3(" + srcVar + "))";
			} break;
			case VisualMaterialNodePortType::Vec4: {
				return "all(bvec4(" + srcVar + "))";
			} break;
			}
		} break;
		case VisualMaterialNodePortType::Vec2: {
			switch (output) {
			case VisualMaterialNodePortType::Float: {
				return "vec2(" + srcVar + ")";
			} break;
			case VisualMaterialNodePortType::Int: {
				return "vec2(float(" + srcVar + "))";
			} break;
			case VisualMaterialNodePortType::UInt: {
				return "vec2(float(" + srcVar + "))";
			} break;
			case VisualMaterialNodePortType::Bool: {
				return "vec2(" + srcVar + " ? 1.0 : 0.0)";
			} break;
			case VisualMaterialNodePortType::Vec3:
			case VisualMaterialNodePortType::Vec4: {
				return "vec2(" + srcVar + ".xy)";
			} break;
			}
		} break;

		case VisualMaterialNodePortType::Vec3: {
			switch (output) {
			case VisualMaterialNodePortType::Float: {
				return "vec3(" + srcVar + ")";
			} break;
			case VisualMaterialNodePortType::Int: {
				return "vec3(float(" + srcVar + "))";
			} break;
			case VisualMaterialNodePortType::UInt: {
				return "vec3(float(" + srcVar + "))";
			} break;
			case VisualMaterialNodePortType::Bool: {
				return "vec3(" + srcVar + " ? 1.0 : 0.0)";
			} break;
			case VisualMaterialNodePortType::Vec2: {
				return "vec3(" + srcVar + ", 0.0)";
			} break;
			case VisualMaterialNodePortType::Vec4: {
				return "vec3(" + srcVar + ".xyz)";
			} break;
			}
		} break;
		case VisualMaterialNodePortType::Vec4: {
			switch (output) {
			case VisualMaterialNodePortType::Float: {
				return "vec4(" + srcVar + ")";
			} break;
			case VisualMaterialNodePortType::Int: {
				return "vec4(float(" + srcVar + "))";
			} break;
			case VisualMaterialNodePortType::UInt: {
				return "vec4(float(" + srcVar + "))";
			} break;
			case VisualMaterialNodePortType::Bool: {
				return "vec4(" + srcVar + " ? 1.0 : 0.0)";
			} break;
			case VisualMaterialNodePortType::Vec2: {
				return "vec4(" + srcVar + ", 0.0, 0.0)";
			} break;
			case VisualMaterialNodePortType::Vec3: {
				return "vec4(" + srcVar + ", 0.0)";
			} break;
			default:
				break;
			}
		} break;
		}
	}

	static std::string GetConnectionCode(VisualMaterial& material, uint32_t nodeID, uint32_t inputPort, const VariableNameMap& nameMap)
	{
		VisualMaterialNode& output = *material.Nodes[nodeID];

		uint32_t connectionID = output.GetConnectedInputPort(inputPort);

		VisualMaterialNodeConnection& connection = material.Connections[connectionID];

		PortCacheID portCacheID(connection);


		VisualMaterialNode& input = *material.Nodes[connection.FromNode];
		
		std::string srcVar = NODE_PORT_NAME_OFFSET(input.PortOffset, connection.FromPort);

		return CastFromPort(output.GetInputPortType(inputPort), input.GetOutputPortType(connection.FromPort), srcVar);
	}

	//Returns the variable name storing the calculations of this node
	static void GenerateCodeForNode(VisualMaterial& material, uint32_t nodeID, VisualMaterial::CodeSegment& code, VariableNameMap& nameMap)
	{
		VisualMaterialNode& node = *material.Nodes[nodeID];


		//Resolve inputs
		for (uint32_t i = 0; i < node.GetInputPortCount(); ++i)
		{
			if (node.IsInputPortConnected(i))
			{
				GetCodeForNodeInput(material, nodeID, i, code, nameMap);
			}
		}

		//Output code
		std::vector<std::string> inputVars;
		inputVars.resize(node.GetInputPortCount());

		for (uint32_t i = 0; i < node.GetInputPortCount(); ++i)
		{
			if (node.IsInputPortConnected(i))
			{
				VisualMaterialNodeConnection& connection = material.Connections[node.GetConnectedInputPort(i)];

				inputVars[i] = GetConnectionCode(material, nodeID, i, nameMap);
			}
		}

		std::string nodeCode = node.GenerateCode(inputVars);

		if(!nodeCode.empty())
		{
			code.MainCode += fmt::format("\t// {}\n", node.GetCaption());

			std::size_t offset = 0;
			while (true)
			{
				nodeCode.insert(offset, 1, '\t');
				offset = nodeCode.find('\n', offset + 1) + 1;

				if (offset == nodeCode.length())
					break;
			}
		}

		code.MainCode += nodeCode;
		nameMap.insert(PortCacheID(nodeID, 0));
	}

	static void GenerateGlobalCodeForNode(VisualMaterial& material, uint32_t nodeID, VisualMaterial::CodeSegment& code, VariableNameMap& nameMap, uint32_t& currentSet, uint32_t& currentBinding)
	{
		VisualMaterialNode& node = *material.Nodes[nodeID];


		//Resolve inputs
		for (uint32_t i = 0; i < node.GetInputPortCount(); ++i)
		{
			if (node.IsInputPortConnected(i))
			{
				GetGlobalCodeForNodeInput(material, nodeID, i, code, nameMap, currentSet, currentBinding);
			}
		}

		if (node.Parameter.Type != VisualMaterialParameterType::None)
		{
			std::string decl;

			bool isTexture = node.Parameter.Type == VisualMaterialParameterType::Texture;

			if (isTexture)
			{
				if(node.Parameter.Active)
					decl = "texture2D " + node.Parameter.Name;
				else
					decl = "texture2D " + NODE_DESCRIPTOR_NAME(node.NodeID, 0);

				code.TextureCode += fmt::format("layout(set = {}, binding = {}) uniform {};\n", currentSet, currentBinding, decl);

				if (node.Parameter.Active)
				{
					material.DefaultTextureValues[node.Parameter.Name] = node.GetDescriptorTexture(0);
					auto& param = material.Parameters[node.Parameter.Name];
					param.Type = node.Parameter.Type;
					param.Set = currentSet;
					param.Binding = currentBinding;
				}
				else
				{
					material.DefaultTextureValues[NODE_DESCRIPTOR_NAME(node.NodeID, 0)] = node.GetDescriptorTexture(0);
				}

				++currentBinding;

				if (currentBinding == 16)
				{
					++currentSet;
					currentBinding = 0;
				}
			}
			else if (node.Parameter.Active)
			{
				std::string type;

				switch (node.Parameter.Type)
				{
				case VisualMaterialParameterType::Bool: type = "bool"; break;
				case VisualMaterialParameterType::Int: type = "int"; break;
				case VisualMaterialParameterType::UInt: type = "uint"; break;
				case VisualMaterialParameterType::Float: type = "float"; break;
				case VisualMaterialParameterType::Vec2: type = "vec2"; break;
				case VisualMaterialParameterType::Vec3: type = "vec3"; break;
				case VisualMaterialParameterType::Vec4: type = "vec4"; break;
				}

				decl = fmt::format("{} {}", type, node.Parameter.Name);

				if (code.Uniforms.empty())
				{
					code.UniformSet = currentSet;
					code.UniformBinding = currentBinding;

					++currentBinding;

					if (currentBinding == 16)
					{
						++currentSet;
						currentBinding = 0;
					}
				}

				code.Uniforms.emplace_back(node.Parameter.Type, fmt::format("\t{};\n", decl));

				auto& param = material.Parameters[node.Parameter.Name];
				param.Type = node.Parameter.Type;
			}
		}

		nameMap.insert(PortCacheID(nodeID, 0));
	}

	void VisualMaterial::Recompile()
	{
		Parameters.clear();

		Code = GenerateCode();

		Shader = ShaderLibrary::CompileFromSource(Code.Code);


		if (!Code.Uniforms.empty())
		{
			const DescriptorReflection& descriptor = Shader->GetReflection().Sets.at(Code.UniformSet).Bindings.at(Code.UniformBinding).Descriptor;

			UniformLayout.Size = descriptor.Uniform.Size;
			UniformLayout.Set = Code.UniformSet;
			UniformLayout.Binding = Code.UniformBinding;

			for (auto& [name, offset] : descriptor.Uniform.Elements)
			{
				KD_ASSERT(Parameters.find(name) != Parameters.end());
				Parameters.at(name).ByteOffset = offset;
			}
		}

		{
			GraphicsPipelineSpecification specs;

			specs.Input.Bindings.push_back(VertexInputBinding({
				{"a_Position",Format::RGB32F},
				{"a_TexCoords",Format::RG32F},
				{"a_Normal",Format::RGB32F},
				{"a_Tangent",Format::RGB32F},
				{"a_BiTangent",Format::RGB32F},
				}));
			specs.Multisample.Samples = TextureSamples::x1;
			specs.Primitive = PrimitiveTopology::TriangleList;
			specs.Rasterization.FrontCCW = true;
			specs.Rasterization.CullMode = PipelineCullMode::Front;
			specs.Shader = Shader;
			specs.RenderPass = Renderer3D::GetDeferredPassRenderPass();
			specs.Subpass = 0;
			specs.DepthStencil.DepthTest = true;
			specs.DepthStencil.DepthWrite = true;
			specs.DepthStencil.DepthCompareOperator = CompareOp::Less;

			Pipeline = GraphicsPipeline::Create(specs);
		}

		++Version;
	}


	VisualMaterial::VisualMaterial()
	{
		CreateNode<VisualMaterialNodeFragmentOutput>();
	}

	VisualMaterial::CodeSegment VisualMaterial::GenerateCode()
	{
		CodeSegment codeSegment;
		codeSegment = {};

		codeSegment.MainCode += "\to_Position = vec4(Input.FragPos, 1.0);\n"
			"\tvec3 Normal = vec3(0.0, 0.0, 1.0), Albedo = vec3(1.0), Emissive = vec3(0.0);\n"
			"\tfloat Metallic = 0.0, Roughness = 1.0;\n";

		GenerateCodeForNode(*this, 0, codeSegment, std::unordered_set<PortCacheID>());

		codeSegment.MainCode += "\to_Albedo = vec4(Albedo, 1.0); o_Normal = vec4(normalize(mat3(Input.T, Input.B, Input.N) * Normal), 1.0); o_Emissive = vec4(Emissive, 1.0); o_MetallicRoughness = vec2(Metallic, Roughness);\n";

		uint32_t currentSet = 3;
		uint32_t currentBinding = 0;
		GenerateGlobalCodeForNode(*this, 0, codeSegment, std::unordered_set<PortCacheID>(), currentSet, currentBinding);

		
		std::string code;

		code += "#version 460 core\n"
			"#include \"SceneData.glsli\"\n"
			"#include \"Sampler.glsli\"\n"
			"#include \"globals.glsli\"\n";

		code += "layout(set = 1, binding = 0) uniform _SceneData\n"
			"{\n"
			"	SceneData u_SceneData;\n"
			"};\n"
			"\n";

		code +=	"#ifdef vertex\n"
				"layout(location = 0)in vec3 a_Position;\n"
				"layout(location = 1)in vec2 a_TexCoords;\n"
				"layout(location = 2)in vec3 a_Normal;\n"
				"layout(location = 3)in vec3 a_Tangent;\n"
				"layout(location = 4)in vec3 a_BiTangent;\n"
				"\n"
				"layout(push_constant) uniform DrawData {\n"
				"	uint InstanceOffset;\n"
				"};\n"
				"\n"
				"layout(set = 2, binding = 0) buffer _InstanceData\n"
				"{\n"
				"	mat4 InstanceTransform[];\n"
				"};\n"
				"\n"
				"layout(location = 0) out VS_OUT {\n"
				"	vec3 FragPos;\n"
				"	vec3 T;\n"
				"	vec3 N;\n"
				"	vec3 B;\n"
				"	vec2 TexCoords;\n"
				"} Output;\n"
				"\n"
				"void main() {\n"
				"	mat4 transform = InstanceTransform[gl_InstanceIndex + InstanceOffset];\n"
				"\n"
				"	vec4 pos = transform * vec4(a_Position, 1.0);\n"
				"\n"
				"	gl_Position = u_SceneData.ViewProj * pos;\n"
				"\n"
				"	mat3 normalMat = mat3(transform);\n"
				"\n"
				"	Output.FragPos = vec3(pos);\n"
				"	Output.T = normalize(normalMat * a_Tangent);\n"
				"	Output.B = normalize(normalMat * a_BiTangent);\n"
				"	Output.N = normalize(normalMat * a_Normal);\n"
				"	Output.T = Output.T - (dot(Output.T, Output.N)) * Output.N;\n"
				"	Output.B = Output.B - (dot(Output.B, Output.N)) * Output.N - (dot(Output.B, Output.T)) * Output.T;\n"
				"	Output.TexCoords = a_TexCoords;\n"
				"\n"
				"}\n"
				"#endif\n";

		code += "\n";

		code += "#ifdef fragment\n"
			"layout (location = 0) out vec4 o_Position;\n"
			  "layout (location = 1) out vec4 o_Normal;\n"
			  "layout (location = 2) out vec4 o_Albedo;\n"
			  "layout (location = 3) out vec2 o_MetallicRoughness;\n"
			  "layout (location = 4) out vec4 o_Emissive;\n";
		code +=
			"layout(location = 0) in VS_OUT{\n"
			"	vec3 FragPos;\n"
			"	vec3 T;\n"
			"	vec3 N;\n"
			"	vec3 B;\n"
			"	vec2 TexCoords;\n"
			"} Input;\n";

		code += codeSegment.TextureCode;

		if (!codeSegment.Uniforms.empty())
		{
			std::sort(codeSegment.Uniforms.begin(), codeSegment.Uniforms.end(), [](auto& a, auto& b)
				{
					VisualMaterialParameterType typeA = a.first;
					VisualMaterialParameterType typeB = b.first;

					uint32_t alignA = Utils::GetBaseAlignmentForParameterType(typeA);
					uint32_t alignB = Utils::GetBaseAlignmentForParameterType(typeB);


					if (alignA == alignB)
						return Utils::GetSizeForParameterType(typeA) > Utils::GetSizeForParameterType(typeB);

					return alignA > alignB;
			});

			code += fmt::format("layout(set = {}, binding = {}) uniform UnifomData {{\n", codeSegment.UniformSet, codeSegment.UniformBinding);

			for (auto& [type, decl] : codeSegment.Uniforms)
			{
				code += decl;
			}

			code += "};\n";
		}

		code += "void main() {\n";

		code += codeSegment.MainCode;

		code += "}\n";

		code += "#endif";

		codeSegment.Code = code;

		return codeSegment;
	}

}
