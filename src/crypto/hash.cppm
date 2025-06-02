export module toria.crypto:hash;
#ifdef __INTELLISENSE__
#include "common.cppm"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <memory>
#include <span>
#include <string_view>
#else
import std;
import :common;
#endif  // __INTELLISENSE__

namespace toria
{
	namespace crypto
	{
		export template<HashAlgo HashAlgo>
		class hash : public HashAlgo
		{
		public:
			constexpr hash() noexcept { this->reset(); }

			template<class CharType, class Traits>
			constexpr void hashString(std::basic_string_view<CharType, Traits> str) {
				this->reset();
				std::span<const std::byte> bytes;
				if consteval {
					std::vector<std::byte> tempBytes{};
					tempBytes.resize(str.size() * sizeof(CharType));
					for (std::size_t i = 0; i < str.size() * sizeof(CharType);
						 i += sizeof(CharType)) {
						std::uint32_t c = str[i];
						for (std::size_t charSize = 0; charSize < sizeof(CharType); ++charSize) {
							tempBytes[i + charSize] = std::byte((c >> (8 * charSize)) & 0xff);
						}
					}
					bytes = std::span<const std::byte>(tempBytes);
					this->update(bytes);
				}
				else {
					bytes = std::span<const std::byte>(
						reinterpret_cast<const std::byte*>(str.data()),
						str.size() * sizeof(CharType));
					this->update(bytes);
				}
				this->finalize();
			}

			constexpr void hashString(std::string_view str) { this->hashString<char>(str); }

			template<class CharType, class Traits>
			void hashStream(const std::basic_ifstream<CharType, Traits>& stream) {
				this->reset();
				if (!stream.is_open())
					return;
			}

			void hashFile(std::filesystem::path& path) {
				if (!std::filesystem::exists(path))
					throw;
				else if (std::filesystem::is_directory(path))
					return false;
				std::ifstream stream{path, std::ios::in | std::ios::binary};
				this->hashStream(stream);
			}

			template<std::size_t Bytes>
			constexpr hash_err get_bytes(std::span<std::byte, Bytes> bytesOut) const
				requires (Bytes <= HashAlgo::HashSize)
			{
				return this->get_digest(bytesOut);
			}
		};
	}  // namespace crypto
}  // namespace toria
