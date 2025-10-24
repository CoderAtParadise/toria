export module toria.crypto:std;

import std;
import :common;
import :hash;

template<toria::crypto::is_hashing_algorithm hashing_algorithm>
struct std::formatter<toria::crypto::hash<hashing_algorithm>>
{
	// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
	constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	auto
	format(const toria::crypto::hash<hashing_algorithm>& hash, std::format_context& ctx) const {
		std::array<std::byte, hashing_algorithm::hash_size> bytes;
		auto error = hash.get_bytes(std::span<std::byte, hashing_algorithm::hash_size>(bytes));
		if (error != toria::crypto::hash_err::SUCCESS)
			throw std::format_error("Hashing Error: Bytes have not been finalized");
		auto context = ctx.out();
		for (const std::byte& byte : bytes) {
			context = std::format_to(ctx.out(), "{:02x}", std::to_integer<std::uint8_t>(byte));
		}
		return context;
	}
};
