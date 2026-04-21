export module toria.util:string_utils;

import std;
import :basic_fixed_string;

namespace toria::util
{
	export std::wstring to_wide_string(std::string_view str);

	export std::string from_wide_string(std::wstring_view str);

	export std::string_view ltrim(const std::string_view str,const std::string_view whitespace = " \t") {
		return str.substr(str.find_first_not_of(whitespace));
	}

	export std::string_view rtrim(const std::string_view str,const std::string_view whitespace = " \t") {
		return str.substr(0,str.find_last_not_of(whitespace) + 1);
	}

	export std::string_view trim(const std::string_view str,const std::string_view whitespace = " \t") {
		return ltrim(rtrim(str, whitespace), whitespace);
	}

	export constexpr bool isalpha(const char c) {
			return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
	}

	export constexpr bool isdigits(const char c) {
		return c >= '0' && c <= '9';
	}

	export constexpr bool isalnum(const char c) {
		return isalpha(c) || isdigits(c);
	}
}  // namespace toria::util
