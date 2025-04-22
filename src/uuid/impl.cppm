export module toria.uuid:impl;
#ifdef __INTELLISENSE__
#include <array>
#include <charconv>
#include <format>
#include <ranges>
#include <span>
#else
import std;
#endif

namespace toria
{
	namespace uuid
	{
		export class uuid
		{
		private:
			constexpr uuid(std::uint8_t inital) noexcept { m_bytes.fill(inital); }

		public:
			enum class variant
			{
				ncs,
				rfc,
				microsoft,
				reserved,
				nil,
				max,
			};

		public:
			constexpr uuid() noexcept = default;

			constexpr uuid(std::uint8_t (&arr)[16]) noexcept {
				std::copy(std::cbegin(arr), std::cend(arr), m_bytes.begin());
			}

			constexpr uuid(std::span<std::uint8_t> bytes) noexcept {
				std::ranges::copy(bytes, m_bytes.begin());
			}
			constexpr uuid(const std::array<std::uint8_t, 16>& arr) noexcept
				: m_bytes(arr) {}

			constexpr uuid(std::forward_iterator auto first, std::forward_iterator auto last) {
				if (std::distance(first, last) == 16)
					std::copy(first, last, m_bytes.begin());
			}

			constexpr uuid(std::ranges::range auto range)
				: uuid(range.begin(), range.end()) {}

			constexpr uuid(std::string_view str) noexcept {
				if (str.size() != 36)
					return;
				auto parse_hex = [](std::string_view str, std::uint8_t& hexOut) {
					auto [_, ec] = std::from_chars(str.data(), str.data() + str.size(), hexOut, 16);
					return ec == std::errc{};
				};
				bool parseSuccess = true;
				for (std::size_t idx{0}, hyphenCount{0}; idx < 16; idx++) {
					if (idx == 4 || idx == 6 || idx == 8 || idx == 10)
						hyphenCount++;
					parseSuccess = parseSuccess &&
						parse_hex(str.substr(idx * 2 + hyphenCount, 2), m_bytes[idx]);
				}
				if (!parseSuccess)
					m_bytes = {{0x00}};
			}

			[[nodiscard]] constexpr std::size_t version() const noexcept { return m_bytes[6] >> 4; }

			[[nodiscard]] constexpr variant variant() const noexcept {
				auto var = m_bytes[8] >> 4;
				if (var <= 0b0111)
					return is_nil() ? variant::nil : variant::ncs;
				else if (var <= 0b1011)
					return variant::rfc;
				else if (var <= 0b1101)
					return variant::microsoft;
				else
					return is_max() ? variant::max : variant::reserved;
			}

			[[nodiscard]] constexpr bool is_nil() const noexcept {
				constexpr static uuid nil_uuid{};
				return this->operator==(nil_uuid);
			}

			[[nodiscard]] constexpr bool is_max() const noexcept {
				constexpr static uuid max_uuid{static_cast<std::uint8_t>(0xff)};
				return this->operator==(max_uuid);
			}

			constexpr void swap(uuid& rhs) noexcept { m_bytes.swap(rhs.m_bytes); }

			[[nodiscard]] std::span<const std::byte, 16> bytes() const noexcept {
				return std::span<const std::byte, 16>{
					reinterpret_cast<const std::byte*>(m_bytes.data()), 16};
			}

			constexpr bool operator==(const uuid& rhs) const noexcept {
				return this->m_bytes == rhs.m_bytes;
			}

			constexpr std::strong_ordering operator<=>(const uuid& rhs) const noexcept {
				return this->m_bytes <=> rhs.m_bytes;
			}

		private:
			std::array<std::uint8_t, 16> m_bytes{{0}};
			friend std::formatter<uuid>;
			friend std::hash<uuid>;
		};

		export void swap(uuid& lhs, uuid& rhs) {
			lhs.swap(rhs);
		}
	}  // namespace uuid
}  // namespace toria
