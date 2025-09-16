export module toria.util:byte_utils;
#ifdef __INTELLISENSE__
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <numeric>
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

		template<class T>
		concept is_byte_type = std::same_as<std::byte, std::decay_t<T>>;

		template<class T>
		class byte_wrapper
		{
		public:
			static constexpr std::size_t size_byte = sizeof(T);
			using byte_type = std::conditional_t<std::is_const_v<T>, const std::byte, std::byte>;

			template<is_byte_type ByteType, std::size_t Size>
			constexpr byte_wrapper(std::span<ByteType, Size> bytes) noexcept
				requires(Size >= size_byte)
			{
				for (std::size_t idx = 0; idx < size_byte; idx++) {
					m_bytes[idx] = bytes[idx];
				}
			}

			constexpr byte_wrapper(const T& value) noexcept
				: m_bytes(std::bit_cast<std::array<std::byte, size_byte>>(value)) {}

			constexpr byte_type& operator[](std::size_t idx) noexcept { return m_bytes[idx]; }

			constexpr operator T() noexcept { return std::bit_cast<T>(m_bytes); }

			[[nodiscard]] constexpr T value() const noexcept { return std::bit_cast<T>(m_bytes); }

			constexpr operator std::span<byte_type, size_byte>() noexcept { return m_bytes; }

			constexpr std::span<byte_type, size_byte> bytes() const noexcept { return m_bytes; }

			static constexpr std::integral_constant<std::size_t, size_byte> size;

		private:
			std::array<std::byte, size_byte> m_bytes{std::byte{0}};
		};

		template<class T, std::size_t Size>
		class byte_wrapper<std::span<T, Size>>
		{
		public:
			static constexpr std::size_t size_byte = sizeof(T);
			using byte_type = std::conditional_t<std::is_const_v<T>, const std::byte, std::byte>;

			constexpr byte_wrapper(std::span<T, Size> val) noexcept : m_original(val) {
				m_bytes.resize(val.size_bytes());
				std::span<std::byte> bytes{m_bytes};
				for (std::size_t idx = 0; idx < val.size(); idx++) {
					byte_wrapper<T> subval{m_original[idx]};
					for (std::size_t offset = 0; offset < size_byte; offset++) {
						bytes[idx * size_byte + offset] = subval[offset];
					}
				}
			}

			constexpr byte_type& operator[](std::size_t idx) noexcept { return m_bytes[idx]; }

			constexpr operator std::span<T, Size>() noexcept
				requires(!std::is_const_v<T>)
			{
				std::span<byte_type, Size> bytes{m_bytes};
				for (std::size_t idx = 0; idx < m_original.size(); idx++) {
					m_original[idx] =
						byte_wrapper<T>(bytes.subspan(idx * size_byte, size_byte)).value();
				}
				return m_original;
			}

			constexpr operator std::span<byte_type>() noexcept { return m_bytes; }

			constexpr std::span<byte_type> bytes() const noexcept { return m_bytes; }

			constexpr std::size_t size() noexcept { return m_bytes.size(); };

		private:
			std::span<T, Size> m_original;
			std::vector<std::byte> m_bytes{};
		};

		template<is_byte_type Byte, std::size_t Size>
		class byte_wrapper<std::span<Byte, Size>>
		{
		public:
			using byte_type = Byte;
			constexpr byte_wrapper(std::span<byte_type, Size> bytes) noexcept : m_bytes(bytes) {}
			constexpr operator std::span<byte_type, Size>() noexcept { return m_bytes; }
			constexpr std::size_t size() noexcept { return m_bytes.size(); }
			constexpr byte_type& operator[](std::size_t idx) noexcept { return m_bytes[idx]; }

		private:
			std::span<byte_type, Size> m_bytes;
		};

		export template<class To, std::size_t Size>
		constexpr To from_bytes(std::span<std::byte, Size> bytes)
			requires(std::is_trivially_copyable_v<To>)
		{
			return byte_wrapper<To>(bytes);
		}

		export template<class To, std::size_t Size>
		constexpr To from_bytes(std::span<const std::byte, Size> bytes)
			requires(std::is_trivially_copyable_v<To>)
		{
			return byte_wrapper<To>(bytes);
		}

		export template<class T>
		constexpr auto as_bytes(const T& val) {
			return byte_wrapper<const T>{val};
		}

		export template<class CharT, class Traits>
		constexpr auto as_bytes(std::basic_string_view<CharT, Traits> sv) {
			return byte_wrapper<std::span<const CharT>>{
				std::span<const CharT>{sv.data(), sv.size()}};
		}

		// export template<class Dest, class Src>
		// constexpr void memcpy(Dest& dest, const Src& src) {
		//	std::size_t count = std::min(byte_wrapper<Dest>::size(), byte_wrapper<Src>::size());
		//	if consteval {
		//		byte_wrapper<Dest> destBytes{dest};
		//		byte_wrapper<const Src> srcBytes{src};
		//		for (std::size_t idx = 0; idx < count; idx++) {
		//			destBytes[idx] = srcBytes[idx];
		//		}
		//		dest = destBytes;
		//	}
		//	else {
		//		std::memcpy(&dest, &src, count);
		//	}
		// }

		export template<class Dest, std::size_t DestSize, class Src, std::size_t SrcSize>
		constexpr void memcpy(std::span<Dest, DestSize> dest, std::span<const Src, SrcSize> src) {
			std::size_t count = std::min(dest.size_bytes(), src.size_bytes());
			if consteval {
				byte_wrapper<std::span<Dest, DestSize>> destBytes{dest};
				byte_wrapper<std::span<const Src, SrcSize>> srcBytes{src};
				for (std::size_t idx = 0; idx < count; idx++) {
					destBytes[idx] = srcBytes[idx];
				}
				dest = destBytes;
			}
			else {
				std::memcpy(dest.data(), src.data(), count);
			}
		}

		// export template<class Dest, class Src>
		// constexpr void memcpy(Dest& dest, const Src& src, std::size_t count) {
		//	count =
		//		std::min(byte_wrapper<Dest>::size(), std::min(byte_wrapper<Src>::size(), count));
		//	if consteval {
		//		byte_wrapper<Dest> destBytes{dest};
		//		byte_wrapper<const Src> srcBytes{src};
		//		for (std::size_t idx = 0; idx < count; idx++) {
		//			destBytes[idx] = srcBytes[idx];
		//		}
		//		dest = destBytes;
		//	}
		//	else {
		//		std::memcpy(&dest, &src, count);
		//	}
		// }

		export template<class Dest, std::size_t Size>
		constexpr void memset(std::span<Dest, Size> dest, int val, std::size_t count) {
			count = std::min(dest.size_bytes(), count);
			if consteval {
				byte_wrapper<std::span<Dest, Size>> destBytes{dest};
				for (std::size_t idx = 0; idx < count; idx++) {
					destBytes[idx] = std::byte(val);
				}
			}
			else {
				std::memset(dest.data(), val, count);
			}
		}
	}  // namespace util
}  // namespace toria
