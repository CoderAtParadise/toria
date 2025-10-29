export module toria.util:string_utils;

import std;
import :basic_fixed_string;

namespace toria::util
{
	export constexpr std::uint32_t as_number(const std::string_view str) {
		std::uint32_t ret = 0;
		std::from_chars(str.data(), str.data() + str.size(), ret);
		return ret;
	}

	export constexpr bool is_digits(const std::string_view str) {
		return !str.empty() &&
			   std::ranges::all_of(str, [](const char c) { return c >= '0' && c <= '9'; });
	}

	export constexpr bool is_alpha(const std::string_view str) {
		return !str.empty() && std::ranges::all_of(str, [](const char c) {
			return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
		});
	}

	export constexpr bool is_alphanum(const std::string_view str) {
		return is_alpha(str) || is_digits(str);
	}

	export constexpr std::string_view numeric = "0123456789";
	export constexpr std::string_view alpha =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	// export constexpr std::string_view alphanum = concatenate_strings(alpha, numeric);

	export constexpr bool
	contains(const std::string_view str, const std::string_view allowedCharacters) {
		return !str.empty() && std::ranges::all_of(str, [allowedCharacters](const char c) {
			return allowedCharacters.contains(c);
		});
	}

	export template<fixed_string In, fixed_string Delim>
	consteval std::size_t count() {
		if constexpr (!In.empty()) {
			std::size_t count = 0;
			std::size_t offset = 0;
			while (offset < In.size() - 1) {
				if (auto nextOffset = In.find_first_of(Delim, offset); nextOffset != In.npos) {
					++count;
					offset = nextOffset + 1;
				}
				else
					break;
			}
			return count;
		}
		return 0;
	}
}  // namespace toria::util
