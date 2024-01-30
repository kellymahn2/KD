#pragma once

#include "Kaidel/Core/Base.h"
#include <EnTT.hpp>
#include <glm/glm.hpp>


#include <unordered_map>
#include <vector>




namespace Kaidel {

	class Entity;

	enum class InterpolationFunction {
		None,
		LinearBezier,
		QuadraticBezier,
		CubicBezier,
		Spline,
	};

	template<typename T>
	struct KeyFrame {
		T KeyFrameValue;
		std::vector<T> Intermediates;
		InterpolationFunction Function;

		T GetValue(const KeyFrame<T>& next, float time) {
			float t = (time - Timer) / (next.Timer - Timer);

			if (t < 0.0f)
				t = 0.0f;
			if (t > 1.0f)
				t = 1.0f;
			switch (Function)
			{
			case InterpolationFunction::LinearBezier: {
				const T& p0 = KeyFrameValue;
				const T& p1 = next.KeyFrameValue;
				return (1.0f - t) * p0 + t * p1;
			}
			case InterpolationFunction::QuadraticBezier: {
				const T& p0 = KeyFrameValue;
				const T& p1 = Intermediates[0];
				const T& p2 = next.KeyFrameValue;
				return (1.0f - t) * (1.0f - t)*p0 + 2 * (1.0f - t) * t * p1 + t * t * p2;
			}
			case InterpolationFunction::CubicBezier:
				break;
			case InterpolationFunction::Spline:
				break;
			}
			return {};
		}

		float Timer;
		bool IsComplete;
	};


	template<typename T>
	T Lerp(const T& p0, const T& p1, float t) {
		return (1.0f - t) * p0 + t * p1;
	}




	template<typename T>
	struct AnimationProperty {

		void Push(const T& value, float time, InterpolationFunction function) {
			uint64_t findIndex = GetKeyFrameIndexAtTime(time);
			if (findIndex == -1) {
				KeyFrame<T> frame;
				frame.IsComplete = false;
				frame.Timer = time;
				frame.Function = function;
				frame.KeyFrameValue = value;
				uint64_t insertIndex = FrameStorage.size();
				FrameStorage.push_back(frame);
				std::sort(FrameStorage.begin(), FrameStorage.end(), [](const KeyFrame<T>& a, const KeyFrame<T>& b)
					{
						if (a.Timer < b.Timer)
							return true;
						if (a.Timer >= b.Timer)
							return false;
					});
				ResolveIntermediates(insertIndex - 1);
				Duration = std::max(time, Duration);
			}
			else {
				FrameStorage[findIndex].KeyFrameValue = value;
				ResolveIntermediates(findIndex - 1);
				ResolveIntermediates(findIndex);
			}
		}

		void ResolveIntermediates(uint64_t index) {
			if (index < 0 || index >= FrameStorage.size() - 1)
				return;
			KeyFrame<T>& first = FrameStorage[index];
			first.Intermediates.clear();
			KeyFrame<T>& second = FrameStorage[index + 1];
			switch (first.Function)
			{
			case InterpolationFunction::LinearBezier: {}break;
			case InterpolationFunction::QuadraticBezier: {
				T p1 = Lerp<T>(first.KeyFrameValue, second.KeyFrameValue, .5f);
				first.Intermediates.push_back(p1);
			}break;
			case InterpolationFunction::CubicBezier: {
				T p1 = Lerp<T>(first.KeyFrameValue, second.KeyFrameValue, 1.0f / 3.0f);
				T p2 = Lerp<T>(first.KeyFrameValue, second.KeyFrameValue, 2.0f / 3.0f);
				first.Intermediates.push_back(p1);
				first.Intermediates.push_back(p2);
			}break;
			case InterpolationFunction::Spline: {}break;
			}
		}


		bool KeyFrameExistsAtTime(float time) {
			for (auto& frame : FrameStorage) {
				if (frame.Timer == time)
					return true;
			}
			return false;
		}

		uint64_t GetKeyFrameIndexAtTime(float time) {
			for (uint64_t i = 0; i < FrameStorage.size(); ++i) {
				if (FrameStorage[i].Timer == time)
					return i;
			}
			return -1;
		}

		uint64_t GetKeyFrameIndexAtTimeApprox(float time) {
			if (FrameStorage.empty())
				return -1;
			for (uint64_t i = FrameStorage.size() - 1; i >= 0; --i) {
				if (FrameStorage[i].Timer <= time)
					return i;
			}
			return -1;
		}

