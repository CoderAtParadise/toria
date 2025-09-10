export module toria.crypto:std;
#ifdef __INTELLISENSE__
#include <cstddef>
#include <cstdint>
#include <format>
#include "crypto/hash.cppm"
#else
import std;
import :common;
import :hash;
#endif  // __INTELLISENSE__

namespace std
{
	template<toria::crypto::is_hashing_algorithm hashing_algorithm>
	struct formatter<toria::crypto::hash<hashing_algorithm>>
	{
		constexpr auto parse(std::format_parse_context& ctx) {
			return ctx.begin();
		}

		auto format(const toria::crypto::hash<hashing_algorithm>& hash, std::format_context& ctx) const {
			std::array<std::byte, hashing_algorithm::hash_size> bytes;
			hash.get_bytes(std::span<std::byte, hashing_algorithm::hash_size>(bytes));
			auto context = ctx.out();
			for (std::byte byte : bytes) {
				context = format_to(ctx.out(), "{:02x}", std::to_integer<std::uint8_t>(byte));
			}
			return context;
		}
	};
}  // namespace std
