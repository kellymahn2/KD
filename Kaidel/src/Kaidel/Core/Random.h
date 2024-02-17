#pragma once
#include <random>
namespace Kaidel {
	class Random {
	public:

		static int16_t GenerateInt16(int16_t min, int16_t max) {
			std::uniform_int_distribution<int16_t> distribution(min, max - 1);
			return distribution(m_RandomEngine);
		}

		static uint16_t GenerateUInt16(uint16_t min, uint16_t max) {
			std::uniform_int_distribution<uint16_t> distribution(min, max - 1);
			return distribution(m_RandomEngine);
		}

		static int32_t GenerateInt32(int32_t min, int32_t max) {
			std::uniform_int_distribution<int32_t> distribution(min, max - 1);
			return distribution(m_RandomEngine);
		}

		static uint32_t GenerateUInt32(uint32_t min, uint32_t max) {
			std::uniform_int_distribution<uint32_t> distribution(min, max - 1);
			return distribution(m_RandomEngine);
		}

		static int64_t GenerateInt64(int64_t min, int64_t max) {
			std::uniform_int_distribution<int64_t> distribution(min, max - 1);
			return distribution(m_RandomEngine);
		}

		static uint64_t GenerateUInt64(uint64_t min, uint64_t max) {
			std::uniform_int_distribution<uint64_t> distribution(min, max - 1);
			return distribution(m_RandomEngine);
		}

		static float GenerateFloat(float min, float max) {
			std::uniform_real_distribution<float> distribution(min, max);
			return distribution(m_RandomEngine);
		}

		static double GenerateDouble(double min, double max) {
			std::uniform_real_distribution<double> distribution(min, max);
			return distribution(m_RandomEngine);
		}
	private:
		static inline std::random_device m_RandomDevice;
		static inline std::mt19937 m_RandomEngine;
		friend class Application;
	};
}
