export module toria.semver:std;

import std;
import :impl;
import :range;

template<>
struct std::formatter<toria::semver::identifier>
{
	static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	static auto format(const toria::semver::identifier& value, std::format_context& ctx) {
		if (std::holds_alternative<std::uint32_t>(value.m_value)) {
			return std::formatter<std::uint32_t>().format(
				std::get<std::uint32_t>(value.m_value), ctx);
		}
		if (std::holds_alternative<std::string_view>(value.m_value)) {
			return std::formatter<std::string_view>().format(
				std::get<std::string_view>(value.m_value), ctx);
		}
		return ctx.out();
	}
};

template<toria::semver::identifier_storage Storage>
struct std::formatter<Storage>
{
	// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
	constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	auto format(const Storage& value, std::format_context& ctx) const {
		if (value.empty())
			return ctx.out();
		auto out = std::format_to(ctx.out(), "{}", value.at(0));
		for (auto it = value.begin() + 1; it != value.end(); ++it) {
			out = std::format_to(out, ".{}", *it);
		}
		return out;
	}
};

template<
	toria::semver::identifier_storage PrereleaseTagStorage,
	toria::semver::identifier_storage BuildMetadataTagStorage>
struct std::formatter<toria::semver::version_impl<PrereleaseTagStorage, BuildMetadataTagStorage>>
{
	// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
	constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	auto format(
		const toria::semver::version_impl<PrereleaseTagStorage, BuildMetadataTagStorage>& version,
		std::format_context& ctx) const {
		auto out = std::format_to(
			ctx.out(), "{}.{}.{}", version.major(), version.minor(), version.patch());
		if (!version.prerelease_tag().empty()) {
			out = format_to(out, "-{}", version.prerelease_tag());
		}
		if (!version.build_metadata().empty()) {
			out = format_to(out, "+{}", version.build_metadata());
		}
		return out;
	}
};
