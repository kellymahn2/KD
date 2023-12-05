#pragma once


#include "Kaidel/Assets/Assets.h"
#include "Kaidel/Core/UUID.h"
#include "Kaidel/Core/Base.h"



#include <vector>
#include <list>
#include <chrono>
#include <glm/glm.hpp>
#include <map>
namespace Kaidel {

	namespace AnimationPropertyTypes {
		struct Translation {
			glm::vec3 TargetTranslation;
		};
	}
	class Entity;
	void ApplyTranslation(glm::vec3* current, glm::vec3* target, float t, Entity& entity);
	void AddDefaultTranslation(const glm::vec3& default, UUID id);
	void SetDefaultTranslation(Entity& entity);



	


	enum class AnimationPropertyState {
		Playing,Paused,Stopped,Finished = Stopped
	};
	enum class AnimationState{
		Playing, Paused, Stopped, Finished = Stopped
	};
	enum class AnimatedPropertyType {
		None,Translate
	};
	
	struct KeyFrame {
		struct {
			uint8_t Data[80];
			float StartTime;
			float EndTime;
			template<typename T>
			inline T& Get() {
				return *(reinterpret_cast<T*>(Data));
			}
		}AnimationData;
	};









	



	class AnimatedProperty{
	public:
		
		~AnimatedProperty() = default;
		void AddKeyFrame(const KeyFrame& keyFrame) {
			m_KeyFrames.push_back(keyFrame);
			m_Duration = keyFrame.AnimationData.EndTime;
		}
		void SetPropertyType(AnimatedPropertyType type) {
			m_Type = type;
		}
		AnimatedPropertyType GetPropertyType() const{
			return m_Type;
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
				if (m_CurrentFrame + 1 < m_KeyFrames.size()&&m_KeyFrames[m_CurrentFrame+1].AnimationData.StartTime <= m_CurrentTime) {
					++m_CurrentFrame;
				}
			}
			return m_State;
		}
		void Apply(Entity& entity) {
			if (m_State != AnimationPropertyState::Playing)
				return;
			float t = (m_CurrentTime - GetCurrentKeyFrame().AnimationData.StartTime) / (GetCurrentKeyFrame().AnimationData.EndTime - GetCurrentKeyFrame().AnimationData.StartTime);
			if (m_CurrentFrame == 0) {

				switch(m_Type){
					case AnimatedPropertyType::Translate:
					{
						ApplyTranslation(nullptr, &GetCurrentKeyFrame<AnimationPropertyTypes::Translation>().TargetTranslation, t, entity);
						break;
					}
				}
			}
			else {
				switch (m_Type) {
					case AnimatedPropertyType::Translate: {
						auto& aptt = m_KeyFrames[m_CurrentFrame - 1].AnimationData.Get< AnimationPropertyTypes::Translation>();
						ApplyTranslation( &aptt.TargetTranslation, &GetCurrentKeyFrame<AnimationPropertyTypes::Translation>()
							.TargetTranslation, t, entity);
						break;

					}
				}
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
			return (m_DeltaTime) / (m_KeyFrames[m_CurrentFrame].AnimationData.EndTime - m_KeyFrames[m_CurrentFrame].AnimationData.StartTime);
		}
		KeyFrame& GetCurrentKeyFrame() { 
			return m_KeyFrames.at(m_CurrentFrame); 
		}
		template<typename T>
		T& GetCurrentKeyFrame() {
			return m_KeyFrames.at(m_CurrentFrame).AnimationData.Get<T>();
		}
		std::vector<KeyFrame> m_KeyFrames;
		float m_Duration = 0;
		float m_DeltaTime = 0;
		float m_CurrentTime = 0;
		uint64_t m_CurrentFrame = 0;
		AnimationPropertyState m_State = AnimationPropertyState::Stopped;
		AnimatedPropertyType m_Type;
	};

	class Animation : public _Asset {
	public:
		AnimatedProperty& AddProperty(AnimatedPropertyType propertyType) { 
			auto& animatedProperty =  m_AnimatedProperties[(uint64_t)propertyType];
			animatedProperty.SetPropertyType(propertyType);
			return animatedProperty;
		}
		AnimationState Update(float ts) {
			AnimationState state = AnimationState::Finished;
			for (auto& [propertyType, animatedProperty] : m_AnimatedProperties) {
					state = (AnimationState) animatedProperty.Update(ts);
			}
			m_CurrentTime += ts;
			m_AnimationState = state;
			return state;
		}
		void Apply(Entity& entity) {
			for (auto& [propertyType, animatedProperty] : m_AnimatedProperties) {
				animatedProperty.Apply(entity);
			}
		}
		void Play() {
			m_AnimationState = AnimationState::Playing;
			for (auto& [propertyType, animatedProperty] : m_AnimatedProperties) {
				animatedProperty.Play();
			}
			m_CurrentTime = 0.0f;
		}
		float GetTime() { return m_CurrentTime; }
		AnimationState GetState() { return m_AnimationState; }
		inline virtual std::string GetAssetName()override {
			return m_AssetName;
		}
	private:
		AnimationState m_AnimationState;
		std::unordered_map<uint64_t,AnimatedProperty> m_AnimatedProperties;
		float m_CurrentTime;
		friend class Scene;
		std::string m_AssetName;
	};

	class AnimationPlayer : public _Asset {
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
		inline virtual std::string GetAssetName() {
			return m_AssetName;
		}
	private:
		std::list<Ref<Animation>> m_AnimationsPlaying;
		std::string m_AssetName;
	};

	
}
