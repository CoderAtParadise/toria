export module toria.util:byte_utils;
#ifdef __INTELLISENSE__
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>
#else
import std;
#endif

namespace toria
{
	namespace util
	{
		template<class T>
		concept integral_or_byte = std::integral<std::remove_const_t<T>> ||
			std::same_as<std::byte, std::remove_const_t<T>>;

		template<integral_or_byte T>
		class copy_storage
		{
			using byteType = std::conditional_t<std::is_const_v<T>, const std::byte, std::byte>;

		public:
			constexpr copy_storage() noexcept = default;
			constexpr copy_storage(std::span<byteType> spanIn) noexcept
				: m_span(spanIn) {};
			constexpr copy_storage(std::span<T> data) noexcept
				requires (std::integral<T>)
				: m_original(data) {
				as_bytes();
			}
			constexpr byteType& operator[](std::size_t idx) { return m_span[idx]; }
			constexpr std::span<T> from_bytes()
				requires (!std::is_const_v<T>)
			{
				for (std::size_t idx = 0, jdx = 0; jdx < m_span.size(); idx++, jdx += sizeof(T)) {
					m_original[idx] = std::to_integer<T>(m_span[jdx]);
					if constexpr (sizeof(T) > sizeof(std::byte)) {
						for (std::size_t sizeOffset = 1; sizeOffset < sizeof(T); sizeOffset++) {
							m_original[idx] |= std::to_integer<T>(m_span[jdx + sizeOffset])
								<< (sizeOffset * 8);
						}
					}
				}
				return m_original;
			}

		private:
			constexpr void as_bytes() {
				if consteval {
					m_arr.resize(m_original.size_bytes());
					for (auto i = 0; i < m_original.size_bytes(); i += sizeof(T)) {
						m_arr[i] = std::byte(m_original[i / sizeof(T)] & 0xFF);
						if constexpr (sizeof(T) > sizeof(std::byte)) {
							for (std::size_t offset = 1; offset < sizeof(T); offset++) {
								m_arr[i + offset] =
									std::byte((m_original[i / sizeof(T)] >> (offset * 8)) & 0xFF);
							}
						}
					}
					m_span = {m_arr};
				}
				else {
					if constexpr (std::is_const_v<T>)
						m_span = std::as_bytes(m_original);
					else
						m_span = std::as_writable_bytes(m_original);
				}
			}

			std::span<T> m_original;
			std::vector<std::byte> m_arr;
			std::span<byteType> m_span{m_arr};
		};

		export template<integral_or_byte Dest, integral_or_byte Src>
		constexpr void copy_bytes(std::span<Dest> dest, std::span<const Src> src) {
			if consteval {
				copy_storage<Dest> destBytes{dest};
				copy_storage<const Src> srcBytes{src};
				std::size_t count = std::min(dest.size_bytes(), src.size_bytes());
				for (std::size_t idx = 0; idx < count; idx++) {
					destBytes[idx] = srcBytes[idx];
				}
				if constexpr (std::integral<Dest>)
					destBytes.from_bytes();
			}
			else {
				std::size_t count = std::min(dest.size_bytes(), src.size_bytes());
				std::memcpy(dest.data(), src.data(), count);
			}
		}

		template<typename T>
		concept byte_type = std::same_as<int, T> || std::same_as<std::byte, T>;

		export template<integral_or_byte Dest, byte_type Src>
		constexpr void fill_bytes(std::span<Dest> dest, Src src, std::size_t count) {
			count = std::min(dest.size_bytes(), count);
			if consteval {
			copy_storage<Dest> destBytes{dest};
				for (std::size_t idx = 0; idx < count; idx++) {
					if constexpr (std::same_as<int, Src>)
						destBytes[idx] = std::byte(src);
					else
						destBytes[idx] = src;
				}
			}
			else {
				if constexpr (std::same_as<int, Src>)
					std::memset(dest.data(), src, count);
				else
					std::memset(dest.data(), std::to_integer<int>(src), count);
			}
		}
	}  // namespace util
}  // namespace toria