		float Duration = 0.0f;
		uint64_t KeyFrameCount = 0;
		std::vector<KeyFrame<T>> FrameStorage;
	};




	struct TranslationData {
		glm::vec3 TargetTranslation;

		// Addition operator overload
		TranslationData operator+(const TranslationData& rhs) const {
			TranslationData result;
			result.TargetTranslation = TargetTranslation + rhs.TargetTranslation;
			return result;
		}

		// Subtraction operator overload
		TranslationData operator-(const TranslationData& rhs) const {
			TranslationData result;
			result.TargetTranslation = TargetTranslation - rhs.TargetTranslation;
			return result;
		}

		// Global multiplication by a float operator overload
		friend TranslationData operator*(float scalar, const TranslationData& data) {
			TranslationData result;
			result.TargetTranslation = scalar * data.TargetTranslation;
			return result;
		}
	};

	struct RotationData {
		glm::vec3 TargetRotation;

		// Addition operator overload
		RotationData operator+(const RotationData& rhs) const {
			RotationData result;
			result.TargetRotation = TargetRotation + rhs.TargetRotation;
			return result;
		}

		// Subtraction operator overload
		RotationData operator-(const RotationData& rhs) const {
			RotationData result;
			result.TargetRotation = TargetRotation - rhs.TargetRotation;
			return result;
		}

		// Global multiplication by a float operator overload
		friend RotationData operator*(float scalar, const RotationData& data) {
			RotationData result;
			result.TargetRotation = scalar * data.TargetRotation;
			return result;
		}
	};

	struct ScaleData {
		glm::vec3 TargetScale;

		// Addition operator overload
		ScaleData operator+(const ScaleData& rhs) const {
			ScaleData result;
			result.TargetScale = TargetScale + rhs.TargetScale;
			return result;
		}

		// Subtraction operator overload
		ScaleData operator-(const ScaleData& rhs) const {
			ScaleData result;
			result.TargetScale = TargetScale - rhs.TargetScale;
			return result;
		}

		// Global multiplication by a float operator overload
		friend ScaleData operator*(float scalar, const ScaleData& data) {
			ScaleData result;
			result.TargetScale = scalar * data.TargetScale;
			return result;
		}
	};



	//Used further down the line
	struct AnimationPlayerSettings {
		Entity& Entity;
		float Time;

	};

	class Animation {
	public:
		Animation(InterpolationFunction func)
			:m_Function(func)
		{
			m_RegistryKey = s_Registry.create();
		}

		void PushTranslation(const TranslationData& data,float time) {
			Push(data, time);
		}

		void PushRotation(const RotationData& data, float time) {
			Push(data, time);
		}

		void PushScale(const ScaleData& data, float time) {
			Push(data, time);
		}

		void Update(const AnimationPlayerSettings& settings) {
			if (settings.Time > m_AnimationDuration)
				return;
			
			UpdateTranslations(settings);
			UpdateRotations(settings);
			UpdateScales(settings);
		}

		template<typename T>
		AnimationProperty<T>* GetPropertyMap() {
			return s_Registry.try_get<AnimationProperty<T>>(m_RegistryKey);
		}


		entt::entity GetEnTTID()const { return m_RegistryKey; }


		const entt::registry& GetRegistry() const { return s_Registry; }
	private:

		template<typename T>
		void Push(const T& frame, float time) {
			if (AnimationProperty<T>* ptr = s_Registry.try_get<AnimationProperty<T>>(m_RegistryKey); ptr != nullptr) {
				ptr->Push(frame, time,m_Function);
				m_AnimationDuration = std::max(m_AnimationDuration, ptr->Duration);
				return;
			}
			AnimationProperty<T>& ap = s_Registry.emplace<AnimationProperty<T>>(m_RegistryKey);
			ap.Push(frame, time,m_Function);
			m_AnimationDuration = std::max(m_AnimationDuration, ap.Duration);
		}

		void UpdateTranslations(const AnimationPlayerSettings& settings);
		void UpdateRotations(const AnimationPlayerSettings & settings);
		void UpdateScales(const AnimationPlayerSettings& settings);



	private:

		entt::entity m_RegistryKey{ entt::null };

		float m_AnimationDuration = 0.0f;

		InterpolationFunction m_Function = InterpolationFunction::None;

		static entt::registry s_Registry;

	};


}
