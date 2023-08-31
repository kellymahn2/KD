#include "KDpch.h"
#include "StringComparer.h"
namespace Kaidel {
	constexpr char caseInsensetiveMask = 0b11011111;
	StringComparer::StringComparer(StringCompareFunc func)
		:m_CompareFunc(func)
	{

	}

	bool StringComparer::Compare(const std::string& lhs, const std::string& rhs, StringCompareFunc func)
	{
		if (lhs.length() != rhs.length())
			return false;
		KD_CORE_ASSERT(func & StringCompareFunc_CaseSensetive && 
			func & StringCompareFunc_CaseInsensetive, "Conflicting Values");
		if (func & StringCompareFunc_CaseSensetive)
			return CaseSensetiveCompareImpl(lhs, rhs);
		else if (func & StringCompareFunc_CaseInsensetive)
			return CaseInsensetiveCompareImpl(lhs, rhs);

	}
	bool StringComparer::operator()(const std::string& lhs, const std::string& rhs)
	{
		return StringComparer::Compare(lhs, rhs, m_CompareFunc);
	}

	bool StringComparer::CaseSensetiveCompareImpl(const std::string& lhs, const std::string& rhs)
	{
		for (size_t i = 0; i < lhs.size(); ++i) {
			if (lhs[i] != rhs[i])
				return false;
		}
		return true;
	}

	bool StringComparer::CaseInsensetiveCompareImpl(const std::string& lhs, const std::string& rhs)
	{
		for (size_t i = 0; i < lhs.size(); ++i) {
			if ((lhs[i] & caseInsensetiveMask) != (rhs[i] & caseInsensetiveMask))
				return false;
		}
		return true;
	}

}
