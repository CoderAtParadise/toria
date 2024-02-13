export module toria:md5;
#ifdef __INTELLISENSE__
#include <cstdint>
#include <memory>
#endif  // __INTELLISENSE__
import std;
namespace toria
{
	namespace detail
	{

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
				m_count[0] = 0;
				m_count[1] = 0;
			}

			void process_byte(std::uint8_t byte) {

			}

			void process_bytes(const void* data, std::size_t len) {
				const std::uint8_t* block = static_cast<const std::uint8_t*>(data);
				std::size_t index = m_count[0] / 8 % BLOCK_SIZE;
				if ((m_count[0] += len << 3) < len << 3)
					m_count[1]++;
				m_count[1] += len >> 29;
				std::size_t first = 64 - index;
			}

			void get_digest(digest32_t digest) {

			}

			void get_digest_bytes(digest8_t digest) { encode(digest, m_digest, 16); }

		private:
			static constexpr std::uint32_t s[]{
				7, 12, 22, 17, 5, 9, 14, 20, 4, 11, 16, 23, 6, 10, 15, 21};

			static constexpr std::uint32_t k[]{
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
			static constexpr std::uint32_t BLOCK_SIZE = 64;
			digest32_t m_digest;
			std::uint32_t m_count[2];
			std::uint8_t m_block[BLOCK_SIZE];

		private:
			static std::uint32_t
			left_rotate(std::uint32_t value, const std::size_t count) noexcept {
				return (value << count) ^ (value >> (32 - count));
			}

			static std::uint32_t f(std::uint32_t x, std::uint32_t y, std::uint32_t z) {
				return (x & y) | (~x & z);
			}

			static std::uint32_t g(std::uint32_t x, std::uint32_t y, std::uint32_t z) {
				return (x & z) | (y & ~z);
			}

			static std::uint32_t h(std::uint32_t x, std::uint32_t y, std::uint32_t z) {
				return x ^ y ^ z;
			}

			static std::uint32_t i(std::uint32_t x, std::uint32_t y, std::uint32_t z) {
				return (y ^ x) | ~z;
			}

			static void
			ff(std::uint32_t& a,
			   std::uint32_t b,
			   std::uint32_t c,
			   std::uint32_t d,
			   std::uint32_t x,
			   std::uint32_t s,
			   std::uint32_t ac) {
				a += f(b, c, d) + x + ac;
				a = left_rotate(a, s);
				a += b;
			}

			static void
			gg(std::uint32_t& a,
			   std::uint32_t b,
			   std::uint32_t c,
			   std::uint32_t d,
			   std::uint32_t x,
			   std::uint32_t s,
			   std::uint32_t ac) {
				a += g(b, c, d) + x + ac;
				a = left_rotate(a, s);
				a += b;
			}

			static void
			hh(std::uint32_t& a,
			   std::uint32_t b,
			   std::uint32_t c,
			   std::uint32_t d,
			   std::uint32_t x,
			   std::uint32_t s,
			   std::uint32_t ac) {
				a += h(b, c, d) + x + ac;
				a = left_rotate(a, s);
				a += b;
			}

			static void
			ii(std::uint32_t& a,
			   std::uint32_t b,
			   std::uint32_t c,
			   std::uint32_t d,
			   std::uint32_t x,
			   std::uint32_t s,
			   std::uint32_t ac) {
				a += i(b, c, d) + x + ac;
				a = left_rotate(a, s);
				a += b;
			}

			void process_block(std::uint8_t* block) {
				std::uint32_t a = m_digest[0];
				std::uint32_t b = m_digest[1];
				std::uint32_t c = m_digest[2];
				std::uint32_t d = m_digest[3];
				std::uint32_t x[16];

				decode(x, block, 64);

				ff(a, b, c, d, x[0], s[0], k[0]);   /* 1 */
				ff(d, a, b, c, x[1], s[1], k[1]);   /* 2 */
				ff(c, d, a, b, x[2], s[2], k[2]);   /* 3 */
				ff(b, c, d, a, x[3], s[3], k[3]);   /* 4 */
				ff(a, b, c, d, x[4], s[0], k[4]);   /* 5 */
				ff(d, a, b, c, x[5], s[1], k[5]);   /* 6 */
				ff(c, d, a, b, x[6], s[2], k[6]);   /* 7 */
				ff(b, c, d, a, x[7], s[3], k[7]);   /* 8 */
				ff(a, b, c, d, x[8], s[0], k[8]);   /* 9 */
				ff(d, a, b, c, x[9], s[1], k[9]);   /* 10 */
				ff(c, d, a, b, x[10], s[2], k[10]); /* 11 */
				ff(b, c, d, a, x[11], s[3], k[11]); /* 12 */
				ff(a, b, c, d, x[12], s[0], k[12]); /* 13 */
				ff(d, a, b, c, x[13], s[1], k[13]); /* 14 */
				ff(c, d, a, b, x[14], s[2], k[14]); /* 15 */
				ff(b, c, d, a, x[15], s[3], k[15]); /* 16 */

				/* Round 2 */
				gg(a, b, c, d, x[1], s[4], k[16]);  /* 17 */
				gg(d, a, b, c, x[6], s[5], k[17]);  /* 18 */
				gg(c, d, a, b, x[11], s[6], k[18]); /* 19 */
				gg(b, c, d, a, x[0], s[7], k[19]);  /* 20 */
				gg(a, b, c, d, x[5], s[4], k[20]);  /* 21 */
				gg(d, a, b, c, x[10], s[5], k[21]); /* 22 */
				gg(c, d, a, b, x[15], s[6], k[22]); /* 23 */
				gg(b, c, d, a, x[4], s[7], k[23]);  /* 24 */
				gg(a, b, c, d, x[9], s[4], k[24]);  /* 25 */
				gg(d, a, b, c, x[14], s[5], k[25]); /* 26 */
				gg(c, d, a, b, x[3], s[6], k[26]);  /* 27 */
				gg(b, c, d, a, x[8], s[7], k[27]);  /* 28 */
				gg(a, b, c, d, x[13], s[4], k[28]); /* 29 */
				gg(d, a, b, c, x[2], s[5], k[29]);  /* 30 */
				gg(c, d, a, b, x[7], s[6], k[30]);  /* 31 */
				gg(b, c, d, a, x[12], s[7], k[31]); /* 32 */

				/* Round 3 */
				hh(a, b, c, d, x[5], s[8], k[32]);   /* 33 */
				hh(d, a, b, c, x[8], s[9], k[33]);   /* 34 */
				hh(c, d, a, b, x[11], s[10], k[34]); /* 35 */
				hh(b, c, d, a, x[14], s[11], k[35]); /* 36 */
				hh(a, b, c, d, x[1], s[8], k[36]);   /* 37 */
				hh(d, a, b, c, x[4], s[9], k[37]);   /* 38 */
				hh(c, d, a, b, x[7], s[10], k[38]);  /* 39 */
				hh(b, c, d, a, x[10], s[11], k[39]); /* 40 */
				hh(a, b, c, d, x[13], s[8], k[40]);  /* 41 */
				hh(d, a, b, c, x[0], s[9], k[41]);   /* 42 */
				hh(c, d, a, b, x[3], s[10], k[42]);  /* 43 */
				hh(b, c, d, a, x[6], s[11], k[43]);  /* 44 */
				hh(a, b, c, d, x[9], s[8], k[44]);   /* 45 */
				hh(d, a, b, c, x[12], s[9], k[45]);  /* 46 */
				hh(c, d, a, b, x[15], s[10], k[46]); /* 47 */
				hh(b, c, d, a, x[2], s[11], k[47]);  /* 48 */

				/* Round 4 */
				ii(a, b, c, d, x[0], s[12], k[48]);  /* 49 */
				ii(d, a, b, c, x[7], s[13], k[49]);  /* 50 */
				ii(c, d, a, b, x[14], s[14], k[50]); /* 51 */
				ii(b, c, d, a, x[5], s[15], k[51]);  /* 52 */
				ii(a, b, c, d, x[12], s[12], k[52]); /* 53 */
				ii(d, a, b, c, x[3], s[13], k[53]);  /* 54 */
				ii(c, d, a, b, x[10], s[14], k[54]); /* 55 */
				ii(b, c, d, a, x[1], s[15], k[55]);  /* 56 */
				ii(a, b, c, d, x[8], s[12], k[56]);  /* 57 */
				ii(d, a, b, c, x[15], s[13], k[57]); /* 58 */
				ii(c, d, a, b, x[6], s[14], k[58]);  /* 59 */
				ii(b, c, d, a, x[13], s[15], k[59]); /* 60 */
				ii(a, b, c, d, x[4], s[12], k[60]);  /* 61 */
				ii(d, a, b, c, x[11], s[13], k[61]); /* 62 */
				ii(c, d, a, b, x[2], s[14], k[62]);  /* 63 */
				ii(b, c, d, a, x[9], s[15], k[63]);  /* 64 */

				m_digest[0] += a;
				m_digest[1] += b;
				m_digest[2] += c;
				m_digest[3] += d;
			}

			void encode(std::uint8_t* output, std::uint32_t* input, std::size_t len) {
				for (std::size_t i = 0, j = 0; j < len; i++, j += 4) {
					output[j] = static_cast<std::uint8_t>(input[i]) & 0xff;
					output[j + 1] = static_cast<std::uint8_t>(input[i] >> 8) & 0xff;
					output[j + 2] = static_cast<std::uint8_t>(input[i] >> 16) & 0xff;
					output[j + 3] = static_cast<std::uint8_t>(input[i] >> 24) & 0xff;
				}
			}

			void decode(std::uint32_t* output, std::uint8_t* input, std::size_t len) {
				for (std::size_t i = 0, j = 0; j < len; i++, j += 4) {
					output[i] = static_cast<std::uint32_t>(input[j]) |
						static_cast<std::uint32_t>(input[j + 1]) << 8 |
						static_cast<std::uint32_t>(input[j + 2]) << 16 |
						static_cast<std::uint32_t>(input[j + 3]) << 24;
				}
			}
		};
	}  // namespace detail
}  // namespace toria
