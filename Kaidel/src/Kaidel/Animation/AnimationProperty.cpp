#include "KDpch.h"
#include "AnimationProperty.h"

#include <glm/common.hpp>
namespace Kaidel {

	namespace Utils {
		static std::array<glm::vec2, 8> SplitCurve(float t, const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3)
		{
			glm::vec2 p4 = glm::mix(p0, p1, t);
			glm::vec2 p5 = glm::mix(p1, p2, t);
			glm::vec2 p6 = glm::mix(p2, p3, t);
			glm::vec2 p7 = glm::mix(p4, p5, t);
			glm::vec2 p8 = glm::mix(p5, p6, t);
			glm::vec2 p9 = glm::mix(p7, p8, t);

			std::array<glm::vec2, 8> array =
			{
				p0, p4, p7, p9,
				p9, p8, p6, p3
			};
			return array;
		}

		static float Bezier(float t, const glm::vec2& p0, const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3) 
		{
			return
				(1.0f - t) * (1.0f - t) * (1.0f - t) * p0.y +
				3.0f * (1.0f - t) * (1.0f - t) * t * p1.y +
				3.0f * (1.0f - t) * t * t * p2.y +
				t * t * t * p3.y;
		}
	}
	float AnimationProperty::Interpolate(float time, uint32_t i)
	{
		KD_CORE_ASSERT(i < m_Keys.size() - 1);
		
		float t = (time - m_Keys[i].MainKey.Point.x) / (m_Keys[i + 1].MainKey.Point.x - m_Keys[i].MainKey.Point.x);

		return Utils::Bezier(t, m_Keys[i].MainKey.Point, m_Keys[i].AfterKey.Point, m_Keys[i + 1].BeforeKey.Point, m_Keys[i + 1].MainKey.Point);
	}
	void AnimationProperty::PushKey(const AnimationSubKey& key)
	{
		AnimationKey actual;
		actual.BeforeKey = key;
		actual.MainKey = key;
		actual.AfterKey = key;

		m_Keys.push_back(actual);
	}
	void AnimationProperty::SplitCurve(float time)
	{
		KD_CORE_ASSERT(m_Keys.size() >= 2);
		uint32_t i = 0;

		for (; i < m_Keys.size(); ++i) {
			if (time > m_Keys[i].MainKey.Point.x) 
			{
				break;
			}
		}

		float t = (time - m_Keys[i].MainKey.Point.x) / (m_Keys[i + 1].MainKey.Point.x - m_Keys[i].MainKey.Point.x);

		std::array<glm::vec2, 8> points = 
			Utils::SplitCurve(t, m_Keys[i].MainKey.Point, m_Keys[i].AfterKey.Point, m_Keys[i + 1].BeforeKey.Point, m_Keys[i + 1].MainKey.Point);

		m_Keys.insert(m_Keys.begin() + i + 1, {});

		m_Keys[i].MainKey.Point = points[0];
		m_Keys[i].AfterKey.Point = points[1];
		
		m_Keys[i + 1].BeforeKey.Point = points[2];
		m_Keys[i + 1].MainKey.Point = points[3];
		m_Keys[i + 1].AfterKey.Point = points[5];
		
		m_Keys[i + 2].BeforeKey.Point = points[6];
		m_Keys[i + 2].MainKey.Point = points[7];
	}
}
