#include "KDpch.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "Kaidel/Core/Application.h"
#include "Kaidel/Events/SettingsEvent.h"
#include <mutex>
#include <glad/glad.h>

namespace Kaidel {
	
	void OpenGLMessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam)
	{
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:         KD_CORE_ASSERT(false, message); return;
			case GL_DEBUG_SEVERITY_MEDIUM:       KD_ERROR(message); return;
			case GL_DEBUG_SEVERITY_LOW:          KD_WARN(message); return;
			case GL_DEBUG_SEVERITY_NOTIFICATION: KD_TRACE(message); return;
		}
		
		KD_CORE_ASSERT(false, "Unknown severity level!");
	}

	void OpenGLRendererAPI::Init()
	{
	#ifdef KD_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);
		
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	#endif

		glDepthFunc(GL_LEQUAL);
		//glEnable(GL_LINE_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		float data[6];
		glGetFloatv(GL_MAX_TESS_GEN_LEVEL, data);
		int x;
		glGetIntegerv(GL_MAX_SAMPLES, &x);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::GetViewport(uint32_t& x, uint32_t& y, uint32_t& width, uint32_t& height) {
		GLint viewport[4]; // [x, y, width, height]
		glGetIntegerv(GL_VIEWPORT, viewport);
		x = viewport[0];
		y = viewport[1];
		width = viewport[2];
		height = viewport[3];
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	int OpenGLRendererAPI::QueryMaxTextureSlots() {
		GLint maxTextures;
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxTextures);
		return maxTextures;
	}
	float OpenGLRendererAPI::QueryMaxTessellationLevel() {
		float maxTessLevel;
		glGetFloatv(GL_MAX_TESS_GEN_LEVEL, &maxTessLevel);
		return maxTessLevel;
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}
	void OpenGLRendererAPI::DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount, uint32_t instanceCount){
		/*if (instanceCount == 1) {
			DrawIndexed(vertexArray, indexCount);
			return;
		}*/
		vertexArray->Bind();
		glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
	}
	void OpenGLRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0,vertexCount);
	}
	void OpenGLRendererAPI::DrawPatches(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
		vertexArray->Bind();
		glDrawArrays(GL_PATCHES, 0, vertexCount);
	}

	void OpenGLRendererAPI::DrawPoints(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
		vertexArray->Bind();
		glDrawArrays(GL_POINTS, 0, vertexCount);
	}

	void OpenGLRendererAPI::SetLineWidth(float width) {
		glLineWidth(width);
	}

	void OpenGLRendererAPI::SetPointSize(float pixelSize) {
		glPointSize(pixelSize);
	}


	void OpenGLRendererAPI::SetCullMode(CullMode cullMode) {
		switch (cullMode)
		{
		case Kaidel::CullMode::None:glDisable(GL_CULL_FACE);return;
		case Kaidel::CullMode::Front:glEnable(GL_CULL_FACE); glCullFace(GL_FRONT); return;
		case Kaidel::CullMode::Back:glEnable(GL_CULL_FACE); glCullFace(GL_BACK); return;
		case Kaidel::CullMode::FrontAndBack:glEnable(GL_CULL_FACE); glCullFace(GL_FRONT_AND_BACK); return;
		}
		s_RendererSettings.Culling = cullMode;
	}

	void OpenGLRendererAPI::SetPatchVertexCount(uint32_t count) {
		glPatchParameteri(GL_PATCH_VERTICES, count);
	}
	void OpenGLRendererAPI::SetDefaultTessellationLevels(const glm::vec4& outer, const glm::vec2& inner) {
		glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, &outer.x);
		glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, &inner.x);

	}

}
