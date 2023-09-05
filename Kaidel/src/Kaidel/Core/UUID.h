#pragma once
#include <xhash>
namespace Kaidel {
	class UUID {
	public:

		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;
		operator uint64_t()const { return m_UUID; }
		bool operator==(const UUID& other) { return m_UUID == other.m_UUID; }
	private:
		uint64_t m_UUID;
		friend struct std::hash<UUID>;
	};
}
namespace std {
	template<>
	struct hash<Kaidel::UUID>
	{
		size_t operator()(const Kaidel::UUID& _Keyval) const { return std::hash<uint64_t>()(_Keyval); }
	};

	

}
