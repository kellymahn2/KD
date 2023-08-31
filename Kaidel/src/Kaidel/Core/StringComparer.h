#pragma once
#include <string>
namespace Kaidel {
	enum StringCompareFunc_ {
		StringCompareFunc_CaseSensetive=1<<0,
		StringCompareFunc_CaseInsensetive=1<<1
	};
	typedef int StringCompareFunc;
	class StringComparer
	{
	public:
		StringComparer(StringCompareFunc func);
		static bool Compare(const std::string& lhs, const std::string& rhs, StringCompareFunc func);
		bool operator()(const std::string& lhs, const std::string& rhs);
	private:
		static bool CaseSensetiveCompareImpl(const std::string& lhs, const std::string&rhs);
		static bool CaseInsensetiveCompareImpl(const std::string& lhs, const std::string& rhs);
		StringCompareFunc m_CompareFunc;
	};

}

