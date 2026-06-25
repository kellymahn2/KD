#pragma once
#include <xhash>
namespace Kaidel {
	class UUID {
	public:

		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		uint64_t Get() const { return m_UUID; }

		explicit operator uint64_t() const  { return Get(); }
		bool operator==(const UUID& other) const { return Get() == other.Get(); }
		bool operator!=(const UUID& other) const { return Get() != other.Get(); }
		bool operator!() const { return Get() != 0; }


		static UUID GenerateFromSeed(uint64_t seed);

		static void PushSeed(uint64_t seed);

		static void PopSeed();

	private:
		uint64_t m_UUID;
		friend struct std::hash<UUID>;
	};
}
namespace std {
	template<>
	struct hash<Kaidel::UUID>
	{
		size_t operator()(const Kaidel::UUID& _Keyval) const { return std::hash<uint64_t>()((uint64_t)_Keyval); }
	};
}
