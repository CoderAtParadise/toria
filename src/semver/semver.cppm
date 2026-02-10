export module toria.semver;

import std;
export import :impl;
export import :std;
import toria.util;

namespace toria::semver
{

	export using version = version_impl<dynamic_identifiers, dynamic_identifiers>;
	export template<util::fixed_string PrereleaseTag = "", util::fixed_string BuildMetadata = "">
	using fixed_version = version_impl<
		fixed_identifiers<PrereleaseTag, util::count<PrereleaseTag, ".">() + 1>,
		fixed_identifiers<BuildMetadata, util::count<BuildMetadata, ".">() + 1>>;

	export constexpr fixed_version semver_impl_version{2, 0, 0};
}  // namespace toria::semver
