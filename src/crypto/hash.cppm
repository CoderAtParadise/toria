export module toria.crypto:hash;

import std;
import toria.util;
import :common;

namespace toria::crypto
{
	export template<is_hashing_algorithm hashing_algorithm>
	class hash : public hashing_algorithm
	{
	public:
		constexpr hash() noexcept { this->reset(); }

		template<class CharType, class Traits>
		constexpr void hash_string(std::basic_string_view<CharType, Traits> str) {
			this->reset();
			if consteval {
				auto bytes = util::as_bytes(str);
				this->update(bytes);
			}
			else {
				this->update(std::as_bytes(std::span{str.data(), str.size()}));
			}
			this->finalize();
		}

		constexpr void hash_string(const std::string_view str) { this->hash_string<char>(str); }

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

		void hashFile(const std::filesystem::path& path) {
			if (!std::filesystem::exists(path))
				throw;
			if (std::filesystem::is_directory(path))
				return;
			const std::ifstream stream{path, std::ios::binary};
			return this->hashStream(stream);
		}

		template<std::size_t Bytes>
		[[nodiscard]] constexpr hash_err get_bytes(std::span<std::byte, Bytes> bytesOut) const
			requires(Bytes <= hashing_algorithm::hash_size)
		{
			return this->get_digest(bytesOut);
		}
	};
}  // namespace toria::crypto
