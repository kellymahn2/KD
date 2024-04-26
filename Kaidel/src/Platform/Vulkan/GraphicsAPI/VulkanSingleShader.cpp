#include "KDpch.h"
#include "VulkanSingleShader.h"
#include "Kaidel/Renderer/GraphicsAPI/Shader.h"
#include <shaderc/shaderc.h>

namespace Kaidel {

	namespace Vulkan {



		namespace Utils {

			static shaderc_shader_kind KaidelShaderTypeToShadercType(ShaderType type) {
				switch (type)
				{
				case Kaidel::ShaderType::VertexShader:return shaderc_vertex_shader;
				case Kaidel::ShaderType::FragmentShader: return shaderc_fragment_shader;
				case Kaidel::ShaderType::GeometryShader: return shaderc_geometry_shader;
				case Kaidel::ShaderType::TessellationControlShader: return shaderc_tess_control_shader;
				case Kaidel::ShaderType::TessellationEvaluationShader: return shaderc_tess_evaluation_shader;
				}
				KD_CORE_ASSERT(false);
				return (shaderc_shader_kind)0;
			}

			static shaderc_include_result* IncludeCallback(void* user_data, const char* requested_source, int type, const char* requesting_source, size_t include_depth) {

				std::string includeDir = "";
				if (requesting_source) {
					std::string requestingSource = requesting_source;
					size_t lastSlashIndex = requestingSource.find_last_of("/\\");
					if (lastSlashIndex != std::string::npos) {
						includeDir = requestingSource.substr(0, lastSlashIndex);
					}
				}

				std::string fullPath = includeDir + "/" + requested_source;

				std::ifstream file(fullPath);
				if (!file.is_open()) {
					return nullptr;
				}

				std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				file.close();

				shaderc_include_result* result = new shaderc_include_result;
				result->content = strdup(content.c_str());
				result->content_length = content.size();
				result->source_name = strdup(fullPath.c_str());
				result->source_name_length = fullPath.size();
				result->user_data = nullptr;

				return result;
			}

			static void ReleaseIncludeCallback(void* user_data, shaderc_include_result* include_result) {
				delete[] include_result->content;
				delete[] include_result->source_name;
				delete include_result;
			}

			static std::vector<uint32_t> CompileShader(const std::string& src,const std::string& filePath, ShaderType type) {

				shaderc_compiler_t compiler = shaderc_compiler_initialize();
				shaderc_compile_options_t options = shaderc_compile_options_initialize();

				shaderc_compile_options_set_include_callbacks(options, IncludeCallback, ReleaseIncludeCallback, nullptr /* user_data */);

				shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, src.c_str(), src.size(), KaidelShaderTypeToShadercType(type), filePath.c_str(), "main", options);
				shaderc_compiler_release(compiler);
				shaderc_compile_options_release(options);

				KD_CORE_ASSERT(shaderc_result_get_compilation_status(result) == shaderc_compilation_status_success,"Shader compilation failed");

				std::vector<uint32_t> spirv(shaderc_result_get_length(result) / sizeof(uint32_t));
				memcpy(spirv.data(), shaderc_result_get_bytes(result), shaderc_result_get_length(result));
				shaderc_result_release(result);

				return spirv;
			}

			static std::string ReadFile(const std::string& path) {
				std::ifstream file(path);
				if (!file.is_open())
					return {};

				std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
				file.close();

				return source;
			}

			static VkShaderModule CreateShaderModule(const SingleShaderSpecification& specification) {
				std::vector<uint32_t> spirv;
				if (specification.IsPath) {
					spirv = CompileShader(ReadFile(specification.ControlString), specification.ControlString, specification.Type);
				}
				else {
					spirv = CompileShader(specification.ControlString, {}, specification.Type);
				}

				VK_STRUCT(VkShaderModuleCreateInfo, moduleInfo, SHADER_MODULE_CREATE_INFO);
				moduleInfo.codeSize = spirv.size() * 4;
				moduleInfo.pCode = spirv.data();

				VkShaderModule module = VK_NULL_HANDLE;
				vkCreateShaderModule(VK_DEVICE, &moduleInfo, VK_ALLOCATOR_PTR, &module);
				return module;
			}
		}



		VulkanSingleShader::VulkanSingleShader(const SingleShaderSpecification& specification)
			:m_Specification(specification)
		{
			m_ShaderModule = Utils::CreateShaderModule(specification);
		}
		VulkanSingleShader::~VulkanSingleShader()
		{
			vkDestroyShaderModule(VK_DEVICE, m_ShaderModule, VK_ALLOCATOR_PTR);
		}
	}
}
