export module toria.crypto:std;
#ifdef __INTELLISENSE__
#include "hash.cppm"
#include <cstddef>
#include <cstdint>
#include <format>
#else
import std;
import :hash;
import :common;
#endif  // __INTELLISENSE__

namespace std
{
	template<toria::crypto::is_hashing_algorithm is_hashing_algorithm>
	struct formatter<toria::crypto::hash<is_hashing_algorithm>>
	{
		constexpr auto parse(std::format_parse_context& ctx) {
			return ctx.begin();
		}

		auto format(const toria::crypto::hash<is_hashing_algorithm>& hash, std::format_context& ctx) const {
			std::array<std::byte, is_hashing_algorithm::hash_size> bytes;
			hash.get_bytes(std::span<std::byte, is_hashing_algorithm::hash_size>(bytes));
			auto context = ctx.out();
			for (std::byte byte : bytes) {
				context = format_to(ctx.out(), "{:02x}", std::to_integer<std::uint8_t>(byte));
			}
			return context;
		}
	};
}  // namespace std
