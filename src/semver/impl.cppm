export module toria.semver:impl;

import std;
import toria.util;

namespace toria::semver
{
	export enum class semver_parse_errors {
		EMPTY_STRING,
		INVALID_CHARACTER,
		EMPTY_PRERELEASE_IDENTIFIER,
		EMPTY_METADATA_IDENTIFIER,
		LEADING_ZEROS,
	};

	class identifier
	{
	public:
		constexpr identifier() noexcept = default;

		// NOLINTNEXTLINE(google-explicit-constructor)
		constexpr identifier(std::uint32_t value) noexcept
			: m_value(std::in_place_type<std::uint32_t>, value) {}

		// NOLINTNEXTLINE(google-explicit-constructor)
		constexpr identifier(const std::string_view value) noexcept
			: m_value(std::in_place_type<std::string_view>, value) {}

		constexpr friend bool operator==(const identifier& lhs, const identifier& rhs) {
			if (std::holds_alternative<std::uint32_t>(lhs.m_value) &&
				std::holds_alternative<std::uint32_t>(rhs.m_value)) {
				return std::get<std::uint32_t>(lhs.m_value) == std::get<std::uint32_t>(rhs.m_value);
			}
			if (std::holds_alternative<std::string_view>(lhs.m_value) &&
				std::holds_alternative<std::string_view>(rhs.m_value)) {
				auto tlhs = std::get<std::string_view>(lhs.m_value);
				auto trhs = std::get<std::string_view>(rhs.m_value);

				return std::ranges::lexicographical_compare(tlhs, trhs);
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
				std::holds_alternative<std::string_view>(rhs.m_value)) [[likely]] {
				auto tlhs = std::get<std::string_view>(lhs.m_value);
				auto trhs = std::get<std::string_view>(rhs.m_value);

				return std::lexicographical_compare_three_way(
					tlhs.begin(), tlhs.end(), trhs.begin(), trhs.end());
			}
			return std::strong_ordering::equivalent;
		}

	private:
		friend class std::formatter<identifier>;
		std::variant<bool, std::uint32_t, std::string_view> m_value{};
	};

	static constexpr identifier empty_identifier{""};

	template<util::fixed_string IdentifierString, std::size_t Count>
	class fixed_identifiers : public std::array<identifier, Count>
	{
	public:
		using Container = std::array<identifier, Count>;

		constexpr fixed_identifiers() noexcept {
			if constexpr (!IdentifierString.empty()) {
				std::size_t count = 0;
				std::size_t offset = 0;
				while (count < Count - 1) {
					if (auto nextOffset = IdentifierString.find_first_of(".", offset);
						nextOffset != IdentifierString.npos) {
						this->at(count++) = parseIdentifierSection(
							IdentifierString.substr(offset, nextOffset - offset));
						offset = nextOffset + 1;
					}
					else
						break;
				}
				this->at(count) = parseIdentifierSection(IdentifierString.substr(offset));
			}
		}

		[[nodiscard]] static constexpr identifier
		parseIdentifierSection(const std::string_view in) {
			std::uint32_t out = 0;
			auto [_, ec] = std::from_chars(in.data(), in.data() + in.size(), out);
			if (ec == std::errc::invalid_argument) {
				return (in);
			}
			return (out);
		}

		[[nodiscard]] static constexpr bool empty() noexcept { return IdentifierString.empty(); }

		[[nodiscard]] constexpr std::size_t size() const noexcept {
			return IdentifierString.empty() ? 0 : std::array<identifier, Count>::size();
		}

		[[nodiscard]] static constexpr std::string_view str() noexcept { return IdentifierString; }
	};

	class dynamic_identifiers : public std::vector<identifier>
	{
	public:
		using Container = std::vector<identifier>;
		dynamic_identifiers() noexcept = default;
		dynamic_identifiers(
			const std::string_view identifier_string, std::vector<identifier>&& identifiers)
			: std::vector<identifier>(std::move(identifiers)),
			  m_identifier_string(identifier_string) {};
		[[nodiscard]] std::string_view str() const noexcept { return m_identifier_string; }

	private:
		std::string m_identifier_string{};
	};

	template<class T>
	concept identifier_storage = requires(const T& t) {
		typename T::Container;
		{ t.str() } -> std::same_as<std::string_view>;
		{ t.size() } -> std::same_as<std::size_t>;
		{ t.empty() } -> std::same_as<bool>;
	};

	template<identifier_storage LHS, identifier_storage RHS>
	[[nodiscard]] constexpr std::strong_ordering operator<=>(const LHS& lhs, const RHS& rhs) {
		if (lhs.size() < rhs.size())
			return std::strong_ordering::less;
		if (lhs.size() > rhs.size())
			return std::strong_ordering::greater;
		for (auto idx = 0; idx < lhs.size(); ++idx) {
			auto res = lhs[idx] <=> rhs[idx];
			if (res != std::strong_ordering::equal)
				return res;
		}
		return std::strong_ordering::equal;
	}

