#pragma once
#include <string>
#include "Kaidel/Scene/Scene.h"
#include "Kaidel/Renderer/GraphicsAPI/FullAPI.h"


namespace Kaidel {



	enum class RenderPassOutput {
		Color,
		Buffer,
	};


	enum class RenderPassType {
		Shader,
		Compute
	};


	struct RenderPassVertexInputSpecification {
		Ref<VertexBuffer> Buffer;

		RenderPassVertexInputSpecification(Ref<VertexBuffer> vertexbuffer, const BufferLayout& layout)
			:Buffer(vertexbuffer)
		{
			Buffer->SetLayout(layout);
		}

		RenderPassVertexInputSpecification(Ref<VertexBuffer> vertexbuffer, std::initializer_list<BufferElement> layoutElements)
			:Buffer(vertexbuffer)
		{
			Buffer->SetLayout({ layoutElements });
		}

	};

	using RenderPassFlushFunction = std::function<void(Ref<VertexArray>)>;

	struct RenderPassSpecification {
		std::string DebugName;
		std::vector<Ref<VertexBuffer>> Buffers;
		RenderPassFlushFunction FlushFunction;
	};

	struct RenderPassVertexBufferContent {
		void* Data;
		uint32_t Size;
	};

	



	class RenderPass : public IRCCounter<false>{
	public:

		static Ref<RenderPass> Create(const RenderPassSpecification& spec, Ref<Shader> shader,Ref<Framebuffer> outputBuffer);

		virtual ~RenderPass()= default;


		virtual void SetIndexBuffer(Ref<IndexBuffer> indexBuffer) = 0;

		virtual void SetInputBuffer(Ref<VertexBuffer> inputBuffer, uint32_t index) = 0;
		virtual void SetInputBuffers(std::initializer_list<Ref<VertexBuffer>>  inputBuffers) = 0;

		virtual void SetObjects(Ref<VertexArray> vertexArray,std::initializer_list<Ref<VertexBuffer>>  inputBuffers) = 0;

		virtual void SetInput(Ref<UAVInput> unorderedAccessView);
		virtual void SetInput(Ref<UniformBuffer> uniformBuffer);
		
		virtual void SetInput(Ref<Texture2D> texture);
		virtual void SetInput(Ref<Texture2DArray> array);

		virtual void SetPassShader(Ref<Shader> shader)=0;
		virtual void SetPassShader(Ref<ComputeShader> computeShader) = 0;
		virtual void BeginPass() = 0;

		virtual void Flush(std::initializer_list<RenderPassVertexBufferContent> contents) = 0;

		virtual void EndPass(std::initializer_list<RenderPassVertexBufferContent> contents) = 0;

		virtual void SetOutput(Ref<UAVInput> unorderedAccessView) = 0;
		virtual void SetOutput(Ref<UniformBuffer> uniformBuffer) = 0;

		//Binds to image slot
		virtual void SetOutput(Ref<Texture2D> texture) = 0;
		virtual void SetOutput(Ref<Texture2DArray> array) = 0;

		virtual void SetOutput(Ref<Framebuffer> framebuffer) = 0;
	protected:
		void BindObjects();
		void BindTextures();
		void BindUnorderedAccessViews();
		void BindConstantBuffers();
		void BindTextureArrays();

		void UnbindObjects();
		void UnbindTextures();
		void UnbindUnorderedAccessViews();
		void UnbindConstantBuffers();
		void UnbindTextureArrays();
	protected:

		enum class BindingType {
			Input,
			Output
		};

		template<typename T>
		struct RenderPassBoundObject {
			Ref<T> Object;
			BindingType Type;
		};
		
		std::vector<RenderPassBoundObject<Texture2D>> m_Textures;
		std::vector<RenderPassBoundObject<UAVInput>> m_UnorderedAccessViews;
		std::vector<RenderPassBoundObject<UniformBuffer>> m_ConstantBuffers;
		std::vector<RenderPassBoundObject<Texture2DArray>> m_TextureArrays;
	};


	enum class RenderPassState {
		Idle,
		InPass,
	};


	class RenderPassShaderedColored :public RenderPass{
	public:
		RenderPassShaderedColored(const RenderPassSpecification& spec, Ref<Shader> shader, Ref<Framebuffer> outputBuffer);

		void SetInputBuffer(Ref<VertexBuffer> inputBuffer, uint32_t index) override {};
		void SetInputBuffers(std::initializer_list<Ref<VertexBuffer>> inputBuffers) override;




		void SetPassShader(Ref<Shader> shader) override;
		void SetPassShader(Ref<ComputeShader> computeShader) override;

		void BeginPass() override;

		void Flush(std::initializer_list<RenderPassVertexBufferContent> contents) override;

		void EndPass(std::initializer_list<RenderPassVertexBufferContent> contents) override;

		void SetOutput(Ref<UAVInput> unorderedAccessView) override;
		void SetOutput(Ref<UniformBuffer> uniformBuffer) override;

		//Binds to image slot
		void SetOutput(Ref<Texture2D> texture) override;
		void SetOutput(Ref<Texture2DArray> array) override;
		void SetOutput(Ref<Framebuffer> framebuffer) override;

	private:
	private:
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;

		Ref<IndexBuffer> m_IndexBuffer;

		Ref<VertexArray> m_VertexArray;

		RenderPassState m_State = RenderPassState::Idle;

		RenderPassSpecification m_Specification;
		
		Ref<Shader> m_PassShader;

		Ref<Framebuffer> m_OutputBuffer;

		// Inherited via RenderPass
		void SetIndexBuffer(Ref<IndexBuffer> indexBuffer) override;

		// Inherited via RenderPass
		void SetObjects(Ref<VertexArray> vertexArray, std::initializer_list<Ref<VertexBuffer>> inputBuffers) override;
	};


}
