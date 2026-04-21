export module toria.semver:impl;

import std;
import toria.util;

namespace toria::semver
{
	export enum class semver_parse_error
	{
		OK,
		EMPTY_IDENTIFIER,
		INVALID_CHARACTER,
		LEADING_ZERO,
		NEGATIVE_NUMERIC_IDENTIFIER,
	};

	export struct semver_parse_exception : std::exception
	{
		explicit semver_parse_exception(const semver_parse_error err) noexcept : std::exception(
			error_code_to_message(err).c_str()) {

		}

		static std::string error_code_to_message(const semver_parse_error err) noexcept {
			switch (err) {
				case semver_parse_error::EMPTY_IDENTIFIER:
					return "Error Passing SemVer: An identifier was empty.";
				case semver_parse_error::INVALID_CHARACTER:
					return "Error Passing SemVer: Found an invalid character.";
				case semver_parse_error::LEADING_ZERO:
					return "Error Passing SemVer: Leading zero in a numeric identifier";
				case semver_parse_error::NEGATIVE_NUMERIC_IDENTIFIER:
					return "Error Passing SemVer: Numeric identifier was a negative value";
				default:
					return "Error Passing SemVer: Unknown Error";
			}
		}
	};

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

		std::uint32_t as_numeric() const {
			if (std::holds_alternative<std::uint32_t>(m_value)) {
				return std::get<std::uint32_t>(m_value);
			}
			throw std::bad_variant_access();
		}

