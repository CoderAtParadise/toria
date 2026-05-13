export module toria.semver:identifier;

import std;
import toria.util;
import :parse_error;

namespace toria::semver
{
	class identifier
	{
	public:
		constexpr identifier() noexcept = default;

		explicit constexpr identifier(std::uint32_t value) noexcept
			: m_value(std::in_place_type<std::uint32_t>, value) {
		}

		explicit constexpr identifier(const std::string_view value) noexcept
			: m_value(std::in_place_type<std::string_view>, value) {
		}

		[[nodiscard]] constexpr std::uint32_t as_numeric() const {
			if (std::holds_alternative<std::uint32_t>(m_value)) {
				return std::get<std::uint32_t>(m_value);
			}
			if consteval {
				std::unreachable();
			}
			throw std::bad_variant_access();
		}

		[[nodiscard]] constexpr std::string_view as_string() const {
			if (std::holds_alternative<std::string_view>(m_value)) {
				return std::get<std::string_view>(m_value);
			}
			if consteval {
				std::unreachable();
			}
			throw std::bad_variant_access();
		}

		constexpr friend bool operator==(const identifier& lhs, const identifier& rhs) {
			if (std::holds_alternative<std::uint32_t>(lhs.m_value) &&
			    std::holds_alternative<std::uint32_t>(rhs.m_value)) {
				return std::get<std::uint32_t>(lhs.m_value) == std::get<std::uint32_t>(rhs.m_value);
			}
			if (std::holds_alternative<std::string_view>(lhs.m_value) &&
			    std::holds_alternative<std::string_view>(rhs.m_value)) {
				return std::ranges::lexicographical_compare(std::get<std::string_view>(lhs.m_value),
				                                            std::get<std::string_view>(
					                                            rhs.m_value));
			}
			return false;
		}

		constexpr friend std::strong_ordering
		operator<=>(const identifier& lhs, const identifier& rhs) {
			if (std::holds_alternative<std::uint32_t>(lhs.m_value) &&
			    std::holds_alternative<std::uint32_t>(rhs.m_value)) {
				return std::get<std::uint32_t>(lhs.m_value) <=>
				       std::get<std::uint32_t>(rhs.m_value);
			}
			if (std::holds_alternative<std::string_view>(lhs.m_value) &&
			    std::holds_alternative<std::string_view>(rhs.m_value)) {
				const auto t_lhs = std::get<std::string_view>(lhs.m_value);
				const auto t_rhs = std::get<std::string_view>(rhs.m_value);

				return std::lexicographical_compare_three_way(
					t_lhs.begin(), t_lhs.end(), t_rhs.begin(), t_rhs.end());
			}
			if (std::holds_alternative<std::uint32_t>(lhs.m_value) && std::holds_alternative<
				    std::string_view>(rhs.m_value))
				return std::strong_ordering::less;
			if (std::holds_alternative<
				    std::string_view>(lhs.m_value) && std::holds_alternative<std::uint32_t>(
				    rhs.m_value))
				return std::strong_ordering::greater;
			return std::strong_ordering::equivalent;
		}

	private:
		friend class std::formatter<identifier>;
		std::variant<bool, std::uint32_t, std::string_view> m_value{};
	};

	template<class T>
	concept storage_impl = requires(const T& t)
	{
		{ t.str() } -> std::same_as<std::string_view>;
		{ t.size() } -> std::same_as<std::size_t>;
		{ t.empty() } -> std::same_as<bool>;
		{
			t.operator[](0)
		} -> std::same_as<const identifier&>;
		{
			t.at(0)
		} -> std::same_as<const identifier&>;
	};

	template<class T>
	concept identifier_storage = requires(const T& t)
	{
		{
			t.prerelease_tag()
		} -> storage_impl;
		{ t.build_metadata() } -> storage_impl;
	};

