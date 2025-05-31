export module toria.crypto:sha1;

#ifdef __INTELLISENSE__
#include "common.cppm"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>
#else
import std;
import :common;
#endif  // __INTELLISENSE__

namespace toria
{
	namespace crypto
	{
		export class sha1
		{
		public:
			static constexpr std::size_t HashSize = 5 * sizeof(std::uint32_t);
			static constexpr std::size_t HashSizeBits = HashSize * 8;
			static constexpr std::size_t MessageBlockSize = 64;

			constexpr void reset() {
				m_digest[0] = 0x67452301;
				m_digest[1] = 0xEFCDAB89;
				m_digest[2] = 0x98BADCFE;
				m_digest[3] = 0x10325476;
				m_digest[4] = 0xC3D2E1F0;
				m_messageBlockIndex = 0;
				m_messageLength = 0;
			}

			constexpr void update(std::byte byte) {
				m_messageBlock[m_messageBlockIndex++] = std::to_integer<std::uint8_t>(byte);
				++m_messageLength;
				if (m_messageBlockIndex == MessageBlockSize) {
					m_messageBlockIndex = 0;
					update_block();
				}
			}

			constexpr void update(const std::span<const std::byte> bytes) {
				auto it = bytes.begin();
				while (it != bytes.end()) {
					update(*it);
					it++;
				}
			}

			constexpr void finalize() {
				const std::size_t bitCount = m_messageLength * 8;
				update(std::byte(0x80));
				if (m_messageBlockIndex > 56) {
					while (m_messageBlockIndex != 0)
						update(std::byte(0));
					while (m_messageBlockIndex < 56)
						update(std::byte(0));
				}
				else {
					while (m_messageBlockIndex < 56)
						update(std::byte(0));
				}
				update(std::byte(0));
				update(std::byte(0));
				update(std::byte(0));
				update(std::byte(0));
				update(std::byte((bitCount >> 24) & 0xFF));
				update(std::byte((bitCount >> 16) & 0xFF));
				update(std::byte((bitCount >> 8) & 0xFF));
				update(std::byte(bitCount & 0xFF));
				for (std::size_t idx = 0; idx < HashSize / sizeof(std::uint32_t); idx++) {
					m_digest[idx] = std::byteswap(m_digest[idx]);
				}
			}

			constexpr void get_digest(std::span<std::byte> bytesOut) const {
				std::size_t count = bytesOut.size() / sizeof(std::uint32_t);
				if consteval {
					for (std::size_t idx = 0; idx < count; idx++) {
						bytesOut[idx * 4] = std::byte(m_digest[idx] & 0xFF);
						bytesOut[idx * 4 + 1] = std::byte((m_digest[idx] >> 8) & 0xFF);
						bytesOut[idx * 4 + 2] = std::byte((m_digest[idx] >> 16) & 0xFF);
						bytesOut[idx * 4 + 3] = std::byte((m_digest[idx] >> 24) & 0xFF);
					}
				}
				else {
					std::memcpy(bytesOut.data(), m_digest, bytesOut.size());
				}
			}

		private:
			std::uint32_t m_digest[HashSize / sizeof(std::uint32_t)];
			std::size_t m_messageLength;
			std::uint16_t m_messageBlockIndex;
			std::uint8_t m_messageBlock[MessageBlockSize]{};

		private:
			constexpr void update_block() {
				std::uint32_t w[80]{};
				std::size_t idx = 0;
				for (idx; idx < 16; idx++) {
					w[idx] = static_cast<std::uint32_t>(m_messageBlock[idx * 4 + 0] << 24);
					w[idx] |= static_cast<std::uint32_t>(m_messageBlock[idx * 4 + 1] << 16);
					w[idx] |= static_cast<std::uint32_t>(m_messageBlock[idx * 4 + 2] << 8);
					w[idx] |= static_cast<std::uint32_t>(m_messageBlock[idx * 4 + 3]);
				}

				for (idx = 16; idx < 80; idx++) {
					w[idx] = left_rotate((w[idx - 3] ^ w[idx - 8] ^ w[idx - 14] ^ w[idx - 16]), 1);
				}
				auto [A, B, C, D, E] = m_digest;

				for (idx = 0; idx < 80; ++idx) {
					std::uint32_t f = 0;
					std::uint32_t k = 0;

					if (idx < 20) {
						f = ch(B, C, D);  // SHA_Ch
						//f = (B & C) | (~B & D);
						k = 0x5A827999;
					}
					else if (idx < 40) {
						f = parity(B, C, D);  // SHA_PARITY
						//f = B ^ C ^ D;
						k = 0x6ED9EBA1;
					}
					else if (idx < 60) {
						f = maj(B, C, D);  // SHA_Maj
						//f = (B & C) | (B & D) | (C & D);
						k = 0x8F1BBCDC;
					}
					else {
						f = parity(B, C, D);
						//f = B ^ C ^ D;
						k = 0xCA62C1D6;
					}

					std::uint32_t temp = left_rotate(A, 5) + f + E + k + w[idx];
					E = D;
					D = C;
					C = left_rotate(B, 30);
					B = A;
					A = temp;
				}
				m_digest[0] += A;
				m_digest[1] += B;
				m_digest[2] += C;
				m_digest[3] += D;
				m_digest[4] += E;
			}
		};
	}  // namespace crypto
}  // namespace toria
