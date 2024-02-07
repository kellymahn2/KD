#include "KDpch.h"
#include "Animation.h"
#include "Kaidel/Scene/Entity.h"
#include "Kaidel/Scene/Components.h"
namespace Kaidel {
	entt::registry Animation::s_Registry;
		
	/*glm::vec3 LinearBezier(const std::vector<_Points>& controlPoints, T t) {
		uint32_t n = controlPoints.size() - 1;
		glm::vec3 result{ 0.0f };
		T oneMinusT = 1.0f - t;
		for (uint32_t i = 0; i <= n; ++i) {
			result += (CalcBinomialCoefficient(n, i) * std::pow(oneMinusT, (T)(n - i)) * std::pow(t, (T)i)) * controlPoints[i].Position;
		}
		return result;
	}*/

	void Animation::UpdateTranslations(const AnimationPlayerSettings& settings){
		
		if (m_Function == InterpolationFunction::None)
			return;
		
		AnimationProperty<TranslationData>* property = s_Registry.try_get<AnimationProperty<TranslationData>>(m_RegistryKey);
		if (!property)
			return;
		if (!settings.Entity.HasComponent<TransformComponent>())
			return;
		if (property->Duration <= settings.Time)
			return;	
		auto& tc = settings.Entity.GetComponent<TransformComponent>();
		glm::vec3 interpolatedValue{};
		uint64_t frameIndex = property->GetKeyFrameIndexAtTimeApprox(settings.Time);
		if (frameIndex == -1)
			return;
		auto newPos = property->FrameStorage[frameIndex].GetValue(property->FrameStorage[frameIndex+1],settings.Time);
		MoveEntity(settings.Entity, settings.Entity.GetScene(), newPos.TargetTranslation - tc.Translation, { 0,0,0 });
	}
	void Animation::UpdateRotations(const AnimationPlayerSettings& settings){
		if (m_Function == InterpolationFunction::None)
			return;

		AnimationProperty<RotationData>* property = s_Registry.try_get<AnimationProperty<RotationData>>(m_RegistryKey);
		if (!property)
			return;
		if (!settings.Entity.HasComponent<TransformComponent>())
			return;
		if (property->Duration <= settings.Time)
			return;
		auto& tc = settings.Entity.GetComponent<TransformComponent>();
		glm::vec3 interpolatedValue{};
		uint64_t frameIndex = property->GetKeyFrameIndexAtTimeApprox(settings.Time);
		if (frameIndex == -1)
			return;
		auto newPos = property->FrameStorage[frameIndex].GetValue(property->FrameStorage[frameIndex + 1], settings.Time);
		MoveEntity(settings.Entity, settings.Entity.GetScene(), { 0,0,0 }, newPos.TargetRotation - tc.Rotation);
	}
	void Animation::UpdateScales(const AnimationPlayerSettings& settings) {
		
	}
}
