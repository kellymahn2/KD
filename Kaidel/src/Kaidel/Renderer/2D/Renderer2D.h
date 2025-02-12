#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
#include "Kaidel/Core/BoundedVector.h"

#include <glm/glm.hpp>


namespace Kaidel{

	struct SamplingRegion {
		uint32_t Layer;
		//Top-Left
		glm::vec2 UV0;
		//Bottom-Right
		glm::vec2 UV1;
	};

    struct SpriteVertex{
        glm::vec3 Position;
        glm::vec4 Color;
		glm::vec3 UV;
    };

    class Renderer2D{
    public:
        static void Init();
        static void Shutdown();

        static void Begin(const glm::mat4& cameraVP);

		static void DrawSprite(const glm::mat4& transform, const glm::vec4& color, Ref<Texture> texture, const SamplingRegion& region);
    };
}
