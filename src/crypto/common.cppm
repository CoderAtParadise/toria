export module toria.crypto:common;
#ifdef __INTELLISENSE__
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <span>
#else
import std;
#endif
namespace toria
{
	namespace crypto
	{
		export template<class T>
		concept HashAlgo = requires (T algo) {
			T::HashSize;
			T::HashSizeBits;
			T::MessageBlockSize;
			{ algo.reset() } -> std::same_as<void>;
			{ algo.update(std::declval<const std::span<const std::byte>>()) } -> std::same_as<void>;
			{ algo.update(std::declval<const std::byte>()) } -> std::same_as<void>;
			{ algo.finalize() } -> std::same_as<void>;
			
		} && requires (const T algo) {
			{ algo.get_digest(std::declval<std::span<std::byte>>()) } -> std::same_as<void>;
		};

		export constexpr std::uint32_t
		left_rotate(std::uint32_t value, const std::size_t count) noexcept {
			return (value << count) ^ (value >> (32 - count));
		}

		export constexpr std::uint32_t parity(std::uint32_t x, std::uint32_t y, std::uint32_t z) {
			return x ^ y ^ z;
		}

		export constexpr std::uint32_t ch(std::uint32_t x, std::uint32_t y, std::uint32_t z) {
			return (x & y) | (~x & z);
		}

		export constexpr std::uint32_t maj(std::uint32_t x, std::uint32_t y, std::uint32_t z) {
			return (x & y) | (x & z) | (y & z);
		}
	}  // namespace crypto
}  // namespace toria
