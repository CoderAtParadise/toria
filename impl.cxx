export module toria:impl;

import std;

namespace toria
{
	template<class CharType>
	static constexpr std::uint8_t hex2char(const CharType ch) {
		// 0-9
		if (ch >= static_cast<CharType>('0') && ch <= static_cast<CharType>('9'))
			return static_cast<std::uint8_t>(ch - '0');
		// a-f
		if (ch >= static_cast<CharType>('a') && ch <= static_cast<CharType>('f'))
			return static_cast<std::uint8_t>(10 + ch - 'a');
		// A-F
		if (ch >= static_cast<CharType>('A') && ch <= static_cast<CharType>('F'))
			return static_cast<std::uint8_t>(10 + ch - 'A');
		return 0;
	}

	template<class CharType>
	static constexpr bool is_hex(const CharType ch) {
		return (ch >= static_cast<CharType>('0') && ch <= static_cast<CharType>('9')) ||
			(ch >= static_cast<CharType>('a') && ch <= static_cast<CharType>('f')) ||
			(ch >= static_cast<CharType>('A') && ch <= static_cast<CharType>('F'));
	}

	export enum class variant {
		ncs,
		rfc,
		microsoft,
		reserved,
		nil,
		max,
	};

	export class uuid
	{
	public:
		constexpr uuid() noexcept = default;

		constexpr uuid(std::uint8_t (&arr)[16]) noexcept {
			std::copy(std::cbegin(arr), std::cend(arr), std::begin(m_bytes));
		}

		constexpr uuid(const std::array<std::uint8_t, 16>& arr) noexcept
			: m_bytes(arr) {}

		constexpr uuid(std::span<std::uint8_t, 16> bytes) {
			std::copy(std::cbegin(bytes), std::cend(bytes), std::begin(m_bytes));
		}

		uuid(std::forward_iterator auto first, std::forward_iterator auto last) {
			if (std::distance(first, last) == 16)
				std::copy(first, last, std::begin(m_bytes));
		}

		template<class CharType, class Traits>
		constexpr uuid(std::basic_string_view<CharType, Traits> from) noexcept {
			bool firstDigit = true;
			std::size_t firstIndex = 0;
			std::size_t index = 0;
			if (from.empty())
				return;
			if (from.front() == '{')
				firstIndex = 1;
			if (firstIndex && from.back() != '}')
				return;
			for (std::size_t i = firstIndex; i < from.size() - firstIndex; i++) {
				if (from[i] == '-')
					continue;
				if (index >= 16 || !is_hex(from[i])) {
					m_bytes = {{0}};
					return;
				}
				if (firstDigit) {
					m_bytes[index] = hex2char(from[i] << 4);
				}
				else {
					m_bytes[index] = m_bytes[index] | hex2char(from[i]);
					index++;
				}
				firstDigit = !firstDigit;
			}

			if (index < 16) {
				m_bytes = {{0}};
			}
		}

		[[nodiscard]] constexpr int version() const noexcept {
			return m_bytes[6] >> 4;
		}

		[[nodiscard]] constexpr variant variant() const noexcept {
			auto n = m_bytes[8] >> 4;
			if (n <= 0b0111)
				return is_nil() ? variant::nil : variant::ncs;
			else if (n <= 0b1011)
				return variant::rfc;
			else if (n <= 0b1101)
				return variant::microsoft;
			else
				return is_max() ? variant::max : variant::reserved;
		}

		[[nodiscard]] constexpr bool is_nil() const noexcept {
			for (std::size_t i = 0; i < m_bytes.size(); ++i)
				if (m_bytes[i] != 0)
					return false;
			return true;
		}

		[[nodiscard]] constexpr bool is_max() const noexcept {
			for (std::size_t i = 0; i < m_bytes.size(); ++i)
				if (m_bytes[i] != 0xff)
					return false;
			return true;
		}

		constexpr void swap(uuid& other) noexcept { m_bytes.swap(other.m_bytes); }

		[[nodiscard]] std::span<const std::byte, 16> as_bytes() const noexcept {
			return std::span<const std::byte, 16>(
				reinterpret_cast<const std::byte*>(m_bytes.data()), 16);
		}

		friend constexpr bool operator==(const uuid& lhs, const uuid& rhs) noexcept;
		friend constexpr std::strong_ordering
		operator<=>(const uuid& lhs, const uuid& rhs) noexcept;

	private:
		std::array<std::uint8_t, 16> m_bytes{{0}};

		friend std::formatter<uuid>;
		friend std::hash<uuid>;
	};

	[[nodiscard]] constexpr bool operator==(const uuid& lhs, const uuid& rhs) noexcept {
		return lhs.m_bytes == rhs.m_bytes;
	}

	[[nodiscard]] constexpr std::strong_ordering
	operator<=>(const uuid& lhs, const uuid& rhs) noexcept {
		return lhs.m_bytes <=> rhs.m_bytes;
	}
}  // namespace uuids
