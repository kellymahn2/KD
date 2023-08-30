#include "KDpch.h"
#include "UUID.h"
#include <random>
namespace std {
	template<>
	struct hash<UUID>
	{
		std::size_t operator()(const UUID& uuid) const{
			return hash<uint64_t>()(uuid.m_UUID);
		}
	};
}
namespace Kaidel {
	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_RandomEngine(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	UUID::UUID()
		:m_UUID(s_UniformDistribution(s_RandomEngine))
	{
	}

	UUID::UUID(uint64_t uuid)
		:m_UUID(uuid)
	{
	}

}
