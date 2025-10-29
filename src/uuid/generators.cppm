export module toria.uuid:generators;

import std;
import toria.crypto;
import toria.util;
import :impl;

namespace toria::uuid::generators
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

	static constexpr std::byte mask_lower{0x0f};
	static constexpr std::byte mask_lower_6{0x0f};

	constexpr void set_version_and_variant(
		std::span<std::byte, 16> bytes, const uuid::version_type version,
		const uuid::variant_type variant = uuid::variant_type::rfc_4122) {
		bytes[6] =
			(bytes[6] & mask_lower) | static_cast<std::byte>(std::to_underlying(version) << 4);
		bytes[8] =
			(bytes[8] & mask_lower_6) | static_cast<std::byte>(std::to_underlying(variant) << 4);
	}

	template<std::uniform_random_bit_generator Engine>
	void _generate_random_bytes(std::span<std::uint64_t> generated, Engine* engine) {
		using result_type = std::invoke_result_t<Engine&>;
		std::uniform_int_distribution<result_type> distribution;
		if constexpr (sizeof(result_type) == 16) {
			*reinterpret_cast<result_type*>(generated) = distribution(*engine);
			if (generated.size() == 2) {
				*reinterpret_cast<result_type*>(generated.data()) = distribution(*engine);
			}
			else if (generated.size() == 1) {
				generated[0] = static_cast<std::uint64_t>(distribution(*engine));
			}
		}
		else if constexpr (sizeof(result_type) == 8) {
			generated[0] = distribution(*engine);
			if (generated.size() == 2) {
				generated[1] = distribution(*engine);
			}
		}
		else if constexpr (sizeof(result_type) == 4) {
			generated[0] = distribution(*engine) >> 32 | distribution(*engine);
			if (generated.size() == 2) {
				generated[1] = distribution(*engine) >> 32 | distribution(*engine);
			}
		}
		else if constexpr (sizeof(result_type) == 2) {
			generated[0] = distribution(*engine) >> 48 | distribution(*engine) >> 32 |
						   distribution(*engine) >> 16 | distribution(*engine);
			if (generated.size() == 2) {
				generated[1] = distribution(*engine) >> 48 | distribution(*engine) >> 32 |
							   distribution(*engine) >> 16 | distribution(*engine);
			}
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
			const std::span<std::byte, 16> bytes = std::as_writable_bytes(std::span{generated});
			set_version_and_variant(bytes, uuid::version_type::v4);
			return uuid(bytes);
		}

	private:
		Engine* m_generator = nullptr;
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
			crypto::hash<hashing_algorithm> hash{};
			hash.update(m_namespace_uuid.bytes());

			for (const std::uint32_t& substring : str) {
				hash.update(static_cast<std::byte>(substring & 0xFF));
				if constexpr (!std::same_as<CharType, char>) {
					hash.update(static_cast<std::byte>((substring >> 8) & 0xFF));
					hash.update(static_cast<std::byte>((substring >> 16) & 0xFF));
					hash.update(static_cast<std::byte>((substring >> 24) & 0xFF));
				}
			}

			std::byte digest[16]{};
			std::span<std::byte, 16> bytes{digest, 16};
			hash.finalize();
			static_cast<void>(hash.get_bytes(bytes));
			set_version_and_variant(bytes, Version);
			return uuid{bytes};
		}

		[[nodiscard]] constexpr uuid operator()(const std::string_view str) const {
			return operator()<char>(str);
		}

	private:
		uuid m_namespace_uuid;
	};

	export template<std::uniform_random_bit_generator Engine>
	class v7_generator
	{
		using result_type = std::invoke_result_t<Engine&>;
		static constexpr std::uint64_t sub_ms_bits = 12;
		static constexpr std::uint64_t ns_in_milli = 1000;

	public:
		v7_generator(Engine& engine = default_random<Engine>()) : m_generator(&engine) {}

		template<is_clock Clock = std::chrono::system_clock>
		[[nodiscard]] uuid operator()(std::chrono::time_point<Clock> point) noexcept {
			std::array<std::uint64_t, 2> raw_bytes{0, 0};
			_generate_random_bytes(std::span<std::uint64_t, 1>{&raw_bytes[1], 1}, m_generator);
			const std::uint64_t micro =
				std::chrono::time_point_cast<std::chrono::microseconds>(point)
					.time_since_epoch()
					.count();
			const std::uint64_t milli = micro / 1000;
			const std::uint16_t precision = static_cast<std::uint16_t>(
				((micro % ns_in_milli) * (1 << sub_ms_bits)) / ns_in_milli);
			// Yes this is dumb with the number of byteswap's.
			// Is there any performance difference doing it this way? No...
			// But hey it's works
			raw_bytes[0] =
				std::byteswap(std::byteswap(std::byteswap(milli) >> 16) | precision & 0x0FFF);
			const std::span bytes{std::as_writable_bytes(std::span{raw_bytes})};
			set_version_and_variant(bytes, uuid::version_type::v7);
			return uuid(bytes);
		}

		template<is_clock clock = std::chrono::system_clock>
		[[nodiscard]] uuid operator()() noexcept {
			return operator()(clock::now());
		}

	private:
		Engine* m_generator;
	};
}  // namespace toria::uuid::generators
