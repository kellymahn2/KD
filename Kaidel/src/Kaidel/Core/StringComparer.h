#pragma once
#include <string>
namespace Kaidel {
	enum StringCompareFunc_ {
		StringCompareFunc_Case_Sensetive=1<<0,
		StringCompareFunc_Case_Insensetive=1<<1
	};
	typedef int StringCompareFunc;
	class StringComparer
	{
	public:
		StringComparer(StringCompareFunc func);
		static bool Compare(const std::string& lhs, const std::string& rhs, StringCompareFunc func);
		bool operator()(const std::string& lhs, const std::string& rhs);
	private:
		StringCompareFunc m_CompareFunc;
	};

}

