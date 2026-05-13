export module toria.semver:parse;

import std;
import toria.util;
import :parse_error;
import :identifier;

namespace toria::semver
{
	struct parse_result
	{
		parse_error error{parse_error::error_t::OK};
		bool numeric = false;
	};

	struct numeric_parse_result
	{
		std::from_chars_result error;
		std::uint32_t value;
	};

	struct parse_options
	{
		std::size_t offset = 0;
		bool last = true;
	};

	template<typename F>
	concept semver_rule =
		std::invocable<F, const std::string_view, const numeric_parse_result&, const parse_options&>
		&&
		requires(F&& f,
		         const std::string_view str,
		         const numeric_parse_result& result,
		         const parse_options& options)
		{
			{
				std::invoke(std::forward<F>(f), str, result, options)
			} -> std::same_as<parse_result>;
		};

	struct identifier_count
	{
		std::size_t count = 0;
		std::vector<std::size_t> pos{};
	};

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

	constexpr std::expected<identifier, parse_error> parse_identifier(
		const std::string_view str, parse_options options, semver_rule auto&& rule) {
		std::uint32_t num_val = 0;
		const std::from_chars_result result = std::from_chars(
			str.data(), str.data() + str.size(), num_val);
		if (auto res = rule(str, {result, num_val},options);
			res.numeric && res.error.error != parse_error::error_t::OK)
			return std::unexpected(res.error);
		else {
			return res.numeric ? identifier(num_val) : identifier(str);
		}
	}

	constexpr std::expected<identifier, parse_error> parse_identifier(
		const std::string_view str, parse_options options, semver_rule auto&& rule,
		semver_rule auto&&... rules) {
		if (auto res = parse_identifier(str, options, rule); res.has_value()) {
			return res.value();
		} else {
			return res.error().error == parse_error::error_t::NEGATIVE_NUMERIC_IDENTIFIER || res.
			       error().error == parse_error::error_t::LEADING_ZERO
				       ? std::unexpected(res.error())
				       : parse_identifier(str, {options.offset, sizeof...(rules) <= 1}, rules...);
		}
	}

	constexpr std::expected<std::vector<identifier>, std::vector<parse_error> >
	parse_identifiers(
		const std::string_view str, const std::size_t offset, semver_rule auto&&... rules) {
		const bool last = sizeof...(rules) <= 1;
		std::vector<identifier> identifiers;
		std::vector<parse_error> errors;
		std::size_t identOffset = 0;
		while (identOffset < str.size()) {
			if (const std::size_t nextOffset = str.find_first_of(".", identOffset);
				nextOffset != std::string_view::npos) {
				if (auto res = parse_identifier(str.substr(identOffset, nextOffset - identOffset),
				                                {offset + identOffset, last},
				                                rules...); !res.has_value())
					errors.emplace_back(res.error());
				else {
					identifiers.emplace_back(res.value());
				}
				identOffset = nextOffset + 1;
			} else {
				if (auto res = parse_identifier(str.substr(identOffset), {offset + identOffset, last},
				                                rules...)
					; !res.has_value()) {
					errors.emplace_back(res.error());
				} else {
					identifiers.emplace_back(res.value());
				}
				break;
			}
		}
		if (!errors.empty())
			return std::unexpected(errors);
		return identifiers;
	}

	namespace rule
	{
		constexpr std::uint32_t count_digits(std::uint32_t num) {
			if (num == 0)
				return 1;
			std::uint32_t count = 0;
			while (num > 0) {
				num /= 10;
				count++;
			}
			return count;
		}

		constexpr auto numeric = [](const std::string_view str,
		                            const numeric_parse_result& res,
		                            const parse_options& options) constexpr ->
			parse_result {
			using error_t = parse_error::error_t;
			if (str.empty())
				return {parse_error(error_t::EMPTY_IDENTIFIER, options.offset)};
			if (res.error.ec == std::errc{}) {

			}
			if (const std::uint32_t digits = count_digits(res.value); digits < str.size()) {
				if (str[0] == '0')
					return {{parse_error(error_t::LEADING_ZERO, options.offset)}, true};
				if (str[0] == '-')
					return {parse_error(error_t::NEGATIVE_NUMERIC_IDENTIFIER, options.offset),
					        true};
				const auto pos = str.find_first_not_of("0123456789");
				return {{parse_error{error_t::INVALID_CHARACTER, options.offset + pos}},
				        options.last};
			}
			return {.numeric = true};
		};


		constexpr auto string = [](const std::string_view str,
		                           const numeric_parse_result& res,
		                           const parse_options& options) constexpr ->
			parse_result {
			using error_t = parse_error::error_t;
			if (str.empty())
				return {parse_error(error_t::EMPTY_IDENTIFIER, options.offset)};
			if (const auto pos = str.find_first_not_of(
				"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-"); pos !=
				std::string_view::npos)
				return {parse_error(error_t::INVALID_CHARACTER, options.offset + pos)};
			return {};
		};
	}
}
