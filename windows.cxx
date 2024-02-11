export module toria:windows;

import :impl;

namespace toria
{
	namespace generators
	{
		export class v1_generator
		{
			[[nodiscard]] uuid operator()() noexcept { return uuid{}; }
		};
	}  // namespace generators
}  // namespace uuids
