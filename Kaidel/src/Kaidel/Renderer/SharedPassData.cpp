#include "KDpch.h"
#include "SharedPassData.h"
#include "Kaidel/Scene/Components.h"
#include "Light.h"

namespace Kaidel {

	static bool Initialized = false;

	CameraBufferData CameraData;
	Ref<UniformBuffer> CameraDataUniformBuffer;
	Ref<UniformBuffer> LightCountUniformBuffer;
	struct LightCount {
		int PointLightCount;
		int SpotLightCount;
	};


	static void Initialize() {
		CameraDataUniformBuffer = UniformBuffer::Create(sizeof(CameraBufferData), 0);
		LightCountUniformBuffer = UniformBuffer::Create(sizeof(LightCount), 1);
		MaterialTextureHandler::Init();
		Initialized = true;
	}

	void SharedPassData::SetData() {
		if (!Initialized) {
			Initialize();
		}
		//Point Lights
		{
			auto view = (Config.Scene)->m_Registry.view<TransformComponent, PointLightComponent>();
			for (auto e : view) {
				auto [tc, plc] = view.get<TransformComponent, PointLightComponent>(e);
				auto& light = plc.Light->GetLight();
				light.Position = tc.Translation;
			}
		}
		//Spot Lights
		{

			auto view = (Config.Scene)->m_Registry.view<TransformComponent, SpotLightComponent>();

			for (auto e : view) {

				auto [tc, slc] = view.get<TransformComponent, SpotLightComponent>(e);
				auto& light = slc.Light->GetLight();
				light.Position = glm::vec4(tc.Translation,1.0f);
				float maxCov = CalcLightMaxCoverage(light.QuadraticCoefficient, light.LinearCoefficient, light.ConstantCoefficient, 0.01f);
				if (maxCov == 0.0f || maxCov == INFINITY)
					maxCov = 25.0f;
				light.LightViewProjection = glm::perspective(glm::acos(light.CutOffAngle) * 2.0f, 1.0f, 0.1f, 25.0f) * glm::lookAt(tc.Translation, glm::normalize(glm::vec3(light.Direction)), glm::vec3(0, 1, 0));

			}
		}

		CameraDataUniformBuffer->SetData(&Config.CameraData, sizeof(CameraBufferData));
		CameraDataUniformBuffer->Bind();
		
		DirectionalLight::SetLights();
		PointLight::SetLights();
		SpotLight::SetLights();
		Material::SetMaterials();
		LightCount counts{};
		counts.PointLightCount = PointLight::GetLightCount();
		counts.SpotLightCount = SpotLight::GetLightCount();
		LightCountUniformBuffer->SetData(&counts, sizeof(LightCount));
		LightCountUniformBuffer->Bind();
		MaterialTextureHandler::GetTexturesMap()->Bind(0);
	}
}
