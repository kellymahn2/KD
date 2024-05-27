#pragma once
#include "Kaidel/Renderer/GraphicsAPI/Framebuffer.h"
#include "Kaidel/Core/BoundedVector.h"

#include <glm/glm.hpp>


namespace Kaidel{
    struct SpriteVertex{
        glm::vec3 Position;
        glm::vec4 Color;
    };


    class Renderer2D{
    public:
        static void Init();
        static void Shutdown();

        static void Begin(const glm::mat4& cameraVP,Ref<Framebuffer> outputColorBuffer);
        static void End();


        static void DrawSprite(const glm::mat4& transform,const glm::vec4& materials);
        static void AddSprite(SpriteVertex vertices[4]);
        

    private:
		static void StartSpriteBatch();
		static void FlushSprites();
	private:


    };


}
