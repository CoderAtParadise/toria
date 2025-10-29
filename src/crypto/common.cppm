export module toria.crypto:common;

import std;

namespace toria::crypto
{
	export enum class hash_err { SUCCESS, ALREADY_FINALIZED, NOT_FINALIZED };

	export template<class T>
	concept is_hashing_algorithm = requires(T algo) {
		T::hash_size;
		T::hash_size_bits;
		T::message_block_size;
		{ algo.reset() } -> std::same_as<void>;
		{ algo.update(std::declval<const std::span<const std::byte>>()) } -> std::same_as<hash_err>;
		{ algo.update(std::declval<const std::byte>()) } -> std::same_as<hash_err>;
		{ algo.finalize() } -> std::same_as<hash_err>;
	} && requires(const T algo) {
		{ algo.get_digest(std::declval<std::span<std::byte>>()) } -> std::same_as<hash_err>;
	};

	constexpr std::uint32_t parity(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z) {
		return x ^ y ^ z;
	}

	constexpr std::uint32_t ch(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z) {
		return (x & y) | (~x & z);
	}

	constexpr std::uint32_t maj(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z) {
		return (x & y) | (x & z) | (y & z);
	}

	export constexpr std::byte zero_byte{0};
	export constexpr std::byte max_byte{255};
}  // namespace toria::crypto
