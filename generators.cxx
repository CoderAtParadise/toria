export module toria:generators;

#ifdef __INTELLISENSE__
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
struct uuid;
namespace detail
{
	class sha1;
	class md5;
}  // namespace detail
#endif  // __INTELLISENSE__

import std;
import :uuid;
// #ifdef _WIN32
// import :windows;
// #endif  // _WIN32

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

		template<class T>
		concept is_hash_algorithm =
			requires (T& t, std::uint8_t byte, const void* data, std::uint8_t* digest) {
				typename T::digest8_t;
				t.process_byte(byte);
				t.process_bytes(data, 0ui64);
				t.get_digest_bytes(digest);
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

		export template<std::uint8_t version, is_hash_algorithm hashalgo>
		class name_generator
		{
		public:
			explicit name_generator(const uuid& namespace_uuid) noexcept
				: m_namespace_uuid(namespace_uuid){};

			template<class CharType, class Traits>
			uuid operator()(std::basic_string_view<CharType, Traits> str) {
				hashalgo hash{};
				std::byte bytes[16];
				auto nsbytes = m_namespace_uuid.as_bytes();
				std::copy(std::cbegin(nsbytes), std::cend(nsbytes), bytes);
				hash.process_bytes(bytes, 16);

				for (std::uint32_t c : str) {
					hash.process_byte(static_cast<std::uint8_t>(c & 0xFF));
					if constexpr (!std::same_as<CharType, char>) {
						hash.process_byte(static_cast<std::uint8_t>((c >> 8) & 0xFF));
						hash.process_byte(static_cast<std::uint8_t>((c >> 16) & 0xFF));
						hash.process_byte(static_cast<std::uint8_t>((c >> 24) & 0xFF));
					}
				}

				typename hashalgo::digest8_t digest;
				hash.get_digest_bytes(digest);
				digest[6] &= 0x0F;
				digest[6] |= version << 4;

				digest[8] &= 0xBF;
				digest[8] |= 0x80;

				return uuid{digest, digest + 16};
			}

		private:
			uuid m_namespace_uuid;
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

}  // namespace toria
