export module toria.uuid:std;

import std;
import :impl;

template<>
struct std::formatter<toria::uuid::uuid>
{
	// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
	constexpr auto parse(std::format_parse_context& ctx) {
		// TODO: Support printing in binary and decimal
		// Potential support python/microsoft
		return ctx.begin();
	}

	// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
	auto format(const toria::uuid::uuid& uuid, std::format_context& ctx) const {
		const std::span<const std::byte> bytes = uuid.bytes();
		return std::format_to(
			ctx.out(),
			"{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}"
			"{:02x}{:02x}{:02x}",
			std::to_integer<std::uint8_t>(bytes[0]), std::to_integer<std::uint8_t>(bytes[1]),
			std::to_integer<std::uint8_t>(bytes[2]), std::to_integer<std::uint8_t>(bytes[3]),
			std::to_integer<std::uint8_t>(bytes[4]), std::to_integer<std::uint8_t>(bytes[5]),
			std::to_integer<std::uint8_t>(bytes[6]), std::to_integer<std::uint8_t>(bytes[7]),
			std::to_integer<std::uint8_t>(bytes[8]), std::to_integer<std::uint8_t>(bytes[9]),
			std::to_integer<std::uint8_t>(bytes[10]), std::to_integer<std::uint8_t>(bytes[11]),
			std::to_integer<std::uint8_t>(bytes[12]), std::to_integer<std::uint8_t>(bytes[13]),
			std::to_integer<std::uint8_t>(bytes[14]), std::to_integer<std::uint8_t>(bytes[15]));
	}
};

template<>
struct std::hash<toria::uuid::uuid>
{
	std::size_t operator()(const toria::uuid::uuid& uuid) const noexcept {
#if __cpp_lib_start_lifetime_as >= 202207L
		const std::uint64_t* bytes =
			std::start_lifetime_as_array<const std::uint64_t>(uuid.bytes().data(), 2);
#else
		const std::uint64_t* bytes = reinterpret_cast<const std::uint64_t*>(uuid.bytes().data());
#endif

		return bytes[0] ^ bytes[1];
	}
};