		std::string_view as_string() const {
			if (std::holds_alternative<std::string_view>(m_value)) {
				return std::get<std::string_view>(m_value);
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
			    std::holds_alternative<std::string_view>(rhs.m_value)) {
				const auto tlhs = std::get<std::string_view>(lhs.m_value);
				const auto trhs = std::get<std::string_view>(rhs.m_value);

				return std::lexicographical_compare_three_way(
					tlhs.begin(), tlhs.end(), trhs.begin(), trhs.end());
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

	static constexpr identifier empty_identifier{""};

	template<class T>
	concept identifier_storage = requires(const T& t)
	{
		{ t.str() } -> std::same_as<std::string_view>;
		{ t.size() } -> std::same_as<std::size_t>;
		{ t.empty() } -> std::same_as<bool>;
	};

	template<identifier_storage LHS, identifier_storage RHS>
	[[nodiscard]] constexpr std::strong_ordering operator<=>(const LHS& lhs, const RHS& rhs) {
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

	struct semver_parse_result
	{
		semver_parse_error error;
		bool numeric = false;
	};

	struct numeric_parse_result
	{
		std::from_chars_result error;
		std::uint32_t value;
	};

	template<typename F>
	concept semver_rule_invocable =
		std::invocable<F, const std::string_view, const numeric_parse_result&> && requires(F&& f,
		const std::string_view str,
		const numeric_parse_result& result)
		{
			{ std::invoke(std::forward<F>(f), str, result) } -> std::same_as<semver_parse_result>;
		};

	[[nodiscard]] constexpr std::expected<identifier, semver_parse_error> parse_identifier(
		const std::string_view str, semver_rule_invocable auto&& rule) {
		std::uint32_t numeric_value = 0;
		const std::from_chars_result result = std::from_chars(
			str.data(), str.data() + str.size(), numeric_value);
		auto res = rule(str, {result, numeric_value});
		if (res.error != semver_parse_error::OK)
			return std::unexpected(res.error);
		return res.numeric
			       ? identifier(numeric_value)
			       : identifier(str);
	}

	[[nodiscard]] constexpr std::expected<identifier, semver_parse_error>
	parse_identifier(const std::string_view str, semver_rule_invocable auto&& rule,
	                 semver_rule_invocable auto&&... rules) {
		std::uint32_t numeric_value = 0;
		const std::from_chars_result result = std::from_chars(
			str.data(), str.data() + str.size(), numeric_value);
		auto res = rule(str, {result, numeric_value});
		return res.error == semver_parse_error::OK
			       ? res.numeric
				         ? identifier(numeric_value)
				         : identifier(str)
			       : parse_identifier(str, rules...);
	}

	namespace rule
	{
		constexpr auto numeric = [](const std::string_view str,
		                            const numeric_parse_result& res) constexpr ->
			semver_parse_result {
			if (str.empty())
				return {semver_parse_error::EMPTY_IDENTIFIER};
			if (str.size() > 1 && res.error.ptr == "" && res.error.ec == std::errc{}) {
				if (str[0] == '0')
					return {semver_parse_error::LEADING_ZERO};
			}
			if (res.error.ec == std::errc::invalid_argument) {
				if (str[0] == '-')
					return {semver_parse_error::NEGATIVE_NUMERIC_IDENTIFIER};
				return {semver_parse_error::INVALID_CHARACTER};
			}
			return {semver_parse_error::OK, true};
		};


		constexpr auto string = [](const std::string_view str,
		                           const numeric_parse_result&) constexpr ->
			semver_parse_result {
			if (str.empty())
				return {semver_parse_error::EMPTY_IDENTIFIER};
			if (str.find_first_not_of(
				    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-") !=
			    std::string_view::npos)
				return {semver_parse_error::INVALID_CHARACTER};
			return {semver_parse_error::OK};
		};
	}

	template<util::fixed_string str>
	consteval std::size_t count_identifiers() {
		if constexpr (str.empty())
			return 0;
		else {
			std::size_t count = 0;
			std::size_t offset = 0;
			while (offset < str.size()) {
				if (const std::size_t nextOffset = str.find_first_of(".", offset);
					nextOffset != str.npos) {
					++count;
					offset = nextOffset + 1;
				} else {
					++count;
					break;
				}
			}
			return count;
		}
	}

	template<util::fixed_string IdentifierString>
	class fixed_identifiers : public std::array<identifier, count_identifiers<IdentifierString>()>
	{
	public:
		constexpr fixed_identifiers() noexcept {
			if constexpr (!IdentifierString.empty()) {
				std::size_t count = 0;
				std::size_t offset = 0;
				while (count < this->size()) {
					if (auto nextOffset = IdentifierString.find_first_of(".", offset);
						nextOffset != IdentifierString.npos) {
						this->at(count++) = parse_identifier(
							IdentifierString.substr(offset, nextOffset - offset), rule::numeric,
							rule::string).value();
						offset = nextOffset + 1;
					} else {
						this->at(count) = parse_identifier(IdentifierString.substr(offset),
						                                   rule::numeric, rule::string).value();
						break;
					}
				}
			}
		}

		[[nodiscard]] static constexpr bool empty() noexcept { return IdentifierString.empty(); }

		[[nodiscard]] static constexpr std::string_view str() noexcept { return IdentifierString; }
	};

	std::expected<std::vector<identifier>, semver_parse_error> parse_identifiers(
		const std::string_view str) noexcept {
		if (str.empty())
			return std::unexpected(semver_parse_error::EMPTY_IDENTIFIER);
		std::vector<identifier> identifiers;
		std::size_t offset = 0;
		while (offset < str.size()) {
			if (const std::size_t nextOffset = str.find_first_of(".", offset);
				nextOffset != str.npos) {
				auto res = parse_identifier(str.substr(offset, nextOffset - offset), rule::numeric,
				                            rule::string);
				if (!res.has_value()) {
					return std::unexpected(res.error());
				}
				identifiers.emplace_back(res.value());
				offset = nextOffset + 1;
			} else {
				auto res = parse_identifier(str.substr(offset), rule::numeric, rule::string);
				if (!res.has_value()) {
					return std::unexpected(res.error());
				}
				identifiers.emplace_back(res.value());
				break;
			};
		}
		return identifiers;
	}

	class dynamic_identifiers : public std::vector<identifier>
	{
	public:
		dynamic_identifiers() noexcept = default;

		dynamic_identifiers(const std::string_view identifier_string,
		                    const std::vector<identifier>& identifiers) : std::vector<
				identifier>(identifiers),
			m_identifier_string(
				identifier_string) {
		}

		template<util::fixed_string str>
		explicit dynamic_identifiers(
			fixed_identifiers<str> fixed_identifiers) noexcept : m_identifier_string(str) {
			this->reserve(fixed_identifiers.size());
			std::ranges::copy(fixed_identifiers, this->begin());
		}

		[[nodiscard]] std::string_view str() const noexcept { return m_identifier_string; }

	private
	:
		std::string m_identifier_string{};
	};

	template<identifier_storage PreReleaseTag, identifier_storage BuildMetadata>
	class [[nodiscard]] version_impl
	{
	public:
		constexpr version_impl(
			const std::uint32_t major, const std::uint32_t minor, const std::uint32_t patch,
			PreReleaseTag prerelease_tag = {},
			BuildMetadata build_metadata = {}) noexcept
			: m_major(major), m_minor(minor), m_patch(patch),
			  m_prerelease_tag(std::move(prerelease_tag)),
			  m_build_metadata(std::move(build_metadata)) {
		}

		explicit operator version_impl<dynamic_identifiers, dynamic_identifiers>() const noexcept {
			return {
				major(), minor(), patch(),
				dynamic_identifiers(prerelease_tag()),
				dynamic_identifiers(build_metadata())};
		}

		[[nodiscard]] constexpr std::uint32_t major() const noexcept { return m_major; }
		[[nodiscard]] constexpr std::uint32_t minor() const noexcept { return m_minor; }
		[[nodiscard]] constexpr std::uint32_t patch() const noexcept { return m_patch; }

		[[nodiscard]] constexpr const PreReleaseTag& prerelease_tag() const noexcept {
			return m_prerelease_tag;
		}

		[[nodiscard]] constexpr const BuildMetadata& build_metadata() const noexcept {
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

	private:
		std::uint32_t m_major{0};
		std::uint32_t m_minor{0};
		std::uint32_t m_patch{0};
		PreReleaseTag m_prerelease_tag;
		BuildMetadata m_build_metadata;
	};
} // namespace toria::semver
