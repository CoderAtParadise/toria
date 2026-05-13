export module toria.semver:parse_error;

import std;


export struct parse_error
{
	enum class error_t
	{
		OK,
		EMPTY_IDENTIFIER,
		INVALID_CHARACTER,
		LEADING_ZERO,
		NEGATIVE_NUMERIC_IDENTIFIER,
		INCOMPLETE_VERSION,
		TOO_MANY_IDENTIFIERS,
	};

	explicit constexpr parse_error(const error_t error,const std::size_t pos = 0) : pos(pos),error (error) {}

	std::size_t pos;
	error_t error;
};

constexpr auto incomplete_version =
		parse_error{parse_error::error_t::INCOMPLETE_VERSION};
constexpr auto too_many_identifiers =
	parse_error{parse_error::error_t::TOO_MANY_IDENTIFIERS};

export struct parse_exception : public std::exception
{
	parse_exception(const parse_error err, const std::string_view str) noexcept : std::exception(
		error_code_to_message(err, str).c_str()) {

	}

	static std::string error_code_to_message(const parse_error err,
	                                         const std::string_view str) noexcept {
		switch (err.error) {
			case parse_error::error_t::EMPTY_IDENTIFIER:
				return std::format(
					"Error Parsing SemVer: Empty identifier at position {} while parsing [{}].\n{:>{}}",
					err.pos, str, "^",
					68 + err.pos);
			case parse_error::error_t::INVALID_CHARACTER:
				return std::format(
					"Error Parsing SemVer: Found an invalid character at position {} while parsing [{}].\n{:>{}}",
					 err.pos , str, "^",
					78 + err.pos);
			case parse_error::error_t::LEADING_ZERO:
				return std::format(
					"Error Parsing SemVer: Leading zero in a numeric identifier at position {} while parsing [{}].\n{:>{}}",
					err.pos, str, "^",
					78 + err.pos);
			case parse_error::error_t::NEGATIVE_NUMERIC_IDENTIFIER:
				return std::format(
					"Error Parsing SemVer: Numeric identifier was a negative value at position {} while parsing [{}].\n{:>{}}",
					err.pos, str, "^",
					81 + err.pos);
			case parse_error::error_t::INCOMPLETE_VERSION:
				return std::format("Error Parsing SemVer: Not enough identifiers for SemVer version while parsing [\"{}\"].",str);
			case parse_error::error_t::TOO_MANY_IDENTIFIERS:
				return std::format("Error Parsing SemVer: Too many identifiers for SemVer version while parsing [\"{}\"]",str);
			[[unlikely]] default:
				return std::format("Error Parsing SemVer: Unknown Error when parsing [{}]", str);
		}
	}
};

template<std::size_t Size = std::dynamic_extent>
[[noreturn]] constexpr void unroll_and_throw_parse_error(std::span<const parse_error,Size> errors,const std::string_view str) {
	if consteval {
		throw "error";
	}
	else {
		throw parse_exception(errors[0],str);
	}
}
