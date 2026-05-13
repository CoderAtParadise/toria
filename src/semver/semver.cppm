export module toria.semver;

import std;
export import :impl;
export import :std;
export import :parse_error;
export import :parse;

namespace toria::semver
{
	export template<util::fixed_string PrereleaseTag = "", util::fixed_string BuildMetadata = "">
	auto consteval version(const std::uint32_t major, const std::uint32_t minor,
	                       std::uint32_t patch) {

		constexpr std::size_t pr_count = count_identifiers<PrereleaseTag>();
		constexpr std::size_t bm_count = count_identifiers<BuildMetadata>();
		fixed_identifiers<
			PrereleaseTag, pr_count,
			BuildMetadata, bm_count> identifiers;
		auto prerelease = parse_identifiers(identifiers.prerelease_tag().str(), 0, rule::numeric,
		                                     rule::string);
		auto buildMeta = parse_identifiers(identifiers.build_metadata().str(), 0,
		                                         rule::numeric,
		                                         rule::string);
		if (!prerelease.has_value())
			unroll_and_throw_parse_error(std::span<const parse_error>{prerelease.error()},PrereleaseTag.view());
		if (!buildMeta.has_value())
			unroll_and_throw_parse_error(std::span<const parse_error>{prerelease.error()},BuildMetadata.view());
		identifiers.set_identifiers(prerelease.value(), buildMeta.value());
		return version_impl(major, minor, patch,
		                    identifiers
			);
	}

	export version_impl<dynamic_identifiers> version(const std::uint32_t major,
	                                                 const std::uint32_t minor,
	                                                 const std::uint32_t patch,
	                                                 const std::string_view prerelease_tags =
		                                                 "",
	                                                 const std::string_view build_metadata =
		                                                 "") {
		auto prerelease = parse_identifiers(prerelease_tags, 0, rule::numeric, rule::string);
		auto buildMeta = parse_identifiers(build_metadata, 0, rule::numeric, rule::string);
		std::vector<parse_error> errors;
		if (!prerelease.has_value())
			unroll_and_throw_parse_error(std::span<const parse_error>{prerelease.error()},prerelease_tags);
		if (!buildMeta.has_value())
			unroll_and_throw_parse_error(std::span<const parse_error>{buildMeta.error()},build_metadata);
		return {major, minor, patch,
		        dynamic_identifiers(prerelease_tags, std::move(prerelease.value()), build_metadata,
		                            std::move(buildMeta.value()))};
	}

	export constexpr auto semver_impl_version = version<>(2, 0, 0);

	export template<util::fixed_string Version>
	consteval auto version() {
		const std::vector<identifier> empty_identifiers{};
		if constexpr (Version.empty())
			static_assert("Empty Version String");
		constexpr std::size_t majEnd = Version.find_first_of(".") + 1;
		constexpr std::size_t offset = Version.find_first_of(".",majEnd) + 2; //skip first 3 identifiers and any possible negative value in patch
		constexpr std::size_t prereleaseStart = Version.find_first_of("-",offset);
		constexpr std::size_t buildMetaStart = Version.find_first_of("+",offset);
		constexpr bool noPhasB = buildMetaStart < prereleaseStart;
		if constexpr (prereleaseStart != std::string_view::npos || buildMetaStart !=
		              std::string_view::npos) {
			if constexpr (noPhasB) {
				constexpr auto mmp_str = Version.template substr<0, buildMetaStart>();
				constexpr auto build_meta_str = Version.template substr<buildMetaStart + 1>();
				constexpr std::size_t bmCount = count_identifiers<build_meta_str>();
				fixed_identifiers<"", 0, build_meta_str, bmCount> identifiers;
				const auto mmp = parse_identifiers(mmp_str.view(), 0,
				                                   rule::numeric);
				const auto buildMeta = parse_identifiers(identifiers.build_metadata().str(),
				                                         buildMetaStart,
				                                         rule::numeric, rule::string);
				std::vector<parse_error> errors;
				if (!mmp.has_value())
					errors.append_range(mmp.error());
				else if (mmp.value().size() < 3)
					errors.emplace_back(incomplete_version);
				else if (mmp.value().size() > 3)
					errors.emplace_back(too_many_identifiers);
				if (!buildMeta.has_value())
					errors.append_range(buildMeta.error());
				if (!errors.empty())
					unroll_and_throw_parse_error(std::span<const parse_error>{errors},Version.view());
				identifiers.set_identifiers(empty_identifiers, buildMeta.value());
				return version_impl(
					mmp.value().at(0).as_numeric(), mmp.value().at(0).as_numeric(),
					mmp.value().at(0).as_numeric(), identifiers);
			} else {
				constexpr auto mmp_str = Version.template substr<0, prereleaseStart>();
				const auto mmp = parse_identifiers(mmp_str.view(), 0, rule::numeric);
				if constexpr (buildMetaStart == std::string_view::npos) {
					constexpr auto prerelease_str = Version.template substr<
						prereleaseStart + 1>();
					constexpr std::size_t prCount = count_identifiers<prerelease_str>();
					fixed_identifiers<prerelease_str, prCount, "", 0> identifiers;
					const auto prerelease = parse_identifiers(
						identifiers.prerelease_tag().str(), prereleaseStart + 1, rule::numeric,
						rule::string);
					std::vector<parse_error> errors;
					if (!mmp.has_value())
						errors.append_range(mmp.error());
					else if (mmp.value().size() < 3)
						errors.emplace_back(incomplete_version);
					else if (mmp.value().size() > 3)
						errors.emplace_back(too_many_identifiers);
					if (!prerelease.has_value())
						errors.append_range(prerelease.error());
					if (!errors.empty())
						unroll_and_throw_parse_error(std::span<const parse_error>{errors},Version.view());
					identifiers.set_identifiers(prerelease.value(), empty_identifiers);
					return version_impl(mmp.value().at(0).as_numeric(),
					                    mmp.value().at(1).as_numeric(),
					                    mmp.value().at(2).as_numeric(),
					                    identifiers);
				} else {
					constexpr auto prerelease_str = Version.template substr<
						prereleaseStart + 1, buildMetaStart - prereleaseStart - 1>();
					constexpr auto build_meta_str = Version.template substr<
						buildMetaStart + 1>();
					constexpr std::size_t prCount = count_identifiers<prerelease_str>();
					constexpr std::size_t bmCount = count_identifiers<build_meta_str>();
					fixed_identifiers<
						prerelease_str, prCount,
						build_meta_str, bmCount> identifiers;
					const auto prerelease = parse_identifiers(
						identifiers.prerelease_tag().str(), prereleaseStart + 1, rule::numeric,
						rule::string);
					const auto buildMeta = parse_identifiers(
						identifiers.build_metadata().str(), buildMetaStart + 1, rule::numeric,
						rule::string);
					std::vector<parse_error> errors;
					if (!mmp.has_value())
						errors.append_range(mmp.error());
					else if (mmp.value().size() < 3)
						errors.emplace_back(incomplete_version);
					else if (mmp.value().size() > 3)
						errors.emplace_back(too_many_identifiers);
					if (!prerelease.has_value())
						errors.append_range(prerelease.error());
					if (!buildMeta.has_value())
						errors.append_range(buildMeta.error());
					if (!errors.empty())
						unroll_and_throw_parse_error(std::span<const parse_error>{errors},Version.view());
					identifiers.set_identifiers(prerelease.value(), buildMeta.value());
					return version_impl(mmp.value().at(0).as_numeric(),
					                    mmp.value().at(1).as_numeric(),
					                    mmp.value().at(2).as_numeric(),
					                    identifiers);
				}
			}
		} else {
			const auto mmp = parse_identifiers(Version, 0, rule::numeric);
			std::vector<parse_error> errors;
			if (!mmp.has_value())
				errors.append_range(mmp.error());
			else if (mmp.value().size() != 3)
				errors.emplace_back(incomplete_version);
			if (!errors.empty())
				unroll_and_throw_parse_error(std::span<const parse_error>{errors},Version.view());
			return version_impl(mmp.value().at(0).as_numeric(),
			                    mmp.value().at(1).as_numeric(),
			                    mmp.value().at(2).as_numeric(),
			                    fixed_identifiers<"", 0, "", 0>{});
		}
	}

