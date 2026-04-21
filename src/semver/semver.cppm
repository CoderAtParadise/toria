export module toria.semver;

import std;
export import :impl;
export import :std;

namespace toria::semver
{
	export using version = version_impl<dynamic_identifiers, dynamic_identifiers>;
	export template<util::fixed_string PrereleaseTag = "", util::fixed_string BuildMetadata = "">
	using fixed_version = version_impl<
		fixed_identifiers<PrereleaseTag>,
		fixed_identifiers<BuildMetadata> >;

	export constexpr fixed_version semver_impl_version{2, 0, 0};

	// Here because of link error with std::expected
	export std::expected<version_impl<dynamic_identifiers, dynamic_identifiers>, semver_parse_error>
	parse(const std::string_view str) noexcept {
		if (str.empty())
			return std::unexpected(semver_parse_error::EMPTY_IDENTIFIER);
		std::size_t offset = str.find_first_of(".", 0) + 1;
		const std::size_t majorIdx = offset;
		offset = str.find_first_of(".", offset) + 1;
		const std::size_t minorIdx = offset;
		// more one more just incase there is a negative it will be handled later
		const std::size_t prereleaseIdx = str.find_first_of("-", offset + 1);
		const std::size_t buildMetaIdx = str.find_first_of("+", offset);

		std::string_view mmp;
		std::string_view prerelease_str;
		std::string_view build_meta_str;
		const bool noPhasB = buildMetaIdx < prereleaseIdx;
		if (prereleaseIdx != std::string_view::npos || buildMetaIdx != std::string_view::npos) {
			if (noPhasB) {
				mmp = str.substr(0, buildMetaIdx);
				prerelease_str = "";
				build_meta_str = str.substr(buildMetaIdx + 1);
			} else {
				mmp = str.substr(0, prereleaseIdx);
				if (buildMetaIdx == std::string_view::npos) {
					prerelease_str = str.substr(prereleaseIdx + 1);
					build_meta_str = "";
				} else {
					prerelease_str = str.substr(prereleaseIdx + 1,
					                            buildMetaIdx - prereleaseIdx - 1);
					build_meta_str = str.substr(buildMetaIdx + 1);
				}
			}
		} else {
			mmp = str;
			prerelease_str = "";
			build_meta_str = "";
		}

		const auto major = parse_identifier(mmp.substr(0, majorIdx - 1),
		                                    rule::numeric);
		if (!major.has_value())
			return std::unexpected(major.error());
		const auto minor = parse_identifier(mmp.substr(majorIdx, minorIdx - 1),
		                                    rule::numeric);
		if (!minor.has_value())
			return std::unexpected(minor.error());
		const auto patch = parse_identifier(mmp.substr(minorIdx), rule::numeric);
		if (!patch.has_value())
			return std::unexpected(patch.error());
		std::expected<std::vector<identifier>, semver_parse_error> prerelease;
		std::expected<std::vector<identifier>, semver_parse_error> build_meta;
		if (noPhasB || prereleaseIdx == std::string_view::npos) {
			prerelease = std::vector<identifier>();
		} else {
			prerelease = parse_identifiers(prerelease_str);
			if (!prerelease.has_value())
				return std::unexpected(prerelease.error());
		}
		if (buildMetaIdx == std::string_view::npos) {
			build_meta = std::vector<identifier>();
		} else {
			build_meta = parse_identifiers(build_meta_str);
			if (!build_meta.has_value())
				return std::unexpected(build_meta.error());
		}
		return version_impl(
			major.value().as_numeric(), minor.value().as_numeric(), patch.value().as_numeric(),
			dynamic_identifiers(prerelease_str, prerelease.value()),
			dynamic_identifiers(build_meta_str, build_meta.value()));
	}

	export version_impl<dynamic_identifiers, dynamic_identifiers> try_parse(
		const std::string_view str) {
		auto version = parse(str);
		if (!version.has_value())
			throw semver_parse_exception(version.error());
		return version.value();
	}
} // namespace toria::semver
