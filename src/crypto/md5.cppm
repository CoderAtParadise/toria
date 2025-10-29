export module toria.crypto:md5;
import std;
import toria.util;
import :common;

namespace toria::crypto
{
	export class md5
	{
	public:
		static constexpr std::size_t hash_size = 4 * sizeof(std::uint32_t);
		static constexpr std::size_t hash_size_bits = hash_size * 8;
		static constexpr std::size_t message_block_size = 64;

		constexpr md5() noexcept { reset(); }

		constexpr void reset() noexcept {
			m_digest[0] = 0x67452301;
			m_digest[1] = 0xEFCDAB89;
			m_digest[2] = 0x98BADCFE;
			m_digest[3] = 0x10325476;
			m_bits[0] = 0;
			m_bits[1] = 0;
			m_finalized = false;
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		constexpr hash_err update(const std::byte byte) {
			std::array temp{byte};
			return update(temp);
		}

		constexpr hash_err update(const std::span<const std::byte> bytes) {
			return this->update(bytes, false);
		}

		constexpr hash_err finalize() noexcept {
			if (m_finalized)
				return hash_err::ALREADY_FINALIZED;
			m_finalized = true;
			std::size_t used = (m_bits[0] >> 3) & 0x3F;
			const std::size_t count = 64u - 1u - used;
			std::span block{m_block};
			block[used++] = 0x80;
			if (count < 8) {
				util::memset(block.subspan(used), 0, count);
				transform();
				util::memset(block, 0, 56);
			}
			else
				util::memset(block.subspan(used), 0, count - 8);
			const std::span<const std::uint32_t, 2> countBytes{m_bits};
			util::memcpy(block.subspan(56), countBytes);
			transform();
			return hash_err::SUCCESS;
		}

		template<std::size_t Size>
		[[nodiscard]] constexpr hash_err
		get_digest(std::span<std::byte, Size> bytesOut) const noexcept {
			if (!m_finalized)
				return hash_err::NOT_FINALIZED;
			std::span digest{m_digest};
			util::memcpy(bytesOut, digest);
			return hash_err::SUCCESS;
		}

	private:
		std::array<std::uint32_t, hash_size / sizeof(std::uint32_t)> m_digest{};
		std::array<std::uint32_t, 2> m_bits{};
		bool m_finalized = false;
		std::array<std::uint8_t, message_block_size> m_block{};

		static constexpr std::uint32_t S[]{7, 12, 17, 22, 5, 9,  14, 20,
										   4, 11, 16, 23, 6, 10, 15, 21};
		static constexpr std::uint32_t K[]{
			0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613,
			0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193,
			0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d,
			0x2441453,  0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
			0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122,
			0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
			0xd4ef3085, 0x4881d05,  0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, 0xf4292244,
			0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
			0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb,
			0xeb86d391};

	private:
		/*
		 * Basic MD5 functions which have been optimized based on Colin Plub's implementation
		 */
		static constexpr auto F(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z) {
			return z ^ (x & (y ^ z));
		}

		static constexpr auto G(const std::uint32_t x, const std::uint32_t y,const std::uint32_t z) {
			return y ^ (z & (x ^ y));
		}

		static constexpr auto I(const std::uint32_t x, const std::uint32_t y, const std::uint32_t z) {
			return y ^ (x | ~z);
		}

		static constexpr void Step(
			std::invocable<std::uint32_t, std::uint32_t, std::uint32_t> auto f, std::uint32_t& w,
			std::uint32_t x, std::uint32_t y, std::uint32_t z, std::uint32_t data,
			std::uint32_t S) {
			w += f(x, y, z) + data;
			w = std::rotl(w, S);
			w += x;
		}

		constexpr hash_err update(const std::span<const std::byte> bytes,const bool finalizing) {
			if (m_finalized && !finalizing)
				return hash_err::ALREADY_FINALIZED;
			std::size_t used = m_bits[0];
			std::size_t bytesLen = bytes.size();
			if ((m_bits[0] += (bytesLen << 3)) < used)
				m_bits[1]++;
			m_bits[1] += bytesLen >> 29;
			used = (used >> 3) & 0x3F;
			const std::size_t available = 64 - used;
			std::size_t bytesOffset = 0;
			const std::span<std::uint8_t> block = std::span(m_block);
			if (used) {
				const auto offsetBlock = block.subspan(used);
				if (bytesLen < available) {
					util::memcpy(offsetBlock, bytes);
					return hash_err::SUCCESS;
				}
				util::memcpy(offsetBlock, bytes.subspan(0, available));
				transform();
				bytesOffset += used;
				bytesLen -= used;
			}
			while (bytesLen >= 64) {
				util::memcpy(block, bytes.subspan(bytesOffset, 64));
				transform();
				bytesLen -= 64;
				bytesOffset += 64;
			}
			if (bytesLen > 0)
				util::memcpy(block, bytes.subspan(bytesOffset, bytesLen));
			return hash_err::SUCCESS;
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		constexpr void transform() noexcept {
			std::array<std::uint32_t, 16> buffer{};
			auto [A, B, C, D] = m_digest;

			decode(buffer, m_block);

			Step(F, A, B, C, D, buffer[0] + K[0], S[0]);   /* 1 */
			Step(F, D, A, B, C, buffer[1] + K[1], S[1]);   /* 2 */
			Step(F, C, D, A, B, buffer[2] + K[2], S[2]);   /* 3 */
			Step(F, B, C, D, A, buffer[3] + K[3], S[3]);   /* 4 */
			Step(F, A, B, C, D, buffer[4] + K[4], S[0]);   /* 5 */
			Step(F, D, A, B, C, buffer[5] + K[5], S[1]);   /* 6 */
			Step(F, C, D, A, B, buffer[6] + K[6], S[2]);   /* 7 */
			Step(F, B, C, D, A, buffer[7] + K[7], S[3]);   /* 8 */
			Step(F, A, B, C, D, buffer[8] + K[8], S[0]);   /* 9 */
			Step(F, D, A, B, C, buffer[9] + K[9], S[1]);   /* 10 */
			Step(F, C, D, A, B, buffer[10] + K[10], S[2]); /* 11 */
			Step(F, B, C, D, A, buffer[11] + K[11], S[3]); /* 12 */
			Step(F, A, B, C, D, buffer[12] + K[12], S[0]); /* 13 */
			Step(F, D, A, B, C, buffer[13] + K[13], S[1]); /* 14 */
			Step(F, C, D, A, B, buffer[14] + K[14], S[2]); /* 15 */
			Step(F, B, C, D, A, buffer[15] + K[15], S[3]); /* 16 */

			/* Round 2 */
			Step(G, A, B, C, D, buffer[1] + K[16], S[4]);  /* 17 */
			Step(G, D, A, B, C, buffer[6] + K[17], S[5]);  /* 18 */
			Step(G, C, D, A, B, buffer[11] + K[18], S[6]); /* 19 */
			Step(G, B, C, D, A, buffer[0] + K[19], S[7]);  /* 20 */
			Step(G, A, B, C, D, buffer[5] + K[20], S[4]);  /* 21 */
			Step(G, D, A, B, C, buffer[10] + K[21], S[5]); /* 22 */
			Step(G, C, D, A, B, buffer[15] + K[22], S[6]); /* 23 */
			Step(G, B, C, D, A, buffer[4] + K[23], S[7]);  /* 24 */
			Step(G, A, B, C, D, buffer[9] + K[24], S[4]);  /* 25 */
			Step(G, D, A, B, C, buffer[14] + K[25], S[5]); /* 26 */
			Step(G, C, D, A, B, buffer[3] + K[26], S[6]);  /* 27 */
			Step(G, B, C, D, A, buffer[8] + K[27], S[7]);  /* 28 */
			Step(G, A, B, C, D, buffer[13] + K[28], S[4]); /* 29 */
			Step(G, D, A, B, C, buffer[2] + K[29], S[5]);  /* 30 */
			Step(G, C, D, A, B, buffer[7] + K[30], S[6]);  /* 31 */
			Step(G, B, C, D, A, buffer[12] + K[31], S[7]); /* 32 */

			/* Round 3 */
			Step(parity, A, B, C, D, buffer[5] + K[32], S[8]);   /* 33 */
			Step(parity, D, A, B, C, buffer[8] + K[33], S[9]);   /* 34 */
			Step(parity, C, D, A, B, buffer[11] + K[34], S[10]); /* 35 */
			Step(parity, B, C, D, A, buffer[14] + K[35], S[11]); /* 36 */
			Step(parity, A, B, C, D, buffer[1] + K[36], S[8]);   /* 37 */
			Step(parity, D, A, B, C, buffer[4] + K[37], S[9]);   /* 38 */
			Step(parity, C, D, A, B, buffer[7] + K[38], S[10]);  /* 39 */
			Step(parity, B, C, D, A, buffer[10] + K[39], S[11]); /* 40 */
			Step(parity, A, B, C, D, buffer[13] + K[40], S[8]);  /* 41 */
			Step(parity, D, A, B, C, buffer[0] + K[41], S[9]);   /* 42 */
			Step(parity, C, D, A, B, buffer[3] + K[42], S[10]);  /* 43 */
			Step(parity, B, C, D, A, buffer[6] + K[43], S[11]);  /* 44 */
			Step(parity, A, B, C, D, buffer[9] + K[44], S[8]);   /* 45 */
			Step(parity, D, A, B, C, buffer[12] + K[45], S[9]);  /* 46 */
			Step(parity, C, D, A, B, buffer[15] + K[46], S[10]); /* 47 */
			Step(parity, B, C, D, A, buffer[2] + K[47], S[11]);  /* 48 */

			/* Round 4 */
			Step(I, A, B, C, D, buffer[0] + K[48], S[12]);  /* 49 */
			Step(I, D, A, B, C, buffer[7] + K[49], S[13]);  /* 50 */
			Step(I, C, D, A, B, buffer[14] + K[50], S[14]); /* 51 */
			Step(I, B, C, D, A, buffer[5] + K[51], S[15]);  /* 52 */
			Step(I, A, B, C, D, buffer[12] + K[52], S[12]); /* 53 */
			Step(I, D, A, B, C, buffer[3] + K[53], S[13]);  /* 54 */
			Step(I, C, D, A, B, buffer[10] + K[54], S[14]); /* 55 */
			Step(I, B, C, D, A, buffer[1] + K[55], S[15]);  /* 56 */
			Step(I, A, B, C, D, buffer[8] + K[56], S[12]);  /* 57 */
			Step(I, D, A, B, C, buffer[15] + K[57], S[13]); /* 58 */
			Step(I, C, D, A, B, buffer[6] + K[58], S[14]);  /* 59 */
			Step(I, B, C, D, A, buffer[13] + K[59], S[15]); /* 60 */
			Step(I, A, B, C, D, buffer[4] + K[60], S[12]);  /* 61 */
			Step(I, D, A, B, C, buffer[11] + K[61], S[13]); /* 62 */
			Step(I, C, D, A, B, buffer[2] + K[62], S[14]);  /* 63 */
			Step(I, B, C, D, A, buffer[9] + K[63], S[15]);  /* 64 */

			m_digest[0] += A;
			m_digest[1] += B;
			m_digest[2] += C;
			m_digest[3] += D;
		}

		static constexpr void
		encode(std::span<std::uint8_t> output, const std::span<std::uint32_t> input) {
			if consteval {
				for (std::size_t i = 0, j = 0; j < input.size(); ++i, j += 4) {
					output[j] = static_cast<std::uint8_t>(input[i]) & 0xff;
					output[j + 1] = static_cast<std::uint8_t>(input[i] >> 8) & 0xff;
					output[j + 2] = static_cast<std::uint8_t>(input[i] >> 16) & 0xff;
					output[j + 3] = static_cast<std::uint8_t>(input[i] >> 24) & 0xff;
				}
			}
			else {
				std::memcpy(output.data(), input.data(), input.size());
			}
		}

		static constexpr void decode(
			std::span<std::uint32_t, 16> output,
			const std::span<std::uint8_t, message_block_size> input) {
			if consteval {
				for (std::size_t i = 0, j = 0; j < input.size(); ++i, j += 4) {
					output[i] = static_cast<std::uint32_t>(input[j]) |
								static_cast<std::uint32_t>(input[j + 1]) << 8 |
								static_cast<std::uint32_t>(input[j + 2]) << 16 |
								static_cast<std::uint32_t>(input[j + 3]) << 24;
				}
			}
			else {
				std::memcpy(output.data(), input.data(), input.size());
			}
		}
	};
}  // namespace toria::crypto
