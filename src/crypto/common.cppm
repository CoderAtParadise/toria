export module toria.crypto:common;
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
	namespace crypto
	{

		export enum class hash_err { SUCCESS, ALREADY_FINALIZED, NOT_FINALIZED };

		export template<class T>
		concept is_hashing_algorithm = requires (T algo) {
			T::hash_size;
			T::hash_size_bits;
			T::message_block_size;
			{ algo.reset() } -> std::same_as<void>;
			{
				algo.update(std::declval<const std::span<const std::byte>>())
			} -> std::same_as<hash_err>;
			{ algo.update(std::declval<const std::byte>()) } -> std::same_as<hash_err>;
			{ algo.finalize() } -> std::same_as<hash_err>;
		} && requires (const T algo) {
			{ algo.get_digest(std::declval<std::span<std::byte>>()) } -> std::same_as<hash_err>;
		};

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
