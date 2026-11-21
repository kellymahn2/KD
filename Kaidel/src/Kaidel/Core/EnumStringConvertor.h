#include <string_view>
#include <vector>

namespace ConvertorDetail
{
	template<typename T> struct Identity { using type = T; };
	template<typename T> using IdentityT = typename Identity<T>::type;

	// Only used inside decltype / as a value: returns the first argument
	template<typename E, typename... Rest>
	constexpr E FirstOf(E first, Rest...) { return first; }

	template<typename E>
	struct Entry { E Value; const char* Name; };

	template<typename E>
	void Fill(std::vector<Entry<E>>&) {}

	template<typename E, typename... Rest>
	void Fill(std::vector<Entry<E>>& table, IdentityT<E> key, const char* name, Rest... rest)
	{
		table.push_back({ key, name });
		Fill<E>(table, rest...);
	}

	template<typename E, typename... Args>
	std::vector<Entry<E>> Build(Args... args)
	{
		std::vector<Entry<E>> table;
		Fill<E>(table, args...);
		return table;
	}
}

#define DECL_CONVERTOR(e2sName, s2eName, ...)                                                      \
	inline const char* e2sName(decltype(ConvertorDetail::FirstOf(__VA_ARGS__)) value)              \
	{                                                                                              \
		using E = decltype(ConvertorDetail::FirstOf(__VA_ARGS__));                                 \
		static const auto table = ConvertorDetail::Build<E>(__VA_ARGS__);                          \
		for (const auto& e : table)                                                                \
			if (e.Value == value) return e.Name;                                                   \
		return "Unknown";                                                                          \
	}                                                                                              \
	inline decltype(ConvertorDetail::FirstOf(__VA_ARGS__)) s2eName(std::string_view str)           \
	{                                                                                              \
		using E = decltype(ConvertorDetail::FirstOf(__VA_ARGS__));                                 \
		static const auto table = ConvertorDetail::Build<E>(__VA_ARGS__);                          \
		for (const auto& e : table)                                                                \
			if (str == e.Name) return e.Value;                                                     \
		return ConvertorDetail::FirstOf(__VA_ARGS__);                                              \
	}