	template<identifier_storage LHS, identifier_storage RHS>
	[[nodiscard]] constexpr bool operator==(const LHS& lhs, const RHS& rhs) {
		if (lhs.size() != rhs.size())
			return false;
		for (auto idx = 0; idx < lhs.size(); ++idx) {
			if (lhs[idx] != rhs[idx])
				return false;
		}
		return true;
	}

	template<identifier_storage PrereleaseTagStorage, identifier_storage BuildMetadataTagStorage>
	class [[nodiscard]] version_impl
	{
	public:
		constexpr version_impl(
			const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch,
			PrereleaseTagStorage  prerelease_tag = {},
			BuildMetadataTagStorage  build_metadata = {}) noexcept
			: m_major(major), m_minor(minor), m_patch(patch), m_prerelease_tag(std::move(prerelease_tag)),
			  m_build_metadata(std::move(build_metadata)) {}

		operator version_impl<dynamic_identifiers, dynamic_identifiers>() const noexcept {
			std::vector<identifier> prereleaseTags{};
			std::vector<identifier> buildMetadata{};
			if (prerelease_tag().size() != 0) {
				prereleaseTags.resize(prerelease_tag().size());
				std::ranges::copy(prerelease_tag(), prereleaseTags.begin());
			}
			if (build_metadata().size() != 0) {
				buildMetadata.resize(build_metadata().size());
				std::ranges::copy(build_metadata(), buildMetadata.begin());
			}
			return {
				major(), minor(), patch(),
				dynamic_identifiers{prerelease_tag().str(), std::move(prereleaseTags)},
				dynamic_identifiers{build_metadata().str(), std::move(buildMetadata)}};
		}

		[[nodiscard]] constexpr std::uint32_t major() const noexcept { return m_major; }
		[[nodiscard]] constexpr std::uint32_t minor() const noexcept { return m_minor; }
		[[nodiscard]] constexpr std::uint32_t patch() const noexcept { return m_patch; }

		[[nodiscard]] constexpr const PrereleaseTagStorage& prerelease_tag() const noexcept {
			return m_prerelease_tag;
		}

		[[nodiscard]] constexpr const BuildMetadataTagStorage& build_metadata() const noexcept {
			return m_build_metadata;
		}

		template<
			bool UsePrerelease = true, identifier_storage LPRTAg, identifier_storage LBM,
			identifier_storage RPRTag, identifier_storage RBM>
		constexpr friend bool
		operator==(const version_impl<LPRTAg, LBM>& lhs, const version_impl<RPRTag, RBM>& rhs) {
			if (lhs.major() != rhs.major() || lhs.minor() != rhs.minor() ||
				lhs.patch() != rhs.patch())
				return false;
			if (!UsePrerelease)
				return true;
			return lhs.prerelease_tag() <=> rhs.prerelease_tag();
		}

		template<
			bool UsePrerelease = true, identifier_storage LPRTAg, identifier_storage LBM,
			identifier_storage RPRTag, identifier_storage RBM>
		constexpr friend std::strong_ordering
		operator<=>(const version_impl<LPRTAg, LBM>& lhs, const version_impl<RPRTag, RBM>& rhs) {
			const auto majorCmp = lhs.major() <=> rhs.major();
			if (majorCmp != std::strong_ordering::equal)
				return majorCmp;
			const auto minorCmp = lhs.minor() <=> rhs.minor();
			if (minorCmp != std::strong_ordering::equal)
				return minorCmp;
			const auto patchCmp = lhs.patch() <=> rhs.patch();
			if (!UsePrerelease || patchCmp != std::strong_ordering::equal) {
				return patchCmp;
			}
			return lhs.prerelease_tag() <=> rhs.prerelease_tag();
		}

		template<bool UsePrerelease = true, identifier_storage PRTag, identifier_storage BM>
		constexpr friend std::strong_ordering
		operator<=>(const version_impl<PRTag, BM>& lhs, const std::string_view rhs) {
			return operator<=> <UsePrerelease>(lhs, _parse(rhs));
		}

		[[nodiscard]] static constexpr version_impl<dynamic_identifiers, dynamic_identifiers>
		_parse(const std::string_view in) {
			if (!in.empty()) {
			}
			return {0, 0, 0};
		}

	private:
		std::uint32_t m_major{0};
		std::uint32_t m_minor{0};
		std::uint32_t m_patch{0};
		PrereleaseTagStorage m_prerelease_tag;
		BuildMetadataTagStorage m_build_metadata;
	};
}  // namespace toria::semver
