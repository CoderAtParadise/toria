export module toria.uuid:sha1;
#ifdef __INTELLISENSE__
#include <cstddef>
#include <cstdint>
#include <memory>
#else
import std;
#endif  // __INTELLISENSE__


namespace toria
{
	namespace uuid
	{
		namespace crypto
		{
			class sha1
			{
			public:
				using digest32_t = std::uint32_t[5];
				using digest8_t = std::uint8_t[20];

				sha1() { reset(); }

				void reset() noexcept {
					m_digest[0] = 0x67452301;
					m_digest[1] = 0xEFCDAB89;
					m_digest[2] = 0x98BADCFE;
					m_digest[3] = 0x10325476;
					m_digest[4] = 0xC3D2E1F0;
					m_blockByteIndex = 0;
					m_count = 0;
				}

				void process_byte(std::uint8_t octet) {
					this->m_block[this->m_blockByteIndex++] = octet;
					++this->m_count;
					if (m_blockByteIndex == BLOCK_SIZE) {
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
					const std::size_t bitCount = this->m_count * 8;
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
				static constexpr std::uint32_t BLOCK_SIZE = 64;
				digest32_t m_digest;
				std::uint8_t m_block[BLOCK_SIZE];
				std::size_t m_blockByteIndex;
				std::size_t m_count;

			private:
				static std::uint32_t
				left_rotate(std::uint32_t value, const std::size_t count) noexcept {
					return (value << count) ^ (value >> (32 - count));
				}

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
							f = (b & c) | (~b & d);  // SHA_Ch
							k = 0x5A827999;
						}
						else if (i < 40) {
							f = b ^ c ^ d;  // SHA_Parity
							k = 0x6ED9EBA1;
						}
						else if (i < 60) {
							f = (b & c) | (b & d) | (c & d);  // SHA_Maj
							k = 0x8F1BBCDC;
						}
						else {
							f = b ^ c ^ d;  // SHA_Parity
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
			};
		}  // namespace crypto
	}  // namespace detail
}  // namespace toria
