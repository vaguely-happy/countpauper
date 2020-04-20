#pragma once

#include <string>
#include <vector>
#include <set>

namespace Engine
{
    std::vector<std::wstring> Split(const std::wstring& str, wchar_t delimiter);
    std::vector<std::string> Split(const std::string& str, char delimiter);
    std::set<std::wstring> SplitSet(const std::wstring& str, wchar_t delimiter);
    std::set<std::string> SplitSet(const std::string& str, char delimiter);
	extern const std::wstring_view whitespace;
    std::wstring Strip(const std::wstring& str, const std::wstring_view& trash = whitespace);
    std::string UpperCase(const std::string& str);

}