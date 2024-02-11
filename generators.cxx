export module toria:generators;

#ifdef __INTELLISENSE__
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
struct uuid;
#endif  // __INTELLISENSE__

import std;
import :impl;
//#ifdef _WIN32
//import :windows;
//#endif  // _WIN32

namespace toria
{
	namespace generators
	{
		export std::mt19937& default_random() noexcept {
			static std::random_device rd;
			static std::mt19937 gen(rd());
			return gen;
		}

		template<class T>
		concept is_clock = std::chrono::is_clock_v<T>;

		[[nodiscard]] std::uint32_t
		left_rotate(std::uint32_t value, const std::size_t count) noexcept {
			return (value << count) ^ (value >> (32 - count));
		}

		class v3_generator
		{
		public:
			v3_generator(const uuid& nsuuid)
				: m_nsuuid(nsuuid) {}

			template<class CharType, class Traits>
			[[nodiscard]] uuid operator()(std::basic_string_view<CharType, Traits> str) {}

		private:
			class md5
			{
			public:
				using digest32_t = std::uint32_t[4];
				using digest8_t = std::uint8_t[16];

				void reset() {
					m_digest[0] = 0x6452301;
					m_digest[1] = 0xEFCDAB89;
					m_digest[2] = 0x98BADCFE;
					m_digest[3] = 0x10325476;
					m_blockByteIndex = 0;
					m_byteCount = 0;
				}

			private:
				digest32_t m_digest;
				std::size_t m_blockByteIndex;
				std::size_t m_byteCount;
			};
			md5 m_hasher;
			uuid m_nsuuid;
		};

		export template<class engine>
		class v4_generator
		{
		public:
			v4_generator(engine& engine = default_random())
				: m_generator(&engine) {}
			[[nodiscard]] uuid operator()() {
				alignas(std::uint32_t) std::uint8_t bytes[16]{};
				for (int i = 0; i < 16; i += 4)
					*reinterpret_cast<std::uint32_t*>(bytes + i) = m_distribution(*m_generator);
				// version must be 0x4
				bytes[6] &= 0x4F;
				bytes[6] |= 0x40;
				// variant must be 0x8
				bytes[8] &= 0xBF;
				bytes[8] |= 0x80;

				return uuid(std::begin(bytes), std::end(bytes));
			}

		private:
			std::uniform_int_distribution<std::uint32_t> m_distribution;
			engine* m_generator;
		};

		export class v5_generator
		{
		public:
			explicit v5_generator(const uuid& namespaceUuid) noexcept
				: m_nsuuid(namespaceUuid){};

			template<class CharType, class Traits>
			[[nodiscard]] uuid operator()(std::basic_string_view<CharType, Traits> str) {
				m_hasher.reset();
				std::byte bytes[16];
				auto nsbytes = m_nsuuid.as_bytes();
				std::copy(std::cbegin(nsbytes), std::cend(nsbytes), bytes);
				m_hasher.process_bytes(bytes, 16);

				for (std::uint32_t c : str) {
					m_hasher.process_byte(static_cast<std::uint8_t>(c & 0xFF));
					if constexpr (!std::same_as<CharType, char>) {
						m_hasher.process_byte(static_cast<std::uint8_t>((c >> 8) & 0xFF));
						m_hasher.process_byte(static_cast<std::uint8_t>((c >> 16) & 0xFF));
						m_hasher.process_byte(static_cast<std::uint8_t>((c >> 24) & 0xFF));
					}
				}

				sha1::digest8_t digest;
				m_hasher.get_digest_bytes(digest);
				digest[6] &= 0x5F;
				digest[6] |= 0x50;

				digest[8] &= 0xBF;
				digest[8] |= 0x80;

				return uuid{digest, digest + 16};
			}

		private:
			class sha1
			{
			public:
				using digest32_t = std::uint32_t[5];
				using digest8_t = std::uint8_t[20];

				static constexpr std::uint32_t block_bytes = 64;

				sha1() { reset(); }

