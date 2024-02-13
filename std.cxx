export module toria:std;

import std;
import :uuid;


namespace std
{
	template<>
	struct formatter<toria::uuid>
	{
		constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

		auto format(const toria::uuid& uuids, std::format_context& ctx) const {
			return format_to(
				ctx.out(),
				"{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}"
				"{:02x}{:02x}{:02x}",
				uuids.m_bytes[0],
				uuids.m_bytes[1],
				uuids.m_bytes[2],
				uuids.m_bytes[3],
				uuids.m_bytes[4],
				uuids.m_bytes[5],
				uuids.m_bytes[6],
				uuids.m_bytes[7],
				uuids.m_bytes[8],
				uuids.m_bytes[9],
				uuids.m_bytes[10],
				uuids.m_bytes[11],
				uuids.m_bytes[12],
				uuids.m_bytes[13],
				uuids.m_bytes[14],
				uuids.m_bytes[15]);
		}
	};

	template<>
	struct hash<toria::uuid>
	{
		std::size_t operator()(const toria::uuid& uuid) const {
			std::uint64_t l = static_cast<uint64_t>(uuid.m_bytes[0]) << 56 |
				static_cast<uint64_t>(uuid.m_bytes[1]) << 48 |
				static_cast<uint64_t>(uuid.m_bytes[2]) << 40 |
				static_cast<uint64_t>(uuid.m_bytes[3]) << 32 |
				static_cast<uint64_t>(uuid.m_bytes[4]) << 24 |
				static_cast<uint64_t>(uuid.m_bytes[5]) << 16 |
				static_cast<uint64_t>(uuid.m_bytes[6]) << 8 |
				static_cast<uint64_t>(uuid.m_bytes[7]);
			std::uint64_t h = static_cast<uint64_t>(uuid.m_bytes[8]) << 56 |
				static_cast<uint64_t>(uuid.m_bytes[9]) << 48 |
				static_cast<uint64_t>(uuid.m_bytes[10]) << 40 |
				static_cast<uint64_t>(uuid.m_bytes[11]) << 32 |
				static_cast<uint64_t>(uuid.m_bytes[12]) << 24 |
				static_cast<uint64_t>(uuid.m_bytes[13]) << 16 |
				static_cast<uint64_t>(uuid.m_bytes[14]) << 8 |
				static_cast<uint64_t>(uuid.m_bytes[15]);
			return std::size_t(l ^ h);
		}
	};
}