	// Here because of link error with std::expected
	export std::expected<version_impl<dynamic_identifiers>, std::vector<parse_error> >
	version(const std::string_view str) noexcept {
		if (str.empty())
			return std::unexpected(std::vector{incomplete_version});
		std::size_t offset = str.find_first_of(".") +1;
		offset = str.find_first_of(".",offset) + 2; //skip first 3 identifiers and any possible negative value in patch
		const std::size_t prereleaseStart = str.find_first_of("-", offset);
		const std::size_t buildMetaStart = str.find_first_of("+", offset);
		const bool noPhasB = buildMetaStart < prereleaseStart;

		std::string_view mmp_str;
		std::string_view prerelease_str;
		std::string_view build_meta_str;
		if (prereleaseStart != std::string_view::npos || buildMetaStart != std::string_view::npos) {
			if (noPhasB) {
				mmp_str = str.substr(0, buildMetaStart);
				prerelease_str = "";
				build_meta_str = str.substr(buildMetaStart + 1);
			} else {
				mmp_str = str.substr(0, prereleaseStart);
				if (buildMetaStart == std::string_view::npos) {
					prerelease_str = str.substr(prereleaseStart + 1);
					build_meta_str = "";
				} else {
					prerelease_str = str.substr(prereleaseStart + 1,
					                            buildMetaStart - prereleaseStart - 1);
					build_meta_str = str.substr(buildMetaStart + 1);
				}
			}
		} else {
			mmp_str = str;
			prerelease_str = "";
			build_meta_str = "";
		}

		const auto mmp = parse_identifiers(mmp_str, 0, rule::numeric);
		auto prerelease = parse_identifiers(prerelease_str, prereleaseStart + 1, rule::numeric,rule::string);
		auto buildMeta = parse_identifiers(build_meta_str, buildMetaStart + 1, rule::numeric,rule::string);
		std::vector<parse_error> errors;

		if (!mmp.has_value())
			errors.append_range(mmp.error());
		else if (mmp.value().size() < 3)
						errors.emplace_back(incomplete_version);
		else if (mmp.value().size() > 3)
				errors.emplace_back(too_many_identifiers);
		if (!prerelease.has_value())
			errors.append_range(prerelease.error());
		if (!buildMeta.has_value())
			errors.append_range(buildMeta.error());
		if (!errors.empty())
			return std::unexpected(errors);
		return version_impl(mmp.value().at(0).as_numeric(),
		                    mmp.value().at(1).as_numeric(),
		                    mmp.value().at(2).as_numeric(),
		                    dynamic_identifiers(
			                    prerelease_str,
			                    std::move(prerelease.value()), build_meta_str,
			                    std::move(buildMeta.value())));
	}

	export version_impl<dynamic_identifiers> try_parse(
		const std::string_view str) {
		auto _version = version(str);
		if (!_version.has_value()) {
			unroll_and_throw_parse_error(std::span<const parse_error>{_version.error()},str);
		}
		return _version.value();
	}

} // namespace toria::semver