				void reset() noexcept {
					m_digest[0] = 0x67452301;
					m_digest[1] = 0xEFCDAB89;
					m_digest[2] = 0x98BADCFE;
					m_digest[3] = 0x10325476;
					m_digest[4] = 0xC3D2E1F0;
					m_blockByteIndex = 0;
					m_byteCount = 0;
				}

				void process_byte(std::uint8_t octet) {
					this->m_block[this->m_blockByteIndex++] = octet;
					++this->m_byteCount;
					if (m_blockByteIndex == block_bytes) {
						this->m_blockByteIndex = 0;
						process_block();
					}
				}

				void process_block(void const* const start, void const* const end) {
					const std::uint8_t* begin = static_cast<const std::uint8_t*>(start);
					const std::uint8_t* finish = static_cast<const std::uint8_t*>(end);
					while (begin != finish) {
						process_byte(*begin);
						begin++;
					}
				}

				void process_bytes(void const* const data, size_t const len) {
					const std::uint8_t* block = static_cast<const std::uint8_t*>(data);
					process_block(block, block + len);
				}

				void get_digest(digest32_t digest) {
					const std::size_t bitCount = this->m_byteCount * 8;
					process_byte(0x80);
					if (this->m_blockByteIndex > 56) {
						while (m_blockByteIndex != 0) {
							process_byte(0);
						}
						while (m_blockByteIndex < 56) {
							process_byte(0);
						}
					}
					else {
						while (m_blockByteIndex < 56) {
							process_byte(0);
						}
					}
					process_byte(0);
					process_byte(0);
					process_byte(0);
					process_byte(0);
					process_byte(static_cast<std::uint8_t>((bitCount >> 24) & 0xFF));
					process_byte(static_cast<std::uint8_t>((bitCount >> 16) & 0xFF));
					process_byte(static_cast<std::uint8_t>((bitCount >> 8) & 0xFF));
					process_byte(static_cast<std::uint8_t>((bitCount) & 0xFF));

					std::memcpy(digest, m_digest, 5 * sizeof(std::uint32_t));
				}

				void get_digest_bytes(digest8_t digest) {
					digest32_t d32;
					get_digest(d32);
					size_t di = 0;
					digest[di++] = static_cast<std::uint8_t>(d32[0] >> 24);
					digest[di++] = static_cast<std::uint8_t>(d32[0] >> 16);
					digest[di++] = static_cast<std::uint8_t>(d32[0] >> 8);
					digest[di++] = static_cast<std::uint8_t>(d32[0] >> 0);

					digest[di++] = static_cast<std::uint8_t>(d32[1] >> 24);
					digest[di++] = static_cast<std::uint8_t>(d32[1] >> 16);
					digest[di++] = static_cast<std::uint8_t>(d32[1] >> 8);
					digest[di++] = static_cast<std::uint8_t>(d32[1] >> 0);

					digest[di++] = static_cast<std::uint8_t>(d32[2] >> 24);
					digest[di++] = static_cast<std::uint8_t>(d32[2] >> 16);
					digest[di++] = static_cast<std::uint8_t>(d32[2] >> 8);
					digest[di++] = static_cast<std::uint8_t>(d32[2] >> 0);

					digest[di++] = static_cast<std::uint8_t>(d32[3] >> 24);
					digest[di++] = static_cast<std::uint8_t>(d32[3] >> 16);
					digest[di++] = static_cast<std::uint8_t>(d32[3] >> 8);
					digest[di++] = static_cast<std::uint8_t>(d32[3] >> 0);

					digest[di++] = static_cast<std::uint8_t>(d32[4] >> 24);
					digest[di++] = static_cast<std::uint8_t>(d32[4] >> 16);
					digest[di++] = static_cast<std::uint8_t>(d32[4] >> 8);
					digest[di++] = static_cast<std::uint8_t>(d32[4] >> 0);
				}

