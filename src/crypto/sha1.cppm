export module toria.crypto:sha1;

import std;
import toria.util;
import :common;
namespace toria::crypto
{
	export class sha1
	{
	public:
		static constexpr std::size_t hash_size = 5 * sizeof(std::uint32_t);
		static constexpr std::size_t hash_size_bits = hash_size * 8;
		static constexpr std::size_t message_block_size = 64;

		constexpr sha1() noexcept { this->reset(); }

		constexpr void reset() noexcept {
			m_digest[0] = 0x67452301;
			m_digest[1] = 0xEFCDAB89;
			m_digest[2] = 0x98BADCFE;
			m_digest[3] = 0x10325476;
			m_digest[4] = 0xC3D2E1F0;
			m_messageBlock.fill(0);
			m_messageBlockIndex = 0;
			m_messageLength = 0;
			m_finalized = false;
		}

		constexpr hash_err update(const std::byte byte) noexcept {
			return this->update(byte, false);
		}

		// NOLINTNEXTLINE(readability-make-member-function-const)
		constexpr hash_err update(const std::span<const std::byte> bytes) noexcept {
			if (m_finalized)
				return hash_err::ALREADY_FINALIZED;
			auto it = bytes.begin();
			while (it != bytes.end()) {
				update(*it);
				++it;
			}
			return hash_err::SUCCESS;
		}

		constexpr hash_err finalize() noexcept {
			if (m_finalized)
				return hash_err::ALREADY_FINALIZED;
			m_finalized = true;
			const std::size_t bitCount = m_messageLength * 8;
			update(static_cast<std::byte>(0x80), true);
			if (m_messageBlockIndex > 56) {
				while (m_messageBlockIndex != 0)
					update(zero_byte, true);
				while (m_messageBlockIndex < 56)
					update(zero_byte, true);
			}
			else {
				while (m_messageBlockIndex < 56)
					update(zero_byte, true);
			}
			update(static_cast<std::byte>((bitCount >> 56) & 0xFF), true);
			update(static_cast<std::byte>((bitCount >> 48) & 0xFF), true);
			update(static_cast<std::byte>((bitCount >> 40) & 0xFF), true);
			update(static_cast<std::byte>((bitCount >> 32) & 0xFF), true);
			update(static_cast<std::byte>((bitCount >> 24) & 0xFF), true);
			update(static_cast<std::byte>((bitCount >> 16) & 0xFF), true);
			update(static_cast<std::byte>((bitCount >> 8) & 0xFF), true);
			update(static_cast<std::byte>(bitCount & 0xFF), true);
			for (std::size_t idx = 0; idx < hash_size / sizeof(std::uint32_t); idx++) {
				m_digest[idx] = std::byteswap(m_digest[idx]);
			}
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
		std::uint16_t m_messageBlockIndex = 0;
		bool m_finalized = false;
		std::size_t m_messageLength{};
		std::array<std::uint8_t, message_block_size> m_messageBlock{};

	private:
		constexpr hash_err update(std::byte byte, bool finalizing) {
			if (m_finalized && !finalizing)
				return hash_err::ALREADY_FINALIZED;
			m_messageBlock[m_messageBlockIndex++] = std::to_integer<std::uint8_t>(byte);
			++m_messageLength;
			if (m_messageBlockIndex == message_block_size) {
				m_messageBlockIndex = 0;
				update_block();
			}
			return hash_err::SUCCESS;
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		constexpr void update_block() noexcept {
			std::array<std::uint32_t, 80> w{};
			std::size_t idx;
			for (idx = 0; idx < 16; idx++) {
				w[idx] = static_cast<std::uint32_t>(m_messageBlock[idx * 4 + 0] << 24);
				w[idx] |= static_cast<std::uint32_t>(m_messageBlock[idx * 4 + 1] << 16);
				w[idx] |= static_cast<std::uint32_t>(m_messageBlock[idx * 4 + 2] << 8);
				w[idx] |= static_cast<std::uint32_t>(m_messageBlock[idx * 4 + 3]);
			}

			for (idx = 16; idx < 80; idx++) {
				w[idx] = std::rotl((w[idx - 3] ^ w[idx - 8] ^ w[idx - 14] ^ w[idx - 16]), 1);
			}
			auto [A, B, C, D, E] = m_digest;

			for (idx = 0; idx < 80; ++idx) {
				std::uint32_t f = 0;
				std::uint32_t k = 0;

				if (idx < 20) {
					f = ch(B, C, D);  // SHA_Ch
					k = 0x5A827999;
				}
				else if (idx < 40) {
					f = parity(B, C, D);  // SHA_PARITY
					k = 0x6ED9EBA1;
				}
				else if (idx < 60) {
					f = maj(B, C, D);  // SHA_Maj
					k = 0x8F1BBCDC;
				}
				else {
					f = parity(B, C, D);
					k = 0xCA62C1D6;
				}

				const std::uint32_t temp = std::rotl(A, 5) + f + E + k + w[idx];
				E = D;
				D = C;
				C = std::rotl(B, 30);
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
}  // namespace toria::crypto
