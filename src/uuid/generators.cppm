export module toria.uuid:generators;

#ifdef __INTELLISENSE__
#include "impl.cppm"
#include <chrono>
#include <random>
#else
import std;
import :impl;
import toria.crypto;

#endif  // __INTELLISENSE__

namespace toria
{
	namespace uuid
	{

		template<class T>
		concept is_clock = std::chrono::is_clock_v<T>;

		namespace generators
		{
			// If made static msvc is saying the definition cannot be found
			constexpr void set_version_and_variant(std::span<std::byte, 16> bytes, uuid::version_type version) {
				bytes[6] = (bytes[6] & std::byte(0x0f)) | std::byte(std::to_underlying(version) << 4);
				bytes[8] = (bytes[8] & std::byte(0x3f)) | std::byte(0x80);
			}

			export std::mt19937_64& default_random() noexcept {
				static std::random_device rd;
				static std::mt19937_64 gen(rd());
				return gen;
			}

			export template<class Engine>
			class v4_generator
			{
			public:
				v4_generator(Engine& engine = default_random())
					: m_generator(&engine) {}
				[[nodiscard]] uuid operator()() noexcept {
					std::uint64_t generated[2] = {
						m_distribution(*m_generator), m_distribution(*m_generator)};
					std::span<std::byte, 16> bytes{reinterpret_cast<std::byte*>(generated), 16};
					set_version_and_variant(bytes, uuid::version_type::v4);
					return uuid(bytes);
				}

			private:
				std::uniform_int_distribution<std::uint64_t> m_distribution;
				Engine* m_generator;
			};

			export template<uuid::version_type Version, toria::crypto::is_hashing_algorithm is_hashing_algorithm>
			class name_generator
			{
			public:
				constexpr explicit name_generator(const uuid& namespace_uuid) noexcept
					: m_namespace_uuid(namespace_uuid) {};

				template<class CharType, class Traits>
				[[nodiscard]] constexpr uuid operator()(std::basic_string_view<CharType, Traits> str) const {
					toria::crypto::hash<is_hashing_algorithm> hash{};
					hash.update(m_namespace_uuid.bytes());

					for (std::uint32_t substring : str) {
						hash.update(std::byte(substring & 0xFF));
						if constexpr (!std::same_as<CharType, char>) {
							hash.update(std::byte((substring >> 8) & 0xFF));
							hash.update(std::byte((substring >> 16) & 0xFF));
							hash.update(std::byte((substring >> 24) & 0xFF));
						}
					}

					std::byte digest[16]{};
					std::span<std::byte, 16> bytes{digest, 16};
					hash.finalize();
					hash.get_bytes(bytes);
					set_version_and_variant(bytes, Version);
					return uuid{bytes};
				}

				[[nodiscard]]constexpr uuid operator()(std::string_view str) const {
					return operator()<char>(str);
				}

			private:
				uuid m_namespace_uuid;
			};

			export template<class Engine>
			class v7_generator
			{
			public:
				enum class monotonicity
				{
					base,
					sub_milli,
					counter,
					sub_milli_counter
				};

				v7_generator(
					monotonicity monotonicity = monotonicity::base,
					Engine& Engine = default_random())
					: m_monotonicity(monotonicity)
					, m_generator(&Engine) {}
				// TODO support optional seeded counter

				template<is_clock clock = std::chrono::system_clock>
				[[nodiscard]] uuid operator()(std::chrono::time_point<clock> point) noexcept {
					std::uint64_t generated[2] = {
						m_distribution(*m_generator), m_distribution(*m_generator)};
					std::span<std::byte, 16> bytes{reinterpret_cast<std::byte*>(generated), 16};
					std::uint64_t time =
						std::chrono::time_point_cast<std::chrono::milliseconds>(point)
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
						auto micro = std::chrono::time_point_cast<std::chrono::microseconds>(point)
										 .time_since_epoch()
										 .count();
						int precision = 4096 * ((micro - time * 1000) / 1000.0f);
						bytes[6] = precision >> 8;
						bytes[7] = precision;
					}
					else if (m_monotonicity == monotonicity::counter) {}
					if (m_monotonicity == monotonicity::sub_milli_counter) {}

					set_version_and_variant(bytes, uuid::version_type::v7);
					return uuid(bytes);
				}

				[[nodiscard]] uuid operator()() noexcept {
					return operator()(std::chrono::system_clock::now());
				}

			private:
				std::uniform_int_distribution<std::uint64_t> m_distribution;
				Engine* m_generator;
				monotonicity m_monotonicity;
			};
		}  // namespace generators
	}  // namespace uuid
}  // namespace toria