	[[nodiscard]] constexpr std::strong_ordering operator<=>(
		const storage_impl auto& lhs, const storage_impl auto& rhs) {
		if (lhs.size() == 0 && rhs.size() > 0)
			return std::strong_ordering::greater;
		if (lhs.size() > 0 && rhs.size() == 0)
			return std::strong_ordering::less;
		for (auto idx = 0; idx < std::min(lhs.size(), rhs.size()); ++idx) {
			auto res = lhs[idx] <=> rhs[idx];
			if (res != std::strong_ordering::equal)
				return res;
		}
		return lhs.size() <=> rhs.size();
	}

	[[nodiscard]] constexpr bool operator==(const storage_impl auto& lhs,
	                                        const storage_impl auto& rhs) {
		if (lhs.size() != rhs.size())
			return false;
		for (auto idx = 0; idx < lhs.size(); ++idx) {
			if (lhs[idx] != rhs[idx])
				return false;
		}
		return true;
	}

	template<util::fixed_string PrStr, std::size_t PRCount, util::fixed_string BMStr, std::size_t
	         BMCount>
	class fixed_identifiers
	{
	private:
		template<util::fixed_string Str, std::size_t Count>
		struct storage : std::array<identifier, Count>
		{
			consteval storage() = default;
			consteval void set_identifiers(const std::vector<identifier>& vals) {
				std::ranges::copy(vals,this->begin());
			}
			[[nodiscard]] constexpr std::string_view str() const noexcept { return Str.view(); }
		};

	public:
		consteval fixed_identifiers() = default;

		constexpr const auto& prerelease_tag() const noexcept {
			return m_prerelease_storage;
		}

		constexpr const auto& build_metadata() const noexcept {
			return m_build_metadata_storage;
		}

		consteval void set_identifiers(const std::vector<identifier>& prIdentifiers,const std::vector<identifier>& bmIdentifiers) noexcept {
			m_prerelease_storage.set_identifiers(prIdentifiers);
			m_build_metadata_storage.set_identifiers(bmIdentifiers);
		}

	private:
		storage<PrStr, PRCount> m_prerelease_storage;
		storage<BMStr, BMCount> m_build_metadata_storage;
	};

	class dynamic_identifiers
	{
	private:
		class storage : public std::vector<identifier>
		{
		public:
			storage() noexcept = default;

			storage(const std::string_view str,
			        std::vector<identifier>&& identifiers) noexcept : std::vector<
					identifier>(std::move(identifiers)), m_identifier_str(str) {
			}

			[[nodiscard]] std::string_view str() const noexcept { return m_identifier_str; }

		private:
			friend class dynamic_identifiers;
			std::string m_identifier_str;
		};

	public:
		dynamic_identifiers() noexcept = default;

		dynamic_identifiers(const std::string_view pr_str, std::vector<identifier>&& pr_identifiers,
		                    const std::string_view bm_str,
		                    std::vector<identifier>&& bm_identifiers)
			noexcept : m_prerelease_storage({pr_str, std::move(pr_identifiers)}),
			           m_build_metadata_storage({bm_str, std::move(bm_identifiers)}) {
		}

		template<util::fixed_string PRStr, std::size_t PRCount, util::fixed_string BMStr,
		         std::size_t BMCount>
		explicit dynamic_identifiers(
			fixed_identifiers<PRStr, PRCount, BMStr, BMCount>&& other) {
			m_prerelease_storage.m_identifier_str = other.prerelease().str();
			m_build_metadata_storage.m_identifier_str = other.build_metadata().str();
			m_prerelease_storage.reserve(other.prerelease().size());
			m_build_metadata_storage.reserve(other.build_metadata().size());
			std::ranges::copy(other.prerelease(), m_prerelease_storage.begin());
			std::ranges::copy(other.build_metadata(), m_build_metadata_storage.begin());
		}

		const storage& prerelease_tag() const noexcept { return m_prerelease_storage; }
		const storage& build_metadata() const noexcept { return m_build_metadata_storage; }

	private:
		storage m_prerelease_storage;
		storage m_build_metadata_storage;
	};

	constexpr identifier empty_identifier{""};
}
