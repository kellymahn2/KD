#pragma once

#include <vector>
#include <list>
#include <chrono>
#include <glm/glm.hpp>
#include <map>
#include "Kaidel/Core/UUID.h"
#include "Kaidel/Core/Base.h"
namespace Kaidel {

	class Entity;


	enum class AnimationPropertyState {
		Playing,Paused,Stopped,Finished = Stopped
	};
	enum class AnimationState{
		Playing, Paused, Stopped, Finished = Stopped
	};
	enum class AnimatedPropertyType {
		None,Translate
	};
	template<AnimatedPropertyType _PropertyType>
	struct KeyFrame {
	};
	template<>
	struct KeyFrame<AnimatedPropertyType::Translate> {
		float StartTime{};
		float EndTime{};
		glm::vec3 TargetTranslation{};
		static void Apply(KeyFrame& last, float t, Entity& entity);
	};

	class AnimatedPropertyBase {
	public:
		virtual AnimationPropertyState Update(float ts) = 0;
		virtual void Apply(Entity& entity) = 0;
		virtual ~AnimatedPropertyBase() = default;
		template<AnimatedPropertyType _PropertyType>
		void AddKeyFrame(const KeyFrame<_PropertyType> keyFrame);

		virtual void Play() {
			m_State = AnimationPropertyState::Playing;
			m_CurrentTime = 0.0f;
		}
		virtual void Pause() {
			m_State = AnimationPropertyState::Paused;
		}
		virtual void Stop() {
			m_State = AnimationPropertyState::Stopped;
			m_CurrentFrame = -1;
		}
		virtual bool IsFinished() {
			return m_State == AnimationPropertyState::Finished;
		}
	protected: 
		int m_CurrentFrame;
		float m_CurrentTime = 0;
		AnimationPropertyState m_State = AnimationPropertyState::Stopped;
	};




	template<AnimatedPropertyType _PropertyType>
	class AnimatedProperty : public AnimatedPropertyBase{
	public:
		~AnimatedProperty() = default;
		void AddKeyFrame(const KeyFrame<_PropertyType>& keyFrame) {
			m_KeyFrames.push_back(keyFrame);
			m_Duration = keyFrame.EndTime;
		}
		AnimationPropertyState Update(float deltaTime) {
			if (m_State != AnimationPropertyState::Playing)
				return m_State;
			m_CurrentTime += deltaTime;
			m_DeltaTime = deltaTime;
			if (m_CurrentTime >= m_Duration) {
				m_CurrentTime = m_Duration;
				m_State = AnimationPropertyState::Finished;
			}
			else {
				if (m_CurrentFrame + 1 < m_KeyFrames.size()&&m_KeyFrames[m_CurrentFrame+1].StartTime <= m_CurrentTime) {
					++m_CurrentFrame;
				}
			}
			return m_State;
		}
		void Apply(Entity& entity) {
			if (m_State != AnimationPropertyState::Playing)
				return;
			float t = (m_CurrentTime - GetCurrentKeyFrame().StartTime) / (GetCurrentKeyFrame().EndTime - GetCurrentKeyFrame().StartTime);
			if (m_CurrentFrame == 0) {
				ApplyTranslation(nullptr, &GetCurrentKeyFrame().TargetTranslation, t, entity);
			}
			else {
				ApplyTranslation( &m_KeyFrames[m_CurrentFrame - 1].TargetTranslation, &GetCurrentKeyFrame().TargetTranslation, t, entity);
			}

			//KeyFrame<_PropertyType>::Apply(m_KeyFrames.at(m_CurrentFrame), m_DeltaTime, entity);
		}
		void Play() {
			m_State = AnimationPropertyState::Playing;
			m_CurrentTime = 0.0f;
			m_CurrentFrame = 0;
		}
		void Pause() {
			m_State = AnimationPropertyState::Paused;
		}
		void Stop() {
			m_State = AnimationPropertyState::Stopped;
			m_CurrentFrame = 0;
		}
		bool IsFinished() {
			return m_State == AnimationPropertyState::Finished;
		}
	private:
		float GetParemeter() {
			return (m_DeltaTime) / (m_KeyFrames[m_CurrentFrame].EndTime - m_KeyFrames[m_CurrentFrame].StartTime);
		}
		auto& GetCurrentKeyFrame() { return m_KeyFrames.at(m_CurrentFrame); }
		std::vector<KeyFrame<_PropertyType>> m_KeyFrames;
		float m_Duration = 0;
		float m_DeltaTime;
		
	};

	template<AnimatedPropertyType _PropertyType>
	inline void AnimatedPropertyBase::AddKeyFrame(const KeyFrame<_PropertyType> keyFrame)
	{
		reinterpret_cast<AnimatedProperty<_PropertyType>*>(this)->AddKeyFrame(keyFrame);
	}
	class Animation {
	public:
		Ref<AnimatedPropertyBase> AddProperty(AnimatedPropertyType propertyType) { 
			auto& animatedProperty = m_AnimatedProperties[(uint64_t)propertyType];
			if (!animatedProperty)
			{

				switch (propertyType)
				{
				case Kaidel::AnimatedPropertyType::Translate: animatedProperty = CreateRef<AnimatedProperty<AnimatedPropertyType::Translate>>(); break;
				default:
					KD_CORE_ASSERT(false);
				}

			}
			return animatedProperty;
				
		}
		AnimationState Update(float ts) {
			AnimationState state = AnimationState::Finished;
			for (auto& [propertyType, animatedProperty] : m_AnimatedProperties) {
					state = (AnimationState) animatedProperty->Update(ts);
			}
			m_CurrentTime += ts;
			m_AnimationState = state;
			return state;
		}
		void Apply(Entity& entity) {
			for (auto& [propertyType, animatedProperty] : m_AnimatedProperties) {
				animatedProperty->Apply(entity);
			}
		}
		void Play() {
			m_AnimationState = AnimationState::Playing;
			for (auto& [propertyType, animatedProperty] : m_AnimatedProperties) {
				animatedProperty->Play();
			}
			m_CurrentTime = 0.0f;
		}
		float GetTime() { return m_CurrentTime; }
		AnimationState GetState() { return m_AnimationState; }
		
	private:
		AnimationState m_AnimationState;
		std::unordered_map<uint64_t, Ref<AnimatedPropertyBase>> m_AnimatedProperties;
		float m_CurrentTime;
		friend class Scene;
	};

	class AnimationPlayer {
	public:
		void Play(Ref<Animation> animation) {
			m_AnimationsPlaying.push_back(animation);
			animation->Play();
		}
		void Update(Entity& entity,float ts,Ref<Animation> anim) {
			auto it = std::find(m_AnimationsPlaying.begin(), m_AnimationsPlaying.end(), anim);
			KD_CORE_ASSERT(it!= m_AnimationsPlaying.end());
			auto state = anim->Update(ts);
			anim->Apply(entity);
			if (state == AnimationState::Finished)
				m_AnimationsPlaying.erase(it);
		}
	private:
		std::list<Ref<Animation>> m_AnimationsPlaying;

	};

	void ApplyTranslation(glm::vec3* current,glm::vec3* target, float t, Entity& entity);
	void AddDefaultTranslation(const glm::vec3& default, UUID id);
	void SetDefaultTranslation(Entity& entity);
}
