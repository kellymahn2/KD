#include "KDpch.h"
#include "Kaidel/Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Kaidel {

	RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;
	glm::mat4 _GetTransform(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl) {
		glm::mat4 rotation = glm::toMat4(glm::quat(rot));
		/*switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL: {*/
		return glm::translate(glm::mat4(1.0f), pos) * rotation * glm::scale(glm::mat4(1.0f), scl);
		/*	}
			case RendererAPI::API::DirectX: {
				glm::vec3 npos = pos ;
				glm::mat4 f(1.0f);
				f[2][2] = -1.0f;
				return f*glm::translate(glm::mat4(1.0f), npos)
					* rotation
					* glm::scale(glm::mat4(1.0f), scl);
			}
			}*/
		
	}
	glm::vec4 _GetUVs() {
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGL: {
			return { 0,1,1,0 };
		}
		case RendererAPI::API::DirectX: {
			return { 0,0,1,1 };
		}
		}
		KD_CORE_ASSERT(false);
		return {};
	}

	Scope<RendererAPI> RendererAPI::Create()
	{
		switch (s_API)
		{
			case RendererAPI::API::None:    KD_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return CreateScope<OpenGLRendererAPI>();
			case RendererAPI::API::Vulkan: return CreateScope<VulkanRendererAPI>();
			//case RendererAPI::API::DirectX: return CreateScope<D3DRendererAPI>();
		}

		KD_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
