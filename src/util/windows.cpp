module;
#include <windows.h>
module toria.util;

namespace toria::util
{
	std::wstring to_wide_string(const std::string_view str) {
		const int count =
			MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
		auto wStr = std::wstring(count + 10, 0);
		MultiByteToWideChar(
			CP_UTF8, 0, &str[0], static_cast<int>(str.size()), wStr.data(), count);
		return wStr;
	}

	std::string from_wide_string(const std::wstring_view str) {
		if (str.empty()) return "";
		const int count = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()),
		                                      nullptr, 0, nullptr, nullptr);

		auto out = std::string(count, 0);
		WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), out.data(), count,
		                    nullptr, nullptr);
		return out;
	}
}
