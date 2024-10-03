#pragma once
#include <string>

inline std::wstring to_wstring(std::string str) {
	return std::wstring(str.begin(), str.end());
}

inline std::string to_string(std::wstring str) {
	return std::string(str.begin(), str.end());
}