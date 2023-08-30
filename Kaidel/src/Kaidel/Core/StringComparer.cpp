#include "KDpch.h"
#include "StringComparer.h"
namespace Kaidel {
	StringComparer::StringComparer(StringCompareFunc func)
		:m_CompareFunc(func)
	{

	}

	bool StringComparer::Compare(const std::string& lhs, const std::string& rhs, StringCompareFunc func)
	{
		if (lhs.length() != rhs.length())
			return false;

	}

	bool StringComparer::operator()(const std::string& lhs, const std::string& rhs)
	{
		return StringComparer::Compare(lhs, rhs, m_CompareFunc);
	}

}
