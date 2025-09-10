export module toria.crypto:hash;
#ifdef __INTELLISENSE__
#include "crypto/common.cppm"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <istream>
#include <memory>
#include <span>
#include <stop_token>
#include <string_view>
#else
import std;
import toria.util;
import :common;
#endif  // __INTELLISENSE__

namespace toria
{
	namespace crypto
	{
		export template<is_hashing_algorithm hashing_algorithm>
		class hash : public hashing_algorithm
		{
		public:
			constexpr hash() noexcept { this->reset(); }

			template<class CharType, class Traits>
			constexpr void hashString(std::basic_string_view<CharType, Traits> str) {
				this->reset();
				if consteval {
					std::span<const std::byte> bytes = util::as_bytes(str);
					this->update(bytes);
				}
				else {
					this->update(std::as_bytes(std::span{str.data(), str.size()}));
				}
				this->finalize();
			}

			constexpr void hashString(std::string_view str) { this->hashString<char>(str); }

			template<class CharType, class Traits>
			void hashStream(const std::basic_ifstream<CharType, Traits>& stream) {
				this->reset();
				if (!stream.is_open())
					return;
				std::array<CharType, 4096> buffer{};
				auto bytesRead = stream.read(buffer.data(), buffer.size());
				while (bytesRead > 0) {
					this->update(std::as_bytes(std::span(buffer)));
					bytesRead = stream.read(buffer.data(), buffer.size());
				}
				this->finalize();
			}

			void hashFile(std::filesystem::path& path) {
				if (!std::filesystem::exists(path))
					throw;
				else if (std::filesystem::is_directory(path))
					return false;
				std::ifstream stream{path, std::ios::binary};
				return this->hashStream(stream);
			}

			template<std::size_t Bytes>
			constexpr hash_err get_bytes(std::span<std::byte, Bytes> bytesOut) const
				requires (Bytes <= hashing_algorithm::hash_size)
			{
				return this->get_digest(bytesOut);
			}
		};
	}  // namespace crypto
}  // namespace toria
