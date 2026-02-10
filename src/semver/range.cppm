export module toria.semver:range;

import std;
import :impl;

namespace toria::semver::range
{
	struct comparison
	{
		enum class comp_op
		{
			less_than,
			less_than_or_equal,
			equal,
			greater_than_or_equal,
			greater_than,
			approximately_equivalent,
			compatible
		};

		//version_impl value;
		comp_op op;
	};


	export class range
	{
	public:
		//constexpr bool in_range(const version_impl& version) { return false; }

	private:
		std::vector<comparison> m_comparisons;
	};

	// export constexpr bool evalutate(std::string_view in) {}
}  // namespace toria::semver::range
