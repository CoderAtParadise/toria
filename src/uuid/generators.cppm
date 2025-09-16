export module toria.uuid:generators;

#ifdef __INTELLISENSE__
#include "crypto/common.cppm"
#include "crypto/hash.cppm"
#include "uuid/impl.cppm"
#include <algorithm>
#include <chrono>
#include <random>
#else
import std;
import toria.crypto;
import toria.util;
import :impl;

#endif  // __INTELLISENSE__

namespace toria
{
	namespace uuid
	{
		namespace generators
		{

			template<class T>
			concept is_clock = std::chrono::is_clock_v<T>;

			template<std::uniform_random_bit_generator Engine>
			static Engine& default_random(std::invoke_result_t<Engine&> seed) {
				static Engine engine(seed);
				return engine;
			}

			template<std::uniform_random_bit_generator Engine>
			static Engine& default_random() {
				static std::random_device random_device;
				return default_random<Engine>(random_device());
			}

			constexpr void set_version_and_variant(
				std::span<std::byte, 16> bytes, uuid::version_type version,
				uuid::variant_type variant = uuid::variant_type::rfc_4122) {
				bytes[6] =
					(bytes[6] & std::byte(0x0f)) | std::byte(std::to_underlying(version) << 4);
				bytes[8] =
					(bytes[8] & std::byte(0x3f)) | std::byte(std::to_underlying(variant) << 4);
			}

			template<std::uniform_random_bit_generator Engine>
			void _generate_random_bytes(std::uint64_t* generated, Engine* engine) {
				using result_type = std::invoke_result_t<Engine&>;
				std::uniform_int_distribution<result_type> distribution;
				if constexpr (sizeof(result_type) == 16) {
					*reinterpret_cast<result_type*>(generated) = distribution(*engine);
				}
				else if constexpr (sizeof(result_type) == 8) {
					generated[0] = distribution(*engine);
					generated[1] = distribution(*engine);
				}
				else if constexpr (sizeof(result_type) == 4) {
					*reinterpret_cast<result_type*>(generated)[0] = distribution(*engine);
					*reinterpret_cast<result_type*>(generated)[1] = distribution(*engine);
					*reinterpret_cast<result_type*>(generated)[2] = distribution(*engine);
					*reinterpret_cast<result_type*>(generated)[3] = distribution(*engine);
				}
				else if constexpr (sizeof(result_type) == 2) {
					*reinterpret_cast<result_type*>(generated)[0] = distribution(*engine);
					*reinterpret_cast<result_type*>(generated)[1] = distribution(*engine);
					*reinterpret_cast<result_type*>(generated)[2] = distribution(*engine);
					*reinterpret_cast<result_type*>(generated)[3] = distribution(*engine);
					*reinterpret_cast<result_type*>(generated)[4] = distribution(*engine);
					*reinterpret_cast<result_type*>(generated)[5] = distribution(*engine);
					*reinterpret_cast<result_type*>(generated)[6] = distribution(*engine);
					*reinterpret_cast<result_type*>(generated)[7] = distribution(*engine);
				}
				else
					static_assert(true, "Unable to generate random bytes");
			}

			export template<std::uniform_random_bit_generator Engine>
			class v4_generator
			{
			public:
				v4_generator(Engine& engine = default_random<Engine>()) : m_generator(&engine) {}
				[[nodiscard]] uuid operator()() noexcept {
					std::uint64_t generated[2]{0, 0};
					_generate_random_bytes(generated, m_generator);
					std::span<std::byte, 16> bytes =
						std::as_writable_bytes(std::span<std::uint64_t, 2>{generated});
					set_version_and_variant(bytes, uuid::version_type::v4);
					return uuid(bytes);
				}

			private:
				Engine* m_generator;
			};

			export template<
				uuid::version_type Version, toria::crypto::is_hashing_algorithm hashing_algorithm>
			class name_generator
			{
			public:
				constexpr explicit name_generator(const uuid& namespace_uuid) noexcept
					: m_namespace_uuid(namespace_uuid) {};

				template<class CharType, class Traits>
				[[nodiscard]] constexpr uuid
				operator()(std::basic_string_view<CharType, Traits> str) const {
					toria::crypto::hash<hashing_algorithm> hash{};
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

				[[nodiscard]] constexpr uuid operator()(std::string_view str) const {
					return operator()<char>(str);
				}

			private:
				uuid m_namespace_uuid;
			};

			export template<std::uniform_random_bit_generator Engine>
			class v7_generator
			{
				using result_type = std::invoke_result_t<Engine&>;

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
					Engine& Engine = default_random<Engine>())
					: m_monotonicity(monotonicity), m_generator(&Engine) {}
				// TODO support optional seeded counter

				template<is_clock clock = std::chrono::system_clock>
				[[nodiscard]] uuid operator()(std::chrono::time_point<clock> point) noexcept {
					std::uint64_t generated[2]{0, 0};
					_generate_random_bytes(generated, m_generator);
					std::span<std::byte, 16> bytes =
						std::as_writable_bytes(std::span<std::uint64_t, 2>{generated});
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
					else if (m_monotonicity == monotonicity::counter) {
					}
					if (m_monotonicity == monotonicity::sub_milli_counter) {
					}

					set_version_and_variant(bytes, uuid::version_type::v7);
					return uuid(bytes);
				}

				template<is_clock clock = std::chrono::system_clock>
				[[nodiscard]] uuid operator()() noexcept {
					return operator()(clock::now());
				}

			private:
				Engine* m_generator;
				monotonicity m_monotonicity;
			};
		}  // namespace generators
	}  // namespace uuid
}  // namespace toria
