#include "KDpch.h"
#include "UUID.h"
#include <random>
namespace Kaidel {
	static std::atomic<uint64_t> s_SeedCounter = 0;

	struct UUIDThreadData {
		std::mt19937_64 RandomEngine;
		std::stack<uint64_t> IDStack;

		UUIDThreadData()
		{
			uint64_t seed1 = std::random_device{}();
			uint64_t seed2 = std::hash<std::thread::id>{}(std::this_thread::get_id());
			uint64_t seed3 = s_SeedCounter.fetch_add(1, std::memory_order_relaxed);
			uint64_t combinedSeed = seed1 ^ (seed2 + 0x9e3779b97f4a7c15ULL + (seed3 << 6) + (seed3 >> 2));
			RandomEngine = std::mt19937_64{ combinedSeed };
		}
	};

	static thread_local UUIDThreadData s_Data;

	UUID::UUID()
		: m_UUID(0)
	{
		std::uniform_int_distribution<uint64_t> distribution(1);

		m_UUID = distribution(s_Data.RandomEngine);
	}

	UUID::UUID(uint64_t uuid)
		:m_UUID(uuid)
	{
	}

	UUID UUID::GenerateFromSeed(uint64_t seed)
	{
		s_Data.RandomEngine.seed(seed);

		return UUID();
	}

	void UUID::PushSeed(uint64_t seed)
	{
		//s_Data.IDStack.push(*s_Data.RandomEngine);

	}

	void UUID::PopSeed()
	{

	}

}