			private:
				void process_block() {
					std::uint32_t w[80];
					for (size_t i = 0; i < 16; i++) {
						w[i] = static_cast<std::uint32_t>(m_block[i * 4 + 0] << 24);
						w[i] |= static_cast<std::uint32_t>(m_block[i * 4 + 1] << 16);
						w[i] |= static_cast<std::uint32_t>(m_block[i * 4 + 2] << 8);
						w[i] |= static_cast<std::uint32_t>(m_block[i * 4 + 3]);
					}
					for (size_t i = 16; i < 80; i++) {
						w[i] = left_rotate((w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]), 1);
					}

					std::uint32_t a = m_digest[0];
					std::uint32_t b = m_digest[1];
					std::uint32_t c = m_digest[2];
					std::uint32_t d = m_digest[3];
					std::uint32_t e = m_digest[4];

					for (std::size_t i = 0; i < 80; ++i) {
						std::uint32_t f = 0;
						std::uint32_t k = 0;

						if (i < 20) {
							f = (b & c) | (~b & d);
							k = 0x5A827999;
						}
						else if (i < 40) {
							f = b ^ c ^ d;
							k = 0x6ED9EBA1;
						}
						else if (i < 60) {
							f = (b & c) | (b & d) | (c & d);
							k = 0x8F1BBCDC;
						}
						else {
							f = b ^ c ^ d;
							k = 0xCA62C1D6;
						}
						uint32_t temp = left_rotate(a, 5) + f + e + k + w[i];
						e = d;
						d = c;
						c = left_rotate(b, 30);
						b = a;
						a = temp;
					}

					m_digest[0] += a;
					m_digest[1] += b;
					m_digest[2] += c;
					m_digest[3] += d;
					m_digest[4] += e;
				}

			private:
				digest32_t m_digest;
				std::uint8_t m_block[64];
				std::size_t m_blockByteIndex;
				std::size_t m_byteCount;
			};

			sha1 m_hasher;
			uuid m_nsuuid;
		};

		class v6_generator
		{};

		export template<class engine, is_clock clock>
		class v7_generator
		{
		public:
			enum class monotonicity
			{
				none,
				sub_milli,
				counter,
				sub_milli_counter
			};

			v7_generator(
				monotonicity monotonicity = monotonicity::none, engine& engine = default_random())
				: m_monotonicity(monotonicity)
				, m_generator(&engine) {}
			// TODO support optional seeded counter
			[[nodiscard]] uuid operator()() noexcept {
				alignas(std::uint32_t) std::uint8_t bytes[16]{};
				for (int i = 0; i < 16; i += 4)
					*reinterpret_cast<std::uint32_t*>(bytes + i) = m_distribution(*m_generator);

				auto now = clock::now();
				std::uint64_t time = std::chrono::time_point_cast<std::chrono::milliseconds>(now)
										 .time_since_epoch()
										 .count();
				// set time from the 48 least-significant bits
				bytes[0] = time >> 40;
				bytes[1] = time >> 32;
				bytes[2] = time >> 24;
				bytes[3] = time >> 16;
				bytes[4] = time >> 8;
				bytes[5] = time;

				if (m_monotonicity == monotonicity::sub_milli ||
					m_monotonicity == monotonicity::sub_milli_counter) {
					auto micro = std::chrono::time_point_cast<std::chrono::microseconds>(now)
									 .time_since_epoch()
									 .count();
					int precision = 4096 * ((micro - time * 1000) / 1000.0f);
					bytes[6] = precision >> 8;
					bytes[7] = precision;
				}
				else if (m_monotonicity == monotonicity::counter) {}
				if (m_monotonicity == monotonicity::sub_milli_counter) {}

				//  version must be
				bytes[6] &= 0x7F;
				bytes[6] |= 0x70;
				// variant must be 0x8
				bytes[8] &= 0xBF;
				bytes[8] |= 0x80;
				return uuid(std::begin(bytes), std::end(bytes));
			}

		private:
			std::uniform_int_distribution<std::uint32_t> m_distribution;
			engine* m_generator;
			monotonicity m_monotonicity;
		};
	}  // namespace generators

}  // namespace uuids
