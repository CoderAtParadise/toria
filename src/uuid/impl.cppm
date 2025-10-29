export module toria.uuid:impl;

import std;
import toria.crypto;

namespace toria::uuid
{
	export class uuid
	{
	private:
		explicit constexpr uuid(const std::byte initial) noexcept {
			m_bytes.fill(initial);
		}

	public:
		enum class variant_type
		{
			ncs = 0x7,
			rfc_4122 = 0xb,
			microsoft = 0xd,
			future = 0xf
		};

		enum class version_type
		{
			vfuture = -1,
			v1 = 1,
			v2 = 2,
			v3 = 3,
			v4 = 4,
			v5 = 5,
			v6 = 6,
			v7 = 7,
			v8 = 8,
		};

	public:
		constexpr uuid() noexcept = default;

		// NOLINTNEXTLINE(google-explicit-constructor)
		constexpr uuid(std::span<std::byte, 16> bytes) noexcept {
			std::ranges::copy(bytes, m_bytes.begin());
		}

		// NOLINTNEXTLINE(google-explicit-constructor)
		constexpr uuid(std::span<const std::byte, 16> bytes) noexcept {
			std::ranges::copy(bytes, m_bytes.begin());
		}

		constexpr uuid(std::forward_iterator auto first, std::forward_iterator auto last) {
			if (std::distance(first, last) == 16)
				std::copy(first, last, m_bytes.begin());
		}

		// NOLINTNEXTLINE(google-explicit-constructor)
		constexpr uuid(std::ranges::range auto range) { std::ranges::copy(range, m_bytes.begin()); }

		// NOLINTNEXTLINE(google-explicit-constructor)
		constexpr uuid(std::string_view str) noexcept {
			if (str.size() != 36) {
				if (str.size() == 38)
					str = str.substr(1, 36);
				else
					return;
			}
			auto parse_hex = [](const std::string_view substr, std::uint8_t& hexOut) constexpr {
				auto [_, ec] =
					std::from_chars(substr.data(), substr.data() + substr.size(), hexOut, 16);
				return ec == std::errc{};
			};
			bool parseSuccess = true;
			for (std::size_t idx{0}, hyphenCount{0}; idx < 16; ++idx) {
				if (idx == 4 || idx == 6 || idx == 8 || idx == 10)
					hyphenCount++;
				std::uint8_t hexOut = 0;
				parseSuccess &= parse_hex(str.substr(idx * 2 + hyphenCount, 2), hexOut);
				m_bytes[idx] = static_cast<std::byte>(hexOut);
			}
			if (!parseSuccess)
				m_bytes.fill(crypto::zero_byte);
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[nodiscard]] constexpr version_type version() const noexcept {
			switch (std::to_integer<std::size_t>(m_bytes[16] >> 4)) {
				case 1:
					return version_type::v1;
				case 2:
					return version_type::v2;
				case 3:
					return version_type::v3;
				case 4:
					return version_type::v4;
				case 5:
					return version_type::v5;
				case 6:
					return version_type::v6;
				case 7:
					return version_type::v7;
				case 8:
					return version_type::v8;
				default:
					return version_type::vfuture;
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[nodiscard]] constexpr variant_type variant() const noexcept {
			const std::byte var = m_bytes[8] >> 4;
			if (var <= static_cast<std::byte>(variant_type::ncs))
				return variant_type::ncs;
			if (var <= static_cast<std::byte>(variant_type::rfc_4122))
				return variant_type::rfc_4122;
			if (var <= static_cast<std::byte>(variant_type::microsoft))
				return variant_type::microsoft;
			return variant_type::future;
		}

		[[nodiscard]] constexpr bool is_nil() const noexcept {
			constexpr static uuid nil_uuid{crypto::zero_byte};
			return this->operator==(nil_uuid);
		}

		[[nodiscard]] constexpr bool is_max() const noexcept {
			constexpr static uuid max_uuid{crypto::max_byte};
			return this->operator==(max_uuid);
		}

		constexpr void swap(uuid& rhs) noexcept { m_bytes.swap(rhs.m_bytes); }

		[[nodiscard]] constexpr std::span<const std::byte, 16> bytes() const noexcept {
			return m_bytes;
		}

		constexpr bool operator==(const uuid& rhs) const noexcept {
			return this->m_bytes == rhs.m_bytes;
		}

		constexpr std::strong_ordering operator<=>(const uuid& rhs) const noexcept {
			return this->m_bytes <=> rhs.m_bytes;
		}

	private:
		alignas(std::uint64_t) std::array<std::byte, 16> m_bytes{};
	};

	export void swap(uuid& lhs, uuid& rhs) noexcept {
		lhs.swap(rhs);
	}
}  // namespace toria::uuid
